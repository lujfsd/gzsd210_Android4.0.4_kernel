/*
 *  smdk_wm8580.c
 *
 *  Copyright (c) 2009 Samsung Electronics Co. Ltd
 *  Author: Jaswinder Singh <jassi.brar@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <sound/soc.h>
#include <linux/clk.h>
#include <sound/pcm_params.h>

#include <asm/mach-types.h>
#include <mach/regs-clock.h>


#include "../codecs/wm8960.h"
#include "i2s.h"
#include "s3c-i2s-v2.h"

static int s3c6410_hifi_hw_params(struct snd_pcm_substream *substream,	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int psr = 1;
	int bfs, rfs, ret;
	struct clk    *clk_epll;
	unsigned int pll_out;
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U8:
	case SNDRV_PCM_FORMAT_S8:
		bfs = 16;
		break;
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;
		break;
	default:
		return -EINVAL;
	}

	switch (params_rate(params)) {
	case 16000:
	case 22050:
	case 32000:
	case 44100:
	case 48000:
	case 88200:
	case 96000:
		rfs = 256;
		break;
	case 64000:
		rfs = 384;
		break;
	case 8000:
	case 11025:
		rfs = 512;
		break;
	default:
		return -EINVAL;
	}

	pll_out = params_rate(params) * rfs;

	switch (pll_out) {
        case 4096000:
        case 5644800:
        case 6144000:
        case 8467200:
        case 9216000:
                psr = 8;
                break;
        case 8192000:
        case 11289600:
        case 12288000:
        case 16934400:
        case 18432000:
                psr = 4;
                break;
        case 22579200:
        case 24576000:
        case 33868800:
        case 36864000:
                psr = 2;
                break;
        case 67737600:
        case 73728000:
                psr = 1;
                break;
        default:
                printk("Not yet supported!\n");
                return -EINVAL;
        }
	clk_epll = clk_get(NULL, "fout_epll");
        if (IS_ERR(clk_epll)) {
                printk(KERN_ERR
                        "failed to get fout_epll\n");
                return -EBUSY;
        }

	pll_out *= psr;
	clk_set_rate(clk_epll, pll_out);

#if 0
	ret = snd_soc_dai_set_sysclk(cpu_dai, 2, 0, SND_SOC_CLOCK_OUT);
        if (ret < 0){
                printk(KERN_ERR
                        "smdkv210 : SND_SOC_CLOCK_OUT  setting error!\n");
                return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, 1, 0, SND_SOC_CLOCK_IN);
        if (ret < 0){
                printk(KERN_ERR
                        "smdkv210 : SND_SOC_CLOCK_IN  setting error!\n");
                return ret;
	}
#else

	ret = snd_soc_dai_set_sysclk(cpu_dai, 2, 0, SND_SOC_CLOCK_IN);
        if (ret < 0){
                printk(KERN_ERR
                        "smdkv210 : SND_SOC_CLOCK_IN  setting error!\n");
                return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, 1, 0, SND_SOC_CLOCK_OUT);
        if (ret < 0){
                printk(KERN_ERR
                        "smdkv210 : SND_SOC_CLOCK_OUT  setting error!\n");
                return ret;
	}

#endif

	ret = snd_soc_dai_set_fmt(codec_dai, (SND_SOC_DAIFMT_CBS_CFS |SND_SOC_DAIFMT_I2S |SND_SOC_DAIFMT_NB_NF));
        if (ret < 0) {
                printk(KERN_ERR
                        "smdkv210 : codec_dai setting error!\n");
                return ret;
        }

	ret = snd_soc_dai_set_fmt(cpu_dai, (SND_SOC_DAIFMT_CBS_CFS|SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF));
        if (ret < 0) {
                printk(KERN_ERR
                        "smdkv210 : cpu_dai setting error!\n");
                return ret;
        }

	clk_put(clk_epll);
	return 0;
}

/*
 * SMDK WM8960 DAI operations.
 */
static struct snd_soc_ops s3C6410_hifi_ops = {
	.hw_params = s3c6410_hifi_hw_params,
};

/* machine dapm widgets */
static const struct snd_soc_dapm_widget s3c6410_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Audio Out1", NULL),
	SND_SOC_DAPM_MIC("my Mic", NULL),
	SND_SOC_DAPM_MIC("my Line IN", NULL),
};


static const struct snd_kcontrol_new wm8960_s3c6410_controls[] = {
	SOC_DAPM_PIN_SWITCH("Audio Out1"),
	SOC_DAPM_PIN_SWITCH("my Mic"),
	SOC_DAPM_PIN_SWITCH("my Line IN"),

};

/* example machine audio_mapnections */
static const struct snd_soc_dapm_route audio_map[] = {

	/* Connections to the ... */
	{"Audio Out1", NULL, "HP_L"},
	{"Audio Out1", NULL, "HP_R"},
        
	/* Mic */
	{"LINPUT1", NULL, "MICB"},
	{"MICB", NULL, "my Mic"},

	/* Line in */
	{"LINPUT3", NULL, "my Line IN"},
	{"RINPUT3", NULL, "my Line IN"},
	
};

static int s3C6410_wm8960_init(struct snd_soc_pcm_runtime *rtd)
{
	int err;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	//unsigned short val;


	/* add board specific widgets */
	snd_soc_dapm_new_controls(dapm,s3c6410_dapm_widgets, ARRAY_SIZE(s3c6410_dapm_widgets));

	err = snd_soc_add_controls(codec, wm8960_s3c6410_controls,
		ARRAY_SIZE(wm8960_s3c6410_controls));

	if (err < 0)
		return err;

	/* setup board specific audio path audio_map */
	snd_soc_dapm_add_routes(dapm, audio_map,ARRAY_SIZE(audio_map));

	/* set endpoints to default off mode */
	snd_soc_dapm_enable_pin(dapm, "Audio Out1");
	snd_soc_dapm_enable_pin(dapm, "my Mic");


	return 0;
}

static struct snd_soc_dai_link s3c6410_dai[] = {
	{
		.name		= "wm8960",
		.stream_name	= "wm8960-hifi",
		.cpu_dai_name	= "samsung-i2s.0",
		.codec_dai_name	= "wm8960-hifi",
		.platform_name	= "samsung-audio",
		.codec_name	= "wm8960-codec.0-001a",
		.init		= s3C6410_wm8960_init,
		.ops		= &s3C6410_hifi_ops,
	},
}; 

static struct snd_soc_card snd_soc_s3c6410 = {
	.name = "gzsd210",
	.dai_link = s3c6410_dai,
	.num_links = ARRAY_SIZE(s3c6410_dai),
};



static struct platform_device *s3c6410_snd_device;
static struct platform_device *gzsd6410_snd_wm8960_device;
static int __init s3c6410_init(void)
{	
	int ret;
	printk("wm8960 support for gzsd210!\n");

	gzsd6410_snd_wm8960_device = platform_device_alloc("wm8960-codec", -1);
	if (!gzsd6410_snd_wm8960_device)
		return -ENOMEM;

	ret = platform_device_add(gzsd6410_snd_wm8960_device);
	if (ret)
	{
		platform_device_put(gzsd6410_snd_wm8960_device);
		return ret;
	}

	s3c6410_snd_device = platform_device_alloc("soc-audio", -1);
	if (!s3c6410_snd_device)
		return -ENOMEM;

	
	platform_set_drvdata(s3c6410_snd_device, &snd_soc_s3c6410);
	
	ret = platform_device_add(s3c6410_snd_device);
	if (ret) {
		platform_device_put(s3c6410_snd_device);
		return ret;
	}
	
	return ret;
}

static void __exit s3c6410_exit(void)
{
	platform_device_unregister(s3c6410_snd_device);
}

module_init(s3c6410_init);
module_exit(s3c6410_exit);

MODULE_AUTHOR("Jiang jianjun");
MODULE_DESCRIPTION("ALSA SoC WM8960 S3C6410");
MODULE_LICENSE("GPL");
