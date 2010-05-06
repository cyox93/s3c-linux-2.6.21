/*
 * param.c
 * Boot parameter interface
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/proc_fs.h>

#include <asm/uaccess.h>

#include "canopus.h"

#define Q_ENV_TOTAL_SIZE	0x20000
#define Q_ENV_HEADER_SIZE	(sizeof(unsigned long))
#define Q_ENV_SIZE (Q_ENV_TOTAL_SIZE - Q_ENV_HEADER_SIZE)
#define Q_ENV_OFFSET	0x7fc0000

typedef	struct environment_s {
	unsigned long	crc;		/* CRC32 over data bytes	*/
	unsigned char	data[Q_ENV_SIZE]; /* Environment data		*/
} env_t;


/* for the proc file system */
#define Q_PARAM_PROC_VALUE	"prm"
#define Q_PARAM_MAX_LENGTH	50

#define Q_PARAM_WRITE_COMPLETE	('=')

#ifdef Q_PARAM_DEBUG 
#define PARAM_PRINTK(x...)       printk(x)
#else
#define PARAM_PRINTK(x...)       do { } while (0)
#endif


char* _proc_env_name;
struct proc_dir_entry *_param_proc_value = NULL;
extern struct mtd_info *s3c_mtd;


int q_param_proc_init(void);
static int q_param_proc_clear(void);
static int q_param_proc_vaule_read(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data);
static int q_param_proc_vaule_write(struct file *file, const char __user *buffer,
			  unsigned long count, void *data);


int q_param_proc_init(void) 
{
	if (_param_proc_value) {
		return -ENOMEM;
	}

	/* read only key value file in proc dir */
	_param_proc_value = create_proc_entry(Q_PARAM_PROC_VALUE, 0644, NULL);
	if (!_param_proc_value) {
		q_param_proc_clear();
		return -ENOMEM;
	}
	/* End make proc dir and files */

	_proc_env_name = (char*) kmalloc(sizeof(char) * Q_PARAM_MAX_LENGTH, GFP_KERNEL);
	if (!_proc_env_name)
		return -ENOMEM;

	_param_proc_value->read_proc = q_param_proc_vaule_read;
	_param_proc_value->write_proc = q_param_proc_vaule_write;
	_param_proc_value->owner = THIS_MODULE;

	return 0;
}

static int q_param_proc_clear(void) 
{
	if (_param_proc_value == NULL) 
		return -1;

	if (_proc_env_name) { 
		kfree(_proc_env_name);
		_proc_env_name = NULL;
	}

	remove_proc_entry(Q_PARAM_PROC_VALUE, NULL);
	return 0;
}


static int q_param_proc_vaule_read(char *buf, char **start, off_t offset, int count,
			 int *eof, void *data)
{
	int len = 0;
	int length, i, j, nxt = 0;
	env_t *tmp_env = NULL;

	if (!_proc_env_name || _proc_env_name[0] == '\0') {
		len = sprintf(buf, "%c", '\0');
		return len;
	}

	if (_proc_env_name[0] == '=') {
		len = sprintf(buf, "%s\n", "done");
		goto _read_exit;
	}

	length = Q_ENV_TOTAL_SIZE;
	tmp_env = (env_t *)kmalloc(sizeof(char) * length, GFP_KERNEL);
	if (nand_read_skip_bad(s3c_mtd, Q_ENV_OFFSET, &length, (unsigned char*)tmp_env))
		goto _read_exit;

	for (i=0; tmp_env->data[i] != '\0'; i=nxt+1) {
		for (nxt=i; tmp_env->data[nxt] != '\0'; ++nxt) ;

		for (j=i; j<nxt; j++)  {
			if (_proc_env_name[j-i] != tmp_env->data[j])
				break;
		}

		if (j-i != 0 && j-i == strlen(_proc_env_name) && tmp_env->data[j] == '=') {
			j++;
			len = sprintf(buf, "%s\n", tmp_env->data+j);
			break;
		}
	}

	if (!len) len = sprintf(buf, "%c", '\0');

_read_exit:

	_proc_env_name[0] = '\0';
	if (tmp_env) kfree(tmp_env);
	return len;
}

static int q_param_proc_vaule_write(struct file *file, const char __user *buffer,
			  unsigned long count, void *data)
{
	char *buf = NULL;
	char *name, *value;
	env_t *tmp_env = NULL;

	if (count > Q_PARAM_MAX_LENGTH)
		count = Q_PARAM_MAX_LENGTH;

	_proc_env_name[0] = '\0';

	buf = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	buf[count] = '\0';

	/* work around \n when echo'ing into proc */
	if (buf[count - 1] == '\n')
		buf[count - 1] = '\0';


	if (!count || strchr(buf, '=')) 
		goto _write_exit;

	if (buf[0] != 'r' && buf[0] != 'w')
		goto _write_exit;

	name = strchr(buf, ' ');
	if (!name) goto _write_exit;
	name++;


	PARAM_PRINTK("[KERNEL] BUF : %s\n", buf);

	if (buf[0] == 'r') {
		/* read Bootparam */
		value = strchr(name, ' ');
		if (!value) value = buf+count;
		value++;

		snprintf(_proc_env_name, value - name, "%s", name);
		PARAM_PRINTK("[KERNEL] PROC_NAME: %s\n", _proc_env_name);
	} else {
		int length;
		char* last, env_name[Q_PARAM_MAX_LENGTH];

		int i, j, nxt, is_exist = 0;
		i = j = nxt = 0;
		
		value = strchr(name, ' ');
		if (!value) value = buf+count;
		value++;

		snprintf(env_name, value - name, "%s", name);

		length = Q_ENV_TOTAL_SIZE;
		tmp_env = (env_t *)kmalloc(sizeof(char) * length, GFP_KERNEL);
		if (nand_read_skip_bad(s3c_mtd, Q_ENV_OFFSET, &length, (unsigned char*)tmp_env)) {
			printk("PRM! nand read error\n");
			goto _write_exit;
		}

		for (i=0; tmp_env->data[i] != '\0'; i=nxt+1)  {
			for (nxt=i; tmp_env->data[nxt] != '\0'; ++nxt) ;

			for (j=i; j<nxt; j++) 
				if (env_name[j-i] != tmp_env->data[j])
					break;

			if (j-i != 0 && j-i == strlen(env_name) && tmp_env->data[j] == '=') {
				is_exist = 1;
				break;
			}
		}

		if ((value-1) == (buf+count) || is_exist) {
			/* remove and replace */
			char* str = tmp_env->data + i;
			char* end = tmp_env->data + nxt;

			//PARAM_PRINTK("BEFORE I: %d, NXT: %d\n", i, nxt);
			if (*++end == '\0') {
				if (str > (char*)tmp_env->data) {
					str--;
				} else {
					*str = '\0';
				}
			} else {
				for (;;) {
					*str = *end++;
					if ((*str == '\0') && (*end == '\0'))
						break;
					str++;
				}
			}
			*++str = '\0';

			i = (char*) str - (char*) tmp_env->data;
			nxt = (char*) end - (char*) tmp_env->data;
			//PARAM_PRINTK("AFTER I: %d, NXT: %d\n", i, nxt);
		} 

		if ((value-1) != (buf+count)) {
			last = tmp_env->data + i;
			last += snprintf(last, Q_ENV_SIZE - i - 1, "%s=%s", env_name, value);
			*last = '\0';
			*++last = '\0';
		}

#if 0
		/* print all enviroment */
		for (i=0; tmp_env->data[i] != '\0'; i=nxt+1) {
			for (nxt=i; tmp_env->data[nxt] != '\0'; ++nxt) ;

			for (j=i; j<nxt; j++) 
				PARAM_PRINTK("%c", tmp_env->data[j]);

			PARAM_PRINTK("\n");
		}
#endif

		tmp_env->crc = q_uboot_crc32(0, tmp_env->data, Q_ENV_SIZE);
		if (q_nand_erase(s3c_mtd, Q_ENV_OFFSET, Q_ENV_TOTAL_SIZE)) {
			printk("PRM! nand erase error\n");
			goto _write_exit;
		}

		length = Q_ENV_TOTAL_SIZE;
		if (nand_write_skip_bad(s3c_mtd, Q_ENV_OFFSET, &length, (unsigned char*)tmp_env)) {
			printk("PRM! nand write error\n");
			goto _write_exit;
		}

		_proc_env_name[0] = Q_PARAM_WRITE_COMPLETE;
	}

	PARAM_PRINTK("done\n");
	
_write_exit:
	if (buf) kfree(buf);
	if (tmp_env) kfree(tmp_env);

	return count;
}

