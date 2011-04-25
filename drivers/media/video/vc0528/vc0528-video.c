/*-----------------------------------------------------------------------------
 * FILE NAME : vc0528-video.c
 * 
 * PURPOSE : 
 * 
 * Copyright 1999 - 2011 UniData Communication Systems, Inc.
 * All right reserved. 
 * 
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * NOTES:
 *
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
#include <linux/version.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>

#include <media/video-buf.h>
#include <media/v4l2-common.h>
#include <media/vc0528.h>

/*_____________________ Constants Definitions _______________________________*/
#define _VC0528_V4L_MAX_FRAME_RATE		30
#define _VC0528_V4L_DEFAULT_FRAME_RATE		3

/*_____________________ Type definitions ____________________________________*/
#define _VC0528_FRAME_TIME(be)			\
	(1000 / be->streamparm.parm.capture.timeperframe.denominator)
#define _VC0528_FRAME_RATE(f)			(f.denominator / f.numerator)

/*_____________________ Imported Variables __________________________________*/

/*_____________________ Variables Definitions _______________________________*/

/*_____________________ Local Declarations __________________________________*/
static int video_nr = -1;

static int _dq_intr_cnt = 0;
static int _dq_timeout_cnt = 0;

#ifdef CONFIG_PM
static int _vc0528_v4l2_suspend(struct platform_device *pdev, pm_message_t state);
static int _vc0528_v4l2_resume(struct platform_device *pdev);
#else
#define _vc0528_v4l2_suspend	NULL
#define _vc0528_v4l2_resume	NULL
#endif

static int _vc0528_v4l_open(struct inode *inode, struct file *file);
static int _vc0528_v4l_close(struct inode *inode, struct file *file);
static ssize_t _vc0528_v4l_read(struct file *file, char *buf, size_t count, loff_t * ppos);
static int _vc0528_v4l_ioctl(struct inode *inode,
		struct file *file, unsigned int cmd, unsigned long arg);
static int _vc0528_mmap(struct file *file, struct vm_area_struct *vma);
static unsigned int _vc0528_poll(struct file *file, poll_table * wait);

static struct platform_driver _vc0528_v4l2_driver = {
	.driver = {
		.name =	"canopus_vc0528",
		.owner = THIS_MODULE,
		.bus = &platform_bus_type,
	},
	.probe = NULL,
	.remove = NULL,
	.suspend = _vc0528_v4l2_suspend,
	.resume = _vc0528_v4l2_resume,
	.shutdown = NULL,
};

static struct file_operations _vc0528_v4l_fops = {
	.owner = THIS_MODULE,
	.open = _vc0528_v4l_open,
	.release = _vc0528_v4l_close,
	.read = _vc0528_v4l_read,
	.ioctl = _vc0528_v4l_ioctl,
	.mmap = _vc0528_mmap,
	.poll = _vc0528_poll,
};

static struct video_device _vc0528_v4l_template = {
	.owner = THIS_MODULE,
	.name = "Canopus Camera",
	.type = 0,
	.type2 = VID_TYPE_CAPTURE,
	.hardware = 0,
	.fops = &_vc0528_v4l_fops,
	.release = video_device_release,
};

static int _vc0528_v4l_streamon(vc0528_data *be);
static int _vc0528_v4l_streamoff(vc0528_data *be);

/*_____________________ Internal Functions __________________________________*/
static int
_vc0528_free_frame_buf(vc0528_data *be)
{
	int i;

	for (i = 0; i < VC0528_FRAME_NUM; i++) {
		if (be->frame[i].vaddress != 0) {
			dma_free_coherent(0,
					be->frame[i].buffer.length,
					be->frame[i].vaddress,
					be->frame[i].paddress);
			be->frame[i].vaddress = 0;
		}
	}

	return 0;
}

static int
_vc0528_allocate_frame_buf(vc0528_data *be, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		be->frame[i].vaddress = dma_alloc_coherent(0,
				PAGE_ALIGN(be->v2f.fmt.pix.sizeimage),
				&be->frame[i].paddress,
				GFP_DMA | GFP_KERNEL);
		if (be->frame[i].vaddress == 0) {
			pr_debug("%s failed.\n", __func__);
			_vc0528_free_frame_buf(be);
			return -ENOBUFS;
		}

		be->frame[i].buffer.index = i;
		be->frame[i].buffer.flags = V4L2_BUF_FLAG_MAPPED;
		be->frame[i].buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		be->frame[i].buffer.length =
		    PAGE_ALIGN(be->v2f.fmt.pix.sizeimage);
		be->frame[i].buffer.memory = V4L2_MEMORY_MMAP;
		be->frame[i].buffer.m.offset = be->frame[i].paddress;
		be->frame[i].index = i;
	}

	return 0;
}

static void
_vc0528_free_frames(vc0528_data * be)
{
	int i;

	for (i = 0; i < VC0528_FRAME_NUM; i++) {
		be->frame[i].buffer.flags = V4L2_BUF_FLAG_MAPPED;
	}

	be->enc_counter = 0;

	INIT_LIST_HEAD(&be->ready_q);
	INIT_LIST_HEAD(&be->working_q);
	INIT_LIST_HEAD(&be->done_q);
}

static int
_vc0528_v4l2_buffer_status(vc0528_data * be, struct v4l2_buffer *buf)
{
	/* check range */
	if (buf->index < 0 || buf->index >= VC0528_FRAME_NUM) {
		pr_debug("%s buffers not allocated\n", __func__);
		return -EINVAL;
	}

	memcpy(buf, &(be->frame[buf->index].buffer), sizeof(*buf));
	return 0;
}

static int
_vc0528_v4l_dqueue(vc0528_data *be, struct v4l2_buffer *buf)
{
	int retval = 0;
	struct vc0528_v4l_frame *frame;
	unsigned long lock_flags;

	if (!wait_event_interruptible_timeout(be->enc_queue,
			be->enc_counter != 0, 1 * HZ)) {
		return -ETIME;
	} else if (signal_pending(current)) {
		return -ERESTARTSYS;
	}

	spin_lock_irqsave(&be->int_lock, lock_flags);

	be->enc_counter--;

	frame = list_entry(be->done_q.next, struct vc0528_v4l_frame, queue);
	list_del(be->done_q.next);
	if (frame->buffer.flags & V4L2_BUF_FLAG_DONE) {
		frame->buffer.flags &= ~V4L2_BUF_FLAG_DONE;
	} else if (frame->buffer.flags & V4L2_BUF_FLAG_QUEUED) {
		printk(KERN_ERR "VIDIOC_DQBUF: Buffer not filled.\n");
		frame->buffer.flags &= ~V4L2_BUF_FLAG_QUEUED;
		retval = -EINVAL;
	} else if ((frame->buffer.flags & 0x7) == V4L2_BUF_FLAG_MAPPED) {
		printk(KERN_ERR "VIDIOC_DQBUF: Buffer not queued.\n");
		retval = -EINVAL;
	}

	buf->bytesused = frame->buffer.bytesused;
	buf->index = frame->index;
	buf->flags = frame->buffer.flags;
	buf->m = be->frame[frame->index].buffer.m;

	spin_unlock_irqrestore(&be->int_lock, lock_flags);

	return retval;
}

static inline int
_valid_mode(u32 palette)
{
	return (palette == V4L2_PIX_FMT_JPEG);
}

static int
_verify_preview(vc0528_data * be, struct v4l2_window *win)
{
	struct fb_info *fb = (struct fb_info *)registered_fb[0];

	if (win->w.width + win->w.left > fb->var.xres)
		win->w.width = fb->var.xres - win->w.left;
	if (win->w.height + win->w.top > fb->var.yres)
		win->w.height = fb->var.yres - win->w.top;

	return 0;
}

static int
_vc0528_v4l2_g_fmt(vc0528_data * be, struct v4l2_format *f)
{
	int retval = 0;

	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		f->fmt.pix.width = be->v2f.fmt.pix.width;
		f->fmt.pix.height = be->v2f.fmt.pix.height;
		f->fmt.pix.sizeimage = be->v2f.fmt.pix.sizeimage;
		f->fmt.pix.pixelformat = be->v2f.fmt.pix.pixelformat;
		f->fmt.pix.bytesperline = be->v2f.fmt.pix.bytesperline;
		f->fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
		retval = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		f->fmt.win = be->win;
		break;
	default:
		retval = -EINVAL;
	}

	return retval;
}

static int
_vc0528_v4l2_s_fmt(vc0528_data * be, struct v4l2_format *f)
{
	int retval = 0;
	int size = 0;
	int bytesperline = 0;

	switch (f->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (!_valid_mode(f->fmt.pix.pixelformat)) {
			printk(KERN_DEBUG"%s: format not supported\n", __func__);
			retval = -EINVAL;
		}

		be->v2f.fmt.pix.width = f->fmt.pix.width;
		be->v2f.fmt.pix.height = f->fmt.pix.height;
		retval = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		retval = _verify_preview(be, &f->fmt.win);
		be->win = f->fmt.win;
		break;
	default:
		retval = -EINVAL;
	}
	return retval;
}

static int
_vc0528_v4l2_s_param(vc0528_data *be, struct v4l2_streamparm *parm)
{
	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		printk(KERN_ERR "%s : invalid type\n", __func__);
		return -EINVAL;
	}

	if (_VC0528_FRAME_RATE(parm->parm.capture.timeperframe)
			> _VC0528_FRAME_RATE(be->standard.frameperiod)) {
		printk(KERN_ERR "%s frame rate %d larger "
		       "than standard supported %d\n", __func__,
		       _VC0528_FRAME_RATE(parm->parm.capture.timeperframe),
		       _VC0528_FRAME_RATE(be->standard.frameperiod));
		return -EINVAL;
	}

	be->streamparm.parm.capture.timeperframe.denominator = parm->parm.capture.timeperframe.denominator;
	be->streamparm.parm.capture.timeperframe.numerator = parm->parm.capture.timeperframe.numerator;

	return 0;
}

static void
_vc0528_v4l_capture_cb(vc0528_data *be, void *param, int length)
{
	struct vc0528_v4l_frame *frame;
	unsigned long lock_flags;

	if (list_empty(&be->working_q)) return ;

	if (length <= 0) {
		printk(KERN_DEBUG"%s : capture failed\n", __func__);
		return ;
	}

	spin_lock_irqsave(&be->int_lock, lock_flags);

	frame = list_entry(be->working_q.next, struct vc0528_v4l_frame, queue);

	frame->buffer.bytesused = length;
	frame->buffer.flags |= V4L2_BUF_FLAG_DONE;
	frame->buffer.flags &= ~V4L2_BUF_FLAG_QUEUED;

	list_del(be->working_q.next);
	list_add_tail(&frame->queue, &be->done_q);

	if (!list_empty(&be->ready_q)) {
		frame = list_entry(be->ready_q.next, struct vc0528_v4l_frame, queue);
		vc0528_capture_set_info(frame->vaddress, frame->buffer.length);

		list_del(be->ready_q.next);
		list_add_tail(&frame->queue, &be->working_q);
	}

	spin_unlock_irqrestore(&be->int_lock, lock_flags);

	/* Wake up the queue */
	be->enc_counter++;
	wake_up_interruptible(&be->enc_queue);
}

static int
_vc0528_v4l_streamon(vc0528_data *be)
{
	struct vc0528_v4l_frame *ready_frame;
	int ret = -1;
	unsigned long lock_flags;

	if (list_empty(&be->ready_q)) return -EINVAL;

	spin_lock_irqsave(&be->int_lock, lock_flags);

	ready_frame = list_entry(be->ready_q.next, struct vc0528_v4l_frame, queue);

	be->capture_pid = current->pid;
	ret = vc0528_capture_video(_VC0528_FRAME_RATE(be->streamparm.parm.capture.timeperframe),
			ready_frame->vaddress, ready_frame->buffer.length,
			_vc0528_v4l_capture_cb, NULL);

	if (ret == 0) {
		list_del(be->ready_q.next);
		list_add_tail(&ready_frame->queue, &be->working_q);
	}

	spin_unlock_irqrestore(&be->int_lock, lock_flags);

	return 0;
}

static int
_vc0528_v4l_streamoff(vc0528_data *be)
{
	unsigned long lock_flags;

	if (!be) return -EIO;

	spin_lock_irqsave(&be->int_lock, lock_flags);

	vc0528_capture_stop();
	_vc0528_free_frames(be);
	be->capture_pid = 0;

	spin_unlock_irqrestore(&be->int_lock, lock_flags);

	return 0;
}

static int
_vc0528_v4l_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct video_device *dev = video_devdata(file);
	vc0528_data *be = dev->priv;

	_dq_intr_cnt = 0;
	_dq_timeout_cnt = 0;

	if (!be) {
		printk(KERN_ERR"Internal error, vc0528_data not found!\n");
		return -ENODEV;
	}

	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	if (signal_pending(current)) {
		ret = -1;
		goto _v4l_open_error;
	}

	if (be->open_count == 0) {
		be->enc_counter= 0;
		INIT_LIST_HEAD(&be->ready_q);
		INIT_LIST_HEAD(&be->working_q);
		INIT_LIST_HEAD(&be->done_q);

		vc0528_reset();
		ret = vc0528_init();
		if (ret) goto _v4l_open_error;

		ret = vc0528_camera_on();
		if (ret) goto _v4l_open_error;

		be->open_count++;
	}

	file->private_data = dev;

_v4l_open_error:
	if (ret) vc0528_reset();
	up(&be->busy_lock);

	return ret;
}

static int
_vc0528_v4l_close(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct video_device *dev = video_devdata(file);
	vc0528_data *be = dev->priv;

	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	/* for the case somebody hit the ctrl C */
	if (be->overlay_pid == current->pid) {
		ret = vc0528_preview_stop();
		be->overlay_on = false;
	}

	if (be->capture_pid == current->pid) {
		ret = _vc0528_v4l_streamoff(be);
		be->capture_on = false;
		wake_up_interruptible(&be->enc_queue);
	}

	if (--be->open_count == 0) {
		_vc0528_free_frame_buf(be);
		file->private_data = NULL;

		wake_up_interruptible(&be->enc_queue);
		_vc0528_free_frames(be);
		be->enc_counter++;

		vc0528_end();
	}

	if (be->open_count == -1)
		be->open_count = 0;

	up(&be->busy_lock);

	return ret;
}

static ssize_t
_vc0528_v4l_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	int ret = 0;
	struct video_device *dev = video_devdata(file);
	vc0528_data *be = dev->priv;

	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	/* Video capture and still image capture are exclusive */
	if (be->capture_on == true) {
		ret = -EBUSY;
		goto _v4l_read_end;
	}

	ret = vc0528_capture_still(buf, count);

_v4l_read_end:
	up(&be->busy_lock);
	if (ret < 0)
	      return ret;
      else if (ret == 0)
	      return -1;

      return ret;
}

static int
_vc0528_v4l_do_ioctl(struct inode *inode,
		struct file *file, unsigned int ioctlnr, void *arg)
{
	int ret = 0;
	struct video_device *dev = video_devdata(file);
	vc0528_data *be = dev->priv;
	int retval = 0;
	unsigned long lock_flags;

	if (!be)
		return -EBADF;

	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	switch (ioctlnr) {
		/*!
		 * V4l2 VIDIOC_QUERYCAP ioctl
		 */
	case VIDIOC_QUERYCAP:{
			struct v4l2_capability *cap = arg;
			strcpy(cap->driver, "canopus_v4l2");
			cap->version = KERNEL_VERSION(0, 1, 0);
			cap->capabilities = V4L2_CAP_VIDEO_CAPTURE
					| V4L2_CAP_VIDEO_OVERLAY
					| V4L2_CAP_STREAMING
					| V4L2_CAP_READWRITE;
			cap->card[0] = '\0';
			cap->bus_info[0] = '\0';
			retval = 0;
			break;
		}

		/*!
		 * V4l2 VIDIOC_G_FMT ioctl
		 */
	case VIDIOC_G_FMT:{
			struct v4l2_format *gf = arg;
			retval = _vc0528_v4l2_g_fmt(be, gf);
			break;
		}

		/*!
		 * V4l2 VIDIOC_S_FMT ioctl
		 */
	case VIDIOC_S_FMT:{
			struct v4l2_format *sf = arg;
			retval = _vc0528_v4l2_s_fmt(be, sf);
			break;
		}

		/*!
		 * V4l2 VIDIOC_REQBUFS ioctl
		 */
	case VIDIOC_REQBUFS:{
			struct v4l2_requestbuffers *req = arg;
			int i;
			if (req->count > VC0528_FRAME_NUM) {
				pr_info("VIDIOC_REQBUFS: not enough buffer\n");
				req->count = VC0528_FRAME_NUM;
			}

			if ((req->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) ||
			    ((req->memory != V4L2_MEMORY_MMAP)
			     && (req->memory != V4L2_MEMORY_USERPTR))) {
				pr_debug("VIDIOC_REQBUFS: wrong buffer type\n");
				retval = -EINVAL;
				break;
			}

			_vc0528_v4l_streamoff(be);
			_vc0528_free_frame_buf(be);

			if (req->memory == V4L2_MEMORY_MMAP)
				retval = _vc0528_allocate_frame_buf(be, req->count);
			else if (req->memory == V4L2_MEMORY_USERPTR) {
				for (i = 0; i < req->count; i++) {
					be->frame[i].vaddress = 0;
					be->frame[i].buffer.index = i;
					be->frame[i].buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					be->frame[i].buffer.flags = 0;
					be->frame[i].buffer.length = 0;
					be->frame[i].buffer.memory = V4L2_MEMORY_USERPTR;
					be->frame[i].buffer.m.offset = 0;
					be->frame[i].index = i;
				}
			}
			break;
		}

		/*!
		 * V4l2 VIDIOC_QUERYBUF ioctl
		 */
	case VIDIOC_QUERYBUF:{
			struct v4l2_buffer *buf = arg;
			int index = buf->index;

			if (buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
				pr_debug
				    ("VIDIOC_QUERYBUFS: wrong buffer type\n");
				retval = -EINVAL;
				break;
			}

			memset(buf, 0, sizeof(buf));
			buf->index = index;

			down(&be->param_lock);
			retval = _vc0528_v4l2_buffer_status(be, buf);
			up(&be->param_lock);
			break;
		}

		/*!
		 * V4l2 VIDIOC_QBUF ioctl
		 */
	case VIDIOC_QBUF:{
			struct v4l2_buffer *buf = arg;
			int index = buf->index;

			pr_debug("VIDIOC_QBUF: %d\n", buf->index);

			spin_lock_irqsave(&be->int_lock, lock_flags);
			if (be->frame[index].buffer.memory == V4L2_MEMORY_MMAP) {
				if ((be->frame[index].buffer.flags & 0x7) == V4L2_BUF_FLAG_MAPPED) {
					be->frame[index].buffer.flags |= V4L2_BUF_FLAG_QUEUED;
					list_add_tail(&be->frame[index].queue, &be->ready_q);
				} else if (be->frame[index].buffer.flags & V4L2_BUF_FLAG_QUEUED) {
					pr_debug("%s: buffer already queued\n", __func__);
				} else if (be->frame[index].buffer.flags & V4L2_BUF_FLAG_DONE) {
					pr_debug("%s: overwrite done buffer.\n", __func__);
					be->frame[index].buffer.flags &= ~V4L2_BUF_FLAG_DONE;
					be->frame[index].buffer.flags |= V4L2_BUF_FLAG_QUEUED;
				}
			} else {
				be->frame[index].buffer.flags = (V4L2_BUF_FLAG_MAPPED | V4L2_BUF_FLAG_QUEUED);
				be->frame[index].buffer.m.offset = buf->m.offset;
				be->frame[index].paddress = buf->m.offset;
				be->frame[index].buffer.length = buf->length;
				list_add_tail(&be->frame[index].queue, &be->ready_q);
			}

			buf->flags = be->frame[index].buffer.flags;
			spin_unlock_irqrestore(&be->int_lock, lock_flags);
			break;
		}

		/*!
		 * V4l2 VIDIOC_DQBUF ioctl
		 */
	case VIDIOC_DQBUF:{
			struct v4l2_buffer *buf = arg;

			retval = _vc0528_v4l_dqueue(be, buf);

			break;
		}

		/*!
		 * V4l2 VIDIOC_STREAMON ioctl
		 */
	case VIDIOC_STREAMON:{
			be->capture_on = true;
			retval = _vc0528_v4l_streamon(be);
			break;
		}

		/*!
		 * V4l2 VIDIOC_STREAMOFF ioctl
		 */
	case VIDIOC_STREAMOFF:{
			retval = _vc0528_v4l_streamoff(be);
			be->capture_on = false;
			break;
		}

		/*!
		 * V4l2 VIDIOC_OVERLAY ioctl
		 */
	case VIDIOC_OVERLAY:{
			int *on = arg;
			if (*on) {
				be->overlay_on = true;
				retval = vc0528_preview_start();
			}
			if (!*on) {
				be->overlay_on = false;
				retval = vc0528_preview_stop();
			}
			break;
		}

	case VIDIOC_G_PARM:{
			struct v4l2_streamparm *parm = arg;
			if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
				pr_debug("VIDIOC_G_PARM invalid type\n");
				retval = -EINVAL;
				break;
			}
			parm->parm.capture = be->streamparm.parm.capture;
			break;
		}

	case VIDIOC_S_PARM:{
			struct v4l2_streamparm *parm = arg;
			retval = _vc0528_v4l2_s_param(be, parm);
			break;
		}

		/* linux v4l2 bug, kernel c0485619 user c0405619 */
	case VIDIOC_ENUMSTD:{
			struct v4l2_standard *e = arg;
			*e = be->standard;
			pr_debug("VIDIOC_ENUMSTD call\n");
			retval = 0;
			break;
		}

	case VIDIOC_G_STD: {
			v4l2_std_id *e = arg;
			*e = be->standard.id;
			break;
		}

	case VIDIOC_S_STD:
	case VIDIOC_G_OUTPUT:
	case VIDIOC_S_OUTPUT:
	case VIDIOC_G_INPUT:
		break;

	case VIDIOC_CROPCAP:
	case VIDIOC_G_CROP:
	case VIDIOC_S_CROP:
	case VIDIOC_G_FBUF:
	case VIDIOC_S_FBUF:
	case VIDIOC_G_CTRL:
	case VIDIOC_S_CTRL:
	case VIDIOC_ENUMOUTPUT:
	case VIDIOC_ENUM_FMT:
	case VIDIOC_TRY_FMT:
	case VIDIOC_QUERYCTRL:
	case VIDIOC_ENUMINPUT:
	case VIDIOC_S_INPUT:
	case VIDIOC_G_TUNER:
	case VIDIOC_S_TUNER:
	case VIDIOC_G_FREQUENCY:
	case VIDIOC_S_FREQUENCY:
	default:
		retval = -EINVAL;
		break;
	}

	up(&be->busy_lock);

	return ret;
}

static int
_vc0528_v4l_ioctl(struct inode *inode,
		struct file *file, unsigned int cmd, unsigned long arg)
{
	return video_usercopy(inode, file, cmd, arg, _vc0528_v4l_do_ioctl);
}

static int
_vc0528_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret = 0;
	struct video_device *dev = video_devdata(file);
	unsigned long size;
	vc0528_data *be= dev->priv;

	printk(KERN_DEBUG"pgoff=0x%lx, start=0x%lx, end=0x%lx\n",
		 vma->vm_pgoff, vma->vm_start, vma->vm_end);

	/* make this _really_ smp-safe */
	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	size = vma->vm_end - vma->vm_start;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (remap_pfn_range(vma, vma->vm_start,
			    vma->vm_pgoff, size, vma->vm_page_prot)) {
		printk(KERN_DEBUG"%s: remap_pfn_range failed\n", __func__);
		ret = -ENOBUFS;
		goto _vc0528_mmap_exit;
	}

	vma->vm_flags &= ~VM_IO;	/* using shared anonymous pages */

_vc0528_mmap_exit:
	up(&be->busy_lock);

	return ret;
}

static unsigned int
_vc0528_poll(struct file *file, poll_table * wait)
{
	struct video_device *dev = video_devdata(file);
	vc0528_data *be= dev->priv;
	wait_queue_head_t *queue = NULL;
	int res = POLLIN | POLLRDNORM;

	if (down_interruptible(&be->busy_lock))
		return -EINTR;

	queue = &be->enc_queue;
	poll_wait(file, queue, wait);

	up(&be->busy_lock);

	return res;
}

static int
_vc0528_v4l2_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int
_vc0528_v4l2_resume(struct platform_device *pdev)
{
	return 0;
}

/*_____________________ Program Body ________________________________________*/
static int __init
vc0528_v4l2_init(void)
{
	int ret = 0, i;

	printk(KERN_INFO "VC0528 V4L2 device driver init\n");

	vc0528_map_io();

	if (!g_vc0528) {
		printk(KERN_ERR "vc0528 init data failed\n");
		return ret;
	}

	g_vc0528->video_dev = video_device_alloc();
	if (g_vc0528->video_dev == NULL)
		return -1;

	*(g_vc0528->video_dev) = _vc0528_v4l_template;

	video_set_drvdata(g_vc0528->video_dev, g_vc0528);
	g_vc0528->video_dev->minor = -1;

	if (ret != 0) {
		printk(KERN_ERR "vc0528 init data failed\n");
		return ret;
	}

	ret = platform_driver_register(&_vc0528_v4l2_driver);
	if (ret != 0) {
		video_device_release(g_vc0528->video_dev);
		g_vc0528->video_dev = NULL;
		printk(KERN_ERR"vc0528 register_driver failed\n");

		return ret;
	}

	if (video_register_device(g_vc0528->video_dev, VFL_TYPE_GRABBER, video_nr) == -1) {
		platform_driver_unregister(&_vc0528_v4l2_driver);
		video_device_release(g_vc0528->video_dev);
		g_vc0528->video_dev = NULL;
		printk(KERN_ERR"video_register_device failed\n");

		return -1;
	}

	g_vc0528->standard.index = 0;
	g_vc0528->standard.id = V4L2_STD_UNKNOWN;
	g_vc0528->standard.frameperiod.denominator = _VC0528_V4L_MAX_FRAME_RATE;
	g_vc0528->standard.frameperiod.numerator = 1;
	g_vc0528->standard.framelines = 480;
	g_vc0528->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	g_vc0528->streamparm.parm.capture.timeperframe.denominator = _VC0528_V4L_DEFAULT_FRAME_RATE;
	g_vc0528->streamparm.parm.capture.timeperframe.numerator = 1;
	g_vc0528->streamparm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	g_vc0528->overlay_on = false;
	g_vc0528->capture_on = false;

	g_vc0528->v2f.fmt.pix.sizeimage = 640 * 480 * 3 / 2;
	g_vc0528->v2f.fmt.pix.bytesperline = 480 * 3 / 2;
	g_vc0528->v2f.fmt.pix.width = 640;
	g_vc0528->v2f.fmt.pix.height = 480;
	g_vc0528->v2f.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;

	g_vc0528->win.w.width = 240;
	g_vc0528->win.w.height = 320;
	g_vc0528->win.w.left = 0;
	g_vc0528->win.w.top = 0;

	for (i = 0; i < VC0528_FRAME_NUM; i++) {
		g_vc0528->frame[i].width = 0;
		g_vc0528->frame[i].height = 0;
		g_vc0528->frame[i].paddress = 0;
	}

	init_MUTEX(&g_vc0528->busy_lock);
	init_MUTEX(&g_vc0528->param_lock);

	init_waitqueue_head(&g_vc0528->enc_queue);
	//INIT_DELAYED_WORK(&g_vc0528->capture_work, _vc0528_capture_delayed_work);

	g_vc0528->int_lock = SPIN_LOCK_UNLOCKED;
	spin_lock_init(&g_vc0528->int_lock);

	return ret;
}

static void __exit
vc0528_v4l2_exit(void)
{
	video_unregister_device(g_vc0528->video_dev);
	g_vc0528->video_dev = NULL;
	platform_driver_unregister(&_vc0528_v4l2_driver);

	if (g_vc0528->open_count) {
		printk(KERN_DEBUG"camera open -- setting ops to NULL\n");
	} else {
		printk(KERN_DEBUG"freeing camera\n");
	}
}

/*_____________________ Linux Macro _________________________________________*/
module_init(vc0528_v4l2_init);
module_exit(vc0528_v4l2_exit);

module_param(video_nr, int, 0444);

MODULE_DESCRIPTION("VC0528 v4l2 device driver");
MODULE_AUTHOR("kimkh@udcsystems.com");
MODULE_LICENSE("GPL");
