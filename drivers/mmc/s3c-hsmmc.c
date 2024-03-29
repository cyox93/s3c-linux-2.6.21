/*
 *  linux/drivers/mmc/s3c-hsmmc.c - Samsung S3C24XX HS-MMC driver
 *
 * $Id: s3c-hsmmc.c,v 1.73 2008/04/13 23:56:19 jsgood Exp $
 *
 *  Copyright (C) 2006 Samsung Electronics, All Rights Reserved.
 *  by Suh, Seung-Chull<sc.suh@samsung.com>
 *
 *  This driver is made for High Speed MMC interface. This interface
 *  is adopted and implemented since s3c2443 was made.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Modified by Ryu,Euiyoul <steven.ryu@samsung.com>
 *  Modified by Suh, Seung-chull to support s3c6400
 *
 * Note: s3c-hsmmc.c can be used for 1 channel only at same time.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/highmem.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/protocol.h>
#include <linux/mmc/card.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/irq.h>

#include <asm/dma.h>
#include <asm/dma-mapping.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/sizes.h>
#include <asm/mach/mmc.h>

#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-hsmmc.h>
#include <asm/plat-s3c24xx/clock.h>
#include <asm/arch/dma.h>

#include <asm/arch/hsmmc.h>


#ifdef CONFIG_S3CMMC_DEBUG
#define DBG(x...)       printk(PFX x)
#else
#define DBG(x...)       do { } while (0)
#endif

#include "s3c-hsmmc.h"

#define DRIVER_NAME "s3c-hsmmc"
#define PFX DRIVER_NAME ": "

#define MULTICARD_ON_SINGLEBUS_SUPPORT
#undef  MULTICARD_ON_SINGLEBUS_SUPPORT

#define RESSIZE(ressource) (((ressource)->end - (ressource)->start)+1)

#if defined(CONFIG_PM)
struct s3c_hsmmc_host *global_host[3];
#endif

static void s3c_hsmmc_tasklet_finish (unsigned long param);
static int card_detect = 0;
/*****************************************************************************\
 *                                                                           *
 * Low level functions                                                       *
 *                                                                           *
\*****************************************************************************/

static struct s3c_hsmmc_cfg s3c_hsmmc_platform = {
	.hwport = 0,
	.enabled = 0,
	.host_caps = (MMC_CAP_4_BIT_DATA | MMC_CAP_MULTIWRITE),
	.base = NULL,
	.highspeed = 0,

	/* ctrl for mmc */
	.fd_ctrl[0] = {
		.ctrl2    = 0xC0004100,	/* ctrl2 for mmc */
		.ctrl3[0] = 0x80808080,	/* ctrl3 for low speed */
		.ctrl3[1] = 0x00000080,	/* ctrl3 for high speed */
		.ctrl4 = 0,
	},
	
	/* ctrl for sd */
	.fd_ctrl[1] = {
		.ctrl2    = 0xC0000100,	/* ctrl2 for sd */
		.ctrl3[0] = 0,		/* ctrl3 for low speed */
		.ctrl3[1] = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0,
	},
};

/* s3c_hsmmc_get_platdata
 *
 * get the platform data associated with the given device, or return
 * the default if there is none
 */

static struct s3c_hsmmc_cfg *s3c_hsmmc_get_platdata (struct device *dev)
{
	if (dev->platform_data != NULL)
		return (struct s3c_hsmmc_cfg *)dev->platform_data;

	return &s3c_hsmmc_platform;
}

static void s3c_hsmmc_reset (struct s3c_hsmmc_host *host, u8 mask)
{
	unsigned long timeout;

	s3c_hsmmc_writeb(mask, S3C_HSMMC_SWRST);

	if (mask & S3C_HSMMC_RESET_ALL)
		host->clock = (uint)-1;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (s3c_hsmmc_readb(S3C_HSMMC_SWRST) & mask) {
		if (timeout == 0) {
			printk("%s: Reset 0x%x never completed. \n",
				mmc_hostname(host->mmc), (int)mask);
			return;
		}
		timeout--;
		mdelay(1);
	}

}

static void s3c_hsmmc_ios_init (struct s3c_hsmmc_host *host)
{
	u32 intmask;

	s3c_hsmmc_reset(host, S3C_HSMMC_RESET_ALL);

	intmask = S3C_HSMMC_INT_BUS_POWER | S3C_HSMMC_INT_DATA_END_BIT |
		S3C_HSMMC_INT_DATA_CRC | S3C_HSMMC_INT_DATA_TIMEOUT | S3C_HSMMC_INT_INDEX |
		S3C_HSMMC_INT_END_BIT | S3C_HSMMC_INT_CRC | S3C_HSMMC_INT_TIMEOUT |
		S3C_HSMMC_INT_CARD_REMOVE | S3C_HSMMC_INT_CARD_INSERT |
		S3C_HSMMC_INT_DATA_AVAIL | S3C_HSMMC_INT_SPACE_AVAIL |
		S3C_HSMMC_INT_DATA_END | S3C_HSMMC_INT_RESPONSE;

#ifdef CONFIG_HSMMC_SCATTERGATHER
	intmask |= S3C_HSMMC_INT_DMA_END;
#endif
	s3c_hsmmc_writel(intmask, S3C_HSMMC_NORINTSTSEN);
	s3c_hsmmc_writel(intmask, S3C_HSMMC_NORINTSIGEN);
}

/*****************************************************************************\
 *                                                                           *
 * Tasklets                                                                  *
 *                                                                           *
\*****************************************************************************/

static void s3c_hsmmc_tasklet_card (ulong param)
{
	struct s3c_hsmmc_host *host;
	unsigned long iflags;

	host = (struct s3c_hsmmc_host*)param;
	spin_lock_irqsave(&host->lock, iflags);

	if (!(s3c_hsmmc_readl(S3C_HSMMC_PRNSTS) & S3C_HSMMC_CARD_PRESENT)) {
		if (host->mrq) {
			printk(KERN_ERR "%s: Card removed during transfer!\n",
				mmc_hostname(host->mmc));
			printk(KERN_ERR "%s: Resetting controller.\n",
				mmc_hostname(host->mmc));

			s3c_hsmmc_reset(host, S3C_HSMMC_RESET_CMD);
			s3c_hsmmc_reset(host, S3C_HSMMC_RESET_DATA);

			host->mrq->cmd->error = MMC_ERR_FAILED;

			/* tasklet_schedule(&host->finish_tasklet); */
			s3c_hsmmc_tasklet_finish((unsigned long) host);
		}
	}
 
	spin_unlock_irqrestore(&host->lock, iflags);

	mmc_detect_change(host->mmc, msecs_to_jiffies(500));
}

static void s3c_hsmmc_activate_led(struct s3c_hsmmc_host *host)
{
#if 0
	u8 ctrl;

	ctrl = s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL);
	ctrl |= S3C_HSMMC_CTRL_LED;
	s3c_hsmmc_writeb(ctrl, S3C_HSMMC_HOSTCTL);
#endif

#if 0	// for 6400
	s3c_gpio_cfgpin(S3C_GPJ13, S3C_GPJ13_SD0LED);
#endif
}

static void s3c_hsmmc_deactivate_led(struct s3c_hsmmc_host *host)
{
#if 0
	u8 ctrl;

	ctrl = s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL);
	ctrl &= ~S3C_HSMMC_CTRL_LED;
	s3c_hsmmc_writeb(ctrl, S3C_HSMMC_HOSTCTL);
#endif

#if 0	// for 6400
	s3c_gpio_cfgpin(S3C_GPJ13, S3C_GPJ13_INP);
#endif
}

/*****************************************************************************\
 *                                                                           *
 * Core functions                                                            *
 *                                                                           *
\*****************************************************************************/

#ifdef CONFIG_HSMMC_SCATTERGATHER
static inline uint s3c_hsmmc_build_dma_table (struct s3c_hsmmc_host *host,
						struct mmc_data *data)
{
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C2443)
	uint i, j = 0, sub_num = 0;
	dma_addr_t addr;
	uint size, length, end;
	int boundary, xor_bit;
	struct scatterlist * sg = data->sg;

	/* build dma table except last one */
	for (i=0; i<(host->sg_len-1); i++) {
		addr = sg[i].dma_address;
		size = sg[i].length;
		DBG("%d - addr: %08x, size: %08x\n", i, addr, size);

		for (; (j<CONFIG_S3C_HSMMC_MAX_HW_SEGS*4) && size; j++) {
			end = addr + size;
			xor_bit = min(7+(2+8+2), fls(addr^end) -1);

			DBG("%08x %08x %08x %d\n", addr, size, end, xor_bit);

			host->dblk[j].dma_address = addr;

			length = (end & ~((1<<xor_bit)-1)) - addr;
			boundary = xor_bit - (2+8+2);
			DBG("length: %x, boundary: %d\n", length, boundary);

			if (length < S3C_HSMMC_MALLOC_SIZE) {
				boundary = 0;

				if ((addr+length) & (S3C_HSMMC_MALLOC_SIZE-1)) {
					void *dest;

					DBG("#########error fixing: %08x, %x\n", addr, length);
					dest = host->sub_block[sub_num] + S3C_HSMMC_MALLOC_SIZE - length;
					if (data->flags & MMC_DATA_WRITE) { /* write */
						memcpy(dest, phys_to_virt(addr), length);
					}

					host->dblk[j].original = phys_to_virt(addr);
					host->dblk[j].dma_address = dma_map_single(NULL, dest, length, host->dma_dir);
					sub_num++;
				}
			}

			host->dblk[j].length = length;
			host->dblk[j].boundary = boundary;
			DBG("   %d: %08x, %08x %x\n",
						j, addr, length, boundary);
			addr += length;
			size -= length;
		}
	}

	/* the last one */
	host->dblk[j].dma_address = sg[i].dma_address;
	host->dblk[j].length = sg[i].length;
	host->dblk[j].boundary = 0x7;

 	return (j+1);
	
#elif defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	uint i;
	struct scatterlist * sg = data->sg;

	/* build dma table except the last one */
	for (i=0; i<(host->sg_len-1); i++) {
		host->sdma_descr_tbl[i].dma_address = sg[i].dma_address;
		host->sdma_descr_tbl[i].length_attr = (sg[i].length << 16) | (S3C_HSMMC_ADMA_ATTR_ACT_TRAN) |
			(S3C_HSMMC_ADMA_ATTR_VALID);
		
		DBG("  ADMA2 descr table[%d] - addr: %08x, size+attr: %08x\n", i, host->sdma_descr_tbl[i].dma_address, 
										host->sdma_descr_tbl[i].length_attr);
	}

	/* the last one */
	host->sdma_descr_tbl[i].dma_address = sg[i].dma_address;
	host->sdma_descr_tbl[i].length_attr = (sg[i].length << 16) | (S3C_HSMMC_ADMA_ATTR_ACT_TRAN) |
			(S3C_HSMMC_ADMA_ATTR_END | S3C_HSMMC_ADMA_ATTR_VALID);
	
	DBG("  ADMA2 descr table[%d] - addr: %08x, size+attr: %08x\n", i, host->sdma_descr_tbl[i].dma_address, 
										host->sdma_descr_tbl[i].length_attr);

 	return (i);
#endif

}
#endif

static inline void s3c_hsmmc_prepare_data (struct s3c_hsmmc_host *host,
						struct mmc_command *cmd)
{
#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	u8 reg8;
#endif
	u32 reg;
	struct mmc_data *data = cmd->data;

	if (data == NULL) {
		reg = s3c_hsmmc_readl(S3C_HSMMC_NORINTSTSEN) | S3C_HSMMC_NIS_CMDCMP;
		s3c_hsmmc_writel(reg, S3C_HSMMC_NORINTSTSEN);
		return;
	}

	reg = s3c_hsmmc_readl(S3C_HSMMC_NORINTSTSEN) & ~S3C_HSMMC_NIS_CMDCMP;
	s3c_hsmmc_writel(reg, S3C_HSMMC_NORINTSTSEN);

	host->dma_dir = (data->flags & MMC_DATA_READ)
				? DMA_FROM_DEVICE : DMA_TO_DEVICE;

#ifdef CONFIG_HSMMC_SCATTERGATHER
	host->sg_len = dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
				host->dma_dir);

	reg = s3c_hsmmc_readl(S3C_HSMMC_NORINTSTSEN);
	if (host->sg_len == 1) {
		reg &= ~S3C_HSMMC_NIS_DMA;
	} else {
		reg |= S3C_HSMMC_NIS_DMA;
	}
	s3c_hsmmc_writel(reg, S3C_HSMMC_NORINTSTSEN);
	
#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	reg8 = s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL);
	reg8 |= S3C_HSMMC_CTRL_ADMA2_32;
	s3c_hsmmc_writeb(reg8, S3C_HSMMC_HOSTCTL);
	DBG("HOSTCTL(0x28) = 0x%02x\n", s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL));
#endif
	DBG("data->flags(direction) =  0x%x\n", data->flags);
	DBG("data->blksz: %d\n", data->blksz);
	DBG("data->blocks: %d\n", data->blocks);
	DBG("data->sg_len: %d\n", data->sg_len);

	DBG("data->sg->addr: 0x%x\n", data->sg->dma_address);
	DBG("data->sg->length: 0x%x\n", data->sg->length);

	host->dma_blk = s3c_hsmmc_build_dma_table(host, data);
	host->next_blk = 0;
#else
	DBG("data->flags(direction) =  0x%x\n", data->flags);
	DBG("data->blksz: %d\n", data->blksz);
	DBG("data->blocks: %d\n", data->blocks);
	DBG("data->sg_len: %d\n", data->sg_len);
	dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
				host->dma_dir);
#endif
}

static inline void s3c_hsmmc_set_transfer_mode (struct s3c_hsmmc_host *host,
					struct mmc_data *data)
{
	u16 mode;

	mode = S3C_HSMMC_TRNS_DMA;

	if (data->stop)
		mode |= S3C_HSMMC_TRNS_ACMD12;

	if (data->flags & MMC_DATA_MULTI)
		mode |= S3C_HSMMC_TRNS_MULTI | S3C_HSMMC_TRNS_BLK_CNT_EN;
	if (data->flags & MMC_DATA_READ)
		mode |= S3C_HSMMC_TRNS_READ;

	s3c_hsmmc_writew(mode, S3C_HSMMC_TRNMOD);
}

static inline void s3c_hsmmc_send_register (struct s3c_hsmmc_host *host)
{
	struct mmc_command *cmd = host->cmd;
	struct mmc_data *data = cmd->data;

	u32 cmd_val;

	if (data) {
		
#ifdef CONFIG_HSMMC_SCATTERGATHER
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C2443)
		struct s3c_hsmmc_dma_blk *dblk;

		dblk = &host->dblk[0];
		s3c_hsmmc_writew(S3C_HSMMC_MAKE_BLKSZ(dblk->boundary, data->blksz),
								S3C_HSMMC_BLKSIZE);
		s3c_hsmmc_writel(dblk->dma_address, S3C_HSMMC_SYSAD);
		
#elif defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
		s3c_hsmmc_writew(S3C_HSMMC_MAKE_BLKSZ(0x7, data->blksz), S3C_HSMMC_BLKSIZE);
		s3c_hsmmc_writel(virt_to_phys(host->sdma_descr_tbl), S3C_HSMMC_ADMASYSADDR);
		DBG("S3C_HSMMC_ADMASYSADDR(0x58) = 0x%08x\n", s3c_hsmmc_readl(S3C_HSMMC_ADMASYSADDR));
#endif	

#else
		s3c_hsmmc_writew(S3C_HSMMC_MAKE_BLKSZ(0x7, data->blksz), S3C_HSMMC_BLKSIZE);
		s3c_hsmmc_writel(sg_dma_address(data->sg), S3C_HSMMC_SYSAD);
#endif
		s3c_hsmmc_writew(data->blocks, S3C_HSMMC_BLKCNT);
		s3c_hsmmc_set_transfer_mode(host, data);
	}

	s3c_hsmmc_writel(cmd->arg, S3C_HSMMC_ARGUMENT);

	cmd_val = (cmd->opcode << 8);
	if (cmd_val == (12<<8))
		cmd_val |= (3 << 6);

	if (cmd->flags & MMC_RSP_136)		/* Long RSP */
		cmd_val |= S3C_HSMMC_CMD_RESP_LONG;
	else if (cmd->flags & MMC_RSP_BUSY)	/* R1B */
		cmd_val |= S3C_HSMMC_CMD_RESP_SHORT_BUSY;
	else if (cmd->flags & MMC_RSP_PRESENT)	/* Normal RSP */
		cmd_val |= S3C_HSMMC_CMD_RESP_SHORT;

	if (cmd->flags & MMC_RSP_OPCODE)
		cmd_val |= S3C_HSMMC_CMD_INDEX;

	if (cmd->flags & MMC_RSP_CRC)
		cmd_val |= S3C_HSMMC_CMD_CRC;

	if (data)
		cmd_val |= S3C_HSMMC_CMD_DATA;

	s3c_hsmmc_writew(cmd_val, S3C_HSMMC_CMDREG);
}

static inline void s3c_hsmmc_send_command (struct s3c_hsmmc_host *host,
						struct mmc_command *cmd)
{
	u32 mask=1;
	ulong timeout;

#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	while (s3c_hsmmc_readl(S3C_HSMMC_CONTROL4) & mask);
#endif

	DBG("Sending cmd=(%d), arg=0x%x\n", cmd->opcode, cmd->arg);

	/* Wait max 10 ms */
	timeout = 10;

	mask = S3C_HSMMC_CMD_INHIBIT;
	if ((cmd->data != NULL) || (cmd->flags & MMC_RSP_BUSY))
		mask |= S3C_HSMMC_DATA_INHIBIT;

	while (s3c_hsmmc_readl(S3C_HSMMC_PRNSTS) & mask) {
		printk("########### waiting controller wakeup\n");
		if (timeout == 0) {
			printk(KERN_ERR "%s: Controller never released "
				"inhibit bit(s).\n", mmc_hostname(host->mmc));
			cmd->error = MMC_ERR_FAILED;

			/* tasklet_schedule(&host->finish_tasklet); */
			s3c_hsmmc_tasklet_finish((unsigned long) host);
			return;
		}
		timeout--;
		mdelay(1);
	}

	mod_timer(&host->timer, jiffies + 10 * HZ);

	host->cmd = cmd;

	s3c_hsmmc_prepare_data(host, cmd);
	s3c_hsmmc_send_register(host);
}

static void s3c_hsmmc_finish_data (struct s3c_hsmmc_host *host)
{
	struct mmc_data *data;
	u16 blocks;

	//BUG_ON(!host->data);
	if(!host->data) return;

	data = host->data;
	host->data = NULL;

	dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
			(data->flags & MMC_DATA_READ)
			? DMA_FROM_DEVICE : DMA_TO_DEVICE);

	/*
	 * Controller doesn't count down when in single block mode.
	 */
	if ((data->blocks == 1) && (data->error == MMC_ERR_NONE))
		blocks = 0;
	else {
		blocks = s3c_hsmmc_readw(S3C_HSMMC_BLKCNT);
	}
	data->bytes_xfered = data->blksz * (data->blocks - blocks);

	if ((data->error == MMC_ERR_NONE) && blocks) {
		printk(KERN_ERR "%s: Controller signalled completion even "
			"though there were blocks left. : %d\n",
			mmc_hostname(host->mmc), blocks);
		data->error = MMC_ERR_FAILED;
	}
	DBG("data->flags(direction) FINISHED =  %d\n", data->flags);
	DBG("data->blksz FINISHED =  %d\n", data->blksz);
	DBG("data->blocks FINISHED =  %d\n", data->blocks);
	DBG("Not FINISHED blocks =  %d\n", blocks);
	DBG("Ending data transfer (%d bytes)\n", data->bytes_xfered);
	DBG("\n");

	/* tasklet_schedule(&host->finish_tasklet); */
	s3c_hsmmc_tasklet_finish((unsigned long) host);
}

static void s3c_hsmmc_finish_command (struct s3c_hsmmc_host *host)
{
	int i;

	//BUG_ON(host->cmd == NULL);
	if(!host->cmd) return;

	if (host->cmd->flags & MMC_RSP_PRESENT) {
		if (host->cmd->flags & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i=0; i<4; i++) {
				host->cmd->resp[i] = s3c_hsmmc_readl(S3C_HSMMC_RSPREG0+ (3-i)*4) << 8;
				if (i != 3)
					host->cmd->resp[i] |= s3c_hsmmc_readb(S3C_HSMMC_RSPREG0 + (3-i)*4-1);
					DBG("cmd (%d) resp[%d] = 0x%x\n", host->cmd->opcode, i, host->cmd->resp[i]);
			}
		} else {
			host->cmd->resp[0] = s3c_hsmmc_readl(S3C_HSMMC_RSPREG0);
			DBG("cmd (%d) resp[%d] = 0x%x\n", host->cmd->opcode, 0, host->cmd->resp[0]);
		}
	}

	host->cmd->error = MMC_ERR_NONE;

	DBG("Ending cmd (%d)\n", host->cmd->opcode);
	DBG("\n");

	if (host->cmd->data)
		host->data = host->cmd->data;
	else
		/* tasklet_schedule(&host->finish_tasklet); */
		s3c_hsmmc_tasklet_finish((unsigned long) host);

	host->cmd = NULL;
}

static void s3c_hsmmc_tasklet_finish (unsigned long param)
{
	struct s3c_hsmmc_host *host;
	unsigned long iflags;
	struct mmc_request *mrq;

	host = (struct s3c_hsmmc_host*)param;

	//BUG_ON(!host->mrq);
	if(!host->mrq) return;

	spin_lock_irqsave(&host->lock, iflags);

	del_timer(&host->timer);

	mrq = host->mrq;
	udelay(10);
	/*
	 * The controller needs a reset of internal state machines
	 * upon error conditions.
	 */
	if ((mrq->cmd->error != MMC_ERR_NONE) ||
		(mrq->data && (mrq->data->error != MMC_ERR_NONE)) ) {
		s3c_hsmmc_reset(host, S3C_HSMMC_RESET_CMD);
		s3c_hsmmc_reset(host, S3C_HSMMC_RESET_DATA);
	}

	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;

	s3c_hsmmc_deactivate_led(host);

	mmiowb();
	spin_unlock_irqrestore(&host->lock, iflags);

	mmc_request_done(host->mmc, mrq);
}

/*****************************************************************************
 *                                                                           *
 * Interrupt handling                                                        *
 *                                                                           *
 *****************************************************************************/

static void s3c_hsmmc_cmd_irq (struct s3c_hsmmc_host *host, u32 intmask)
{
	if (!host->cmd) {
		printk(KERN_ERR "%s: Got command interrupt 0x%08x even "
			"though no command operation was in progress.\n",
			mmc_hostname(host->mmc), (unsigned)intmask);
		return;
	}

	if (intmask & S3C_HSMMC_INT_TIMEOUT)
		host->cmd->error = MMC_ERR_TIMEOUT;
	else if (intmask & S3C_HSMMC_INT_CRC)
		host->cmd->error = MMC_ERR_BADCRC;
	else if (intmask & (S3C_HSMMC_INT_END_BIT | S3C_HSMMC_INT_INDEX))
		host->cmd->error = MMC_ERR_FAILED;
	else
		host->cmd->error = MMC_ERR_INVALID;

	/* tasklet_schedule(&host->finish_tasklet); */
	s3c_hsmmc_tasklet_finish((unsigned long) host);
}

static void s3c_hsmmc_data_irq (struct s3c_hsmmc_host *host, u32 intmask)
{
	if (!host->data) {
		/*
		 * A data end interrupt is sent together with the response
		 * for the stop command.
		 */
		if (intmask & S3C_HSMMC_INT_DATA_END)
			return;

		printk(KERN_ERR "%s: Got data interrupt even though no "
			"data operation was in progress.\n",
			mmc_hostname(host->mmc));
		return;
	}

	if (intmask & S3C_HSMMC_INT_DATA_TIMEOUT)
		host->data->error = MMC_ERR_TIMEOUT;
	else if (intmask & S3C_HSMMC_INT_DATA_CRC)
		host->data->error = MMC_ERR_BADCRC;
	else if (intmask & S3C_HSMMC_INT_DATA_END_BIT)
		host->data->error = MMC_ERR_FAILED;

	if (host->data->error != MMC_ERR_NONE)
		s3c_hsmmc_finish_data(host);
}


/*****************************************************************************\
 *                                                                           *
 * Interrupt handling                                                        *
 *                                                                           *
\*****************************************************************************/

/*
 * ISR for SDI Interface IRQ
 * Communication between driver and ISR works as follows:
 *   host->mrq 			points to current request
 *   host->complete_what	tells the ISR when the request is considered done
 *
 * 1) Driver sets up host->mrq and host->complete_what
 * 2) Driver prepares the transfer
 * 3) Driver enables interrupts
 * 4) Driver starts transfer
 * 5) Driver waits for host->complete_rquest
 * 6) ISR checks for request status (errors and success)
 * 6) ISR sets host->mrq->cmd->error and host->mrq->data->error
 * 7) ISR completes host->complete_request
 * 8) ISR disables interrupts
 * 9) Driver wakes up and takes care of the request
 */

static irqreturn_t s3c_hsmmc_irq (int irq, void *dev_id)
{
	irqreturn_t result = 0;
	struct s3c_hsmmc_host *host = dev_id;
	struct mmc_request *mrq;
	u32 intsts;

#ifdef CONFIG_HSMMC_S3C_IRQ_WORKAROUND
	uint i, org_irq_sts;
#endif
	spin_lock(&host->lock);

	mrq = host->mrq;

	intsts = s3c_hsmmc_readw(S3C_HSMMC_NORINTSTS);

	/* Sometimes, hsmmc does not update its status bit immediately
	 * when it generates irqs. by scsuh.
	 */
#ifdef CONFIG_HSMMC_S3C_IRQ_WORKAROUND
	for (i=0; i<0x1000; i++) {
		if ((intsts = s3c_hsmmc_readw(S3C_HSMMC_NORINTSTS)))
			break;
	}
#endif

	if (unlikely(!intsts)) {
		result = IRQ_NONE;
		goto out;
	}
	intsts = s3c_hsmmc_readl(S3C_HSMMC_NORINTSTS);
#ifdef CONFIG_HSMMC_S3C_IRQ_WORKAROUND
	org_irq_sts = intsts;
#endif

	DBG(PFX "Got interrupt = 0x%08x\n", intsts);
 
	if (unlikely(intsts & S3C_HSMMC_INT_CARD_CHANGE)) {
		u32 reg16;

		if (intsts & S3C_HSMMC_INT_CARD_INSERT)
			printk(PFX "card inserted.\n");
		else if (intsts & S3C_HSMMC_INT_CARD_REMOVE)
			printk(PFX "card removed.\n");

		reg16 = s3c_hsmmc_readw(S3C_HSMMC_NORINTSTSEN);
		s3c_hsmmc_writew(reg16 & ~S3C_HSMMC_INT_CARD_CHANGE,
							S3C_HSMMC_NORINTSTSEN);
		s3c_hsmmc_writew(S3C_HSMMC_INT_CARD_CHANGE, S3C_HSMMC_NORINTSTS);
		s3c_hsmmc_writew(reg16, S3C_HSMMC_NORINTSTSEN);

		intsts &= ~S3C_HSMMC_INT_CARD_CHANGE;

		tasklet_schedule(&host->card_tasklet);
		goto insert;
	}
 
	if (likely(!(intsts & S3C_HSMMC_NIS_ERR))) {
		s3c_hsmmc_writel(intsts, S3C_HSMMC_NORINTSTS);

		if (intsts & S3C_HSMMC_NIS_CMDCMP) {
			DBG("command done\n");
			s3c_hsmmc_finish_command(host);
		}

		if (intsts & S3C_HSMMC_NIS_TRSCMP) {
			DBG("transfer done\n\n");
			s3c_hsmmc_finish_command(host);
			s3c_hsmmc_finish_data(host);
			intsts &= ~S3C_HSMMC_NIS_DMA;
		}

#ifdef CONFIG_HSMMC_SCATTERGATHER
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C2443)
		if (intsts & S3C_HSMMC_NIS_DMA) {
			struct s3c_hsmmc_dma_blk *dblk;

			dblk = &host->dblk[host->next_blk];
			if (dblk->original) {
				/* on read */
				if (host->dma_dir == DMA_FROM_DEVICE) {
					memcpy(dblk->original, phys_to_virt(dblk->dma_address), dblk->length);
				}
				dma_unmap_single(NULL, dblk->dma_address, dblk->length, host->dma_dir);
				dblk->original = 0;
			}

			host->next_blk++;
			dblk = &host->dblk[host->next_blk];

			if (host->next_blk == (host->dma_blk-1)) {
				u32 reg;

				reg = s3c_hsmmc_readl(S3C_HSMMC_NORINTSTSEN);
				reg &= ~S3C_HSMMC_NIS_DMA;
				s3c_hsmmc_writel(reg, S3C_HSMMC_NORINTSTSEN);
			}

			/* We do not handle DMA boundaries, so set it to max (512 KiB) */
			s3c_hsmmc_writew((dblk->boundary<<12 | 0x200), S3C_HSMMC_BLKSIZE);
			s3c_hsmmc_writel(dblk->dma_address, S3C_HSMMC_SYSAD);
		}
#endif
#endif
	} else {
		DBG("command FAIL : found bad irq [0x%8x]\n", intsts);
		DBG("\n");
		if (intsts & S3C_HSMMC_INT_CMD_MASK) {
			s3c_hsmmc_writel(intsts & S3C_HSMMC_INT_CMD_MASK, S3C_HSMMC_NORINTSTS);
			s3c_hsmmc_cmd_irq(host, intsts & S3C_HSMMC_INT_CMD_MASK);
		}

		if (intsts & S3C_HSMMC_INT_DATA_MASK) {
			s3c_hsmmc_writel(intsts & S3C_HSMMC_INT_DATA_MASK, S3C_HSMMC_NORINTSTS);
			s3c_hsmmc_finish_command(host);
			s3c_hsmmc_data_irq(host, intsts & S3C_HSMMC_INT_DATA_MASK);
		}

		intsts &= ~(S3C_HSMMC_INT_CMD_MASK | S3C_HSMMC_INT_DATA_MASK);
	}

	/* XXX: fix later by scsuh */
#if 0
	if (intsts & S3C_HSMMC_INT_BUS_POWER) {
		printk(KERN_ERR "%s: Card is consuming too much power!\n",
			mmc_hostname(host->mmc));
		s3c_hsmmc_writel(S3C_HSMMC_INT_BUS_POWER, S3C_HSMMC_NORINTSTS);
	}

	intsts &= S3C_HSMMC_INT_BUS_POWER;

	if (intsts) {
		printk(KERN_ERR "%s: Unexpected interrupt 0x%08x.\n",
			mmc_hostname(host->mmc), intsts);

		s3c_hsmmc_writel(intsts, S3C_HSMMC_NORINTSTS);
	}
#endif

#ifdef CONFIG_HSMMC_S3C_IRQ_WORKAROUND
	for (i=0; i<0x1000; i++) {
		if (org_irq_sts != s3c_hsmmc_readl(S3C_HSMMC_NORINTSTS))
			break;
	}
#endif

insert:
	result = IRQ_HANDLED;
	mmiowb();

out:
	spin_unlock(&host->lock);

	return result;
}

static irqreturn_t s3c_hsmmc_irq_cd (int irq, void *dev_id)
{
	struct s3c_hsmmc_host *host = dev_id;
	int ext_CD_int = 0;
	
#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	ext_CD_int = readl(S3C2410_GPFDAT);
	ext_CD_int &= 0x2;	/* GPF1 */
#elif defined(CONFIG_CPU_S3C6410)
	ext_CD_int = readl(S3C_GPNDAT);
	ext_CD_int &= 0x2000;	/* GPN13 */
#endif

	if(ext_CD_int && card_detect) {
		printk("s3c-hsmmc channel-0(EXT): card removed.\n");
		set_irq_type(host->irq_cd, IRQT_FALLING);
		card_detect = 0;
	}
	else if(!ext_CD_int && !card_detect) {
		printk("s3c-hsmmc channel-0(EXT): card inserted.\n");
		set_irq_type(host->irq_cd, IRQT_RISING);
		card_detect = 1;
	}
	else
		return IRQ_HANDLED;


	tasklet_schedule(&host->card_tasklet);
	mmiowb();

	spin_unlock(&host->lock);

	return IRQ_HANDLED;
}

static void s3c_hsmmc_check_status (unsigned long data)
{
        struct s3c_hsmmc_host *host = (struct s3c_hsmmc_host *)data;

	s3c_hsmmc_irq(0, host);
}

static void s3c_hsmmc_request (struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct s3c_hsmmc_host *host = mmc_priv(mmc);
	unsigned long flags;

	DBG("hsmmc request: [CMD] opcode:%d arg:0x%08x flags:0x%02x retries:%u\n",
		mrq->cmd->opcode, mrq->cmd->arg, mrq->cmd->flags, mrq->cmd->retries);

	spin_lock_irqsave(&host->lock, flags);

	WARN_ON(host->mrq != NULL);

	s3c_hsmmc_activate_led(host);

	host->mrq = mrq;

	if ((s3c_hsmmc_readl(S3C_HSMMC_PRNSTS) & S3C_HSMMC_CARD_PRESENT) || card_detect) {
		s3c_hsmmc_send_command(host, mrq->cmd);
	} else {
		host->mrq->cmd->error = MMC_ERR_TIMEOUT;

		/* tasklet_schedule(&host->finish_tasklet); */
		s3c_hsmmc_tasklet_finish((unsigned long) host);
	}


	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}

/* return 0: OK
 * return -1: error
 */
static int s3c_set_bus_width (struct s3c_hsmmc_host *host, uint width)
{
	u8 reg;

	reg = s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL);

	switch (width) {
	case MMC_BUS_WIDTH_1:
		reg &= ~(S3C_HSMMC_CTRL_4BIT | S3C_HSMMC_CTRL_8BIT);
		DBG("bus width: 1 bit\n");
		break;
		
	case MMC_BUS_WIDTH_4:
		DBG("bus width: 4 bit\n");
		reg &= ~(S3C_HSMMC_CTRL_8BIT);
		reg |= S3C_HSMMC_CTRL_4BIT;
		break;
		
	case MMC_BUS_WIDTH_8:
		reg |= S3C_HSMMC_CTRL_8BIT;
		DBG("bus width: 8 bit\n");
		break;
		
	default:
		DBG("bus width: Error\n");
		return -1;
	}

	s3c_hsmmc_writeb(reg, S3C_HSMMC_HOSTCTL);

	DBG("HOSTCTL(0x28) = 0x%02x\n", s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL));

	return 0;
}

static void s3c_hsmmc_set_clock (struct s3c_hsmmc_host *host, ulong clock)
{
	struct s3c_hsmmc_cfg *cfg = host->plat_data;
	int cardtype = host->mmc->mode;
	u32 val = 0, tmp_clk = 0, clk_src = 0, i, j;
	u16 div = (u16)-1;
	ulong timeout;	
	u8 ctrl;

	/* if we already set, just out. */
	if (clock == host->clock) {
		printk("%p:host->clock0 : %d Hz\n", host->base, host->clock);
		return;
	}

	/* before setting clock, clkcon must be disabled. */
	s3c_hsmmc_writew(0, S3C_HSMMC_CLKCON);

	s3c_hsmmc_writeb(S3C_HSMMC_TIMEOUT_MAX, S3C_HSMMC_TIMEOUTCON);

	/* change the edge type according to frequency */
	ctrl = s3c_hsmmc_readb(S3C_HSMMC_HOSTCTL);

	if (cfg->highspeed)
		ctrl |= S3C_HSMMC_CTRL_HIGHSPEED;
	else
		ctrl &= ~S3C_HSMMC_CTRL_HIGHSPEED;
	
	s3c_hsmmc_writeb(ctrl, S3C_HSMMC_HOSTCTL);

	if (clock == 0) {
		DBG(" In case of 0 Hz of clock, I'm afraid DO NOTHING ..\n");
		return;
	}

	/* calculate optimal clock. by scsuh */
	DBG("Requested clock is  %ld Hz\n", clock);

	for (i=0; i<NUM_OF_HSMMC_CLKSOURCES; i++) {
		if ((tmp_clk = clk_get_rate(host->clk[i])) <= clock) {
			if (tmp_clk >= val) {
				val = tmp_clk;
				div = 0;
				clk_src = i+1;
			}
		}

		for (j=0x1; j<=0x80; j <<= 1) {
			tmp_clk = clk_get_rate(host->clk[i]) / (j<<1);
			if ((val < tmp_clk) && (tmp_clk <= clock)) {
				val = tmp_clk;
				div = j;
				clk_src = i+1;
				break;
			}
		}
		DBG("   tmp_val[%d]: %d\n", i, val);
	}

	DBG("Optimal clock : %d Hz, div: 0x%x, SelBaseclk_src: %d\n", val, div, clk_src);

	/* CONTROL2 */
	s3c_hsmmc_writel(cfg->fd_ctrl[cardtype].ctrl2 | (clk_src << 4), S3C_HSMMC_CONTROL2);

	/* CONTROL3 */
	if (clock > 25000000)
		s3c_hsmmc_writel(cfg->fd_ctrl[cardtype].ctrl3[SPEED_HIGH], S3C_HSMMC_CONTROL3);
	else
		s3c_hsmmc_writel(cfg->fd_ctrl[cardtype].ctrl3[SPEED_NORMAL], S3C_HSMMC_CONTROL3);

#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	s3c_hsmmc_writel(cfg->fd_ctrl[cardtype].ctrl4 << 16, S3C_HSMMC_CONTROL4);
#endif

	s3c_hsmmc_writew(((div << 8) | S3C_HSMMC_CLOCK_INT_EN), S3C_HSMMC_CLKCON);

	timeout = 10;
	while (!((val = s3c_hsmmc_readw(S3C_HSMMC_CLKCON))
				& S3C_HSMMC_CLOCK_INT_STABLE)) {
		if (!timeout) {
			printk("Error in INTERNAL clock stabilization: %08x\n", val);
			return;
		}
		timeout--;
		mdelay(1);
	}

	s3c_hsmmc_writew(val | S3C_HSMMC_CLOCK_CARD_EN, S3C_HSMMC_CLKCON);

	timeout = 10;
	while (!((val = s3c_hsmmc_readw(S3C_HSMMC_CLKCON))
				& S3C_HSMMC_CLOCK_EXT_STABLE)) {
		if (!timeout) {
			printk("Error in EXTERNAL clock stabilization: %08x\n", val);
			return;
		}
		timeout--;
		mdelay(1);
	}
}


static void s3c_hsmmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct s3c_hsmmc_host *host = mmc_priv(mmc);

	unsigned long iflags;

	spin_lock_irqsave(&host->lock, iflags);

	/*
	 * Reset the chip on each power off.
	 * Should clear out any weird states.
	 */
	if (ios->power_mode == MMC_POWER_OFF) {
		s3c_hsmmc_writew(0, S3C_HSMMC_NORINTSIGEN);
		s3c_hsmmc_ios_init(host);
	}

	if (host->plat_data->enabled)
		hsmmc_set_gpio(host->plat_data->hwport, host->plat_data->bus_width);

	DBG("\nios->clock: %d Hz\n", ios->clock);
	s3c_hsmmc_set_clock(host, ios->clock);

	DBG("\nios->bus_width: %d\n", ios->bus_width);
	s3c_set_bus_width(host, ios->bus_width);

	DBG("S3C_HSMMC_CONTROL2(0x80) = 0x%08x\n", s3c_hsmmc_readl(S3C_HSMMC_CONTROL2));
	DBG("S3C_HSMMC_CONTROL3(0x84) = 0x%08x\n", s3c_hsmmc_readl(S3C_HSMMC_CONTROL3));
	DBG("S3C_HSMMC_CLKCON  (0x2c) = 0x%04x\n", s3c_hsmmc_readw(S3C_HSMMC_CLKCON));

	if (ios->power_mode == MMC_POWER_OFF)
		s3c_hsmmc_writeb(S3C_HSMMC_POWER_OFF, S3C_HSMMC_PWRCON);
	else
		s3c_hsmmc_writeb(S3C_HSMMC_POWER_ON_ALL, S3C_HSMMC_PWRCON);

	udelay(1000);
	spin_unlock_irqrestore(&host->lock, iflags);
}

static struct mmc_host_ops s3c_hsmmc_ops = {
	.request	= s3c_hsmmc_request,
	.set_ios	= s3c_hsmmc_set_ios,
};

static int s3c_hsmmc_probe (struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct s3c_hsmmc_host *host;
	struct s3c_hsmmc_cfg *plat_data;
	uint i;
	int ret;

	mmc = mmc_alloc_host(sizeof(struct s3c_hsmmc_host), &pdev->dev);
	if (!mmc)
		ret = -ENOMEM;

	plat_data = s3c_hsmmc_get_platdata(&pdev->dev);

	host = mmc_priv(mmc);

	host->mmc = mmc;

	host->plat_data = plat_data;

	host->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!host->mem) {
		printk("Failed to get io memory region resouce.\n");
		ret = -ENOENT;
		goto probe_free_host;
	}

	host->mem = request_mem_region(host->mem->start,
		RESSIZE(host->mem), pdev->name);
	if (!host->mem) {
		printk("Failed to request io memory region.\n");
		ret = -ENOENT;
		goto probe_free_host;
	}

	host->base = ioremap(host->mem->start, (host->mem->end - host->mem->start)+1);
	if (host->base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		return -ENOMEM;
	}

	host->irq = platform_get_irq(pdev, 0);
	if (host->irq == 0) {
		printk("Failed to get interrupt resouce.\n");
		ret = -EINVAL;
		goto untasklet;
	}

	/* To detect a card inserted on channel 0,  an external interrupt is used. */
	if ((plat_data->enabled == 1) && (plat_data->hwport == 0)) {
		host->irq_cd = platform_get_irq(pdev, 1);
		if (host->irq_cd == 0) {
			printk("Failed to get interrupt resouce.\n");
			ret = -EINVAL;
			goto untasklet;
		}
#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
		set_irq_type(host->irq_cd, IRQT_BOTHEDGE);
#elif defined(CONFIG_CPU_S3C6410) 
		set_irq_type(host->irq_cd, IRQT_LOW);
#endif
	}

	host->flags |= S3C_HSMMC_USE_DMA;

#ifdef CONFIG_HSMMC_SCATTERGATHER
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C2443)
	for (i=0; i<S3C_HSMMC_MAX_SUB_BUF; i++)
		host->sub_block[i] = kmalloc(S3C_HSMMC_MALLOC_SIZE, GFP_KERNEL);
#endif
#endif

	s3c_hsmmc_reset(host, S3C_HSMMC_RESET_ALL);

	/* register some clock sources if exist */
	for (i=0; i<NUM_OF_HSMMC_CLKSOURCES; i++) {
		host->clk[i] = clk_get(&pdev->dev, plat_data->clk_name[i]);
		if (IS_ERR(host->clk[i])) {
			ret = PTR_ERR(host->clk[i]);
			host->clk[i] = ERR_PTR(-ENOENT);
		}

		if (clk_enable(host->clk[i])) {
			host->clk[i] = ERR_PTR(-ENOENT);
		}
		if (!IS_ERR(host->clk[i])) {
			DBG("MMC clock source[%d], %s is %ld Hz\n",i, plat_data->clk_name[i], clk_get_rate(host->clk[i]));
		}
	}

	mmc->ops = &s3c_hsmmc_ops;
	mmc->ocr_avail = MMC_VDD_32_33|MMC_VDD_33_34;
	mmc->f_min = 400 * 1000; /* at least 400kHz */

	/* you must make sure that our hsmmc block can support
	 * up to 52MHz. by scsuh
	 */
	mmc->f_max = 100 * MHZ;
	mmc->caps = plat_data->host_caps;
	DBG("mmc->caps: %08x\n", mmc->caps);

	spin_lock_init(&host->lock);

	/*
	 * Maximum number of segments. Hardware cannot do scatter lists.
	 * XXX: must modify later. by scsuh
	 */
#ifdef CONFIG_HSMMC_SCATTERGATHER
	mmc->max_hw_segs = CONFIG_S3C_HSMMC_MAX_HW_SEGS;
	mmc->max_phys_segs = CONFIG_S3C_HSMMC_MAX_HW_SEGS;
#else
	mmc->max_hw_segs = 1;
#endif

	/*
	 * Maximum number of sectors in one transfer. Limited by DMA boundary
	 * size (512KiB), which means (512 KiB/512=) 1024 entries.
	 */
	mmc->max_sectors = 128;	/* 65535/512=128 */

	/*
	 * Maximum segment size. Could be one segment with the maximum number
	 * of sectors.
	 */
	mmc->max_blk_size = 512;
	mmc->max_seg_size = mmc->max_sectors * mmc->max_blk_size;
	
	mmc->max_blk_count = mmc->max_sectors;
	mmc->max_req_size = mmc->max_seg_size;
	

	init_timer(&host->timer);
        host->timer.data = (unsigned long)host;
        host->timer.function = s3c_hsmmc_check_status;
        host->timer.expires = jiffies + HZ;

	/*
	 * Init tasklets.
	 */
	tasklet_init(&host->card_tasklet,
		s3c_hsmmc_tasklet_card, (unsigned long)host);

#if 0
	tasklet_init(&host->finish_tasklet,
		s3c_hsmmc_tasklet_finish, (unsigned long)host);
#endif

	ret = request_irq(host->irq, s3c_hsmmc_irq, 0, DRIVER_NAME, host);
	if (ret)
		goto untasklet;

#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
	if ((plat_data->enabled == 1) && (plat_data->hwport == 0)) {
		ret = request_irq(host->irq_cd, s3c_hsmmc_irq_cd, 0, DRIVER_NAME, host);
		if (ret)
			goto untasklet;
	}
#endif

	s3c_hsmmc_ios_init(host);

	mmc_add_host(mmc);

#if defined(CONFIG_PM)
	global_host[plat_data->hwport] = host;
#endif

	printk(KERN_INFO "[s3c_hsmmc_probe]: %s.%d: at 0x%p with irq %d. clk src:",
			pdev->name, pdev->id, host->base, host->irq);

	for (i=0; i<NUM_OF_HSMMC_CLKSOURCES; i++) {
		if (!IS_ERR(host->clk[i]))
			printk(" %s", host->clk[i]->name);
	}
	printk("\n");

	return 0;

untasklet:
	tasklet_kill(&host->card_tasklet);
	/* tasklet_kill(&host->finish_tasklet); */

	for (i=0; i<NUM_OF_HSMMC_CLKSOURCES; i++) {
		if (host->clk[i] != ERR_PTR(-ENOENT)) {
			clk_disable(host->clk[i]);
			clk_put(host->clk[i]);
		}
	}

probe_free_host:
	mmc_free_host(mmc);

	return ret;
}

static int s3c_hsmmc_remove(struct platform_device *dev)
{
	struct mmc_host *mmc  = platform_get_drvdata(dev);
	struct s3c_hsmmc_host *host = mmc_priv(mmc);
	int i;

	mmc = host->mmc;

	mmc_remove_host(mmc);

	s3c_hsmmc_reset(host, S3C_HSMMC_RESET_ALL);

	for (i=0; i<NUM_OF_HSMMC_CLKSOURCES; i++) {
		clk_disable(host->clk[i]);
		clk_put(host->clk[i]);
	}

	free_irq(host->irq, host);

	del_timer_sync(&host->timer);

	tasklet_kill(&host->card_tasklet);
	/* tasklet_kill(&host->finish_tasklet); */

#ifdef CONFIG_HSMMC_SCATTERGATHER
#if defined(CONFIG_CPU_S3C6400) || defined(CONFIG_CPU_S3C2443)
	kfree(host->sub_block);
#endif
#endif
	mmc_free_host(mmc);

	return 0;
}

#ifdef CONFIG_PM
static int s3c_hsmmc_suspend(struct platform_device *pdev, pm_message_t state)
{
//	struct mmc_card *card;
	struct s3c_hsmmc_host *s3c_host = global_host[pdev->id];
	struct mmc_host *host = s3c_host->mmc;

#if 0
	/*
	 * Fix me later by jsgood
	 */
	list_for_each_entry(card, &host->cards, node) {
		if (card->state & MMC_STATE_PRESENT)
			card->state &= ~MMC_STATE_PRESENT;
	}
#endif
	mmc_suspend_host(host, state);

	return 0;
}

static int s3c_hsmmc_resume(struct platform_device *pdev)
{
	struct s3c_hsmmc_host *s3c_host = global_host[pdev->id];
	struct mmc_host *host = s3c_host->mmc;

	s3c_hsmmc_ios_init(s3c_host);
	mmc_resume_host(host);

	return 0;
}
#else
#define s3c_hsmmc_suspend NULL
#define s3c_hsmmc_resume NULL
#endif


static struct platform_driver s3c_hsmmc_driver =
{
        .probe          = s3c_hsmmc_probe,
        .remove         = s3c_hsmmc_remove,
        .suspend 	= s3c_hsmmc_suspend,
	.resume		= s3c_hsmmc_resume,
	.driver		= {
		.name	= "s3c-hsmmc",
		.owner	= THIS_MODULE,
	},
};

static int __init s3c_hsmmc_drv_init(void)
{
	return platform_driver_register(&s3c_hsmmc_driver);
}

static void __exit s3c_hsmmc_drv_exit(void)
{
	platform_driver_unregister(&s3c_hsmmc_driver);
}


module_init(s3c_hsmmc_drv_init);
module_exit(s3c_hsmmc_drv_exit);


MODULE_DESCRIPTION("S3C SD HOST I/F 1.0 and 2.0 Driver");
MODULE_LICENSE("GPL");
