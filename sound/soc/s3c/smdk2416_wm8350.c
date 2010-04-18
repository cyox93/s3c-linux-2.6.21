/*
 * smdk6400_wm8753.c  --  SoC audio for Neo1973
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    20th Jan 2007   Initial version.
 *    05th Feb 2007   Rename all to Neo1973
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <asm/arch/regs-iis.h>

#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/arch/audio.h>
#include <asm/io.h>
#include <asm/arch/spi-gpio.h>
#include <asm/arch/regs-s3c2450-clock.h>

#include <linux/regulator/consumer.h>
#include <linux/mfd/wm8350/core.h>
#include <linux/mfd/wm8350/audio.h> 

#include "s3c-pcm.h"
#include "s3c-i2s.h"

/* define the scenarios */
#define SMDK6400_AUDIO_OFF		0
#define SMDK6400_CAPTURE_MIC1		3
#define SMDK6400_STEREO_TO_HEADPHONES	2
#define SMDK6400_CAPTURE_LINE_IN	1

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

struct smdk2416_pcm_state {
	int lr_clk_active;
	int playback_active;
	int capture_active;
};

struct smdk2416_data {
	struct regulator *analog_supply;
	struct wm8350 *wm8350;
};

struct _wm8350_audio {
	unsigned int channels;
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int sysclk;
	unsigned int bclkdiv;
	unsigned int clkdiv;
	unsigned int lr_rate;
	unsigned int prescaler;
};

/* in order of power consumption per rate (lowest first) */
static const struct _wm8350_audio wm8350_audio[] = {
	/* 16 bit stereo modes */
	{2, SNDRV_PCM_FORMAT_S16_LE, 8000, 12288000,
	 WM8350_BCLK_DIV_48, WM8350_DACDIV_3, 32, 24,},
	{2, SNDRV_PCM_FORMAT_S16_LE, 16000, 12288000,
	 WM8350_BCLK_DIV_24, WM8350_DACDIV_3, 32, 12,},
	{2, SNDRV_PCM_FORMAT_S16_LE, 32000, 12288000,
	 WM8350_BCLK_DIV_12, WM8350_DACDIV_3, 32, 6,},
#if 0
	{2, SNDRV_PCM_FORMAT_S16_LE, 48000, 12288000,
	 WM8350_BCLK_DIV_8, WM8350_DACDIV_3, 32, 4,},
	{2, SNDRV_PCM_FORMAT_S16_LE, 96000, 24576000,
	 WM8350_BCLK_DIV_8, WM8350_DACDIV_1, 32, 2,},
	{2, SNDRV_PCM_FORMAT_S16_LE, 11025, 11289600,
	 WM8350_BCLK_DIV_32, WM8350_DACDIV_4, 32, 32,},
	{2, SNDRV_PCM_FORMAT_S16_LE, 22050, 11289600,
	 WM8350_BCLK_DIV_16, WM8350_DACDIV_2, 32, 16,},
#endif
	{2, SNDRV_PCM_FORMAT_S16_LE, 44100, 11289600,
	 WM8350_BCLK_DIV_8, WM8350_DACDIV_3, 32, 8,},
#if 0
	{2, SNDRV_PCM_FORMAT_S16_LE, 88200, 22579200,
	 WM8350_BCLK_DIV_8, WM8350_DACDIV_1, 32, 4,},

	/* 24bit stereo modes */
	{2, SNDRV_PCM_FORMAT_S24_LE, 48000, 12288000,
	 WM8350_BCLK_DIV_4, WM8350_DACDIV_1, 64, 4,},
	{2, SNDRV_PCM_FORMAT_S24_LE, 96000, 24576000,
	 WM8350_BCLK_DIV_4, WM8350_DACDIV_1, 64, 2,},
	{2, SNDRV_PCM_FORMAT_S24_LE, 44100, 11289600,
	 WM8350_BCLK_DIV_4, WM8350_DACDIV_1, 64, 8,},
	{2, SNDRV_PCM_FORMAT_S24_LE, 88200, 22579200,
	 WM8350_BCLK_DIV_4, WM8350_DACDIV_1, 64, 4,},
#endif
};

static int smdk2416_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct smdk2416_pcm_state *state = rtd->private_data;
	int i, found = 0;
	int ret = 0;
	unsigned int regs;
	snd_pcm_format_t format = params_format(params);
	unsigned int rate = params_rate(params);
	unsigned int channels = params_channels(params);

	s3cdbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));

	/* only need to do this once as capture and playback are sync */
	if (state->lr_clk_active > 1)
		return 0;

	/* Select Clock source EPLL */
//	regs = ioremap(S3C2410_PA_CLKPWR,0x20);
	regs = readl(S3C2443_CLKSRC);
	regs &= ~(3<<12);
	regs &= ~(3<<14);
	regs |= S3C2450_CLKSRC_I2S1_EPLL;
	regs = (regs & ~(3<<7))|(2<<7);
	writel(regs, S3C2443_CLKSRC);
	regs |= (1<<6);
	writel(regs, S3C2443_CLKSRC);

	regs = readl(S3C2443_SCLKCON);
	regs |= S3C2443_SCLKCON_I2SCLK_1;
	writel(regs, S3C2443_SCLKCON);

	s3cdbg("%s: %d , params = %d \n", __FUNCTION__, __LINE__, params_rate(params));

	switch (params_rate(params)) {
	case 8000:
	case 16000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		writel(9962, S3C2450_EPLLCON_K);
		writel((49<<16)|(1<<8)|(3<<0) ,S3C2443_EPLLCON);
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		writel(10381, S3C2450_EPLLCON_K);
		writel((45<<16)|(1<<8)|(2<<0) ,S3C2443_EPLLCON);
		break;
	default:
		printk("Unsupported rate = %d\n", params_rate(params));
		break;
	}

	for (i=0; i <ARRAY_SIZE(wm8350_audio); i++) {
		if (rate == wm8350_audio[i].rate &&
			format == wm8350_audio[i].format &&
			channels == wm8350_audio[i].channels) {
			found = 1;
			break;
		}
	}
	if (!found)
		return -EINVAL;

	/* codec FLL input is rate from DAC LRC */
	snd_soc_dai_set_pll(codec_dai, 0, rate, wm8350_audio[i].sysclk);

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, 
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_SYNC); 
	if (ret < 0)
		return ret;

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_SYNC); 
	if (ret < 0)
		return ret;

	/* set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8350_MCLK_SEL_MCLK, 
		wm8350_audio[i].sysclk, SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	/* set MCLK division for sample rate */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C24XX_DIV_MCLK,
		S3C2410_IISMOD_32FS);
	if (ret < 0)
		return ret;

	/* set codec BCLK division for sample rate */
	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8350_BCLK_CLKDIV,
						wm8350_audio[i].bclkdiv);
	if (ret < 0)
		return ret;

	/* set prescaler division for sample rate */
	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER,
		((wm8350_audio[i].prescaler/2 -1) << 0x8));

	/* DAI is synchronous and clocked with DAC LRCLK & ADC LRC */
	ret = snd_soc_dai_set_clkdiv(codec_dai,
			       WM8350_DACLR_CLKDIV, wm8350_audio[i].lr_rate);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(codec_dai,
			       WM8350_ADCLR_CLKDIV, wm8350_audio[i].lr_rate);
	if (ret < 0)
		return ret;

	/* now configure DAC and ADC clocks */
	ret = snd_soc_dai_set_clkdiv(codec_dai,
			       WM8350_DAC_CLKDIV, wm8350_audio[i].clkdiv);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(codec_dai,
			       WM8350_ADC_CLKDIV, wm8350_audio[i].clkdiv);
	if (ret < 0)
		return ret;

#if 0
	/* set the codec register set for capture and play */
	ret = codec_dai->dai_ops.set_tdm_slot(codec_dai, substream->stream, 2);
	if (ret < 0)
		return ret;
#endif

	return 0;
}

static void smdk2416_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *pcm_runtime = substream->private_data;
	struct snd_soc_dai *codec_dai = pcm_runtime->codec_dai;
	struct snd_soc_codec *codec = pcm_runtime->codec;
	struct smdk2416_pcm_state *state = pcm_runtime->private_data;
	struct wm8350 *wm8350 = codec->control_data;

	/* disable the PLL if there are no active Tx or Rx channels */
	if (!codec->active)
		snd_soc_dai_set_pll(codec_dai, 0, 0, 0);
	state->lr_clk_active--;

	/*
	 * We need to keep track of active streams in master mode and
	 * switch LRC source if necessary.
	 */

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		state->capture_active = 0;
	else
		state->playback_active = 0;

	if (state->capture_active)
		wm8350_set_bits(wm8350, WM8350_CLOCK_CONTROL_2,
				WM8350_LRC_ADC_SEL);
	else if (state->playback_active)
		wm8350_clear_bits(wm8350, WM8350_CLOCK_CONTROL_2,
				  WM8350_LRC_ADC_SEL);
}


#if 0
static int smdk2416_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;

	/* disable the PLL */
	return snd_soc_dai_set_pll(codec_dai, 0, 0, 0);
}
#endif

/*
 * smdk2416 WM8350 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk2416_hifi_ops = {
	.hw_params = smdk2416_hifi_hw_params,
	.shutdown  = smdk2416_shutdown,
};

/* need to refine these */
static struct wm8350_audio_platform_data smdk2416_wm8350_setup = {
	.vmid_discharge_msecs = 0,
	.drain_msecs = 0,
	.cap_discharge_msecs = 0,
	.vmid_charge_msecs = 0,
	.vmid_s_curve = WM8350_S_CURVE_SLOW,
	.dis_out4 = WM8350_DISCHARGE_SLOW,
	.dis_out3 = WM8350_DISCHARGE_SLOW,
	.dis_out2 = WM8350_DISCHARGE_SLOW,
	.dis_out1 = WM8350_DISCHARGE_SLOW,
	.vroi_out4 = WM8350_TIE_OFF_500R,
	.vroi_out3 = WM8350_TIE_OFF_500R,
	.vroi_out2 = WM8350_TIE_OFF_500R,
	.vroi_out1 = WM8350_TIE_OFF_500R,
	.vroi_enable = 0,
	.codec_current_on = WM8350_CODEC_ISEL_1_0,
	.codec_current_standby = WM8350_CODEC_ISEL_0_5,
	.codec_current_charge = WM8350_CODEC_ISEL_1_5,
};

/* imx32ads soc_card dapm widgets */
static const struct snd_soc_dapm_widget smdk2416_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("SiMIC", NULL),
	SND_SOC_DAPM_MIC("Mic1 Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
	SND_SOC_DAPM_LINE("Line Out Jack", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

/* imx32ads soc_card audio map */
static const struct snd_soc_dapm_route audio_map[] = {
	/* SiMIC --> IN1LN (with automatic bias) via SP1 */
	{"IN1LN", NULL, "Mic Bias"},
	{"Mic Bias", NULL, "SiMIC"},
	
	/* Mic 1 Jack --> IN1RN and IN1RP (with automatic bias) */
	{"IN1RN", NULL, "Mic Bias"},
	{"IN1RP", NULL, "Mic1 Jack"},
	{"Mic Bias", NULL, "Mic1 Jack"},

	/* Line in Jack --> AUX (L+R) */
	{"IN3R", NULL, "Line In Jack"},
	{"IN3L", NULL, "Line In Jack"},

	/* Out2 --> Headphone Jack */
	{"Headphone Jack", NULL, "OUT2R"},
	{"Headphone Jack", NULL, "OUT2L"},

	/* Out1 --> Line Out Jack */
	{"Line Out Jack", NULL, "OUT1R"},
	{"Line Out Jack", NULL, "OUT1L"},
};

#ifdef CONFIG_PM
static int smdk2416_wm8350_audio_suspend(struct platform_device *pdev,
					 pm_message_t state)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct snd_soc_card *soc_card = wm8350->audio;

	return snd_soc_card_suspend_pcms(soc_card, state);
}

static int smdk2416_wm8350_audio_resume(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct snd_soc_card *soc_card = wm8350->audio;

	return snd_soc_card_resume_pcms(soc_card);
}

#else
#define smdk2416_wm8350_audio_suspend	NULL
#define smdk2416_wm8350_audio_resume	NULL
#endif

static void smdk2416_jack_handler(struct wm8350 *wm8350, int irq, void *data)
{
	struct snd_soc_card *soc_card = (struct snd_soc_card *)data;
	u16 reg;

	/* debounce for 200ms */
	schedule_timeout_interruptible(msecs_to_jiffies(200));
	reg = wm8350_reg_read(wm8350, WM8350_JACK_PIN_STATUS);

	if (reg & WM8350_JACK_R_LVL) {
		snd_soc_dapm_disable_pin(soc_card, "Line Out Jack");
		snd_soc_dapm_enable_pin(soc_card, "Headphone Jack");
	} else {
		snd_soc_dapm_disable_pin(soc_card, "Headphone Jack");
		snd_soc_dapm_enable_pin(soc_card, "Line Out Jack");
	}
	snd_soc_dapm_sync(soc_card);
}

int smdk2416_audio_init(struct snd_soc_card *soc_card)
{
	struct snd_soc_codec *codec;
	struct snd_soc_pcm_runtime *pcm_runtime;
	struct smdk2416_data *audio_data = soc_card->private_data;
	struct wm8350 *wm8350 = audio_data->wm8350;
	struct smdk2416_pcm_state *state;
	int ret;
	u16 reg;

	pcm_runtime = snd_soc_card_get_pcm(soc_card, "HiFi");
	if (pcm_runtime == NULL)
		return -ENODEV;

	codec = snd_soc_card_get_codec(soc_card, wm8350_codec_id);
	if (codec == NULL)
		return -ENODEV;

	state = kzalloc(sizeof(struct smdk2416_pcm_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	pcm_runtime->private_data = state;

	codec->platform_data = &smdk2416_wm8350_setup;
	snd_soc_card_config_codec(codec, NULL, NULL, wm8350);
	snd_soc_card_init_codec(codec, soc_card);

#if 0
	/* add imx32ads specific controls */
	for (i = 0; i < ARRAY_SIZE(imx32ads_wm8350_audio_controls); i++) {
		if ((err = snd_ctl_add(soc_card->card,
				snd_soc_cnew(&imx32ads_wm8350_audio_controls[i],
					codec, NULL))) < 0)
			return err;
	}
#endif

	/* Add imx32ads specific widgets */
	ret = snd_soc_dapm_new_controls(soc_card, codec,
					smdk2416_dapm_widgets,
					ARRAY_SIZE(smdk2416_dapm_widgets));
	if (ret < 0)
		return ret;

	/* set up imx32ads specific audio path audio map */
	ret = snd_soc_dapm_add_routes(soc_card, audio_map,
		ARRAY_SIZE(audio_map));
	if (ret < 0)
		return ret;

	/* disable unused imx32ads WM8350 codec pins */
	snd_soc_dapm_disable_pin(soc_card, "OUT3");
	snd_soc_dapm_disable_pin(soc_card, "OUT4");
	snd_soc_dapm_disable_pin(soc_card, "IN2R");
	snd_soc_dapm_disable_pin(soc_card, "IN2L");
//	snd_soc_dapm_disable_pin(soc_card, "OUT2L");
//	snd_soc_dapm_disable_pin(soc_card, "OUT2R");

	/* connect and enable all imx32ads WM8350 jacks (for now) */
//	snd_soc_dapm_enable_pin(soc_card, "SiMIC");
	snd_soc_dapm_enable_pin(soc_card, "Mic1 Jack");
//	snd_soc_dapm_enable_pin(soc_card, "Mic2 Jack");
//	snd_soc_dapm_enable_pin(soc_card, "Line In Jack");
	snd_soc_dapm_set_policy(soc_card, SND_SOC_DAPM_POLICY_AUTOMATIC);
	snd_soc_dapm_sync(soc_card);

	/* enable slow clock gen for jack detect */
	reg = wm8350_reg_read(wm8350, WM8350_POWER_MGMT_4);
	wm8350_reg_write(wm8350, WM8350_POWER_MGMT_4, reg | WM8350_TOCLK_ENA);

	/* enable jack detect */
	reg = wm8350_reg_read(wm8350, WM8350_JACK_DETECT);
	wm8350_reg_write(wm8350, WM8350_JACK_DETECT, reg | WM8350_JDR_ENA);
#if 0
	wm8350_register_irq(wm8350, WM8350_IRQ_CODEC_JCK_DET_R,
			    smdk2416_jack_handler, soc_card);
	wm8350_unmask_irq(wm8350, WM8350_IRQ_CODEC_JCK_DET_R);
#endif

	return 0;
}

static void smdk2416_audio_exit(struct snd_soc_card *soc_card)
{
	struct snd_soc_pcm_runtime *pcm_runtime;

	pcm_runtime = snd_soc_card_get_pcm(soc_card, "HiFi");
	if (pcm_runtime)
		kfree(pcm_runtime->private_data);
}

static struct snd_soc_pcm_config hifi_pcm_config = {
	.name = "HiFi",
	.codec = wm8350_codec_id,
	.codec_dai = wm8350_codec_dai_id,
	.platform = s3c24xx_platform_id,
	.cpu_dai = s3c_i2s_id,
	.ops = &smdk2416_hifi_ops,
	.playback = 1,
	.capture = 1,
};

static int __init smdk2416_wm8350_audio_probe(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct snd_soc_card *soc_card;
	struct smdk2416_data *audio_data;
	int ret = 0;

	audio_data = kzalloc(sizeof(*audio_data), GFP_KERNEL);
	if (audio_data == NULL)
		return -ENOMEM;

	ret = get_iis_clk(&pdev->dev);
	if (ret < 0) {
		printk(KERN_ERR "%s: cant get iis clock\n", __func__);
		goto ssi_err;
	}

	soc_card = snd_soc_card_create("smdk2416", &pdev->dev,
					 SNDRV_DEFAULT_IDX1,
					 SNDRV_DEFAULT_STR1);

	if (soc_card == NULL) {
		ret = -ENOMEM;
		goto ssi_err;
	}

	audio_data->wm8350 = wm8350;
	soc_card->longname = "WM8350";
	soc_card->init = smdk2416_audio_init,
	soc_card->exit = smdk2416_audio_exit,
	soc_card->private_data = audio_data;
	soc_card->dev = &pdev->dev;
	wm8350->audio = soc_card;

	ret = snd_soc_card_create_pcms(soc_card, &hifi_pcm_config, 1);
	if (ret < 0)
		goto err;

	ret = snd_soc_card_register(soc_card);

	return ret;

err:
	snd_soc_card_free(soc_card);
ssi_err:
	kfree(audio_data);
	put_iis_clk();
	return ret;
}

static int __devexit smdk2416_wm8350_audio_remove(struct platform_device *pdev)
{
	struct wm8350 *wm8350 = platform_get_drvdata(pdev);
	struct snd_soc_card *soc_card = wm8350->audio;
	struct smdk2416_data *audio_data = soc_card->private_data;

#if 0
	wm8350_mask_irq(wm8350, WM8350_IRQ_CODEC_JCK_DET_R);
	wm8350_free_irq(wm8350, WM8350_IRQ_CODEC_JCK_DET_R);
#endif
	snd_soc_card_free(soc_card);
	kfree(audio_data);
	put_iis_clk();
	return 0;
}

static struct platform_driver smdk2416_wm8350_audio_driver = {
	.probe		= smdk2416_wm8350_audio_probe,
	.remove		= __devexit_p(smdk2416_wm8350_audio_remove),
	.suspend	= smdk2416_wm8350_audio_suspend,
	.resume		= smdk2416_wm8350_audio_resume,
	.driver		= {
		.name	= "smdk2416-audio",
	},
};

static int __init smdk2416_wm8350_audio_init(void)
{
	return platform_driver_register(&smdk2416_wm8350_audio_driver);
}

static void __exit smdk2416_wm8350_audio_exit(void)
{
	platform_driver_unregister(&smdk2416_wm8350_audio_driver);
}

module_init(smdk2416_wm8350_audio_init);
module_exit(smdk2416_wm8350_audio_exit);

/* Module information */
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("ALSA SoC WM8753 Neo1973");
MODULE_LICENSE("GPL");
