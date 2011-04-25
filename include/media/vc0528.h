/*-----------------------------------------------------------------------------
 * FILE NAME : vc0528.h
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
 * NOTES: N/A
 *---------------------------------------------------------------------------*/

/*_____________________ Include Header ______________________________________*/
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/smp_lock.h>

#include <media/v4l2-dev.h>

#ifndef __VC0528_H__
#define __VC0528_H__

/*_____________________ Constants Definitions _______________________________*/
#define VC0528_FRAME_NUM			3

enum {
	VC0528_DMA_IDLE,			/*!< idle */
	VC0528_DMA_LCD_WRITE,			/*!< write lcd data */
	VC0528_DMA_BE_READ,			/*!< read camera data */
};

enum {
	VC0528_MODE_NORMAL,
	VC0528_MODE_BYPASS,
};

/*_____________________ Type definitions ____________________________________*/
struct _vc0528_data;
typedef void (*vc0528_func_t)(struct _vc0528_data *be, void *param, int length);

struct vc0528_v4l_frame {
	u32 paddress;
	void *vaddress;
	int count;
	int width;
	int height;

	struct v4l2_buffer buffer;
	struct list_head queue;
	int index;
};

typedef struct _vc0528_data {
	struct video_device *video_dev;

	struct mutex lock;			/*!< lock backend device */

	struct semaphore busy_lock;
	struct semaphore param_lock;

	int open_count;

	int dma_type;
	dma_addr_t dma_addr;
	int dma_size;
	struct work_struct dma_work;

	atomic_t mode;

	struct delayed_work capture_work;
	unsigned int cap_interval;

	vc0528_func_t cap_func;
	void *cap_param;

	wait_queue_head_t enc_queue;
	int enc_counter;

	struct list_head ready_q;
	struct list_head working_q;
	struct list_head done_q;

	spinlock_t int_lock;
	struct vc0528_v4l_frame frame[VC0528_FRAME_NUM];

	struct v4l2_format v2f;
	struct v4l2_window win;

	/* standard */
	struct v4l2_streamparm streamparm;
	struct v4l2_standard standard;

	/* misc status flag */
	bool overlay_on;
	bool capture_on;
	int overlay_pid;
	int capture_pid;
} vc0528_data;

/*_____________________ Variable Declarations _______________________________*/
extern vc0528_data *g_vc0528;

/*_____________________ Function Declarations _______________________________*/
extern void vc0528_map_io(void);
extern void vc0528_lcd_reg(u16 reg);
extern u16 vc0528_lcd_read(u16 reg);
extern void vc0528_lcd_write(u16 reg, u16 data);
extern void vc0528_lcd_data(u16 data);

extern void vc0528_dma_set(void);
extern void vc0528_dma_free(void);
extern void vc0528_dma_request(int type, dma_addr_t addr, int size);
extern void vc0528_lock(void);
extern int vc0528_trylock(void);
extern void vc0528_unlock(void);
extern int vc0528_is_locked(void);

extern int vc0528_init(void);
extern void vc0528_reset(void);
extern void vc0528_end(void);
extern int vc0528_camera_on(void);
extern int vc0528_preview_start(void);
extern int vc0528_preview_stop(void);

extern int vc0528_capture_still(void *buf, unsigned long size);
extern int vc0528_capture_video(int framerate, void *buf, unsigned long size, vc0528_func_t func, void *param);
extern int vc0528_capture_stop(void);
extern int vc0528_timer_start(unsigned int interval);
extern int vc0528_timer_stop(void);

extern int inline vc0528_mode_get(void);

#endif /* __VC0528_H__ */
