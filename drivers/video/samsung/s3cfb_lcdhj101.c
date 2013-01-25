/* linux/drivers/video/samsung/s3cfb_lte480wv.c
 *
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "s3cfb.h"

static struct s3cfb_lcd lcdhj101= {
	.width = 1280,
	.height = 800,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 72,
		.h_bp = 248,
		.h_sw = 53,
		.v_fp = 5,
		.v_fpe = 1,
		.v_bp = 20,
		.v_bpe = 1,
		.v_sw = 4,
	},

	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* name should be fixed as 's3cfb_set_lcd_info' */
void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	lcdhj101.init_ldi = NULL;
	ctrl->lcd = &lcdhj101;
}

