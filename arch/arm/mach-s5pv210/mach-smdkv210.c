/* linux/arch/arm/mach-s5pv210/mach-smdkv210.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/max8698.h>
#include <mach/power-domain.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/console.h>
#include <linux/sysdev.h>
#include <linux/dm9000.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/usb/ch9.h>
#include <linux/spi/spi.h>
#include <linux/gpio_keys.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/spi-clocks.h>
#include <mach/regs-fb.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s3c64xx-spi.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/mfc.h>
#include <plat/s5p-time.h>
#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/csis.h>
#include <plat/jpeg.h>
#include <plat/clock.h> 
#include <plat/regs-otg.h>
#include <plat/otg.h>
#include <plat/ehci.h>
#include <plat/ohci.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <plat/media.h>
#include <mach/media.h>
#include <mach/gpio-smdkc110.h>

#ifdef CONFIG_TOUCHSCREEN_EGALAX
#include <linux/i2c/egalax.h>
#define EETI_TS_DEV_NAME        "egalax_i2c"

static struct egalax_i2c_platform_data  egalax_platdata  = {
        .gpio_int = EGALAX_IRQ,
        .gpio_en = NULL,
        .gpio_rst = NULL,
};
#endif
/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
};

#if defined(CONFIG_REGULATOR_MAX8698)
/* LDO */
static struct regulator_consumer_supply smdkv210_ldo3_consumer[] = {
	REGULATOR_SUPPLY("pd_io", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_io", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_io", "s5p-ehci"),
};

static struct regulator_consumer_supply smdkv210_ldo5_consumer[] = {
	REGULATOR_SUPPLY("AVDD", "0-001b"),
	REGULATOR_SUPPLY("DVDD", "0-001b"),
};

static struct regulator_consumer_supply smdkv210_ldo8_consumer[] = {
	REGULATOR_SUPPLY("pd_core", "s3c-usbgadget"),
	REGULATOR_SUPPLY("pd_core", "s5p-ohci"),
	REGULATOR_SUPPLY("pd_core", "s5p-ehci"),
};

static struct regulator_init_data smdkv210_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo3_data = {
	.constraints	= {
		.name		= "VUOTG_D+VUHOST_D_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo3_consumer),
	.consumer_supplies	= smdkv210_ldo3_consumer,
};

static struct regulator_init_data smdkv210_ldo4_data = {
	.constraints	= {
		.name		= "V_MIPI_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo5_data = {
	.constraints	= {
		.name		= "VMMC+VEXT_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo5_consumer),
	.consumer_supplies	= smdkv210_ldo5_consumer,
};

static struct regulator_init_data smdkv210_ldo6_data = {
	.constraints	= {
		.name		= "VCC_2.6V",
		.min_uV		= 2600000,
		.max_uV		= 2600000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	 = {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo7_data = {
	.constraints	= {
		.name		= "VDAC_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkv210_ldo8_data = {
	.constraints	= {
		.name		= "VUOTG_A+VUHOST_A_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(smdkv210_ldo8_consumer),
	.consumer_supplies	= smdkv210_ldo8_consumer,
};

static struct regulator_init_data smdkv210_ldo9_data = {
	.constraints	= {
		.name		= "VADC+VSYS+VKEY_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

/* BUCK */
static struct regulator_consumer_supply smdkv210_buck1_consumer =
	REGULATOR_SUPPLY("vddarm", NULL);

static struct regulator_consumer_supply smdkv210_buck2_consumer =
	REGULATOR_SUPPLY("vddint", NULL);

static struct regulator_init_data smdkv210_buck1_data = {
	.constraints	= {
		.name		= "VCC_ARM",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1250000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck1_consumer,
};

static struct regulator_init_data smdkv210_buck2_data = {
	.constraints	= {
		.name		= "VCC_INTERNAL",
		.min_uV		= 950000,
		.max_uV		= 1200000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1100000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &smdkv210_buck2_consumer,
};

static struct regulator_init_data smdkv210_buck3_data = {
	.constraints	= {
		.name		= "VCC_MEM",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.always_on	= 1,
		.apply_uV	= 1,
		.state_mem	= {
			.uV	= 1800000,
			.mode	= REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
};

static struct max8698_regulator_data smdkv210_regulators[] = {
	{ MAX8698_LDO2,  &smdkv210_ldo2_data },
	{ MAX8698_LDO3,  &smdkv210_ldo3_data },
	{ MAX8698_LDO4,  &smdkv210_ldo4_data },
	{ MAX8698_LDO5,  &smdkv210_ldo5_data },
	{ MAX8698_LDO6,  &smdkv210_ldo6_data },
	{ MAX8698_LDO7,  &smdkv210_ldo7_data },
	{ MAX8698_LDO8,  &smdkv210_ldo8_data },
	{ MAX8698_LDO9,  &smdkv210_ldo9_data },
	{ MAX8698_BUCK1, &smdkv210_buck1_data },
	{ MAX8698_BUCK2, &smdkv210_buck2_data },
	{ MAX8698_BUCK3, &smdkv210_buck3_data },
};

static struct max8698_platform_data smdkv210_max8698_pdata = {
	.num_regulators = ARRAY_SIZE(smdkv210_regulators),
	.regulators     = smdkv210_regulators,

	/* 1GHz default voltage */
	.dvsarm1        = 0xa,  /* 1.25v */
	.dvsarm2        = 0x9,  /* 1.20V */
	.dvsarm3        = 0x6,  /* 1.05V */
	.dvsarm4        = 0x4,  /* 0.95V */
	.dvsint1        = 0x7,  /* 1.10v */
	.dvsint2        = 0x5,  /* 1.00V */
	.set1       = S5PV210_GPH1(6),
	.set2       = S5PV210_GPH1(7),
	.set3       = S5PV210_GPH0(4),
};
#endif

static struct s3c_ide_platdata smdkv210_ide_pdata __initdata = {
	.setup_gpio	= s5pv210_ide_setup_gpio,
};

static struct platform_device gzsd210_device_adc = {
	.name			= "gzsd210_adc",
	.id				= -1,
	.num_resources	= 0,
};

static uint32_t smdkv210_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_1), KEY(0, 4, KEY_2), KEY(0, 5, KEY_3),
	KEY(0, 6, KEY_4), KEY(0, 7, KEY_5),
	KEY(1, 3, KEY_A), KEY(1, 4, KEY_B), KEY(1, 5, KEY_C),
	KEY(1, 6, KEY_D), KEY(1, 7, KEY_E), KEY(7, 1, KEY_LEFTBRACE)
};

static struct matrix_keymap_data smdkv210_keymap_data __initdata = {
	.keymap		= smdkv210_keymap,
	.keymap_size	= ARRAY_SIZE(smdkv210_keymap),
};

static struct samsung_keypad_platdata smdkv210_keypad_data __initdata = {
	.keymap_data	= &smdkv210_keymap_data,
	.rows		= 8,
	.cols		= 8,
};
#if 0
static struct resource smdkv210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_SROM_BANK5,
		.end	= S5PV210_PA_SROM_BANK5,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_SROM_BANK5 + 2,
		.end	= S5PV210_PA_SROM_BANK5 + 2,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(9),
		.end	= IRQ_EINT(9),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct dm9000_plat_data smdkv210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x08, 0x90, 0x00, 0xa0, 0x02, 0x10 },
};

struct platform_device smdkv210_dm9000 = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smdkv210_dm9000_resources),
	.resource	= smdkv210_dm9000_resources,
	.dev		= {
		.platform_data	= &smdkv210_dm9000_platdata,
	},
};
#else
/* physical address for dm9000a ...kgene.kim@samsung.com */
#define S5PV210_PA_DM9000_A		(0x88001000)
#define S5PV210_PA_DM9000_F		(S5PV210_PA_DM9000_A + 0x300C)

static struct resource smdkv210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_DM9000_A,
		.end	= S5PV210_PA_DM9000_A + SZ_1K*4 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_DM9000_F,
		.end	= S5PV210_PA_DM9000_F + SZ_1K*4 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(9),
		.end	= IRQ_EINT(9),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct dm9000_plat_data smdkv210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x08, 0x90, 0x00, 0xa0, 0x02, 0x10 },
};

struct platform_device smdkv210_dm9000 = {
	.name		= "dm9000",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(smdkv210_dm9000_resources),
	.resource	= smdkv210_dm9000_resources,
	.dev		= {
		.platform_data	= &smdkv210_dm9000_platdata,
	},
};
#endif
#ifdef CONFIG_REGULATOR
static struct regulator_consumer_supply smdkv210_b_pwr_5v_consumers[] = {
        {
                /* WM8580 */
                .supply         = "PVDD",
                .dev_name       = "0-001b",
        },
};

static struct regulator_init_data smdkv210_b_pwr_5v_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = ARRAY_SIZE(smdkv210_b_pwr_5v_consumers),
        .consumer_supplies      = smdkv210_b_pwr_5v_consumers,
};

static struct fixed_voltage_config smdkv210_b_pwr_5v_pdata = {
        .supply_name    = "B_PWR_5V",
        .microvolts     = 5000000,
        .init_data      = &smdkv210_b_pwr_5v_data,
	.gpio		= -1,
};

static struct platform_device smdkv210_b_pwr_5v = {
        .name          = "reg-fixed-voltage",
        .id            = -1,
        .dev = {
                .platform_data = &smdkv210_b_pwr_5v_pdata,
        },
};
#endif
#ifdef CONFIG_TOUCHSCREEN_EGALAX
static struct i2c_gpio_platform_data i2c5_platdata = {
        .sda_pin                = S5PV210_GPB(6),
        .scl_pin                = S5PV210_GPB(7),
        .udelay                 = 2,
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0.
};

//static struct platform_device   s3c_device_i2c5 = {
struct platform_device   s3c_device_i2c5 = {
        .name                   = "i2c-gpio",
        .id                     = 5,
        .dev.platform_data      = &i2c5_platdata,
};

static struct i2c_board_info i2c_devs5[] __initdata = {
        {
                I2C_BOARD_INFO(EETI_TS_DEV_NAME, 0x04),
                .platform_data = &egalax_platdata,
                .irq = IRQ_EINT6,
        },
};
#endif

#ifdef CONFIG_FB_S3C_LTE480WV
#define S5PV210_LCD_WIDTH 800
#define S5PV210_LCD_HEIGHT 480
#define NUM_BUFFER 4
#endif

#ifdef CONFIG_FB_S3C_AT070TN92
#define S5PV210_LCD_WIDTH 800
#define S5PV210_LCD_HEIGHT 480
#define NUM_BUFFER 4
#endif

#ifdef CONFIG_FB_S3C_101WA01S
//#define S5PV210_LCD_WIDTH 1366
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 768
#endif

#ifdef CONFIG_FB_S3C_LCD_NT11003
#define S5PV210_LCD_WIDTH 1024
#define S5PV210_LCD_HEIGHT 600
#define NUM_BUFFER 8
#endif

#ifdef CONFIG_FB_S3C_LCD_HJ101
#define S5PV210_LCD_WIDTH 1280
#define S5PV210_LCD_HEIGHT 800
#define NUM_BUFFER 8
#endif

#ifdef CONFIG_FB_S3C_TL2796
#define S5PV210_LCD_WIDTH 1024 //1280
#define S5PV210_LCD_HEIGHT 768 //800
#define NUM_BUFFER 8
#endif
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (24576 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (S5PV210_LCD_WIDTH * \
                                             S5PV210_LCD_HEIGHT * NUM_BUFFER * \
                                             (CONFIG_FB_S3C_NR_BUFFERS + \
                                                 (CONFIG_FB_S3C_NUM_OVLY_WIN * \
                                                  CONFIG_FB_S3C_NUM_BUF_OVLY_WIN)))
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (8192 * SZ_1K)
#define  S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 (1800 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D (8192 * SZ_1K)

static struct s5p_media_device s5pv210_media_devs[] = {
        [0] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
                .paddr = 0,
        },
        [1] = {
                .id = S5P_MDEV_MFC,
                .name = "mfc",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
                .paddr = 0,
        },
        [2] = {
                .id = S5P_MDEV_FIMC0,
                .name = "fimc0",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
                .paddr = 0,
        },
        [3] = {
                .id = S5P_MDEV_FIMC1,
                .name = "fimc1",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
                .paddr = 0,
        },
        [4] = {
                .id = S5P_MDEV_FIMC2,
                .name = "fimc2",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
                .paddr = 0,
        },
        [5] = {
                .id = S5P_MDEV_JPEG,
                .name = "jpeg",
                .bank = 0,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
                .paddr = 0,
        },
	[6] = {
                .id = S5P_MDEV_FIMD,
                .name = "fimd",
                .bank = 1,
                .memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
                .paddr = 0,
        },
	[7] = {
                .id = S5P_MDEV_PMEM_GPU1,
                .name = "pmem_gpu1",
                .bank = 0, /* OneDRAM */
                .memsize = S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1,
                .paddr = 0,
        },
	[8] = {
		.id = S5P_MDEV_G2D,
		.name = "g2d",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D,
		.paddr = 0,
	},
};

#ifdef CONFIG_BATTERY_GZSD210
struct platform_device sec_device_battery = {
	.name   = "sec-fake-battery",
	.id = -1,
};
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
        .name = "pmem",
        .no_allocator = 1,
        .cached = 1,
        .start = 0,
        .size = 0,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
        .name = "pmem_gpu1",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};
 
static struct android_pmem_platform_data pmem_adsp_pdata = {
        .name = "pmem_adsp",
        .no_allocator = 1,
        .cached = 1,
        .buffered = 1,
        .start = 0,
        .size = 0,
};      

static struct platform_device pmem_device = {
        .name = "android_pmem",
        .id = 0,
        .dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
        .name = "android_pmem",
        .id = 1,
        .dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
        .name = "android_pmem",
        .id = 2,
        .dev = { .platform_data = &pmem_adsp_pdata },
};


static void __init android_pmem_set_platdata(void)
{
        //pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
        //pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);

        pmem_gpu1_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
        pmem_gpu1_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);

        /*pmem_adsp_pdata.start =
                (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_ADSP, 0);
        pmem_adsp_pdata.size =
                (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_ADSP, 0);*/
}
#endif


static void smdkv210_lte480wv_set_power(struct plat_lcd_data *pd,
					unsigned int power)
{
	if (power) {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(0), "GPD0");
		gpio_direction_output(S5PV210_GPD0(0), 1);
		gpio_free(S5PV210_GPD0(0));
#endif

		/* fire nRESET on power up */
		gpio_request(S5PV210_GPH0(6), "GPH0");

		gpio_direction_output(S5PV210_GPH0(6), 1);

		gpio_set_value(S5PV210_GPH0(6), 0);
		mdelay(10);

		gpio_set_value(S5PV210_GPH0(6), 1);
		mdelay(10);

		gpio_free(S5PV210_GPH0(6));
	} else {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(0), "GPD0");
		gpio_direction_output(S5PV210_GPD0(0), 0);
		gpio_free(S5PV210_GPD0(0));
#endif
	}
}

static struct plat_lcd_data smdkv210_lcd_lte480wv_data = {
	.set_power	= smdkv210_lte480wv_set_power,
};

static struct platform_device smdkv210_lcd_lte480wv = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdkv210_lcd_lte480wv_data,
};

#ifdef CONFIG_FB_S3C_AT070TN92
static struct s3cfb_lcd at070tn92 = {
	.width	= 800,//800,
	.height	= 480,//480,
	.bpp = 32,
	.freq = 60,
#if 0 //nc43
	.timing = {
		.h_fp = 0x04,//46,//46,
		.h_bp = 0x2d,//44,//38,
		.h_sw = 0x06,//5,//5,
		.v_fp = 0x02,//3,//3,
		.v_fpe = 1,
		.v_bp = 0x03,//20,//20,
		.v_bpe = 1,
		.v_sw = 0x02,//2,//2,
	},
#else	//at070
#if	1	
	.timing = {
		.h_fp = 20,//46,
		.h_bp = 30,//38,
		.h_sw = 15,//5,
		.v_fp = 3,//3,
		.v_fpe = 1,
		.v_bp = 20,//20,
		.v_bpe = 1,
		.v_sw = 2,//2,
	},
#else	
	.timing = {
		.h_fp = 39,//46,
		.h_bp = 39,//38,
		.h_sw = 47,//5,
		.v_fp = 12,//3,
		.v_fpe = 1,
		.v_bp = 28,//20,
		.v_bpe = 1,
		.v_sw = 2,//2,
	},
#endif

#endif

	.polarity = {
		.rise_vclk	= 0,
		.inv_hsync	= 1,
		.inv_vsync	= 1,
		.inv_vden	= 0,
	},
};


/* LCD on/off or backlight control */
#define GZSD210_BL_GPIO			S5PV210_GPD0(0)
#define GZSD210_BL_PWM			1
static void at070tn92_cfg_gpio(struct platform_device *pdev)
{
        int i;

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
        }

        for (i = 0; i < 8; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
        }
        for (i = 0; i < 4; i++) {
                s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
                s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
        }

        /* mDNIe SEL: why we shall write 0x2 ? */
        writel(0x2, S5P_MDNIE_SEL);
        /* drive strength to max */
        writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
        writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
}

static int at070tn92_backlight_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(GZSD210_BL_GPIO, "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(GZSD210_BL_GPIO, 1);
	gpio_free(GZSD210_BL_GPIO);
        return 0;
}
static int at070tn92_backlight_off(struct platform_device *pdev, int onoff)
{
	int err;

	err = gpio_request(GZSD210_BL_GPIO, "GPD0");
	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(GZSD210_BL_GPIO, 0);
	gpio_free(GZSD210_BL_GPIO);
        return 0;
}
static int at070tn92_reset_lcd(struct platform_device *pdev)
{
        return 0;
}
static struct s3c_platform_fb at070tn92_fb_data __initdata = {
        .hw_ver = 0x62,
        .clk_name       = "sclk_fimd",
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,

        .lcd = &at070tn92,
        .cfg_gpio       = at070tn92_cfg_gpio,
        .backlight_on   = at070tn92_backlight_on,
        .backlight_onoff    = at070tn92_backlight_off,
        .reset_lcd      = at070tn92_reset_lcd,
};
#endif

//steven.lu
#ifdef CONFIG_FB_S3C_LCD_NT11003
static struct s3cfb_lcd lcdnt11003= {
	.width = 1024,
	.height = 600,
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

static void lcdnt11003_cfg_gpio(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* mDNIe SEL: why we shall write 0x2 ? */
	writel(0x2, S5P_MDNIE_SEL);

	/* drive strength to max */
	writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
	writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
}

static int lcdnt11003_backlight_on(struct platform_device *pdev)
{
	return 0;
}

static int lcdnt11003_backlight_off(struct platform_device *pdev)
{
	return 0;
}

static int lcdnt11003_reset_lcd(struct platform_device *pdev)
{
	return 0;
}


static struct s3c_platform_fb lcdnt11003_fb_data __initdata = {
	.hw_ver	= 0x62,
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,

	.lcd = &lcdnt11003,
	.cfg_gpio	= lcdnt11003_cfg_gpio,
	.backlight_on	= lcdnt11003_backlight_on,
	.backlight_onoff    = lcdnt11003_backlight_off,
	.reset_lcd	= lcdnt11003_reset_lcd,
};

#endif

//steven.lu
#ifdef CONFIG_FB_S3C_LCD_HJ101
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

static void lcdhj101_cfg_gpio(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* mDNIe SEL: why we shall write 0x2 ? */
	writel(0x2, S5P_MDNIE_SEL);

	/* drive strength to max */
	writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
	writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
}

static int lcdhj101_backlight_on(struct platform_device *pdev)
{
	return 0;
}

static int lcdhj101_backlight_off(struct platform_device *pdev)
{
	return 0;
}

static int lcdhj101_reset_lcd(struct platform_device *pdev)
{
	return 0;
}


static struct s3c_platform_fb lcdhj101_fb_data __initdata = {
	.hw_ver	= 0x62,
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,

	.lcd = &lcdhj101,
	.cfg_gpio	= lcdhj101_cfg_gpio,
	.backlight_on	= lcdhj101_backlight_on,
	.backlight_onoff    = lcdhj101_backlight_off,
	.reset_lcd	=lcdhj101_reset_lcd,
};

#endif

static int smdkv210_backlight_init(struct device *dev)
{
	int ret;
	//need to check the calling function for this function and remove the call.
	return 0;

	ret = gpio_request(S5PV210_GPD0(0), "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPD for PWM-OUT 3\n");
		return ret;
	}

	/* Configure GPIO pin with S5PV210_GPD_0_0_TOUT_0 */
	s3c_gpio_cfgpin(S5PV210_GPD0(0), S3C_GPIO_SFN(2));

	return 0;
}

static void smdkv210_backlight_exit(struct device *dev)
{
	s3c_gpio_cfgpin(S5PV210_GPD0(0), S3C_GPIO_OUTPUT);
	gpio_free(S5PV210_GPD0(0));
}

static struct platform_pwm_backlight_data smdkv210_backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.lth_brightness = 50,
	.pwm_period_ns	= 25000,
	.init		= smdkv210_backlight_init,
	.exit		= smdkv210_backlight_exit,
};

static struct platform_device smdkv210_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[0].dev,
		.platform_data	= &smdkv210_backlight_data,
	},
};

struct s3c_adc_mach_info {
        /* if you need to use some platform data, add in here*/
        int delay;
        int presc;
        int resolution;
};

static struct platform_device *smdkv210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_fb,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	&s3c_device_i2c5,
#endif
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
#ifdef CONFIG_SND_SAMSUNG_AC97
	&s5pv210_device_ac97,
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
	&s5pv210_device_iis0,
#endif
	&s5pv210_device_spdif,
#ifdef CONFIG_SND_SAMSUNG_PCM
#ifdef CONFIG_SND_SAMSUNG_PCM_USE_I2S1_MCLK
	&s5pv210_device_pcm0,
#endif
#endif 	/*end of CONFIG_SND_SAMSUNG_PCM*/
	&samsung_asoc_dma,
	&samsung_device_keypad,
	&smdkv210_dm9000,
	&smdkv210_lcd_lte480wv,
	&s3c_device_timer[0],
#ifdef	CONFIG_GZSD210_PWM
	&s3c_device_timer[1],
#endif
#ifdef	CONFIG_BACKLIGHT_PWM
	&smdkv210_backlight_device,
#endif
	&s5p_device_ehci,
	&s5p_device_ohci,
	&gzsd210_device_adc,
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_VIDEO_FIMC
        &s3c_device_fimc0,
        &s3c_device_fimc1,
        &s3c_device_fimc2,
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	&s3c_device_csis,
#endif
#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif
#ifdef CONFIG_VIDEO_MFC50
        &s3c_device_mfc,
#endif
#ifdef CONFIG_ANDROID_PMEM
	&pmem_gpu1_device,
#endif
#ifdef CONFIG_SPI_S3C64XX
	&s5pv210_device_spi0,
	&s5pv210_device_spi1,
#endif

#ifdef CONFIG_REGULATOR
        &smdkv210_b_pwr_5v,
#endif

#ifdef CONFIG_S5PV210_POWER_DOMAIN
        &s5pv210_pd_tv,
        &s5pv210_pd_lcd,
        &s5pv210_pd_g3d,
        &s5pv210_pd_mfc,
        &s5pv210_pd_audio,
        &s5pv210_pd_cam,  
#endif
	&s3c_device_g3d,
#ifdef CONFIG_VIDEO_G2D
        &s3c_device_g2d,
#endif
#ifdef CONFIG_BATTERY_GZSD210
	&sec_device_battery,
#endif
#ifdef CONFIG_VIDEO_TV20
        &s5p_device_tvout,
        &s5p_device_cec,
        &s5p_device_hpd,
#endif
#ifdef CONFIG_MTD_NAND
	&s3c_device_nand,   
#endif

};

#ifdef CONFIG_CAMERA_OV3640
#include <media/ov3640_platform.h>
#define	CAM_ITU_CH_B

static int smdkv210_OV3640_power(int onoff)//low_reset GPE1_4
{
	int err;
	if(onoff == 1) {

		err = gpio_request(S5PV210_GPJ1(3), "GPJ1_3"); //reset
		if (err) {
			printk(KERN_ERR "#### failed to request GPJ1_3 for RESET\n");
			return err;
		}

		gpio_direction_output(S5PV210_GPJ1(3), 0);
		mdelay(30);
		gpio_direction_output(S5PV210_GPJ1(3), 1);
		mdelay(2);
		gpio_free(S5PV210_GPJ1(3));
	}
	return 0;

}

static struct ov3640_platform_data ov3640_plat =
{
	.default_width = 2048,
	.default_height = 1536,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 40000000,
	.is_mipi = 0,
};

static struct i2c_board_info  ov3640_i2c_info = 
{
	I2C_BOARD_INFO("OV3640", 0x78>>1),
	.platform_data = &ov3640_plat,
};

static struct s3c_platform_camera ov3640 = {
#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
#else
	.id		= CAMERA_PAR_B,
#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.i2c_busnum	= 0,
	.info		= &ov3640_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam1",
	.clk_rate	= 40000000,
	.line_length	= 2048,
	.width		= 2048,
	.height		= 1536,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 2048,
		.height	= 1536,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,

	.cam_power	= smdkv210_OV3640_power,

};
#endif
/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",
	.clk_rate	= 166750000,
#if defined(CONFIG_VIDEO_S5K4EA)
	.default_cam	= CAMERA_CSI_C,
#else
#ifdef CAM_ITU_CH_A
	.default_cam	= CAMERA_PAR_A,
#else
	.default_cam	= CAMERA_PAR_B,
#endif
#endif
	.camera		= {
#ifdef CONFIG_CAMERA_OV3640
		&ov3640,
#endif
	},
	.hw_ver		= 0x43,
};

#ifdef CONFIG_VIDEO_JPEG_V2
static struct s3c_platform_jpeg jpeg_plat __initdata = {
	.max_main_width	= 800,
	.max_main_height	= 480,
	.max_thumb_width	= 320,
	.max_thumb_height	= 240,
};
#endif

static void __init gzsd210_wifi_init(void)
{
#if 1
	//power
	gpio_request(S5PV210_GPH1(3), "GPH1_3");
	gpio_direction_output(S5PV210_GPH1(3), 1);
	udelay(100);
	gpio_free(S5PV210_GPH1(3));
	//PDn
	gpio_request(S5PV210_GPH1(7), "GPH1_7");
	gpio_direction_output(S5PV210_GPH1(7), 1);
	udelay(100);
	gpio_free(S5PV210_GPH1(7));
	//reset
	gpio_request(S5PV210_GPH1(6), "GPH1_6");
	//gpio_direction_output(S5PV210_GPH1(6), 0);
	//udelay(100);
	gpio_set_value(S5PV210_GPH1(6), 1);
	gpio_free(S5PV210_GPH1(6));

	//cd
	gpio_request(S5PV210_GPG0(2), "GPG0_2");
	gpio_set_value(S5PV210_GPG0(2), 0);
	gpio_free(S5PV210_GPG0(2));
#endif
}

#if 0
static void __init smdkv210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(5), "nCS5");
	s3c_gpio_cfgpin(S5PV210_MP01(5), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(5));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC5);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS5__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS5__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}
#else
static int __init dm9000_set_mac(char *str) {
	unsigned char addr[6];
	unsigned int val;
	int idx = 0;
	char *p = str, *end;

	while (*p && idx < 6) {
		val = simple_strtoul(p, &end, 16);
		if (end <= p) {
			/* convert failed */
			break;
		} else {
			addr[idx++] = val;
			p = end;
			if (*p == ':'|| *p == '-') {
				p++;
			} else {
				break;
			}
		}
	}

	if (idx == 6) {
		printk("Setup ethernet address to %pM\n", addr);
		memcpy(smdkv210_dm9000_platdata.param_addr, addr, 6);
	}

	return 1;
}

__setup("ethmac=", dm9000_set_mac);

static void __init smdkv210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(1), "nCS1");
	s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(1));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC1);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS1__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS1__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}
#endif
#include <sound/wm8960.h>
static struct wm8960_data wm8960_pdata = {
	.capless		= 0,
	.dres			= WM8960_DRES_400R,
};

#ifdef  CONFIG_TOUCHSCREEN_NT11003
#include <linux/input/nt11003.h>
struct nt11003_i2c_platform_data nt11003_pdata;
#endif

#ifdef  CONFIG_TOUCHSCREEN_FT5X06_TS
#include <linux/input/ft5x06_ts.h>
#endif

static struct i2c_board_info smdkv210_i2c_devs0[] __initdata = {
#ifdef	CONFIG_SND_SOC_SAMSUNG_GZSD210_WM8960
	{ 
		I2C_BOARD_INFO("wm8960", 0x1a), 
		.platform_data  = &wm8960_pdata,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_NT11003
	{
		I2C_BOARD_INFO(NT11003_I2C_NAME,0x01),
		.platform_data = &nt11003_pdata,
	},
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5X06_TS
	{
		I2C_BOARD_INFO(FT5X0X_NAME,0x70>>1),
	},
#endif
};

static struct i2c_board_info smdkv210_i2c_devs1[] __initdata = {
#ifdef CONFIG_VIDEO_TV20
        {I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),},
#endif
};

static struct i2c_board_info smdkv210_i2c_devs2[] __initdata = {
#if defined(CONFIG_REGULATOR_MAX8698)
        {
                I2C_BOARD_INFO("max8698", 0xCC >> 1),
                .platform_data  = &smdkv210_max8698_pdata,
        },
#endif
	//{ I2C_BOARD_INFO("wm8580", 0x1a), },//wm8976
	{ I2C_BOARD_INFO("wm8978", 0x1a), },//wm8976
};

#ifdef CONFIG_SPI_S3C64XX

#define SMDK_MMCSPI_CS 0

static struct s3c64xx_spi_csinfo smdk_spi0_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(1),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};
//md by dao to icool210
#if 0
static struct s3c64xx_spi_csinfo smdk_spi1_csi[] = {
	[SMDK_MMCSPI_CS] = {
		.line = S5PV210_GPB(5),
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};
#endif
//end
static struct spi_board_info s3c_spi_devs[] __initdata = {
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 0,
		.chip_select     = 0,
		.controller_data = &smdk_spi0_csi[SMDK_MMCSPI_CS],
	},
//md by dao
#if 0
	{
		.modalias        = "spidev", /* MMC SPI */
		.mode            = SPI_MODE_0,  /* CPOL=0, CPHA=0 */
		.max_speed_hz    = 10000000,
		/* Connected to SPI-0 as 1st Slave */
		.bus_num         = 1,
		.chip_select     = 0,
		.controller_data = &smdk_spi1_csi[SMDK_MMCSPI_CS],
	},
#endif
};
#endif

static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 2,
	.cal_x_max              = 800,
        .cal_y_max              = 480,
        .cal_param              = {
                -13357, -85, 53858048, -95, -8493, 32809514, 65536
        },

};


static void __init smdkv210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
#ifndef CONFIG_S5P_HIGH_RES_TIMERS
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
#endif
	s5p_reserve_bootmem(s5pv210_media_devs,
                        ARRAY_SIZE(s5pv210_media_devs), S5P_RANGE_MFC);
}

#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata smdkc110_hsmmc0_pdata __initdata = {
        .cd_type                = /*S3C_SDHCI_CD_INTERNAL*/S3C_SDHCI_CD_NONE,
        //.wp_gpio                = S5PV210_GPH0(7),
        //.has_wp_gpio    = true,
#if defined(CONFIG_S5PV210_SD_CH0_8BIT)
        .max_width              = 8,
        .host_caps              = MMC_CAP_8_BIT_DATA,
#endif
};      
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata smdkc110_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata smdkc110_hsmmc2_pdata __initdata = {
        .cd_type                = S3C_SDHCI_CD_INTERNAL,
#if defined(CONFIG_S5PV210_SD_CH2_8BIT)
        .max_width              = 8,
        .host_caps              = MMC_CAP_8_BIT_DATA,
#endif
};      
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata smdkc110_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
};
#endif
#if  0
static void __init smdkc110_setup_clocks(void)
{
        struct clk *pclk;
        struct clk *clk;

        /* set MMC0 clock */
        clk = clk_get(&s3c_device_hsmmc0.dev, "sclk_mmc");
        pclk = clk_get(NULL, "mout_mpll");
        clk_set_parent(clk, pclk);
        clk_set_rate(clk, 50*MHZ);

        pr_info("%s: %s: source is %s, rate is %ld\n",
                                __func__, clk->name, clk->parent->name,
                                clk_get_rate(clk));
}
#else
static void setup_hsmmc_clock(struct platform_device *pdev)
{
	struct clk *pclk;
	struct clk *clk;

	/* set HSMMC clock */
	clk = clk_get(&pdev->dev, "sclk_mmc");
	pclk = clk_get(NULL, "mout_mpll");
	clk_set_parent(clk, pclk);
	clk_set_rate(clk, 50*MHZ);

	pr_info("hsmmc%d: %s: source is %s, rate is %ld\n", pdev->id,
			clk->name, clk->parent->name, clk_get_rate(clk));
}

static void __init smdkc110_setup_clocks(void)
{
	setup_hsmmc_clock(&s3c_device_hsmmc0);
	setup_hsmmc_clock(&s3c_device_hsmmc1);
	setup_hsmmc_clock(&s3c_device_hsmmc2);
	setup_hsmmc_clock(&s3c_device_hsmmc3);
}
#endif
/* USB EHCI */
static struct s5p_ehci_platdata smdkv210_ehci_pdata;
static void __init smdkv210_ehci_init(void)
{
        struct s5p_ehci_platdata *pdata = &smdkv210_ehci_pdata;

        s5p_ehci_set_platdata(pdata);
}

/*USB OHCI*/
static struct s5p_ohci_platdata smdkv210_ohci_pdata;
static void __init smdkv210_ohci_init(void)
{
        struct s5p_ohci_platdata *pdata = &smdkv210_ohci_pdata;

        s5p_ohci_set_platdata(pdata);
}

/*USB DEVICE*/
static struct s5p_otg_platdata smdkv210_otg_pdata;
static void __init smdkv210_otg_init(void)
{
        struct s5p_otg_platdata *pdata = &smdkv210_otg_pdata;

        s5p_otg_set_platdata(pdata);
}

static bool console_flushed;

static void flush_console(void)
{
	if (console_flushed)
		return;

	console_flushed = true;

	printk("\n");
	pr_emerg("Restarting %s\n", linux_banner);
	if (!is_console_locked())
		return;

	mdelay(50);

	local_irq_disable();
	if (!console_trylock())
		pr_emerg("flush_console: console was locked! busting!\n");
	else
		pr_emerg("flush_console: console was locked!\n");
	console_unlock();
}

static void smdkc110_pm_restart(char mode, const char *cmd)
{
	flush_console();

	/* On a normal reboot, INFORM6 will contain a small integer
	 * reason code from the notifier hook.  On a panic, it will
	 * contain the 0xee we set at boot.  Write 0xbb to differentiate
	 * a watchdog-timeout-and-reboot (0xee) from a controlled reboot
	 * (0xbb)
	 */
	if (__raw_readl(S5P_INFORM6) == 0xee)
		__raw_writel(0xbb, S5P_INFORM6);

	arm_machine_restart(mode, cmd);
}

static void __init smdkv210_machine_init(void)
{
	arm_pm_restart = smdkc110_pm_restart;
	s3c_pm_init();
#ifdef	CONFIG_CFG80211_WEXT
	gzsd210_wifi_init();
#endif
#ifdef CONFIG_DM9000
	smdkv210_dm9000_init();
#endif
	platform_add_devices(smdkv210_devices, ARRAY_SIZE(smdkv210_devices));

#ifdef CONFIG_ANDROID_PMEM
        android_pmem_set_platdata();
#endif

	samsung_keypad_set_platdata(&smdkv210_keypad_data);
	s3c24xx_ts_set_platdata(&s3c_ts_platform);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, smdkv210_i2c_devs0,
			ARRAY_SIZE(smdkv210_i2c_devs0));
	i2c_register_board_info(1, smdkv210_i2c_devs1,
			ARRAY_SIZE(smdkv210_i2c_devs1));
	i2c_register_board_info(2, smdkv210_i2c_devs2,
		 	ARRAY_SIZE(smdkv210_i2c_devs2));
#ifdef CONFIG_TOUCHSCREEN_EGALAX
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
#endif

	s3c_ide_set_platdata(&smdkv210_ide_pdata);

//	s3c_fb_set_platdata(&smdkv210_lcd0_pdata);
#ifdef CONFIG_FB_S3C_LTE480WV
	s3c_fb_set_platdata(&lte480wv_fb_data);
#endif

#ifdef CONFIG_FB_S3C_AT070TN92
	s3c_fb_set_platdata(&at070tn92_fb_data);
#endif

#ifdef CONFIG_FB_S3C_LCD_NT11003
	s3c_fb_set_platdata(&lcdnt11003_fb_data);
#endif

#ifdef CONFIG_FB_S3C_LCD_HJ101
	s3c_fb_set_platdata(&lcdhj101_fb_data); 
#endif	

#ifdef CONFIG_S3C_DEV_HSMMC
        s3c_sdhci0_set_platdata(&smdkc110_hsmmc0_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
        s3c_sdhci0_set_platdata(&smdkc110_hsmmc1_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
        s3c_sdhci0_set_platdata(&smdkc110_hsmmc2_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
        s3c_sdhci0_set_platdata(&smdkc110_hsmmc3_pdata);
#endif

#ifdef CONFIG_VIDEO_FIMC
        /* fimc */
        s3c_fimc0_set_platdata(&fimc_plat_lsi);
        s3c_fimc1_set_platdata(&fimc_plat_lsi);
        s3c_fimc2_set_platdata(&fimc_plat_lsi);
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	s3c_jpeg_set_platdata(&jpeg_plat);
#endif
#ifdef CONFIG_VIDEO_MFC50
        /* mfc */
        s3c_mfc_set_platdata(NULL);
#endif
        /* spi */
#ifdef CONFIG_SPI_S3C64XX
//md by dao
#if 0
	if (!gpio_request(S5PV210_GPB(1), "SPI_CS0")) {
	    gpio_direction_output(S5PV210_GPB(1), 1);
	    s3c_gpio_cfgpin(S5PV210_GPB(1), S3C_GPIO_SFN(1));
	    s3c_gpio_setpull(S5PV210_GPB(1), S3C_GPIO_PULL_UP);
	    s5pv210_spi_set_info(0, S5PV210_SPI_SRCCLK_PCLK,
		    ARRAY_SIZE(smdk_spi0_csi));
	}

	if (!gpio_request(S5PV210_GPB(5), "SPI_CS1")) {
	    gpio_direction_output(S5PV210_GPB(5), 1);
	    s3c_gpio_cfgpin(S5PV210_GPB(5), S3C_GPIO_SFN(1));
	    s3c_gpio_setpull(S5PV210_GPB(5), S3C_GPIO_PULL_UP);
	    s5pv210_spi_set_info(1, S5PV210_SPI_SRCCLK_PCLK,
		    ARRAY_SIZE(smdk_spi1_csi));
	}
#endif
	spi_register_board_info(s3c_spi_devs, ARRAY_SIZE(s3c_spi_devs));
#endif

	smdkv210_otg_init();
        smdkv210_ehci_init();
        smdkv210_ohci_init();
        clk_xusbxti.rate = 24000000;

	 smdkc110_setup_clocks(); 
}

MACHINE_START(SMDKV210, "SMDKV210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io		= smdkv210_map_io,
	.init_machine	= smdkv210_machine_init,
#ifdef CONFIG_S5P_HIGH_RES_TIMERS
        .timer          = &s5p_systimer,
#else
	.timer		= &s5p_timer,
#endif
MACHINE_END
