/*
 *  s3c24xx-pcm.h --
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  ALSA PCM interface for the Samsung S3C24xx CPU
 */

#ifndef _S3C24XX_PCM_H
#define _S3C24XX_PCM_H

#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)

struct s3c24xx_pcm_dma_params {
	struct s3c2410_dma_client *client;	/* stream identifier */
	int channel;				/* Channel ID */
	dma_addr_t dma_addr;
	int dma_size;			/* Size of the DMA transfer */
#if defined (CONFIG_CPU_S3C6400) || defined (CONFIG_CPU_S3C6410) 
	int dma_num;			/* Number of the DMA */
#endif
};

#define S3C24XX_DAI_I2S			0

/* platform data */
extern struct snd_ac97_bus_ops s3c24xx_ac97_ops;

int get_iis_clk(struct device *dev);
void put_iis_clk(void);

extern const char s3c24xx_platform_id[];
extern const char s3c_i2s_id[];
#endif
