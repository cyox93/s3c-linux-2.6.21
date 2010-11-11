/*-----------------------------------------------------------------------------
   * FILE NAME : vc0525_v4l2.c
   * 
   * PURPOSE :  vc0525 device driver for Canopus K5 v4l2 interface 
   * 
   * Copyright 1999 - 2010 UniData Communication Systems, Inc.
   * All right reserved. 
   * 
   * The code contained herein is licensed under the GNU General Public
   * License. You may obtain a copy of the GNU General Public License
   * Version 2 or later at the following locations:
   *
   * http://www.opensource.org/licenses/gpl-license.html
   * http://www.gnu.org/copyleft/gpl.html
   *
   * NOTES: N/A
   *---------------------------------------------------------------------------*/

/*_____________________ Include Header ______________________________________*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/random.h>
#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_VIDEO_V4L1_COMPAT
/* Include V4L1 specific functions. Should be removed soon */
#include <linux/videodev.h>
#endif
#include <linux/interrupt.h>
#include <media/video-buf.h>
#include <media/v4l2-common.h>
#include <linux/kthread.h>
#include <linux/highmem.h>
#include <linux/freezer.h>
#include "font.h"

#include <asm/pgtable.h>
#include <asm/plat-s3c24xx/s3c2416.h>
#include "unidata_logo.h"
#include "vc0528.h"

/*_____________________ Constants Definitions _______________________________*/
/* Wake up at about 30 fps */
//#define VC0528_WAKE_NUMERATOR 30
#define VC0528_WAKE_NUMERATOR 60
#define VC0528_WAKE_DENOMINATOR 1001
#define VC0528_BUFFER_TIMEOUT     msecs_to_jiffies(600)  /* 0.3 seconds */

/* These timers are for 1 fps - used only for testing */
//#define VC0528_WAKE_DENOMINATOR 30 /* hack for testing purposes */
//#define VC0528_BUFFER_TIMEOUT     msecs_to_jiffies(5000)  /* 5 seconds */

/* Debug for vc0528 v4l2 interface */
#define VC0528_IOCTL_CMD_TEST  		0	
#define VC0528_DEBUG_INFO 	  		0

static int debug = 0;
#define dprintk(level,fmt, arg...)					\
	do {								\
		if (debug >= (level))				\
		printk("vc0528: " fmt , ## arg);	  \
	} while (0)


//#define CONFIG_VC0528_SCATTER  	1
#define VC0528_MAJOR_VERSION 		0
#define VC0528_MINOR_VERSION 		4
#define VC0528_RELEASE 				0
#define VC0528_VERSION KERNEL_VERSION(VC0528_MAJOR_VERSION, VC0528_MINOR_VERSION, VC0528_RELEASE)


/*_____________________ Local Declarations __________________________________*/
static unsigned int vid_limit = 16;	/* Video memory limit, in Mb */
static struct video_device vc0528;	/* Video device */
static int video_nr = -1;		 	/* /dev/videoN, -1 for autodetect */

static int __iomem *_test_addr;

char testv4l2_buf[5] = {
	0x0,
	0x1,
	0x2,
	0x3,
	0x4
};

/* supported controls */
static struct v4l2_queryctrl 
vc0528_qctrl[] = {
	{
		.id            = V4L2_CID_AUDIO_VOLUME,
		.name          = "Volume",
		.minimum       = 0,
		.maximum       = 65535,
		.step          = 65535/100,
		.default_value = 65535,
		.flags         = 0,
		.type          = V4L2_CTRL_TYPE_INTEGER,
	},{
		.id            = V4L2_CID_BRIGHTNESS,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Brightness",
		.minimum       = 0,
		.maximum       = 255,
		.step          = 1,
		.default_value = 127,
		.flags         = 0,
	}, {
		.id            = V4L2_CID_CONTRAST,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Contrast",
		.minimum       = 0,
		.maximum       = 255,
		.step          = 0x1,
		.default_value = 0x10,
		.flags         = 0,
	}, {
		.id            = V4L2_CID_SATURATION,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Saturation",
		.minimum       = 0,
		.maximum       = 255,
		.step          = 0x1,
		.default_value = 127,
		.flags         = 0,
	}, {
		.id            = V4L2_CID_HUE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Hue",
		.minimum       = -128,
		.maximum       = 127,
		.step          = 0x1,
		.default_value = 0,
		.flags         = 0,
	}
};

static int qctrl_regs[ARRAY_SIZE(vc0528_qctrl)];


/*_____________________ Basic structures _______________________________*/
struct vc0528_fmt {
	char  *name;
	u32   fourcc;          /* v4l2 format id */
	int   depth;
};

static struct vc0528_fmt format = {
	.name     = "4:2:2, packed, JPEG",
	.fourcc   = V4L2_PIX_FMT_JPEG,
	.depth    = 16,
};

struct sg_to_addr {
	int pos;
	struct scatterlist *sg;
};

/* buffer for one video frame */
struct vc0528_buffer {
	/* common v4l buffer stuff -- must be first */
	struct videobuf_buffer vb;
	struct vc0528_fmt        *fmt;
#ifdef CONFIG_VC0528_SCATTER
	struct sg_to_addr      *to_addr;
#endif
};

struct vc0528_dmaqueue {
	struct list_head        active;
	struct list_head        queued;
	struct timer_list       timeout;

	/* thread for generating video stream*/
	struct task_struct     *kthread;
	wait_queue_head_t       wq;
	/* Counters to control fps rate */
	int                     frame;
	int                     ini_jiffies;
};

static LIST_HEAD(vc0528_devlist);

struct vc0528_dev {
	struct list_head        vc0528_devlist;

	struct semaphore        lock;

	int                     users;

	/* various device info */
	unsigned int            resources;
	struct video_device     vfd;

	struct vc0528_dmaqueue  vidq;

	/* Several counters */
	int                     h,m,s,us,jiffies;
	char                    timestr[13];
};

struct vc0528_fh {
	struct vc0528_dev      *dev;

	/* video capture */
	struct vc0528_fmt      *fmt;
	unsigned int            width,height;
	struct videobuf_queue   vb_vidq;

	enum v4l2_buf_type      type;
};

static struct vc0528_dev   *bend_dev;

/*_____________________ DMA and thread functions _______________________________*/

/* Bars and Colors should match positions */

enum colors {
	WHITE,
	AMBAR,
	CYAN,
	GREEN,
	MAGENTA,
	RED,
	BLUE
};

static u8 color_bars[8][3] = {
	/* R   G   B */
	{204,204,204},	/* white */
	{208,208,  0},  /* ambar */
	{  0,206,206},  /* cyan */
	{  0,239,  0},  /* green */
	{239,  0,239},  /* magenta */
	{205,  0,  0},  /* red */
	{  0,  0,255},  /* blue */
	{  0,  0,  0}
};

#define TO_Y(r,g,b) (((16829*r +33039*g +6416*b  + 32768)>>16)+16)
/* RGB to  V(Cr) Color transform */
#define TO_V(r,g,b) (((28784*r -24103*g -4681*b  + 32768)>>16)+128)
/* RGB to  U(Cb) Color transform */
#define TO_U(r,g,b) (((-9714*r -19070*g +28784*b + 32768)>>16)+128)

#define TSTAMP_MIN_Y 24
#define TSTAMP_MAX_Y TSTAMP_MIN_Y+15
#define TSTAMP_MIN_X 64

#ifdef CONFIG_VC0528_SCATTER
static void prep_to_addr(struct sg_to_addr to_addr[],
			 struct videobuf_buffer *vb)
{
	int i, pos=0;

	for (i=0;i<vb->dma.nr_pages;i++) {
		to_addr[i].sg=&vb->dma.sglist[i];
		to_addr[i].pos=pos;
		pos += vb->dma.sglist[i].length;
	}
}

static int get_addr_pos(int pos, int pages, struct sg_to_addr to_addr[])
{
	int p1=0,p2=pages-1,p3=pages/2;

	/* Sanity test */
	BUG_ON (pos>=to_addr[p2].pos+to_addr[p2].sg->length);

	while (p1+1<p2) {
		if (pos < to_addr[p3].pos) {
			p2=p3;
		} else {
			p1=p3;
		}
		p3=(p1+p2)/2;
	}
	if (pos >= to_addr[p2].pos)
		p1=p2;

	return (p1);
}
#endif

#ifdef CONFIG_VC0528_SCATTER
static void 
gen_line(struct sg_to_addr to_addr[],int inipos,int pages,int wmax,
	     int hmax, int line, char *timestr)
#else
static void 
gen_line(char *basep,int inipos,int wmax,
	     int hmax, int line, char *timestr)
#endif
{
	int  w,i,j,pos=inipos,y;
	char *p,*s;
	u8   chr,r,g,b,color;
#ifdef CONFIG_VC0528_SCATTER
	int pgpos,oldpg;
	char *basep;
	struct page *pg;

	unsigned long flags;
	spinlock_t spinlock;

	spin_lock_init(&spinlock);

	/* Get first addr pointed to pixel position */
	oldpg=get_addr_pos(pos,pages,to_addr);
	pg=pfn_to_page(sg_dma_address(to_addr[oldpg].sg) >> PAGE_SHIFT);
	spin_lock_irqsave(&spinlock,flags);
	basep = kmap_atomic(pg, KM_BOUNCE_READ)+to_addr[oldpg].sg->offset;
#endif

	/* We will just duplicate the second pixel at the packet */
	wmax/=2;

	/* Generate a standard color bar pattern */
	for (w=0;w<wmax;w++) {
		r=color_bars[w*7/wmax][0];
		g=color_bars[w*7/wmax][1];
		b=color_bars[w*7/wmax][2];

		for (color=0;color<4;color++) {
#ifdef CONFIG_VC0528_SCATTER
			pgpos=get_addr_pos(pos,pages,to_addr);
			if (pgpos!=oldpg) {
				pg=pfn_to_page(sg_dma_address(to_addr[pgpos].sg) >> PAGE_SHIFT);
				kunmap_atomic(basep, KM_BOUNCE_READ);
				basep= kmap_atomic(pg, KM_BOUNCE_READ)+to_addr[pgpos].sg->offset;
				oldpg=pgpos;
			}
			p=basep+pos-to_addr[pgpos].pos;
#else
			p=basep+pos;
#endif

			switch (color) {
				case 0:
				case 2:
					*p=TO_Y(r,g,b);		/* Luminance */
					break;
				case 1:
					*p=TO_U(r,g,b);		/* Cb */
					break;
				case 3:
					*p=TO_V(r,g,b);		/* Cr */
					break;
			}
			pos++;
		}
	}

	/* Checks if it is possible to show timestamp */
	if (TSTAMP_MAX_Y>=hmax)
		goto end;
	if (TSTAMP_MIN_X+strlen(timestr)>=wmax)
		goto end;

	/* Print stream time */
	if (line>=TSTAMP_MIN_Y && line<=TSTAMP_MAX_Y) {
		j=TSTAMP_MIN_X;
		for (s=timestr;*s;s++) {
			chr=rom8x16_bits[(*s-0x30)*16+line-TSTAMP_MIN_Y];
			for (i=0;i<7;i++) {
				if (chr&1<<(7-i)) { /* Font color*/
					r=color_bars[BLUE][0];
					g=color_bars[BLUE][1];
					b=color_bars[BLUE][2];
					r=g=b=0;
					g=198;
				} else { /* Background color */
					r=color_bars[WHITE][0];
					g=color_bars[WHITE][1];
					b=color_bars[WHITE][2];
					r=g=b=0;
				}

				pos=inipos+j*2;
				for (color=0;color<4;color++) {
#ifdef CONFIG_VC0528_SCATTER
					pgpos=get_addr_pos(pos,pages,to_addr);
					if (pgpos!=oldpg) {
						pg=pfn_to_page(sg_dma_address(
								to_addr[pgpos].sg)
								>> PAGE_SHIFT);
						kunmap_atomic(basep,
								KM_BOUNCE_READ);
						basep= kmap_atomic(pg,
							KM_BOUNCE_READ)+
							to_addr[pgpos].sg->offset;
						oldpg=pgpos;
					}
					p=basep+pos-to_addr[pgpos].pos;
#else
					p=basep+pos;
#endif

					y=TO_Y(r,g,b);

					switch (color) {
						case 0:
						case 2:
							*p=TO_Y(r,g,b);		/* Luminance */
							break;
						case 1:
							*p=TO_U(r,g,b);		/* Cb */
							break;
						case 3:
							*p=TO_V(r,g,b);		/* Cr */
							break;
					}
					pos++;
				}
				j++;
			}
		}
	}


end:
#ifdef CONFIG_VC0528_SCATTER
	kunmap_atomic(basep, KM_BOUNCE_READ);
	spin_unlock_irqrestore(&spinlock,flags);
#else
	return;
#endif
}

static void 
vc0528_fillbuff(struct vc0528_dev *dev,struct vc0528_buffer *buf)
{
	int h,pos=0;
	int hmax  = buf->vb.height;
	int wmax  = buf->vb.width;
	struct timeval ts;
	unsigned int test_value = 0x1;
	ctrl_jpeg_read jpeg;
#ifdef CONFIG_VC0528_SCATTER
	struct sg_to_addr *to_addr=buf->to_addr;
	struct videobuf_buffer *vb=&buf->vb;
#endif

#ifdef CONFIG_VC0528_SCATTER
	/* Test if DMA mapping is ready */
	if (!sg_dma_address(&vb->dma.sglist[0]))
		return;

	prep_to_addr(to_addr,vb);

	/* Check if there is enough memory */
	BUG_ON(buf->vb.dma.nr_pages << PAGE_SHIFT < (buf->vb.width*buf->vb.height)*2);
#endif


#if 0
	dprintk(2,"vb:boff:0x%x, bsize:0x%x, baddr:0x%x\n, ",
			(unsigned long)buf->vb.boff,
   			(unsigned long)buf->vb.bsize,
			(unsigned long)buf->vb.baddr);
	dprintk(2,"size:0x%x, bytesperline:0x%x, remap:0x%x\n",
			(unsigned long)buf->vb.size,
   			(unsigned long)buf->vb.bytesperline,
   			(unsigned long)buf->vb.remap);

	dprintk(2,"dma:offset:0x%x, pages:0x%x, nr_pages:0x%x\n",
			(unsigned long)buf->vb.dma.offset,
   			(unsigned long)buf->vb.dma.pages,
			(unsigned long)buf->vb.dma.nr_pages);
	dprintk(2,"vmalloc:0x%x, sglen:0x%x\n",
			(unsigned long)buf->vb.dma.vmalloc,
			(unsigned long)buf->vb.dma.sglen);
#endif

    _test_addr = buf->vb.remap; 
	/* Updates stream time */

	dev->us+=jiffies_to_usecs(jiffies-dev->jiffies);
	dev->jiffies=jiffies;
	if (dev->us>=1000000) {
		dev->us-=1000000;
		dev->s++;
		if (dev->s>=60) {
			dev->s-=60;
			dev->m++;
			if (dev->m>60) {
				dev->m-=60;
				dev->h++;
				if (dev->h>24)
					dev->h-=24;
			}
		}
	}

	dprintk(2,"kvm_addr: 0x%08lx size= %d\n",(unsigned long)_test_addr,(unsigned long)buf->vb.size);
	sprintf(dev->timestr,"%02d:%02d:%02d:%03d", dev->h,dev->m,dev->s,(dev->us+500)/1000);
	dprintk(2,"[buffer fill start  :%s]\n",dev->timestr);

	jpeg.frame_buf  = buf->vb.remap;
	jpeg.frbuf_size = buf->vb.size;
	jpeg.frame_rate = 1;
	jpeg.frame_max  = 0xffffffff;
	canopus_bedev_ioctl(VC0528_CAMERA_JPEG_READ,&jpeg);
	buf->vb.jpeg_size = jpeg.frame_size;
	dprintk(2,"[jpeg_size  :0x%x]\n",buf->vb.jpeg_size);

	/* Updates stream time */
	dev->us+=jiffies_to_usecs(jiffies-dev->jiffies);
	dev->jiffies=jiffies;
	if (dev->us>=1000000) {
		dev->us-=1000000;
		dev->s++;
		if (dev->s>=60) {
			dev->s-=60;
			dev->m++;
			if (dev->m>60) {
				dev->m-=60;
				dev->h++;
				if (dev->h>24)
					dev->h-=24;
			}
		}
	}

	sprintf(dev->timestr,"%02d:%02d:%02d:%03d", dev->h,dev->m,dev->s,(dev->us+500)/1000);
	dprintk(2,"[buffer fill end  :%s]\n", dev->timestr);

	/* Advice that buffer was filled */
	buf->vb.state = STATE_DONE;
	buf->vb.field_count++;
	do_gettimeofday(&ts);
	buf->vb.ts = ts;

	list_del(&buf->vb.queue);
	wake_up(&buf->vb.done);
}


#if 0
static void 
vc0528_fillbuff(struct vc0528_dev *dev,struct vc0528_buffer *buf)
{
	int h,pos=0;
	int hmax  = buf->vb.height;
	int wmax  = buf->vb.width;
	struct timeval ts;
#ifdef CONFIG_VC0528_SCATTER
	struct sg_to_addr *to_addr=buf->to_addr;
	struct videobuf_buffer *vb=&buf->vb;
#else
	char *tmpbuf;
#endif

#ifdef CONFIG_VC0528_SCATTER
	/* Test if DMA mapping is ready */
	if (!sg_dma_address(&vb->dma.sglist[0]))
		return;

	prep_to_addr(to_addr,vb);

	/* Check if there is enough memory */
	BUG_ON(buf->vb.dma.nr_pages << PAGE_SHIFT < (buf->vb.width*buf->vb.height)*2);
#else
	if (buf->vb.dma.varea) {
		tmpbuf=kmalloc (wmax*2, GFP_KERNEL);
	} else {
		tmpbuf=buf->vb.dma.vmalloc;
	}

#endif

	for (h=0;h<hmax;h++) {
#ifdef CONFIG_VC0528_SCATTER
		gen_line(to_addr,pos,vb->dma.nr_pages,wmax,hmax,h,dev->timestr);
#else
		if (buf->vb.dma.varea) {
#if 0			
			printk("tempbuf: 0x%x\n",tmpbuf);
			printk("wmax:%d, hmax:%d\n",wmax,hmax);
			printk("h:%d dev->timestr:0x%x\n",h,dev->timestr);
#endif 
			gen_line(tmpbuf,0,wmax,hmax,h,dev->timestr);

			/* FIXME: replacing to __copy_to_user */
			//if (copy_to_user(buf->vb.dma.varea+pos,tmpbuf,wmax*2)!=0)
			//	dprintk(2,"vc0528fill copy_to_user failed.\n");
		} else {
			gen_line(tmpbuf,pos,wmax,hmax,h,dev->timestr);
		}
#endif
		pos += wmax*2;
	}

	/* Updates stream time */

	dev->us+=jiffies_to_usecs(jiffies-dev->jiffies);
	dev->jiffies=jiffies;
	if (dev->us>=1000000) {
		dev->us-=1000000;
		dev->s++;
		if (dev->s>=60) {
			dev->s-=60;
			dev->m++;
			if (dev->m>60) {
				dev->m-=60;
				dev->h++;
				if (dev->h>24)
					dev->h-=24;
			}
		}
	}
	sprintf(dev->timestr,"%02d:%02d:%02d:%03d",
			dev->h,dev->m,dev->s,(dev->us+500)/1000);

	dprintk(2,"vc0528fill at %s: Buffer 0x%08lx size= %d\n",dev->timestr,
			(unsigned long)buf->vb.dma.varea,pos);

	dprintk(2,"vb:boff:0x%x, bsize:0x%x, baddr:0x%x, ",
			(unsigned long)buf->vb.boff,
   			(unsigned long)buf->vb.bsize,
			(unsigned long)buf->vb.baddr);
	dprintk(2,"size:0x%x, bytesperline:0x%x\n",
			(unsigned long)buf->vb.size,
   			(unsigned long)buf->vb.bytesperline);

	dprintk(2,"dma:offset:0x%x, pages:0x%x, nr_pages:0x%x ",
			(unsigned long)buf->vb.dma.offset,
   			(unsigned long)buf->vb.dma.pages,
			(unsigned long)buf->vb.dma.nr_pages);
	dprintk(2,"vmalloc:0x%x, sglen:0x%x\n",
			(unsigned long)buf->vb.dma.vmalloc,
			(unsigned long)buf->vb.dma.sglen);

	/* Advice that buffer was filled */
	buf->vb.state = STATE_DONE;
	buf->vb.field_count++;
	do_gettimeofday(&ts);
	buf->vb.ts = ts;

	list_del(&buf->vb.queue);
	wake_up(&buf->vb.done);
}
#endif 

static int 
restart_video_queue(struct vc0528_dmaqueue *dma_q);

//static void 
void 
vc0528_thread_tick(struct vc0528_dmaqueue  *dma_q)
{
	struct vc0528_buffer    *buf;
	struct vc0528_dev *dev= container_of(dma_q,struct vc0528_dev,vidq);

	int bc;

	/* Announces videobuf that all went ok */
	for (bc = 0;; bc++) {
		if (list_empty(&dma_q->active)) {
  		dprintk(1,"No active queue to serve bc: %d\n",bc);
			break;
		}
 		//dprintk(1,"empty buf cnt %x\n",bc);

		buf = list_entry(dma_q->active.next,
				 struct vc0528_buffer, vb.queue);

		/* Nobody is waiting something to be done, just return */
		if (!waitqueue_active(&buf->vb.done)) {
			//mod_timer(&dma_q->timeout, jiffies+VC0528_BUFFER_TIMEOUT);
			return;
		}

		do_gettimeofday(&buf->vb.ts);
		dprintk(2,"[%p/%d] wakeup\n",buf,buf->vb.i);

		/* Fill buffer */
		vc0528_fillbuff(dev,buf);

		if (list_empty(&dma_q->active)) {
			//del_timer(&dma_q->timeout);
		} else {
			//mod_timer(&dma_q->timeout, jiffies+VC0528_BUFFER_TIMEOUT);
		}
	}
#if 0
  	if (bc != 1)
		dprintk(1,"%s: %d buffers handled (should be 1)\n",__FUNCTION__,bc);
#endif	
}

static void 
vc0528_sleep(struct vc0528_dmaqueue  *dma_q)
{
	int timeout;
	DECLARE_WAITQUEUE(wait, current);

//  dprintk(1,"%s dma_q=0x%08lx\n",__FUNCTION__,(unsigned long)dma_q);

	add_wait_queue(&dma_q->wq, &wait);
	if (!kthread_should_stop()) {
		dma_q->frame++;

		/* Calculate time to wake up */
		timeout=dma_q->ini_jiffies+msecs_to_jiffies((dma_q->frame*VC0528_WAKE_NUMERATOR*1000)/VC0528_WAKE_DENOMINATOR)-jiffies;

		if (timeout <= 0) {
			int old=dma_q->frame;
			dma_q->frame=(jiffies_to_msecs(jiffies-dma_q->ini_jiffies)*VC0528_WAKE_DENOMINATOR)/(VC0528_WAKE_NUMERATOR*1000)+1;

			timeout=dma_q->ini_jiffies+msecs_to_jiffies((dma_q->frame*VC0528_WAKE_NUMERATOR*1000)/VC0528_WAKE_DENOMINATOR)-jiffies;

//			dprintk(1,"underrun, losed %d frames. "
//				  "Now, frame is %d. Waking on %d jiffies\n",
//					dma_q->frame-old,dma_q->frame,timeout);
		}else{
//			dprintk(1,"will sleep for %i jiffies\n",timeout);
		}

		vc0528_thread_tick(dma_q);

		if (timeout >= 0)
			schedule_timeout_interruptible (timeout);
	}

	remove_wait_queue(&dma_q->wq, &wait);
	try_to_freeze();
}

static int 
vc0528_thread(void *data)
{
	struct vc0528_dmaqueue  *dma_q=data;

	dprintk(1,"thread started\n");

	//mod_timer(&dma_q->timeout, jiffies+VC0528_BUFFER_TIMEOUT);

	for (;;) {
		vc0528_sleep(dma_q);

		if (kthread_should_stop())
			break;
	}
	dprintk(1, "thread: exit\n");
	return 0;
}

static int 
vc0528_start_thread(struct vc0528_dmaqueue  *dma_q)
{
	dma_q->frame=0;
	dma_q->ini_jiffies=jiffies;

	dprintk(1,"%s\n",__FUNCTION__);

	dma_q->kthread = kthread_run(vc0528_thread, dma_q, "vc0528");

	if (IS_ERR(dma_q->kthread)) {
		printk(KERN_ERR "vc0528: kernel_thread() failed\n");
		return PTR_ERR(dma_q->kthread);
	}
	/* Wakes thread */
	wake_up_interruptible(&dma_q->wq);

	dprintk(1,"returning from %s\n",__FUNCTION__);
	return 0;
}

static void 
vc0528_stop_thread(struct vc0528_dmaqueue  *dma_q)
{
	dprintk(1,"%s\n",__FUNCTION__);
	/* shutdown control thread */
	if (dma_q->kthread) {
		kthread_stop(dma_q->kthread);
		dma_q->kthread=NULL;
	}
}

static int 
restart_video_queue(struct vc0528_dmaqueue *dma_q)
{
	struct vc0528_buffer *buf, *prev;
	struct list_head *item;

	dprintk(1,"%s dma_q=0x%08lx\n",__FUNCTION__,(unsigned long)dma_q);

	if (!list_empty(&dma_q->active)) {
		buf = list_entry(dma_q->active.next, struct vc0528_buffer, vb.queue);
		dprintk(2,"restart_queue [%p/%d]: restart dma\n",
			buf, buf->vb.i);

		dprintk(1,"Restarting video dma\n");
		vc0528_stop_thread(dma_q);
//		vc0528_start_thread(dma_q);

		/* cancel all outstanding capture / vbi requests */
		list_for_each(item,&dma_q->active) {
			buf = list_entry(item, struct vc0528_buffer, vb.queue);

			list_del(&buf->vb.queue);
			buf->vb.state = STATE_ERROR;
			wake_up(&buf->vb.done);
		}
		//mod_timer(&dma_q->timeout, jiffies+VC0528_BUFFER_TIMEOUT);

		return 0;
	}

	prev = NULL;
	for (;;) {
		if (list_empty(&dma_q->queued))
			return 0;
		buf = list_entry(dma_q->queued.next, struct vc0528_buffer, vb.queue);
		if (NULL == prev) {
			list_del(&buf->vb.queue);
			list_add_tail(&buf->vb.queue,&dma_q->active);

			dprintk(1,"Restarting video dma\n");
			vc0528_stop_thread(dma_q);
			vc0528_start_thread(dma_q);

			buf->vb.state = STATE_ACTIVE;
			//mod_timer(&dma_q->timeout, jiffies+VC0528_BUFFER_TIMEOUT);
			dprintk(2,"[%p/%d] restart_queue - first active\n",
				buf,buf->vb.i);

		} else if (prev->vb.width  == buf->vb.width  &&
			   prev->vb.height == buf->vb.height &&
			   prev->fmt       == buf->fmt) {
			list_del(&buf->vb.queue);
			list_add_tail(&buf->vb.queue,&dma_q->active);
			buf->vb.state = STATE_ACTIVE;
			dprintk(2,"[%p/%d] restart_queue - move to active\n",
				buf,buf->vb.i);
		} else {
			return 0;
		}
		prev = buf;
	}
}

static void 
vc0528_vid_timeout(unsigned long data)
{
#if 0	
	struct vc0528_dev      *dev  = (struct vc0528_dev*)data;
	struct vc0528_dmaqueue *vidq = &dev->vidq;
	struct vc0528_buffer   *buf;

	while (!list_empty(&vidq->active)) {
		buf = list_entry(vidq->active.next, struct vc0528_buffer, vb.queue);
		list_del(&buf->vb.queue);
		buf->vb.state = STATE_ERROR;
		wake_up(&buf->vb.done);
		dprintk(1,"[%p/%d] timeout\n", buf, buf->vb.i);
	}

	restart_video_queue(vidq);
#endif	
}


/*_____________________ Videobuf operations __________________________________*/
static int
buffer_setup(struct videobuf_queue *vq, unsigned int *count, unsigned int *size)
{
	struct vc0528_fh *fh = vq->priv_data;

	*size = fh->width*fh->height*2;

	if (0 == *count)
		*count = 32;
	while (*size * *count > vid_limit * 1024 * 1024)
		(*count)--;
	return 0;
}

static void 
free_buffer(struct videobuf_queue *vq, struct vc0528_buffer *buf)
{
	dprintk(1,"%s\n",__FUNCTION__);

	if (in_interrupt())
		BUG();

#ifdef CONFIG_VC0528_SCATTER
	/*FIXME: Maybe a spinlock is required here */
	kfree(buf->to_addr);
	buf->to_addr=NULL;
#endif

	videobuf_waiton(&buf->vb,0,0);
	videobuf_dma_unmap(vq, &buf->vb.dma);
	videobuf_dma_free(&buf->vb.dma);
	buf->vb.state = STATE_NEEDS_INIT;
}

#define norm_maxw() 1024
#define norm_maxh() 768
static int
buffer_prepare(struct videobuf_queue *vq, struct videobuf_buffer *vb,
						enum v4l2_field field)
{
	struct vc0528_fh     *fh  = vq->priv_data;
	struct vc0528_buffer *buf = container_of(vb,struct vc0528_buffer,vb);
	int rc, init_buffer = 0;

//	dprintk(1,"%s, field=%d\n",__FUNCTION__,field);

	BUG_ON(NULL == fh->fmt);
	if (fh->width  < 48 || fh->width  > norm_maxw() ||
	    fh->height < 32 || fh->height > norm_maxh())
		return -EINVAL;
	buf->vb.size = fh->width*fh->height*2;
	if (0 != buf->vb.baddr  &&  buf->vb.bsize < buf->vb.size)
		return -EINVAL;

	if (buf->fmt       != fh->fmt    ||
	    buf->vb.width  != fh->width  ||
	    buf->vb.height != fh->height ||
	buf->vb.field  != field) {
		buf->fmt       = fh->fmt;
		buf->vb.width  = fh->width;
		buf->vb.height = fh->height;
		buf->vb.field  = field;
		init_buffer = 1;
	}

	if (STATE_NEEDS_INIT == buf->vb.state) {
		if (0 != (rc = videobuf_iolock(vq,&buf->vb,NULL)))
			goto fail;
	}

	buf->vb.state = STATE_PREPARED;

#ifdef CONFIG_VC0528_SCATTER
	if (NULL == (buf->to_addr = kmalloc(sizeof(*buf->to_addr) * vb->dma.nr_pages,GFP_KERNEL))) {
		rc=-ENOMEM;
		goto fail;
	}
#endif
	return 0;

fail:
	free_buffer(vq,buf);
	return rc;
}

static void
buffer_queue(struct videobuf_queue *vq, struct videobuf_buffer *vb)
{
	struct vc0528_buffer    *buf     = container_of(vb,struct vc0528_buffer,vb);
	struct vc0528_fh        *fh      = vq->priv_data;
	struct vc0528_dev       *dev     = fh->dev;
	struct vc0528_dmaqueue  *vidq    = &dev->vidq;
	struct vc0528_buffer    *prev;

	if (!list_empty(&vidq->queued)) {
		dprintk(1,"adding vb queue=0x%08lx\n",(unsigned long)&buf->vb.queue);
		list_add_tail(&buf->vb.queue,&vidq->queued);
		buf->vb.state = STATE_QUEUED;
		dprintk(2,"[%p/%d] buffer_queue - append to queued\n",
			buf, buf->vb.i);
	} else if (list_empty(&vidq->active)) {
		list_add_tail(&buf->vb.queue,&vidq->active);

		buf->vb.state = STATE_ACTIVE;
		//mod_timer(&vidq->timeout, jiffies+VC0528_BUFFER_TIMEOUT);
		dprintk(2,"[%p/%d] buffer_queue - first active\n",
			buf, buf->vb.i);

		vc0528_start_thread(vidq);
	} else {
		prev = list_entry(vidq->active.prev, struct vc0528_buffer, vb.queue);
		if (prev->vb.width  == buf->vb.width  &&
		    prev->vb.height == buf->vb.height &&
		    prev->fmt       == buf->fmt) {
			list_add_tail(&buf->vb.queue,&vidq->active);
			buf->vb.state = STATE_ACTIVE;
			dprintk(2,"[%p/%d] buffer_queue - append to active\n",
				buf, buf->vb.i);

		} else {
			list_add_tail(&buf->vb.queue,&vidq->queued);
			buf->vb.state = STATE_QUEUED;
			dprintk(2,"[%p/%d] buffer_queue - first queued\n",
				buf, buf->vb.i);
		}

	}
}

static void 
buffer_release(struct videobuf_queue *vq, struct videobuf_buffer *vb)
{
	struct vc0528_buffer   *buf  = container_of(vb,struct vc0528_buffer,vb);
	struct vc0528_fh       *fh   = vq->priv_data;
	struct vc0528_dev      *dev  = (struct vc0528_dev*)fh->dev;
	struct vc0528_dmaqueue *vidq = &dev->vidq;

	dprintk(1,"%s\n",__FUNCTION__);

	vc0528_stop_thread(vidq);

	free_buffer(vq,buf);
}

#ifdef CONFIG_VC0528_SCATTER
static int 
vc0528_map_sg(void *dev, struct scatterlist *sg, int nents,
		       int direction)
{
	int i;

	dprintk(1,"%s, number of pages=%d\n",__FUNCTION__,nents);
	BUG_ON(direction == DMA_NONE);

	for (i = 0; i < nents; i++ ) {
		BUG_ON(!sg[i].page);

		sg_dma_address(&sg[i]) = page_to_phys(sg[i].page) + sg[i].offset;
	}

	return nents;
}

static int 
vc0528_unmap_sg(void *dev,struct scatterlist *sglist,int nr_pages,
			 int direction)
{
	dprintk(1,"%s\n",__FUNCTION__);
	return 0;
}

static int 
vc0528_dma_sync_sg(void *dev,struct scatterlist *sglist, int nr_pages,
			    int direction)
{
//	dprintk(1,"%s\n",__FUNCTION__);

//	flush_write_buffers();
	return 0;
}
#endif

static struct videobuf_queue_ops 
vc0528_video_qops = {
	.buf_setup      = buffer_setup,
	.buf_prepare    = buffer_prepare,
	.buf_queue      = buffer_queue,
	.buf_release    = buffer_release,

	/* Non-pci handling routines */
//	.vb_map_sg      = vc0528_map_sg,
//	.vb_dma_sync_sg = vc0528_dma_sync_sg,
//	.vb_unmap_sg    = vc0528_unmap_sg,
};


/*_____________________ IOCTL handling _______________________________*/
static int 
res_get(struct vc0528_dev *dev, struct vc0528_fh *fh)
{
	/* is it free? */
	down(&dev->lock);
	if (dev->resources) {
		/* no, someone else uses it */
		up(&dev->lock);
		return 0;
	}
	/* it's free, grab it */
	dev->resources =1;
	dprintk(1,"res: get\n");
	up(&dev->lock);
	return 1;
}

static int 
res_locked(struct vc0528_dev *dev)
{
	return (dev->resources);
}

static void 
res_free(struct vc0528_dev *dev, struct vc0528_fh *fh)
{
	down(&dev->lock);
	dev->resources = 0;
	dprintk(1,"res: put\n");
	up(&dev->lock);
}

/*_____________________ IOCTL vidioc handling _______________________________*/
static int 
vidioc_querycap(struct file *file, void  *priv,
					struct v4l2_capability *cap)
{
	strcpy(cap->driver, "vc0528");
	strcpy(cap->card, "vc0528");
	cap->version = VC0528_VERSION;
	cap->capabilities =	V4L2_CAP_VIDEO_CAPTURE |
				V4L2_CAP_STREAMING     |
				V4L2_CAP_READWRITE;
	return 0;
}

static int 
vidioc_enum_fmt_cap(struct file *file, void  *priv,
					struct v4l2_fmtdesc *f)
{
	if (f->index > 0)
		return -EINVAL;

	strlcpy(f->description,format.name,sizeof(f->description));
	f->pixelformat = format.fourcc;
	return 0;
}

static int 
vidioc_g_fmt_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vc0528_fh  *fh=priv;

	f->fmt.pix.width        = fh->width;
	f->fmt.pix.height       = fh->height;
	f->fmt.pix.field        = fh->vb_vidq.field;
	f->fmt.pix.pixelformat  = fh->fmt->fourcc;
	f->fmt.pix.bytesperline =
		(f->fmt.pix.width * fh->fmt->depth) >> 3;
	f->fmt.pix.sizeimage =
		f->fmt.pix.height * f->fmt.pix.bytesperline;

	return (0);
}

static int 
vidioc_try_fmt_cap(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct vc0528_fmt *fmt;
	enum v4l2_field field;
	unsigned int maxw, maxh;

	if (format.fourcc != f->fmt.pix.pixelformat) {
		dprintk(1,"Fourcc format (0x%08x) invalid. Driver accepts "
			"only 0x%08x\n",f->fmt.pix.pixelformat,format.fourcc);
		return -EINVAL;
	}
	fmt=&format;

	field = f->fmt.pix.field;

	if (field == V4L2_FIELD_ANY) {
//		field=V4L2_FIELD_INTERLACED;
		field=V4L2_FIELD_SEQ_TB;
	} else if (V4L2_FIELD_INTERLACED != field) {
		dprintk(1,"Field type invalid.\n");
		return -EINVAL;
	}

	maxw  = norm_maxw();  // 1024
	maxh  = norm_maxh();  // 768 

	f->fmt.pix.field = field;
	if (f->fmt.pix.height < 32)
		f->fmt.pix.height = 32;
	if (f->fmt.pix.height > maxh)
		f->fmt.pix.height = maxh;
	if (f->fmt.pix.width < 48)
		f->fmt.pix.width = 48;
	if (f->fmt.pix.width > maxw)
		f->fmt.pix.width = maxw;
	f->fmt.pix.width &= ~0x03;
	f->fmt.pix.bytesperline =
		(f->fmt.pix.width * fmt->depth) >> 3;
	f->fmt.pix.sizeimage =
		f->fmt.pix.height * f->fmt.pix.bytesperline;

	return 0;
}

/*FIXME: This seems to be generic enough to be at videodev2 */
static int 
vidioc_s_fmt_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct vc0528_fh  *fh=priv;
	int ret = vidioc_try_fmt_cap(file,fh,f);
	if (ret < 0)
		return (ret);

	fh->fmt           = &format;
	fh->width         = f->fmt.pix.width;
	fh->height        = f->fmt.pix.height;
	fh->vb_vidq.field = f->fmt.pix.field;
	fh->type          = f->type;

	return (0);
}

static int 
vidioc_reqbufs(struct file *file, void *priv, struct v4l2_requestbuffers *p)
{
	struct vc0528_fh  *fh=priv;
	
	dprintk(1,"vidioc_reqbufs\n");
	return (videobuf_reqbufs(&fh->vb_vidq, p));
}

static int 
vidioc_querybuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct vc0528_fh  *fh=priv;

	dprintk(1,"vidioc_querybuf\n");
	return (videobuf_querybuf(&fh->vb_vidq, p));
}

static int 
vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct vc0528_fh  *fh=priv;

	dprintk(1,"vidioc_qbuf\n");  
	return (videobuf_qbuf(&fh->vb_vidq, p));
}

static int 
vidioc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct vc0528_fh  *fh=priv;

	dprintk(1,"vidioc_dqbuf\n");
	return (videobuf_dqbuf(&fh->vb_vidq, p,
				file->f_flags & O_NONBLOCK));
}

#ifdef CONFIG_VIDEO_V4L1_COMPAT
static int 
vidiocgmbuf(struct file *file, void *priv, struct video_mbuf *mbuf)
{
	struct vc0528_fh  *fh=priv;
	struct videobuf_queue *q=&fh->vb_vidq;
	struct v4l2_requestbuffers req;
	unsigned int i;
	int ret;

	req.type   = q->type;
	req.count  = 8;
	req.memory = V4L2_MEMORY_MMAP;
	ret = videobuf_reqbufs(q,&req);
	if (ret < 0)
		return (ret);

	mbuf->frames = req.count;
	mbuf->size   = 0;
	for (i = 0; i < mbuf->frames; i++) {
		mbuf->offsets[i]  = q->bufs[i]->boff;
		mbuf->size       += q->bufs[i]->bsize;
	}
	return (0);
}
#endif

static int 
vidioc_streamon(struct file *file, void *priv, enum v4l2_buf_type i)
{
	struct vc0528_fh  *fh=priv;
	struct vc0528_dev *dev    = fh->dev;

	if (fh->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (i != fh->type)
		return -EINVAL;

	if (!res_get(dev,fh))
		return -EBUSY;
	return (videobuf_streamon(&fh->vb_vidq));
}


void
vc0528_vid_close(struct vc0528_dev *dev)
{
//	struct vc0528_dev      *dev  = bend_dev;
	struct vc0528_dmaqueue *vidq = &dev->vidq;
	struct vc0528_buffer   *buf;

	while (!list_empty(&vidq->active)) {
		buf = list_entry(vidq->active.next, struct vc0528_buffer, vb.queue);
		list_del(&buf->vb.queue);
		buf->vb.state = STATE_ERROR;
		wake_up(&buf->vb.done);
		dprintk(1,"[%p/%d] list clear\n", buf, buf->vb.i);
	}

	restart_video_queue(vidq);
}


static int 
vidioc_streamoff(struct file *file, void *priv, enum v4l2_buf_type i)
{
	struct vc0528_fh  *fh=priv;
	struct vc0528_dev *dev    = fh->dev;

	if (fh->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (i != fh->type)
		return -EINVAL;

	vc0528_vid_close(dev);
	canopus_bedev_ioctl(VC0528_BYPASS_MODE,NULL);   // lcd bypass mode change 
	videobuf_streamoff(&fh->vb_vidq);
	res_free(dev,fh);

	return (0);
}

static int 
vidioc_s_std (struct file *file, void *priv, v4l2_std_id *i)
{
	return 0;
}

/* only one input in this sample driver */
static int 
vidioc_enum_input (struct file *file, void *priv,
				struct v4l2_input *inp)
{
	if (inp->index != 0)
		return -EINVAL;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_NTSC_M;
	strcpy(inp->name,"Camera");

	return (0);
}

static int 
vidioc_g_input (struct file *file, void *priv, unsigned int *i)
{
	*i = 0;

	return (0);
}
static int 
vidioc_s_input (struct file *file, void *priv, unsigned int i)
{
	if (i > 0)
		return -EINVAL;

	return (0);
}

/*_____________________ controls _______________________________*/
static int 
vidioc_queryctrl (struct file *file, void *priv,
				struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vc0528_qctrl); i++)
		if (qc->id && qc->id == vc0528_qctrl[i].id) {
			memcpy(qc, &(vc0528_qctrl[i]),
				sizeof(*qc));
			return (0);
		}

	return -EINVAL;
}

static int 
vidioc_g_ctrl (struct file *file, void *priv,
				struct v4l2_control *ctrl)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vc0528_qctrl); i++)
		if (ctrl->id == vc0528_qctrl[i].id) {
			ctrl->value=qctrl_regs[i];
			return (0);
		}

	return -EINVAL;
}

static int 
vidioc_s_ctrl (struct file *file, void *priv,
				struct v4l2_control *ctrl)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vc0528_qctrl); i++)
		if (ctrl->id == vc0528_qctrl[i].id) {
			if (ctrl->value <
				vc0528_qctrl[i].minimum
				|| ctrl->value >
				vc0528_qctrl[i].maximum) {
					return (-ERANGE);
				}
			qctrl_regs[i]=ctrl->value;
			return (0);
		}
	return -EINVAL;
}


/*_____________________ File operations for the device ____________________________*/
#define line_buf_size(norm) (norm_maxw(norm)*(format.depth+7)/8)

static int 
vc0528_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct vc0528_dev *h,*dev = NULL;
	struct vc0528_fh *fh;
	struct list_head *list,*_list;
	enum v4l2_buf_type type = 0;
	int i;

	printk("@@@@@@@@@@ vc0528_open\n");
	printk(KERN_DEBUG "vc0528: open called (minor=%d)\n",minor);

#if VC0528_DEBUG_INFO
	/*__________________ list check !!_________________*/
	_list = &vc0528_devlist;
	list = _list->next;
	dprintk(1,"___________ start  open_____________\n");
	while(list != _list){
		dprintk(1,"list: 0x%x , _list: 0x%x\n",list,_list);
		list = list->next;	
	}
	dprintk(1,"list: 0x%x , _list: 0x%x\n",list,_list);

	dprintk(1,"_unidata_logo[] size : 0x%x\n",sizeof(_unidata_logo));

	/*__________________ list check !!_________________*/
#endif

	list_for_each(list,&vc0528_devlist) {
		h = list_entry(list, struct vc0528_dev, vc0528_devlist);
		if (h->vfd.minor == minor) {
			dev  = h;
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			dprintk(1,"find list h:0x%x\n",h);
		}
	}
	if (NULL == dev)
		return -ENODEV;
	
	/* If more than one user, mutex should be added */
	dev->users++;

	dprintk(1,"open minor=%d type=%s users=%d\n",
				minor,v4l2_type_names[type],dev->users);

	/* allocate + initialize per filehandle data */
	fh = kzalloc(sizeof(*fh),GFP_KERNEL);
	if (NULL == fh) {
		dev->users--;
		return -ENOMEM;
	}
	
	file->private_data = fh;
	fh->dev      = dev;

	fh->type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fh->fmt      = &format;
	fh->width    = 640;
	fh->height   = 480;

	/* Put all controls at a sane state */
	for (i = 0; i < ARRAY_SIZE(vc0528_qctrl); i++)
		qctrl_regs[i] =vc0528_qctrl[i].default_value;

	dprintk(1,"Open: fh=0x%08lx, dev=0x%08lx, dev->vidq=0x%08lx\n",
		(unsigned long)fh,(unsigned long)dev,(unsigned long)&dev->vidq);
	dprintk(1,"Open: list_empty queued=%d\n",list_empty(&dev->vidq.queued));
	dprintk(1,"Open: list_empty active=%d\n",list_empty(&dev->vidq.active));

	/* Resets frame counters */
	dev->h=0;
	dev->m=0;
	dev->s=0;
	dev->us=0;
	dev->jiffies=jiffies;
	sprintf(dev->timestr,"%02d:%02d:%02d:%03d",
			dev->h,dev->m,dev->s,(dev->us+500)/1000);

#ifdef CONFIG_VC0528_SCATTER
	videobuf_queue_init(&fh->vb_vidq,VIDEOBUF_DMA_SCATTER, &vc0528_video_qops,
			NULL, NULL,
			fh->type,
			V4L2_FIELD_INTERLACED,
			sizeof(struct vc0528_buffer),fh);
#else
	videobuf_queue_init(&fh->vb_vidq, &vc0528_video_qops,
			NULL, NULL,
			fh->type,
			V4L2_FIELD_INTERLACED,
			sizeof(struct vc0528_buffer),fh);
#endif

	/* VC0528 init */
	canopus_bedev_ioctl(VC0528_INIT,NULL);
	canopus_bedev_ioctl(VC0528_CAMERA_CAPTURE_STILL,NULL);
	return 0;
}

static ssize_t

vc0528_read(struct file *file, char __user *data, size_t count, loff_t *ppos)
{
	struct vc0528_fh        *fh = file->private_data;

	printk("@@@@@@@@@@ vc0528_read\n");
	if (fh->type==V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		if (res_locked(fh->dev))
			return -EBUSY;
		return videobuf_read_one(&fh->vb_vidq, data, count, ppos,
					file->f_flags & O_NONBLOCK);
	}
	return 0;
}

static unsigned int
vc0528_poll(struct file *file, struct poll_table_struct *wait)
{
	struct vc0528_fh        *fh = file->private_data;
	struct vc0528_buffer    *buf;

	printk("@@@@@@@@@@ vc0528_poll\n");
	dprintk(1,"%s\n",__FUNCTION__);

	if (V4L2_BUF_TYPE_VIDEO_CAPTURE != fh->type)
		return POLLERR;

	if (res_get(fh->dev,fh)) {
		dprintk(1,"poll: mmap interface\n");
		/* streaming capture */
		if (list_empty(&fh->vb_vidq.stream))
			return POLLERR;
		buf = list_entry(fh->vb_vidq.stream.next,struct vc0528_buffer,vb.stream);
	} else {
		dprintk(1,"poll: read() interface\n");
		/* read() capture */
		buf = (struct vc0528_buffer*)fh->vb_vidq.read_buf;
		if (NULL == buf)
			return POLLERR;
	}
	poll_wait(file, &buf->vb.done, wait);
	if (buf->vb.state == STATE_DONE ||
	    buf->vb.state == STATE_ERROR)
		return POLLIN|POLLRDNORM;
	return 0;
}

static int 
vc0528_release(struct inode *inode, struct file *file)
{
	struct vc0528_fh         *fh = file->private_data;
	struct vc0528_dev *dev       = fh->dev;
	struct vc0528_dmaqueue *vidq = &dev->vidq;

	int minor = iminor(inode);
	printk("@@@@@@@@@@ vc0528_release\n");

	vc0528_stop_thread(vidq);
	videobuf_mmap_free(&fh->vb_vidq);

	kfree (fh);

	dev->users--;

	canopus_bedev_ioctl(VC0528_BYPASS_MODE,NULL);
	dprintk(1,"close called (minor=%d, users=%d)\n",minor,dev->users);

	return 0;
}

static int
vc0528_mmap(struct file *file, struct vm_area_struct * vma)
{
	struct vc0528_fh        *fh = file->private_data;
	int ret;

	printk("@@@@@@@@@@ vc0528_mmap\n");
	static void __iomem	*_user_addr;
	unsigned int addr1,addr2;

    pgd_t *pgd;     
	pud_t *pud;     
	pmd_t *pmd;     
	pte_t *pte;

	ret=videobuf_mmap_mapper(&fh->vb_vidq, vma);

	/* videobuf_querybuif */
//	_user_addr = (unsigned int *)(vma->vm_start);
//	printk("buffer2: addr: 0x%x, 0x%x\n",vma->vm_start,_user_addr);
//	memcpy(_user_addr,testv4l2_buf,5);

#if 0
	dprintk (1," vma start=0x%08lx\n size=%ld\n pgoff=0x%x\n ret=%d\n",
			(unsigned long)vma->vm_start,
			(unsigned long)vma->vm_end-(unsigned long)vma->vm_start,
			(unsigned long)vma->vm_pgoff,
			ret);
#endif

#if 0
	dprintk (1,"vm_mm:pgd_t = 0x%x\n",(unsigned long*)vma->vm_mm->pgd);
	dprintk (1,"vm_mm:pgd_t = 0x%x\n",*((unsigned long*)vma->vm_mm->pgd));

	pgd = (unsigned long)vma->vm_mm->pgd;
	pud = pud_offset(pgd,(unsigned long)vma->vm_start);
	pmd = pmd_offset(pud,(unsigned long)vma->vm_start);
	pte = pte_offset_kernel(pgd,(unsigned long)vma->vm_start);			 

	printk("pud:0x%x\n, pgd:0x%x\n, pmd:0x%x\n, pte:0x%x\n",
			((unsigned long*)pud),
			((unsigned long*)pgd),
			((unsigned long*)pmd),
			((unsigned long*)pte));
#endif
	return ret;
}


#if VC0528_IOCTL_CMD_TEST
static int
v4l2_do_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct vc0528_dev *video = video_get_drvdata(vdev);
	struct vc0528_fh *handle = (struct vc0528_fh*)file->private_data;
	struct v4l2_buffer *buf=arg;

	int ret = 0;
	int size,cnt=0;
	unsigned int addr1,addr2,addr3,addr4;
	static void __iomem	*_user_addr;
	unsigned long __user_addr;
	unsigned long __user_addr2;
    pgd_t *pgd;     
	pud_t *pud;     
	pmd_t *pmd;     
	pte_t *pte;


	size = _IOC_SIZE(cmd);

	switch(cmd){
	case VIDIOC_REQBUFS:
		/* videobuf_reqbufs */
		break;
	case VIDIOC_QUERYBUF:

	_user_addr = 0xc1234567; 
	__user_addr = 0xc1234567; 

#if 0	
	_user_addr = (unsigned long)0xc0006000; 
	while(cnt!=1024){
		printk("%04d: 0x%08lx: 0x%08lx\n",cnt,(_user_addr+(cnt*4)),*((unsigned long*)_user_addr+(cnt++)));
	};
#endif

#if 1
	dprintk (1," mmap called, vma=0x%08lx\n",(unsigned long)_user_addr);
	dprintk (1,"_____________________________________________\n");
	dprintk (1," VMALLOC_START:0x%x\n",VMALLOC_START);
	dprintk (1," VMALLOC_OFFSET:0x%x\n",VMALLOC_OFFSET);
	dprintk (1," high_memory:0x%x\n",high_memory);
	dprintk (1," PGDIR_SHIFT: %d\n",PGDIR_SHIFT);
	dprintk (1," PAGE_SHIFT: %d\n",PAGE_SHIFT);
	dprintk (1," ARCH_PFN_OFFSET: 0x%08lx\n",ARCH_PFN_OFFSET); 		 
	dprintk (1," mem_map: 0x%08lx\n",mem_map); 						 
	dprintk (1," &init_mm: 0x%08lx\n",(init_mm).pgd);

	pgd = pgd_offset_k(( unsigned long)_user_addr);
	pud = pud_offset(pgd,( unsigned long)_user_addr);
	pmd = pmd_offset(pud,( unsigned long)_user_addr);
	pte = pte_offset_kernel(pmd,(unsigned long)_user_addr);			 
	printk("pgd:0x%x, pud:0x%x, pmd:0x%x, pte:0x%x \n",pgd,pud,pmd,pte);

//	dprintk (1," __pfn_to_page(pfn)",__pfn_to_page(pfn)); 	// __pfn_to_page(pfn)  : (mem_map + ((pfn) - ARCH_PFN_OFFSET)) 
//	dprintk (1," __page_to_pfn(page)",__page_to_pfn(page)); // __page_to_pfn(page) : ((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET) 
//	dprintk (1," pte_pfn(pte)",pte_pfn(pte));  				// pte_pfn(pte)  :	(pte_val(pte) >> PAGE_SHIFT) 
//	dprintk (1," pte_page(pte)",pte_page(pte));      		// pte_page(pte) :	(pfn_to_page(pte_pfn(pte))) 
//	dprintk (1," pte_pfn(pte)",pte_pfn(pte));        		// pte_pfn(pte)  :  (pte_val(pte) >> PAGE_SHIFT)
//	dprintk (1," pte_val(pte)",pte_val(pte)); 		 		// pte_val(x)    :  (x)
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)test_pgd_offset_k());  			 	
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)((init_mm).pgd+test_pgd_index())); 
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)(((init_mm).pgd+test_pgd_index()))); 
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)((init_mm).pgd)+test_pgd_index());  
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)((init_mm).pgd+((unsigned long)(_user_addr)>>21)));  
	dprintk (1," test_pgd_index(addr) 0x%08lx\n",(unsigned long*)test_pgd_index()); 		

	__user_addr2 = ((init_mm).pgd)+test_pgd_index();
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)__user_addr2);  

	__user_addr2 = (unsigned long*)((init_mm).pgd)+test_pgd_index();
	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",(unsigned long*)__user_addr2); 

	dprintk (1," pgd_offset_k(addr): 0x%08lx\n",__user_addr2);  
	dprintk (1,"_____________________________________________\n");
#endif

#if 0		
		/* videobuf_querybuif */
		_user_addr = (unsigned int *)(buf->reserved);
		printk("buffer: addr: 0x%x, 0x%x\n",buf->reserved,_user_addr);
		memcpy(_user_addr,testv4l2_buf,5);
		printk("index: 0x%x\n",buf->index);
#endif
		break;
	case VIDIOC_QBUF:
		/* videobuf_dqbuf */
		break;
	case VIDIOC_STREAMON:
		/* videobuf_streamon */
		break;
	case VIDIOC_STREAMOFF:
		/* videobuf_streamoff */
		break;
	}

	v4l_printk_ioctl(cmd);  	// v4l2 ioctl command print check !!!
	return 0;
}


static int 
vc0528_v4l2_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	return video_usercopy(inode, file, cmd, arg, v4l2_do_ioctl);
}
#endif 

void
vc0528_turning_mode_init(void)
{
	/* VC0528 init */
	canopus_bedev_ioctl(VC0528_INIT,NULL);
	canopus_bedev_ioctl(VC0528_CAMERA_PREVIEW,NULL);
}

void
vc0528_turning_mode_close(void)
{
	
}

void
vc0528_camera_preview(void)
{
	canopus_bedev_ioctl(VC0528_CAMERA_PREVIEW,NULL);
}
		
int video_ioctl_device(struct inode *inode, struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	int size;

	switch(_IOC_TYPE(cmd))
	{
	case 'V':
		printk("@@@@@@@@@@ vc0528_ioctl_device (1)\n");
		video_ioctl2(inode,file,cmd,arg);  // v4l2 command
		break;
	case 'Q':
		printk("@@@@@@@@@@ vc0528_ioctl_device (2), 0x%x\n", cmd);
		switch(cmd){
		case VC0528_CAMERA_SENSOR_NEW_SET: 
		case VC0528_CAMERA_SENSOR_ORG_SET:
		case VC0528_CAMERA_SENSOR_CHECK_NEW:
		case VC0528_CAMERA_SENSOR_CHECK_ORG:
			canopus_bedev_ioctl(cmd,arg);  // vc0528 direct control command
			break;
		case VC0528_CAMERA_TEST_INIT:
			vc0528_turning_mode_init();
			break;
		}
		break;
//		case VC0528_CAMERA_TEST_CLOSE:
//			vc0528_turning_mode_close();
//		break;
		case VC0528_CAMERA_PREVIEW:
			vc0528_camera_preview();

	default:
		printk("@@@@@@@@@@ vc0528_ioctl_device (3), 0x%x\n", cmd);
		return -EINVAL;
		break;
	}

}

static const struct 
file_operations vc0528_fops = {

	.owner		= THIS_MODULE,
	.open       = vc0528_open,
	.release    = vc0528_release,
	.read       = vc0528_read,
	.poll		= vc0528_poll,
#if VC0528_IOCTL_CMD_TEST
#error
    .ioctl      = vc0528_v4l2_ioctl, 	 /* V4L2 ioctl handler only test !! */
#else
	.ioctl      = video_ioctl_device, 	 /* V4L2 ioctl handler */
#endif	
	.mmap		= vc0528_mmap,
	.llseek     = no_llseek,

};

static struct 
video_device vc0528 = {

//	.debug  	= 3,    // by hicys 
	.debug  	= 0, 
	.name		= "vc0528",
	.type		= VID_TYPE_CAPTURE,
	.hardware	= 0,
	.fops       = &vc0528_fops,
	.minor		= -1,
	.release	= video_device_release,

	.vidioc_querycap      = vidioc_querycap,
	.vidioc_enum_fmt_cap  = vidioc_enum_fmt_cap,
	.vidioc_g_fmt_cap     = vidioc_g_fmt_cap,
	.vidioc_try_fmt_cap   = vidioc_try_fmt_cap,
	.vidioc_s_fmt_cap     = vidioc_s_fmt_cap,
	.vidioc_reqbufs       = vidioc_reqbufs,
	.vidioc_querybuf      = vidioc_querybuf,
	.vidioc_qbuf          = vidioc_qbuf,
	.vidioc_dqbuf         = vidioc_dqbuf,
	.vidioc_s_std         = vidioc_s_std,
	.vidioc_enum_input    = vidioc_enum_input,
	.vidioc_g_input       = vidioc_g_input,
	.vidioc_s_input       = vidioc_s_input,
	.vidioc_queryctrl     = vidioc_queryctrl,
	.vidioc_g_ctrl        = vidioc_g_ctrl,
	.vidioc_s_ctrl        = vidioc_s_ctrl,
	.vidioc_streamon      = vidioc_streamon,
	.vidioc_streamoff     = vidioc_streamoff,
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	.vidiocgmbuf          = vidiocgmbuf,
#endif
	.tvnorms              = V4L2_STD_NTSC_M,
	.current_norm         = V4L2_STD_NTSC_M,

};


/*_____________________ Initialization and module stuff __________________________________*/
static int 
__init vc0528_init(void)
{
	int ret;
	struct vc0528_dev *dev;

	dev = kzalloc(sizeof(*dev),GFP_KERNEL);
	if (NULL == dev)
		return -ENOMEM;
	list_add_tail(&dev->vc0528_devlist,&vc0528_devlist);

	/* init video dma queues */
	INIT_LIST_HEAD(&dev->vidq.active);
	INIT_LIST_HEAD(&dev->vidq.queued);
	init_waitqueue_head(&dev->vidq.wq);

	/* initialize locks */
	init_MUTEX(&dev->lock);
#if 0	
	dev->vidq.timeout.function = vc0528_vid_timeout;
	dev->vidq.timeout.data     = (unsigned long)dev;
	init_timer(&dev->vidq.timeout);
#else	
	bend_dev = (unsigned long)dev; 
#endif

	ret = video_register_device(&vc0528, VFL_TYPE_GRABBER, video_nr);
	printk(KERN_INFO "Unidata Systems vc0528 device driver (Load status: %d)\n", ret);
	return ret;
}

static void 
__exit vc0528_exit(void)
{
	struct vc0528_dev *h;
	struct list_head *list;

	while (!list_empty(&vc0528_devlist)) {
		list = vc0528_devlist.next;
		list_del(list);
		h = list_entry(list, struct vc0528_dev, vc0528_devlist);
		kfree (h);
	}
	video_unregister_device(&vc0528);
}


module_init(vc0528_init);
module_exit(vc0528_exit);

MODULE_DESCRIPTION("vimicro vc0528 v4l2 device driver");
MODULE_AUTHOR("yongsuk@udcsystems.com");
MODULE_LICENSE("GPL");

module_param(video_nr, int, 0);

module_param_named(debug,vc0528.debug, int, 0644);
MODULE_PARM_DESC(debug,"activates debug info");

module_param(vid_limit,int,0644);
MODULE_PARM_DESC(vid_limit,"capture memory limit in megabytes");

