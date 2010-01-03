/*
 * linux/sound/arm/pxa2xx-pcm.h -- ALSA PCM interface for the Intel PXA2xx chip
 *
 * Author:	Nicolas Pitre
 * Created:	Nov 30, 2004
 * Copyright:	MontaVista Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _PXA2XX_PCM_H
#define _PXA2XX_PCM_H

struct pxa2xx_pcm_dma_params {
	char *name;			/* stream identifier */
	u32 dcmd;			/* DMA descriptor dcmd field */
	volatile u32 *drcmr;		/* the DMA request channel to use */
	u32 dev_addr;			/* device physical address for DMA */
};

struct pxa2xx_gpio {
	u32 sys;
	u32	rx;
	u32 tx;
	u32 clk;
	u32 frm;
};

/* PXA audio platform IDs */
extern const char pxa_platform_id[];
extern const char pxa2xx_i2s_dai_id[];
extern const char pxa_ac97_hifi_dai_id[];
extern const char pxa_ac97_aux_dai_id[];
extern const char pxa_ac97_mic_dai_id[];

#endif
