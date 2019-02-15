/*
 * amvecm char device driver.
 *
 * Copyright (c) 2010 Frank Zhao<frank.zhao@amlogic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the smems of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

/* Standard Linux headers */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/amlogic/amports/vframe.h>
#include <linux/amlogic/amvecm/amvecm.h>
#include <linux/amlogic/vout/vout_notify.h>
#include "arch/vpp_regs.h"
#include "../tvin/tvin_global.h"
#include "arch/vpp_hdr_regs.h"
#include "arch/hdr_curve.h"

/* use osd rdma reg w/r */
#include "../display/osd/osd_rdma.h"

#include "amcsc.h"

#define pr_csc(fmt, args...)\
	do {\
		if (debug_csc)\
			pr_info(fmt, ## args);\
	} while (0)

signed int vd1_contrast_offset;

signed int saturation_offset;

struct hdr_osd_reg_s hdr_osd_reg = {
	0x00000001, /* VIU_OSD1_MATRIX_CTRL 0x1a90 */
	0x00ba0273, /* VIU_OSD1_MATRIX_COEF00_01 0x1a91 */
	0x003f1f9a, /* VIU_OSD1_MATRIX_COEF02_10 0x1a92 */
	0x1ea801c0, /* VIU_OSD1_MATRIX_COEF11_12 0x1a93 */
	0x01c01e6a, /* VIU_OSD1_MATRIX_COEF20_21 0x1a94 */
	0x00000000, /* VIU_OSD1_MATRIX_COLMOD_COEF42 0x1a95 */
	0x00400200, /* VIU_OSD1_MATRIX_OFFSET0_1 0x1a96 */
	0x00000200, /* VIU_OSD1_MATRIX_PRE_OFFSET2 0x1a97 */
	0x00000000, /* VIU_OSD1_MATRIX_PRE_OFFSET0_1 0x1a98 */
	0x00000000, /* VIU_OSD1_MATRIX_PRE_OFFSET2 0x1a99 */
	0x1fd80000, /* VIU_OSD1_MATRIX_COEF22_30 0x1a9d */
	0x00000000, /* VIU_OSD1_MATRIX_COEF31_32 0x1a9e */
	0x00000000, /* VIU_OSD1_MATRIX_COEF40_41 0x1a9f */
	0x00000000, /* VIU_OSD1_EOTF_CTL 0x1ad4 */
	0x08000000, /* VIU_OSD1_EOTF_COEF00_01 0x1ad5 */
	0x00000000, /* VIU_OSD1_EOTF_COEF02_10 0x1ad6 */
	0x08000000, /* VIU_OSD1_EOTF_COEF11_12 0x1ad7 */
	0x00000000, /* VIU_OSD1_EOTF_COEF20_21 0x1ad8 */
	0x08000001, /* VIU_OSD1_EOTF_COEF22_RS 0x1ad9 */
	0x01c00000, /* VIU_OSD1_OETF_CTL 0x1adc */
	{
		/* eotf table */
		{ /* r map */
			0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0a00,
			0x0c00, 0x0e00, 0x1000, 0x1200, 0x1400, 0x1600,
			0x1800, 0x1a00, 0x1c00, 0x1e00, 0x2000, 0x2200,
			0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
			0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00,
			0x3c00, 0x3e00, 0x4000
		},
		{ /* g map */
			0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0a00,
			0x0c00, 0x0e00, 0x1000, 0x1200, 0x1400, 0x1600,
			0x1800, 0x1a00, 0x1c00, 0x1e00, 0x2000, 0x2200,
			0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
			0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00,
			0x3c00, 0x3e00, 0x4000
		},
		{ /* b map */
			0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0a00,
			0x0c00, 0x0e00, 0x1000, 0x1200, 0x1400, 0x1600,
			0x1800, 0x1a00, 0x1c00, 0x1e00, 0x2000, 0x2200,
			0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
			0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00,
			0x3c00, 0x3e00, 0x4000
		},
		/* oetf table */
		{ /* or map */
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		},
		{ /* og map */
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		},
		{ /* ob map */
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		}
	}
};

static struct vframe_s *dbg_vf;
static struct master_display_info_s dbg_hdr_send;
static struct hdr_info receiver_hdr_info;

static bool debug_csc;
module_param(debug_csc, bool, 0664);
MODULE_PARM_DESC(debug_csc, "\n debug_csc\n");

static bool print_lut_mtx;
module_param(print_lut_mtx, bool, 0664);
MODULE_PARM_DESC(print_lut_mtx, "\n print_lut_mtx\n");

static bool skip_csc_en;
module_param(skip_csc_en, bool, 0664);
MODULE_PARM_DESC(skip_csc_en, "\n skip_csc_en\n");

/* white balance adjust */
static bool cur_eye_protect_mode;

static int num_wb_val = 10;
static int wb_val[10] = {
	0, /* wb enable */
	0, /* -1024~1023, r_pre_offset */
	0, /* -1024~1023, g_pre_offset */
	0, /* -1024~1023, b_pre_offset */
	1024, /* 0~2047, r_gain */
	1024, /* 0~2047, g_gain */
	1024, /* 0~2047, b_gain */
	0, /* -1024~1023, r_post_offset */
	0, /* -1024~1023, g_post_offset */
	0  /* -1024~1023, b_post_offset */
};
module_param_array(wb_val, int, &num_wb_val, 0664);
MODULE_PARM_DESC(wb_val, "\n white balance setting\n");

static enum vframe_source_type_e pre_src_type = VFRAME_SOURCE_TYPE_COMP;
static uint cur_csc_type = 0xffff;
module_param(cur_csc_type, uint, 0444);
MODULE_PARM_DESC(cur_csc_type, "\n current color space convert type\n");

static uint hdr_mode = 2; /* 0: hdr->hdr, 1:hdr->sdr, 2:auto */
module_param(hdr_mode, uint, 0664);
MODULE_PARM_DESC(hdr_mode, "\n set hdr_mode\n");

static uint hdr_process_mode = 1; /* 0: hdr->hdr, 1:hdr->sdr */
static uint cur_hdr_process_mode = 2; /* 0: hdr->hdr, 1:hdr->sdr */
module_param(hdr_process_mode, uint, 0444);
MODULE_PARM_DESC(hdr_process_mode, "\n current hdr_process_mode\n");

uint sdr_mode = 0; /* 0: sdr->sdr, 1:sdr->hdr, 2:auto */
static uint sdr_process_mode = 2; /* 0: sdr->sdr, 1:sdr->hdr */
static uint cur_sdr_process_mode = 2; /* 0: sdr->sdr, 1:sdr->hdr */
static int sdr_saturation_offset = 20; /* 0: sdr->sdr, 1:sdr->hdr */
module_param(sdr_mode, uint, 0664);
MODULE_PARM_DESC(sdr_mode, "\n set sdr_mode\n");
module_param(sdr_process_mode, uint, 0444);
MODULE_PARM_DESC(sdr_process_mode, "\n current hdr_process_mode\n");
module_param(sdr_saturation_offset, int, 0664);
MODULE_PARM_DESC(sdr_saturation_offset, "\n add saturation\n");

static uint force_csc_type = 0xff;
module_param(force_csc_type, uint, 0664);
MODULE_PARM_DESC(force_csc_type, "\n force colour space convert type\n");

static uint cur_hdr_support;
module_param(cur_hdr_support, uint, 0664);
MODULE_PARM_DESC(cur_hdr_support, "\n cur_hdr_support\n");

static uint range_control = 0;
module_param(range_control, uint, 0664);
MODULE_PARM_DESC(range_control, "\n range_control 0:limit-limit 1:full-limit 2:limit-full 3:full-full\n");

static uint cur_range_control = 0;
uint get_range_control(void){
	return range_control;
}
EXPORT_SYMBOL(get_range_control);
/* bit 0: use source primary,
   bit 1: use display primary,
   bit 2: adjust contrast according to source lumin,
   bit 3: adjust saturation according to source lumin */
uint hdr_flag = (1 << 0) | (1 << 1) | (0 << 2) | (0 << 3);
module_param(hdr_flag, uint, 0664);
MODULE_PARM_DESC(hdr_flag, "\n set hdr_flag\n");

static uint rdma_flag =
	(1 << VPP_MATRIX_XVYCC);
module_param(rdma_flag, uint, 0664);
MODULE_PARM_DESC(rdma_flag, "\n set rdma_flag\n");

#define MAX_KNEE_SETTING	35
/* recommended setting for 100 nits panel: */
/* 0,16,96,224,320,544,720,864,1000,1016,1023 */
/* knee factor = 256 */
static int num_knee_setting = MAX_KNEE_SETTING;
static int knee_setting[MAX_KNEE_SETTING] = {
	/* 0, 16, 96, 224, 320, 544, 720, 864, 1000, 1016, 1023 */
	0, 16, 36, 59, 71, 96,
	120, 145, 170, 204, 230, 258,
	288, 320, 355, 390, 428, 470,
	512, 554, 598, 650, 720, 758,
	790, 832, 864, 894, 920, 945,
	968, 980, 1000, 1016, 1023
};

static int num_knee_linear_setting = MAX_KNEE_SETTING;
static int knee_linear_setting[MAX_KNEE_SETTING] = {
	0x000,
	0x010,
	0x02f,
	0x04e,
	0x06d,
	0x08c,
	0x0ab,
	0x0ca,
	0x0e9,
	0x108,
	0x127,
	0x146,
	0x165,
	0x184,
	0x1a3,
	0x1c2,
	0x1e1,
	0x200,
	0x21f,
	0x23e,
	0x25d,
	0x27c,
	0x29b,
	0x2ba,
	0x2d9,
	0x2f8,
	0x317,
	0x336,
	0x355,
	0x374,
	0x393,
	0x3b2,
	0x3d1,
	0x3f0,
	0x3ff
};

static bool lut_289_en = 1;
module_param(lut_289_en, bool, 0664);
MODULE_PARM_DESC(lut_289_en, "\n if enable 289 lut\n");

/*for gxtvbb(968), only 289 point lut for hdr curve set */
/*default for 350nit panel*/
unsigned int lut_289_mapping[LUT_289_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0x9, 0xa, 0xb, 0xd, 0xe, 0x10, 0x11, 0x12,
	  0x14, 0x15, 0x17, 0x18, 0x1a, 0x1c, 0x1d, 0x1f,
	  0x20, 0x22, 0x24, 0x26, 0x27, 0x29, 0x2b, 0x2d,
	 0x2f,  0x31,  0x33,  0x35,  0x37,  0x39,  0x3b,  0x3d,
	 0x40,  0x42,  0x44,  0x46,  0x49,  0x4b,  0x4e,  0x50,
	 0x53,  0x55,  0x58,  0x5a,  0x5d,  0x60,  0x62,  0x65,
	 0x68,  0x6b,  0x6e,  0x71,  0x74,  0x77,  0x7a,  0x7e,
	 0x81,  0x84,  0x87,  0x8b,  0x8e,  0x92,  0x95,  0x99,
	 0x9d,  0xa1,  0xa4,  0xa8,  0xac,  0xb0,  0xb4,  0xb8,
	 0xbc,  0xc1,  0xc5,  0xc9,  0xce,  0xd2,  0xd7,  0xdc,
	 0xe0,  0xe5,  0xea,  0xef,  0xf4,  0xf9,  0xfe,  0x104,
	 0x109,  0x10e,  0x114,  0x11a,  0x11f,  0x125,  0x12b,  0x131,
	 0x137,  0x13d,  0x143,  0x14a,  0x150,  0x156,  0x15d,  0x164,
	 0x16b,  0x172,  0x179,  0x180,  0x187,  0x18e,  0x196,  0x19d,
	 0x1a5,  0x1ad,  0x1b5,  0x1bd,  0x1c5,  0x1cd,  0x1d6,  0x1de,
	 0x1e7,  0x1f0,  0x1f9,  0x202,  0x20b,  0x214,  0x21e,  0x227,
	 0x231,  0x23b,  0x245,  0x24f,  0x25a,  0x264,  0x26f,  0x27a,
	 0x285,  0x290,  0x29c,  0x2a7,  0x2b3,  0x2bf,  0x2cb,  0x2d7,
	 0x2e3,  0x2f0,  0x2fd,  0x30a,  0x317,  0x325,  0x332,  0x33e,
	 0x34a,  0x356,  0x362,  0x36d,  0x377,  0x381,  0x38b,  0x394,
	 0x39c,  0x3a4,  0x3ac,  0x3b3,  0x3ba,  0x3c1,  0x3c7,  0x3cd,
	 0x3d2,  0x3d7,  0x3db,  0x3df,  0x3e3,  0x3e6,  0x3e9,  0x3ec,
	 0x3ef,  0x3f1,  0x3f3,  0x3f5,  0x3f6,  0x3f7,  0x3f8,  0x3f9,
	 0x3fa,  0x3fa,  0x3fb,  0x3fb,  0x3fb,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,  0x3fc,
	 0x3fc, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,
	0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff,
	0x3ff
};

static int knee_factor; /* 0 ~ 256, 128 = 0.5 */
static int knee_interpolation_mode = 1; /* 0: linear, 1: cubic */

module_param_array(knee_setting, int, &num_knee_setting, 0664);
MODULE_PARM_DESC(knee_setting, "\n knee_setting, 256=1.0\n");

module_param_array(knee_linear_setting, int, &num_knee_linear_setting, 0444);
MODULE_PARM_DESC(knee_linear_setting, "\n reference linear knee_setting\n");

module_param(knee_factor, int, 0664);
MODULE_PARM_DESC(knee_factor, "\n knee_factor, 255=1.0\n");

module_param(knee_interpolation_mode, int, 0664);
MODULE_PARM_DESC(knee_interpolation_mode, "\n 0: linear, 1: cubic\n");

#define NUM_MATRIX_PARAM 16
static uint num_customer_matrix_param = NUM_MATRIX_PARAM;
static uint customer_matrix_param[NUM_MATRIX_PARAM] = {
	0, 0, 0,
	0x0d49, 0x1b4d, 0x1f6b,
	0x1f01, 0x0910, 0x1fef,
	0x1fdb, 0x1f32, 0x08f3,
	0, 0, 0,
	1
};

static bool customer_matrix_en = true;

#define INORM	50000
#define BL		16

static u32 bt709_primaries[3][2] = {
	{0.30 * INORM + 0.5, 0.60 * INORM + 0.5},	/* G */
	{0.15 * INORM + 0.5, 0.06 * INORM + 0.5},	/* B */
	{0.64 * INORM + 0.5, 0.33 * INORM + 0.5},	/* R */
};

static u32 bt709_white_point[2] = {
	0.3127 * INORM + 0.5, 0.3290 * INORM + 0.5
};

static u32 bt2020_primaries[3][2] = {
	{0.17 * INORM + 0.5, 0.797 * INORM + 0.5},	/* G */
	{0.131 * INORM + 0.5, 0.046 * INORM + 0.5},	/* B */
	{0.708 * INORM + 0.5, 0.292 * INORM + 0.5},	/* R */
};

static u32 bt2020_white_point[2] = {
	0.3127 * INORM + 0.5, 0.3290 * INORM + 0.5
};

/* 0: off, 1: on */
static int customer_master_display_en;
static uint num_customer_master_display_param = 12;
static uint customer_master_display_param[12] = {
	0.17 * INORM + 0.5, 0.797 * INORM + 0.5,      /* G */
	0.131 * INORM + 0.5, 0.046 * INORM + 0.5,     /* B */
	0.708 * INORM + 0.5, 0.292 * INORM + 0.5,     /* R */
	0.3127 * INORM + 0.5, 0.3290 * INORM + 0.5,   /* W */
	5000 * 10000, 50,
	/* man/min lumin */
	5000, 50
	/* content lumin and frame average */
};

module_param(customer_matrix_en, bool, 0664);
MODULE_PARM_DESC(customer_matrix_en, "\n if enable customer matrix\n");

module_param_array(customer_matrix_param, uint,
	&num_customer_matrix_param, 0664);
MODULE_PARM_DESC(customer_matrix_param,
	"\n matrix from source primary to panel primary\n");

module_param(customer_master_display_en, int, 0664);
MODULE_PARM_DESC(customer_master_display_en,
	"\n if enable customer primaries and white point\n");

module_param_array(customer_master_display_param, uint,
	&num_customer_master_display_param, 0664);
MODULE_PARM_DESC(customer_master_display_param,
	"\n matrix from source primary and white point\n");

/* 0: off, 1: on */
static int customer_hdmi_display_en;
static uint num_customer_hdmi_display_param = 14;
static uint customer_hdmi_display_param[14] = {
	9, /* color priamry = bt2020 */
	16, /* characteristic = st2084 */
	0.17 * INORM + 0.5, 0.797 * INORM + 0.5,      /* G */
	0.131 * INORM + 0.5, 0.046 * INORM + 0.5,     /* B */
	0.708 * INORM + 0.5, 0.292 * INORM + 0.5,     /* R */
	0.3127 * INORM + 0.5, 0.3290 * INORM + 0.5,   /* W */
	9997 * 10000, 0,
	/* man/min lumin */
	5000, 50
	/* content lumin and frame average */
};

module_param(customer_hdmi_display_en, int, 0664);
MODULE_PARM_DESC(customer_hdmi_display_en,
	"\n if enable customer primaries and white point\n");

module_param_array(customer_hdmi_display_param, uint,
	&num_customer_hdmi_display_param, 0664);
MODULE_PARM_DESC(customer_hdmi_display_param,
	"\n matrix from source primary and white point\n");

/* sat offset when > 1200 and <= 1200 */
static uint num_extra_sat_lut = 2;
static uint extra_sat_lut[] = {16, 32};
module_param_array(extra_sat_lut, uint,
	&num_extra_sat_lut, 0664);
MODULE_PARM_DESC(extra_sat_lut,
	 "\n lookup table for saturation match source luminance.\n");

/* norm to 128 as 1, LUT can be changed */
static uint num_extra_con_lut = 5;
static uint extra_con_lut[] = {144, 136, 132, 130, 128};
module_param_array(extra_con_lut, uint,
	&num_extra_con_lut, 0664);
MODULE_PARM_DESC(extra_con_lut,
	 "\n lookup table for contrast match source luminance.\n");

#define clip(y, ymin, ymax) ((y > ymax) ? ymax : ((y < ymin) ? ymin : y))
static const int coef[] = {
	  0,   256,     0,     0, /* phase 0  */
	 -2,   256,     2,     0, /* phase 1  */
	 -4,   256,     4,     0, /* phase 2  */
	 -5,   254,     7,     0, /* phase 3  */
	 -7,   254,    10,    -1, /* phase 4  */
	 -8,   252,    13,    -1, /* phase 5  */
	-10,   251,    16,    -1, /* phase 6  */
	-11,   249,    19,    -1, /* phase 7  */
	-12,   247,    23,    -2, /* phase 8  */
	-13,   244,    27,    -2, /* phase 9  */
	-14,   242,    31,    -3, /* phase 10 */
	-15,   239,    35,    -3, /* phase 11 */
	-16,   236,    40,    -4, /* phase 12 */
	-17,   233,    44,    -4, /* phase 13 */
	-17,   229,    49,    -5, /* phase 14 */
	-18,   226,    53,    -5, /* phase 15 */
	-18,   222,    58,    -6, /* phase 16 */
	-18,   218,    63,    -7, /* phase 17 */
	-19,   214,    68,    -7, /* phase 18 */
	-19,   210,    73,    -8, /* phase 19 */
	-19,   205,    79,    -9, /* phase 20 */
	-19,   201,    83,    -9, /* phase 21 */
	-19,   196,    89,   -10, /* phase 22 */
	-19,   191,    94,   -10, /* phase 23 */
	-19,   186,   100,   -11, /* phase 24 */
	-18,   181,   105,   -12, /* phase 25 */
	-18,   176,   111,   -13, /* phase 26 */
	-18,   171,   116,   -13, /* phase 27 */
	-18,   166,   122,   -14, /* phase 28 */
	-17,   160,   127,   -14, /* phase 28 */
	-17,   155,   133,   -15, /* phase 30 */
	-16,   149,   138,   -15, /* phase 31 */
	-16,   144,   144,   -16  /* phase 32 */
};

int cubic_interpolation(int y0, int y1, int y2, int y3, int mu)
{
	int c0, c1, c2, c3;
	int d0, d1, d2, d3;

	if (mu <= 32) {
		c0 = coef[(mu << 2) + 0];
		c1 = coef[(mu << 2) + 1];
		c2 = coef[(mu << 2) + 2];
		c3 = coef[(mu << 2) + 3];
		d0 = y0; d1 = y1; d2 = y2; d3 = y3;
	} else {
		c0 = coef[((64 - mu) << 2) + 0];
		c1 = coef[((64 - mu) << 2) + 1];
		c2 = coef[((64 - mu) << 2) + 2];
		c3 = coef[((64 - mu) << 2) + 3];
		d0 = y3; d1 = y2; d2 = y1; d3 = y0;
	}
	return (d0 * c0 + d1 * c1 + d2 * c2 + d3 * c3 + 128) >> 8;
}

static int knee_lut_on;
static int cur_knee_factor = -1;
static void load_knee_lut(int on)
{
	int i, j, k;
	int value;
	int final_knee_setting[MAX_KNEE_SETTING];

	if ((cur_knee_factor != knee_factor) && (!lut_289_en)) {
		pr_csc("Knee_factor changed from %d to %d\n",
			cur_knee_factor, knee_factor);
		for (i = 0; i < MAX_KNEE_SETTING; i++) {
			final_knee_setting[i] =
				knee_linear_setting[i] + (((knee_setting[i]
				- knee_linear_setting[i]) * knee_factor) >> 8);
			if (final_knee_setting[i] > 0x3ff)
				final_knee_setting[i] = 0x3ff;
			else if (final_knee_setting[i] < 0)
				final_knee_setting[i] = 0;
		}
		WRITE_VPP_REG(XVYCC_LUT_CTL, 0x0);
		for (j = 0; j < 3; j++) {
			for (i = 0; i < 16; i++) {
				WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT + 2 * j, i);
				value = final_knee_setting[0]
					+ (((final_knee_setting[1]
					- final_knee_setting[0]) * i) >> 4);
				value = clip(value, 0, 0x3ff);
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT + 2 * j,
						value);
				if (j == 0)
					pr_csc("xvycc_lut[%1d][%3d] = 0x%03x\n",
							j, i, value);
			}
			for (i = 16; i < 272; i++) {
				k = 1 + ((i - 16) >> 3);
				WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT + 2 * j, i);
				if (knee_interpolation_mode == 0)
					value = final_knee_setting[k]
						+ (((final_knee_setting[k+1]
						- final_knee_setting[k])
						* ((i - 16) & 0x7)) >> 3);
				else
					value = cubic_interpolation(
						final_knee_setting[k-1],
						final_knee_setting[k],
						final_knee_setting[k+1],
						final_knee_setting[k+2],
						((i - 16) & 0x7) << 3);
				value = clip(value, 0, 0x3ff);
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT + 2 * j,
						value);
				if (j == 0)
					pr_csc("xvycc_lut[%1d][%3d] = 0x%03x\n",
							j, i, value);
			}
			for (i = 272; i < 289; i++) {
				k = MAX_KNEE_SETTING - 2;
				WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT + 2 * j, i);
				value = final_knee_setting[k]
					+ (((final_knee_setting[k+1]
					- final_knee_setting[k])
					* (i - 272)) >> 4);
				value = clip(value, 0, 0x3ff);
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT + 2 * j,
						value);
				if (j == 0)
					pr_csc("xvycc_lut[%1d][%3d] = 0x%03x\n",
							j, i, value);
			}
		}
		cur_knee_factor = knee_factor;
	}

	if ((cur_knee_factor != knee_factor) && (lut_289_en)) {
			WRITE_VPP_REG(XVYCC_LUT_CTL, 0x0);
			WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT, 0);
			for (i = 0; i < LUT_289_SIZE; i++)
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT,
					lut_289_mapping[i]);
			WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT + 2, 0);
			for (i = 0; i < LUT_289_SIZE; i++)
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT + 2,
					lut_289_mapping[i]);
			WRITE_VPP_REG(XVYCC_LUT_R_ADDR_PORT + 4, 0);
			for (i = 0; i < LUT_289_SIZE; i++)
				WRITE_VPP_REG(XVYCC_LUT_R_DATA_PORT + 4,
					lut_289_mapping[i]);
		cur_knee_factor = knee_factor;
	}

	if (on) {
		WRITE_VPP_REG(XVYCC_LUT_CTL, 0x7f);
		knee_lut_on = 1;
	} else {
		WRITE_VPP_REG(XVYCC_LUT_CTL, 0x0f);
		knee_lut_on = 0;
	}
}

/***************************** gxl hdr ****************************/
#define EOTF_INV_LUT_NEG2048_SIZE 16 /* 16 for [-2048, 0), 32 for [0, 1024), 17 for [1024, 2048) */
#define EOTF_INV_LUT_SIZE 32
#define EOTF_INV_LUT_1024_SIZE 17

static unsigned int num_invlut_neg_mapping = EOTF_INV_LUT_NEG2048_SIZE;
static int invlut_y_neg[EOTF_INV_LUT_NEG2048_SIZE] = {
	-2048, -1920, -1792, -1664,
	-1536, -1408, -1280, -1152,
	-1024, -896, -768, -640,
	-512, -384, -256, -128
};

static unsigned int num_invlut_mapping = EOTF_INV_LUT_SIZE;
static unsigned int invlut_y[EOTF_INV_LUT_SIZE] = {
	0, 32, 64, 96, 128, 160, 192, 224,
	256, 288, 320, 352, 384, 416, 448, 480,
	512, 544, 576, 608, 640, 672, 704, 736,
	768, 800, 832, 864, 896, 928, 960, 992
};

static unsigned int num_invlut_1024_mapping = EOTF_INV_LUT_1024_SIZE;
static unsigned int invlut_y_1024[EOTF_INV_LUT_1024_SIZE] = {
	1024, 1088, 1152, 1216,
	1280, 1344, 1408, 1472,
	1536, 1600, 1664, 1728,
	1792, 1856, 1920, 1984,
	2047
};

#define EOTF_LUT_SIZE 33
static unsigned int num_osd_eotf_r_mapping = EOTF_LUT_SIZE;
static unsigned int osd_eotf_r_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

static unsigned int num_osd_eotf_g_mapping = EOTF_LUT_SIZE;
static unsigned int osd_eotf_g_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

static unsigned int num_osd_eotf_b_mapping = EOTF_LUT_SIZE;
static unsigned int osd_eotf_b_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

static unsigned int num_video_eotf_r_mapping = EOTF_LUT_SIZE;
static unsigned int video_eotf_r_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

static unsigned int num_video_eotf_g_mapping = EOTF_LUT_SIZE;
static unsigned int video_eotf_g_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

static unsigned int num_video_eotf_b_mapping = EOTF_LUT_SIZE;
static unsigned int video_eotf_b_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

#define EOTF_COEFF_NORM(a) ((int)((((a) * 4096.0) + 1) / 2))
#define EOTF_COEFF_SIZE 10
#define EOTF_COEFF_RIGHTSHIFT 1
static unsigned int num_osd_eotf_coeff = EOTF_COEFF_SIZE;
static int osd_eotf_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(1.0), EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(1.0), EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(1.0),
	EOTF_COEFF_RIGHTSHIFT /* right shift */
};

static unsigned int num_video_eotf_coeff = EOTF_COEFF_SIZE;
static int video_eotf_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(1.0), EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(1.0), EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(0.0), EOTF_COEFF_NORM(1.0),
	EOTF_COEFF_RIGHTSHIFT /* right shift */
};

static unsigned int reload_mtx;
static unsigned int reload_lut;

/******************** osd oetf **************/

static unsigned int num_osd_oetf_r_mapping = OSD_OETF_LUT_SIZE;
static unsigned int osd_oetf_r_mapping[OSD_OETF_LUT_SIZE] = {
		0, 4, 8, 12,
		16, 20, 24, 28,
		31, 62, 93, 124,
		155, 186, 217, 248,
		279, 310, 341, 372,
		403, 434, 465, 496,
		527, 558, 589, 620,
		651, 682, 713, 744,
		775, 806, 837, 868,
		899, 930, 961, 992,
		1023
};

static unsigned int num_osd_oetf_g_mapping = OSD_OETF_LUT_SIZE;
static unsigned int osd_oetf_g_mapping[OSD_OETF_LUT_SIZE] = {
		0, 4, 8, 12,
		16, 20, 24, 28,
		31, 62, 93, 124,
		155, 186, 217, 248,
		279, 310, 341, 372,
		403, 434, 465, 496,
		527, 558, 589, 620,
		651, 682, 713, 744,
		775, 806, 837, 868,
		899, 930, 961, 992,
		1023
};

static unsigned int num_osd_oetf_b_mapping = OSD_OETF_LUT_SIZE;
static unsigned int osd_oetf_b_mapping[OSD_OETF_LUT_SIZE] = {
		0, 4, 8, 12,
		16, 20, 24, 28,
		31, 62, 93, 124,
		155, 186, 217, 248,
		279, 310, 341, 372,
		403, 434, 465, 496,
		527, 558, 589, 620,
		651, 682, 713, 744,
		775, 806, 837, 868,
		899, 930, 961, 992,
		1023
};

/************ video oetf ***************/

#define VIDEO_OETF_LUT_SIZE 289
static unsigned int num_video_oetf_r_mapping = VIDEO_OETF_LUT_SIZE;
static unsigned int video_oetf_r_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   4,    8,   12,   16,   20,   24,   28,   32,
	  36,   40,   44,   48,   52,   56,   60,   64,
	  68,   72,   76,   80,   84,   88,   92,   96,
	 100,  104,  108,  112,  116,  120,  124,  128,
	 132,  136,  140,  144,  148,  152,  156,  160,
	 164,  168,  172,  176,  180,  184,  188,  192,
	 196,  200,  204,  208,  212,  216,  220,  224,
	 228,  232,  236,  240,  244,  248,  252,  256,
	 260,  264,  268,  272,  276,  280,  284,  288,
	 292,  296,  300,  304,  308,  312,  316,  320,
	 324,  328,  332,  336,  340,  344,  348,  352,
	 356,  360,  364,  368,  372,  376,  380,  384,
	 388,  392,  396,  400,  404,  408,  412,  416,
	 420,  424,  428,  432,  436,  440,  444,  448,
	 452,  456,  460,  464,  468,  472,  476,  480,
	 484,  488,  492,  496,  500,  504,  508,  512,
	 516,  520,  524,  528,  532,  536,  540,  544,
	 548,  552,  556,  560,  564,  568,  572,  576,
	 580,  584,  588,  592,  596,  600,  604,  608,
	 612,  616,  620,  624,  628,  632,  636,  640,
	 644,  648,  652,  656,  660,  664,  668,  672,
	 676,  680,  684,  688,  692,  696,  700,  704,
	 708,  712,  716,  720,  724,  728,  732,  736,
	 740,  744,  748,  752,  756,  760,  764,  768,
	 772,  776,  780,  784,  788,  792,  796,  800,
	 804,  808,  812,  816,  820,  824,  828,  832,
	 836,  840,  844,  848,  852,  856,  860,  864,
	 868,  872,  876,  880,  884,  888,  892,  896,
	 900,  904,  908,  912,  916,  920,  924,  928,
	 932,  936,  940,  944,  948,  952,  956,  960,
	 964,  968,  972,  976,  980,  984,  988,  992,
	 996, 1000, 1004, 1008, 1012, 1016, 1020, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

static unsigned int num_video_oetf_g_mapping = VIDEO_OETF_LUT_SIZE;
static unsigned int video_oetf_g_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   4,    8,   12,   16,   20,   24,   28,   32,
	  36,   40,   44,   48,   52,   56,   60,   64,
	  68,   72,   76,   80,   84,   88,   92,   96,
	 100,  104,  108,  112,  116,  120,  124,  128,
	 132,  136,  140,  144,  148,  152,  156,  160,
	 164,  168,  172,  176,  180,  184,  188,  192,
	 196,  200,  204,  208,  212,  216,  220,  224,
	 228,  232,  236,  240,  244,  248,  252,  256,
	 260,  264,  268,  272,  276,  280,  284,  288,
	 292,  296,  300,  304,  308,  312,  316,  320,
	 324,  328,  332,  336,  340,  344,  348,  352,
	 356,  360,  364,  368,  372,  376,  380,  384,
	 388,  392,  396,  400,  404,  408,  412,  416,
	 420,  424,  428,  432,  436,  440,  444,  448,
	 452,  456,  460,  464,  468,  472,  476,  480,
	 484,  488,  492,  496,  500,  504,  508,  512,
	 516,  520,  524,  528,  532,  536,  540,  544,
	 548,  552,  556,  560,  564,  568,  572,  576,
	 580,  584,  588,  592,  596,  600,  604,  608,
	 612,  616,  620,  624,  628,  632,  636,  640,
	 644,  648,  652,  656,  660,  664,  668,  672,
	 676,  680,  684,  688,  692,  696,  700,  704,
	 708,  712,  716,  720,  724,  728,  732,  736,
	 740,  744,  748,  752,  756,  760,  764,  768,
	 772,  776,  780,  784,  788,  792,  796,  800,
	 804,  808,  812,  816,  820,  824,  828,  832,
	 836,  840,  844,  848,  852,  856,  860,  864,
	 868,  872,  876,  880,  884,  888,  892,  896,
	 900,  904,  908,  912,  916,  920,  924,  928,
	 932,  936,  940,  944,  948,  952,  956,  960,
	 964,  968,  972,  976,  980,  984,  988,  992,
	 996, 1000, 1004, 1008, 1012, 1016, 1020, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

static unsigned int num_video_oetf_b_mapping = VIDEO_OETF_LUT_SIZE;
static unsigned int video_oetf_b_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   4,    8,   12,   16,   20,   24,   28,   32,
	  36,   40,   44,   48,   52,   56,   60,   64,
	  68,   72,   76,   80,   84,   88,   92,   96,
	 100,  104,  108,  112,  116,  120,  124,  128,
	 132,  136,  140,  144,  148,  152,  156,  160,
	 164,  168,  172,  176,  180,  184,  188,  192,
	 196,  200,  204,  208,  212,  216,  220,  224,
	 228,  232,  236,  240,  244,  248,  252,  256,
	 260,  264,  268,  272,  276,  280,  284,  288,
	 292,  296,  300,  304,  308,  312,  316,  320,
	 324,  328,  332,  336,  340,  344,  348,  352,
	 356,  360,  364,  368,  372,  376,  380,  384,
	 388,  392,  396,  400,  404,  408,  412,  416,
	 420,  424,  428,  432,  436,  440,  444,  448,
	 452,  456,  460,  464,  468,  472,  476,  480,
	 484,  488,  492,  496,  500,  504,  508,  512,
	 516,  520,  524,  528,  532,  536,  540,  544,
	 548,  552,  556,  560,  564,  568,  572,  576,
	 580,  584,  588,  592,  596,  600,  604,  608,
	 612,  616,  620,  624,  628,  632,  636,  640,
	 644,  648,  652,  656,  660,  664,  668,  672,
	 676,  680,  684,  688,  692,  696,  700,  704,
	 708,  712,  716,  720,  724,  728,  732,  736,
	 740,  744,  748,  752,  756,  760,  764,  768,
	 772,  776,  780,  784,  788,  792,  796,  800,
	 804,  808,  812,  816,  820,  824,  828,  832,
	 836,  840,  844,  848,  852,  856,  860,  864,
	 868,  872,  876,  880,  884,  888,  892,  896,
	 900,  904,  908,  912,  916,  920,  924,  928,
	 932,  936,  940,  944,  948,  952,  956,  960,
	 964,  968,  972,  976,  980,  984,  988,  992,
	 996, 1000, 1004, 1008, 1012, 1016, 1020, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

#define COEFF_NORM(a) ((int)((((a) * 2048.0) + 1) / 2))
#define MATRIX_5x3_COEF_SIZE 24
/******* osd1 matrix0 *******/
/* default rgb to yuv_full */
static unsigned int num_osd_matrix_coeff = MATRIX_5x3_COEF_SIZE;
static int osd_matrix_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(0.2126),	COEFF_NORM(0.7152),	COEFF_NORM(0.0722),
	COEFF_NORM(-0.11457),	COEFF_NORM(-0.38543),	COEFF_NORM(0.5),
	COEFF_NORM(0.5),	COEFF_NORM(-0.45415),	COEFF_NORM(-0.045847),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static unsigned int num_vd1_matrix_coeff = MATRIX_5x3_COEF_SIZE;
static int vd1_matrix_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(1.0),	COEFF_NORM(0.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(1.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(0.0),	COEFF_NORM(1.0),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static unsigned int num_vd2_matrix_coeff = MATRIX_5x3_COEF_SIZE;
static int vd2_matrix_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(1.0),	COEFF_NORM(0.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(1.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(0.0),	COEFF_NORM(1.0),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static unsigned int num_post_matrix_coeff = MATRIX_5x3_COEF_SIZE;
static int post_matrix_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(1.0),	COEFF_NORM(0.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(1.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(0.0),	COEFF_NORM(1.0),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static unsigned int num_xvycc_matrix_coeff = MATRIX_5x3_COEF_SIZE;
static int xvycc_matrix_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(1.0),	COEFF_NORM(0.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(1.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(0.0),	COEFF_NORM(1.0),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

/****************** osd eotf ********************/
module_param_array(osd_eotf_coeff, int,
	&num_osd_eotf_coeff, 0664);
MODULE_PARM_DESC(osd_eotf_coeff, "\n matrix for osd eotf\n");

module_param_array(osd_eotf_r_mapping, uint,
	&num_osd_eotf_r_mapping, 0664);
MODULE_PARM_DESC(osd_eotf_r_mapping, "\n lut for osd r eotf\n");

module_param_array(osd_eotf_g_mapping, uint,
	&num_osd_eotf_g_mapping, 0664);
MODULE_PARM_DESC(osd_eotf_g_mapping, "\n lut for osd g eotf\n");

module_param_array(osd_eotf_b_mapping, uint,
	&num_osd_eotf_b_mapping, 0664);
MODULE_PARM_DESC(osd_eotf_b_mapping, "\n lut for osd b eotf\n");

module_param_array(osd_matrix_coeff, int,
	&num_osd_matrix_coeff, 0664);
MODULE_PARM_DESC(osd_matrix_coeff, "\n coef for osd matrix\n");

/****************** video eotf ********************/
module_param_array(invlut_y_neg, int,
	&num_invlut_neg_mapping, 0664);
MODULE_PARM_DESC(invlut_y_neg, "\n lut for inv y -2048..0 eotf\n");

module_param_array(invlut_y, uint,
	&num_invlut_mapping, 0664);
MODULE_PARM_DESC(invlut_y, "\n lut for inv y 0..1024 eotf\n");

module_param_array(invlut_y_1024, uint,
	&num_invlut_1024_mapping, 0664);
MODULE_PARM_DESC(invlut_y_1024, "\n lut for inv y 1024..2048 eotf\n");

module_param_array(video_eotf_coeff, int,
	&num_video_eotf_coeff, 0664);
MODULE_PARM_DESC(video_eotf_coeff, "\n matrix for video eotf\n");

module_param_array(video_eotf_r_mapping, uint,
	&num_video_eotf_r_mapping, 0664);
MODULE_PARM_DESC(video_eotf_r_mapping, "\n lut for video r eotf\n");

module_param_array(video_eotf_g_mapping, uint,
	&num_video_eotf_g_mapping, 0664);
MODULE_PARM_DESC(video_eotf_g_mapping, "\n lut for video g eotf\n");

module_param_array(video_eotf_b_mapping, uint,
	&num_video_eotf_b_mapping, 0664);
MODULE_PARM_DESC(video_eotf_b_mapping, "\n lut for video b eotf\n");

/****************** osd oetf ********************/
module_param_array(osd_oetf_r_mapping, uint,
	&num_osd_oetf_r_mapping, 0664);
MODULE_PARM_DESC(osd_oetf_r_mapping, "\n lut for osd r oetf\n");

module_param_array(osd_oetf_g_mapping, uint,
	&num_osd_oetf_g_mapping, 0664);
MODULE_PARM_DESC(osd_oetf_g_mapping, "\n lut for osd g oetf\n");

module_param_array(osd_oetf_b_mapping, uint,
	&num_osd_oetf_b_mapping, 0664);
MODULE_PARM_DESC(osd_oetf_b_mapping, "\n lut for osd b oetf\n");

/****************** video oetf ********************/
module_param_array(video_oetf_r_mapping, uint,
	&num_video_oetf_r_mapping, 0664);
MODULE_PARM_DESC(video_oetf_r_mapping, "\n lut for video r oetf\n");

module_param_array(video_oetf_g_mapping, uint,
	&num_video_oetf_g_mapping, 0664);
MODULE_PARM_DESC(video_oetf_g_mapping, "\n lut for video g oetf\n");

module_param_array(video_oetf_b_mapping, uint,
	&num_video_oetf_b_mapping, 0664);
MODULE_PARM_DESC(video_oetf_b_mapping, "\n lut for video b oetf\n");

/****************** vpp matrix ********************/

module_param_array(vd1_matrix_coeff, int,
	&num_vd1_matrix_coeff, 0664);
MODULE_PARM_DESC(vd1_matrix_coeff, "\n vd1 matrix\n");

module_param_array(vd2_matrix_coeff, int,
	&num_vd2_matrix_coeff, 0664);
MODULE_PARM_DESC(vd2_matrix_coeff, "\n vd2 matrix\n");

module_param_array(post_matrix_coeff, int,
	&num_post_matrix_coeff, 0664);
MODULE_PARM_DESC(post_matrix_coeff, "\n post matrix\n");

module_param_array(xvycc_matrix_coeff, int,
	&num_xvycc_matrix_coeff, 0664);
MODULE_PARM_DESC(xvycc_matrix_coeff, "\n xvycc matrix\n");

/****************** matrix/lut reload********************/

module_param(reload_mtx, uint, 0664);
MODULE_PARM_DESC(reload_mtx, "\n reload matrix coeff\n");

module_param(reload_lut, uint, 0664);
MODULE_PARM_DESC(reload_lut, "\n reload lut settings\n");

static int RGB709_to_YUV709_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(0.2126),	COEFF_NORM(0.7152),	COEFF_NORM(0.0722),
	COEFF_NORM(-0.114572),	COEFF_NORM(-0.385428),	COEFF_NORM(0.5),
	COEFF_NORM(0.5),	COEFF_NORM(-0.454153),	COEFF_NORM(-0.045847),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	0, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static int RGB709_to_YUV709l_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(0.181873),	COEFF_NORM(0.611831),	COEFF_NORM(0.061765),
	COEFF_NORM(-0.100251),	COEFF_NORM(-0.337249),	COEFF_NORM(0.437500),
	COEFF_NORM(0.437500),	COEFF_NORM(-0.397384),	COEFF_NORM(-0.040116),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	64, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static int RGB2020_to_YUV2020l_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(0.224732),	COEFF_NORM(0.580008),	COEFF_NORM(0.050729),
	COEFF_NORM(-0.122176),	COEFF_NORM(-0.315324),	COEFF_NORM(0.437500),
	COEFF_NORM(0.437500),	COEFF_NORM(-0.402312),	COEFF_NORM(-0.035188),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	64, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static int YUV709f_to_YUV709l_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, -512, -512, /* pre offset */
	COEFF_NORM(0.859),	COEFF_NORM(0),	COEFF_NORM(0),
	COEFF_NORM(0),	COEFF_NORM(0.878),	COEFF_NORM(0),
	COEFF_NORM(0),	COEFF_NORM(0),	COEFF_NORM(0.878),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	64, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static int YUV709l_to_YUV709f_coeff[MATRIX_5x3_COEF_SIZE] = {
	-64, -512, -512, /* pre offset */
	COEFF_NORM(1.16895),	COEFF_NORM(0),	COEFF_NORM(0),
	COEFF_NORM(0),	COEFF_NORM(1.14286),	COEFF_NORM(0),
	COEFF_NORM(0),	COEFF_NORM(0),	COEFF_NORM(1.14286),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	0, 512, 512, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

static int YUV709l_to_RGB709_coeff[MATRIX_5x3_COEF_SIZE] = {
	-64, -512, -512, /* pre offset */
	COEFF_NORM(1.16895),	COEFF_NORM(0.00000),	COEFF_NORM(1.79977),
	COEFF_NORM(1.16895),	COEFF_NORM(-0.21408),	COEFF_NORM(-0.53500),
	COEFF_NORM(1.16895),	COEFF_NORM(2.12069),	COEFF_NORM(0.00000),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

#if 0
/*  eotf matrix: RGB2020 to RGB709 */
static int eotf_RGB2020_to_RGB709_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(1.6607056/2), EOTF_COEFF_NORM(-0.5877533/2),
	EOTF_COEFF_NORM(-0.0729065/2),
	EOTF_COEFF_NORM(-0.1245575/2), EOTF_COEFF_NORM(1.1329346/2),
	EOTF_COEFF_NORM(-0.0083771/2),
	EOTF_COEFF_NORM(-0.0181122/2), EOTF_COEFF_NORM(-0.1005249/2),
	EOTF_COEFF_NORM(1.1186371/2),
	EOTF_COEFF_RIGHTSHIFT
};
#endif

static int bypass_coeff[MATRIX_5x3_COEF_SIZE] = {
	0, 0, 0, /* pre offset */
	COEFF_NORM(1.0),	COEFF_NORM(0.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(1.0),	COEFF_NORM(0.0),
	COEFF_NORM(0.0),	COEFF_NORM(0.0),	COEFF_NORM(1.0),
	0, 0, 0, /* 10'/11'/12' */
	0, 0, 0, /* 20'/21'/22' */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

/*  eotf matrix: bypass */
static int eotf_bypass_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(1.0),	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(1.0),	EOTF_COEFF_NORM(0.0),
	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(0.0),	EOTF_COEFF_NORM(1.0),
	EOTF_COEFF_RIGHTSHIFT /* right shift */
};

static int eotf_RGB709_to_RGB2020_coeff[EOTF_COEFF_SIZE] = {
	EOTF_COEFF_NORM(0.627441),	EOTF_COEFF_NORM(0.329285),
	EOTF_COEFF_NORM(0.043274),
	EOTF_COEFF_NORM(0.069092),	EOTF_COEFF_NORM(0.919556),
	EOTF_COEFF_NORM(0.011322),
	EOTF_COEFF_NORM(0.016418),	EOTF_COEFF_NORM(0.088058),
	EOTF_COEFF_NORM(0.895554),
	EOTF_COEFF_RIGHTSHIFT /* right shift */
};

/* post matrix: YUV2020 limit to RGB2020 */
static int YUV2020l_to_RGB2020_coeff[MATRIX_5x3_COEF_SIZE] = {
	-64, -512, -512, /* pre offset */
	COEFF_NORM(1.16895),	COEFF_NORM(0.00000),	COEFF_NORM(1.68526),
	COEFF_NORM(1.16895),	COEFF_NORM(-0.18806),	COEFF_NORM(-0.65298),
	COEFF_NORM(1.16895),	COEFF_NORM(2.15017),	COEFF_NORM(0.00000),
	0, 0, 0, /* 30/31/32 */
	0, 0, 0, /* 40/41/42 */
	0, 0, 0, /* offset */
	0, 0, 0 /* mode, right_shift, clip_en */
};

/* eotf lut: linear */
static unsigned int eotf_33_linear_mapping[EOTF_LUT_SIZE] = {
	0x0000,	0x0200,	0x0400, 0x0600,
	0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600,
	0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600,
	0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600,
	0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000
};

/* osd oetf lut: linear */
static unsigned int oetf_41_linear_mapping[OSD_OETF_LUT_SIZE] = {
		0, 4, 8, 12,
		16, 20, 24, 28,
		31, 62, 93, 124,
		155, 186, 217, 248,
		279, 310, 341, 372,
		403, 434, 465, 496,
		527, 558, 589, 620,
		651, 682, 713, 744,
		775, 806, 837, 868,
		899, 930, 961, 992,
		1023
};

/* following array generated from model, do not edit */
static int video_lut_swtich;
module_param(video_lut_swtich, int, 0664);
MODULE_PARM_DESC(video_lut_swtich, "\n video_lut_swtich\n");

/* gamma=2.200000 lumin=500 boost=0.075000 */
static unsigned int display_scale_factor =
	(unsigned int)((((1.000000) * 4096.0) + 1) / 2);

static unsigned int eotf_33_2084_mapping[EOTF_LUT_SIZE] = {
	    0,     3,     6,    11,    18,    27,    40,    58,
	   84,   119,   169,   237,   329,   455,   636,   881,
	 1209,  1648,  2234,  3014,  4050,  5425,  7251,  9673,
	12889, 13773, 14513, 15117, 15594, 15951, 16196, 16338,
	16383
};

static unsigned int oetf_289_gamma22_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,  125,  171,  206,  235,  259,  281,  302,
	 321,  339,  356,  371,  386,  401,  414,  427,
	 440,  453,  465,  476,  488,  498,  509,  519,
	 529,  539,  549,  558,  568,  577,  586,  595,
	 603,  612,  620,  628,  637,  645,  652,  660,
	 668,  675,  683,  690,  697,  705,  712,  719,
	 726,  733,  740,  748,  755,  763,  771,  778,
	 786,  793,  801,  808,  814,  821,  827,  832,
	 837,  842,  846,  849,  852,  854,  856,  858,
	 860,  862,  864,  866,  867,  869,  871,  873,
	 875,  877,  879,  880,  882,  884,  886,  887,
	 889,  891,  893,  894,  896,  898,  899,  901,
	 902,  904,  906,  907,  909,  910,  912,  914,
	 915,  917,  918,  920,  921,  923,  924,  925,
	 927,  928,  930,  931,  933,  934,  935,  937,
	 938,  939,  941,  942,  943,  945,  946,  947,
	 948,  950,  951,  952,  953,  954,  956,  957,
	 958,  959,  960,  961,  962,  963,  965,  966,
	 967,  968,  969,  970,  971,  972,  973,  974,
	 975,  976,  977,  978,  979,  980,  981,  981,
	 982,  983,  984,  985,  986,  987,  987,  988,
	 989,  990,  991,  991,  992,  993,  994,  995,
	 995,  996,  997,  997,  998,  999,  999, 1000,
	1001, 1001, 1002, 1003, 1003, 1004, 1004, 1005,
	1006, 1006, 1007, 1007, 1008, 1008, 1009, 1009,
	1010, 1010, 1011, 1011, 1012, 1012, 1012, 1013,
	1013, 1014, 1014, 1015, 1015, 1015, 1016, 1016,
	1016, 1017, 1017, 1017, 1018, 1018, 1018, 1019,
	1019, 1019, 1019, 1020, 1020, 1020, 1020, 1020,
	1021, 1021, 1021, 1021, 1021, 1022, 1022, 1022,
	1022, 1022, 1022, 1022, 1022, 1023, 1023, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

/* osd eotf lut: 709 */
static unsigned int osd_eotf_33_709_mapping[EOTF_LUT_SIZE] = {
	    0,   512,  1024,  1536,  2048,  2560,  3072,  3584,
	 4096,  4608,  5120,  5632,  6144,  6656,  7168,  7680,
	 8192,  8704,  9216,  9728, 10240, 10752, 11264, 11776,
	12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872,
	16383
};

/* osd oetf lut: 2084 */
static unsigned int osd_oetf_41_2084_mapping[OSD_OETF_LUT_SIZE] = {
	   0,   21,   43,   64,   79,   90,  101,  111,
	 120,  174,  208,  233,  277,  313,  344,  372,
	 398,  420,  440,  459,  476,  492,  507,  522,
	 536,  549,  561,  574,  585,  596,  606,  616,
	 624,  632,  642,  661,  684,  706,  727,  749,
	1023
};

/* sdr eotf lut: 709 */
static unsigned int eotf_33_sdr_709_mapping[EOTF_LUT_SIZE] = {
	    0,   512,  1024,  1536,  2048,  2560,  3072,  3584,
	 4096,  4608,  5120,  5632,  6144,  6656,  7168,  7680,
	 8192,  8704,  9216,  9728, 10240, 10752, 11264, 11776,
	12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872,
	16383
};

/* sdr oetf lut: 2084 */
static unsigned int oetf_sdr_2084_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,   32,   55,   72,   85,   96,  106,  115,
	 124,  132,  140,  147,  154,  160,  166,  171,
	 176,  181,  186,  190,  194,  198,  204,  208,
	 211,  215,  218,  221,  224,  227,  230,  233,
	 239,  245,  252,  257,  263,  268,  274,  279,
	 283,  288,  292,  296,  302,  306,  310,  315,
	 319,  323,  326,  331,  334,  339,  342,  346,
	 350,  354,  357,  361,  365,  368,  372,  376,
	 379,  382,  386,  389,  392,  396,  399,  402,
	 405,  408,  411,  413,  416,  419,  422,  424,
	 427,  429,  432,  434,  437,  439,  442,  444,
	 446,  449,  451,  454,  456,  459,  461,  463,
	 465,  468,  470,  472,  474,  476,  478,  481,
	 483,  485,  487,  489,  491,  493,  495,  497,
	 499,  501,  503,  505,  507,  509,  511,  513,
	 515,  516,  518,  520,  522,  524,  526,  527,
	 529,  531,  533,  535,  536,  538,  540,  541,
	 543,  545,  546,  548,  550,  551,  553,  555,
	 556,  558,  559,  561,  562,  564,  566,  567,
	 569,  570,  572,  573,  575,  576,  578,  579,
	 580,  582,  584,  585,  586,  588,  589,  591,
	 592,  594,  595,  596,  598,  599,  600,  602,
	 603,  604,  606,  607,  608,  609,  611,  612,
	 613,  615,  616,  617,  618,  619,  620,  621,
	 622,  623,  624,  625,  626,  627,  628,  629,
	 630,  631,  632,  634,  635,  636,  637,  638,
	 640,  641,  642,  644,  646,  648,  651,  654,
	 657,  661,  664,  666,  670,  672,  676,  678,
	 681,  684,  687,  690,  692,  695,  698,  701,
	 704,  706,  709,  712,  715,  718,  720,  723,
	 726,  729,  731,  734,  737,  740,  743,  746,
	 748,  751,  754,  758,  763,  772,  797,  833,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

/* end of array generated from model */

module_param(display_scale_factor, uint, 0664);
MODULE_PARM_DESC(display_scale_factor, "\n display scale factor\n");

/* video oetf: linear */
#if 0
static unsigned int oetf_289_linear_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   4,    8,   12,   16,   20,   24,   28,   32,
	  36,   40,   44,   48,   52,   56,   60,   64,
	  68,   72,   76,   80,   84,   88,   92,   96,
	 100,  104,  108,  112,  116,  120,  124,  128,
	 132,  136,  140,  144,  148,  152,  156,  160,
	 164,  168,  172,  176,  180,  184,  188,  192,
	 196,  200,  204,  208,  212,  216,  220,  224,
	 228,  232,  236,  240,  244,  248,  252,  256,
	 260,  264,  268,  272,  276,  280,  284,  288,
	 292,  296,  300,  304,  308,  312,  316,  320,
	 324,  328,  332,  336,  340,  344,  348,  352,
	 356,  360,  364,  368,  372,  376,  380,  384,
	 388,  392,  396,  400,  404,  408,  412,  416,
	 420,  424,  428,  432,  436,  440,  444,  448,
	 452,  456,  460,  464,  468,  472,  476,  480,
	 484,  488,  492,  496,  500,  504,  508,  512,
	 516,  520,  524,  528,  532,  536,  540,  544,
	 548,  552,  556,  560,  564,  568,  572,  576,
	 580,  584,  588,  592,  596,  600,  604,  608,
	 612,  616,  620,  624,  628,  632,  636,  640,
	 644,  648,  652,  656,  660,  664,  668,  672,
	 676,  680,  684,  688,  692,  696,  700,  704,
	 708,  712,  716,  720,  724,  728,  732,  736,
	 740,  744,  748,  752,  756,  760,  764,  768,
	 772,  776,  780,  784,  788,  792,  796,  800,
	 804,  808,  812,  816,  820,  824,  828,  832,
	 836,  840,  844,  848,  852,  856,  860,  864,
	 868,  872,  876,  880,  884,  888,  892,  896,
	 900,  904,  908,  912,  916,  920,  924,  928,
	 932,  936,  940,  944,  948,  952,  956,  960,
	 964,  968,  972,  976,  980,  984,  988,  992,
	 996, 1000, 1004, 1008, 1012, 1016, 1020, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};

/* video oetf: 709 */
static unsigned int oetf_289_709_mapping[VIDEO_OETF_LUT_SIZE] = {
	   0,   2,   4,   6,   8,  10,  12,  14,
	  16,  18,  20,  22,  24,  26,  28,  30,
	  32,  34,  36,  54,  72,  90, 106, 121,
	 135, 148, 160, 172, 183, 193, 203, 213,
	 222, 231, 239, 248, 256, 264, 272, 279,
	 286, 294, 301, 308, 314, 321, 327, 334,
	 340, 346, 352, 358, 364, 370, 376, 381,
	 387, 392, 398, 403, 408, 413, 418, 423,
	 428, 433, 438, 443, 448, 453, 457, 462,
	 467, 471, 476, 480, 484, 489, 493, 497,
	 502, 506, 510, 514, 518, 522, 527, 531,
	 535, 538, 542, 546, 550, 554, 558, 562,
	 565, 569, 573, 577, 580, 584, 587, 591,
	 595, 598, 602, 605, 609, 612, 616, 619,
	 622, 626, 629, 633, 636, 639, 642, 646,
	 649, 652, 655, 659, 662, 665, 668, 671,
	 674, 678, 681, 684, 687, 690, 693, 696,
	 699, 702, 705, 708, 711, 714, 717, 720,
	 722, 725, 728, 731, 734, 737, 740, 742,
	 745, 748, 751, 754, 756, 759, 762, 765,
	 767, 770, 773, 775, 778, 781, 783, 786,
	 789, 791, 794, 797, 799, 802, 804, 807,
	 809, 812, 815, 817, 820, 822, 825, 827,
	 830, 832, 835, 837, 840, 842, 845, 847,
	 849, 852, 854, 857, 859, 861, 864, 866,
	 869, 871, 873, 876, 878, 880, 883, 885,
	 887, 890, 892, 894, 897, 899, 901, 903,
	 906, 908, 910, 912, 915, 917, 919, 921,
	 924, 926, 928, 930, 932, 935, 937, 939,
	 941, 943, 945, 948, 950, 952, 954, 956,
	 958, 960, 963, 965, 967, 969, 971, 973,
	 975, 977, 979, 981, 984, 986, 988, 990,
	 992,  994,  996,  998, 1000, 1002, 1004, 1006,
	1008, 1010, 1012, 1014, 1016, 1018, 1020, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023,
	1023
};
#endif

#define SIGN(a) ((a < 0) ? "-" : "+")
#define DECI(a) ((a) / 1024)
#define FRAC(a) ((((a) >= 0) ? \
	((a) & 0x3ff) : ((~(a) + 1) & 0x3ff)) * 10000 / 1024)

const char matrix_name[7][16] = {
	"OSD",
	"VD1",
	"POST",
	"XVYCC",
	"EOTF",
	"OSD_EOTF",
	"VD2"
};
static void print_vpp_matrix(int m_select, int *s, int on)
{
	unsigned int size;
	if (s == NULL)
		return;
	if (m_select == VPP_MATRIX_OSD)
		size = MATRIX_5x3_COEF_SIZE;
	else if (m_select == VPP_MATRIX_POST)
		size = MATRIX_5x3_COEF_SIZE;
	else if (m_select == VPP_MATRIX_VD1)
		size = MATRIX_5x3_COEF_SIZE;
	else if (m_select == VPP_MATRIX_VD2)
		size = MATRIX_5x3_COEF_SIZE;
	else if (m_select == VPP_MATRIX_XVYCC)
		size = MATRIX_5x3_COEF_SIZE;
	else if (m_select == VPP_MATRIX_EOTF)
		size = EOTF_COEFF_SIZE;
	else if (m_select == VPP_MATRIX_OSD_EOTF)
		size = EOTF_COEFF_SIZE;
	else
		return;

	pr_csc("%s matrix %s:\n", matrix_name[m_select],
		on ? "on" : "off");

	if (size == MATRIX_5x3_COEF_SIZE) {
		pr_csc(
		"\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
				SIGN(s[0]), DECI(s[0]), FRAC(s[0]),
				SIGN(s[3]), DECI(s[3]), FRAC(s[3]),
				SIGN(s[4]), DECI(s[4]), FRAC(s[4]),
				SIGN(s[5]), DECI(s[5]), FRAC(s[5]),
				SIGN(s[18]), DECI(s[18]), FRAC(s[18]));
		pr_csc(
		"\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
				SIGN(s[1]), DECI(s[1]), FRAC(s[1]),
				SIGN(s[6]), DECI(s[6]), FRAC(s[6]),
				SIGN(s[7]), DECI(s[7]), FRAC(s[7]),
				SIGN(s[8]), DECI(s[8]), FRAC(s[8]),
				SIGN(s[19]), DECI(s[19]), FRAC(s[19]));
		pr_csc(
		"\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
				SIGN(s[2]), DECI(s[2]), FRAC(s[2]),
				SIGN(s[9]), DECI(s[9]), FRAC(s[9]),
				SIGN(s[10]), DECI(s[10]), FRAC(s[10]),
				SIGN(s[11]), DECI(s[11]), FRAC(s[11]),
				SIGN(s[20]), DECI(s[20]), FRAC(s[20]));
		if (s[21]) {
			pr_csc("\t\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
				SIGN(s[12]), DECI(s[12]), FRAC(s[12]),
				SIGN(s[13]), DECI(s[13]), FRAC(s[13]),
				SIGN(s[14]), DECI(s[14]), FRAC(s[14]));
			pr_csc("\t\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
				SIGN(s[15]), DECI(s[15]), FRAC(s[15]),
				SIGN(s[16]), DECI(s[16]), FRAC(s[16]),
				SIGN(s[17]), DECI(s[17]), FRAC(s[17]));
		}
		if (s[22])
			pr_csc("\tright shift=%d\n", s[22]);
	} else {
		pr_csc("\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
			SIGN(s[0]), DECI(s[0]), FRAC(s[0]),
			SIGN(s[1]), DECI(s[1]), FRAC(s[1]),
			SIGN(s[2]), DECI(s[2]), FRAC(s[2]));
		pr_csc("\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
			SIGN(s[3]), DECI(s[3]), FRAC(s[3]),
			SIGN(s[4]), DECI(s[4]), FRAC(s[4]),
			SIGN(s[5]), DECI(s[5]), FRAC(s[5]));
		pr_csc("\t%s%1d.%04d\t%s%1d.%04d\t%s%1d.%04d\n",
			SIGN(s[6]), DECI(s[6]), FRAC(s[6]),
			SIGN(s[7]), DECI(s[7]), FRAC(s[7]),
			SIGN(s[8]), DECI(s[8]), FRAC(s[8]));
		if (s[9])
			pr_csc("\tright shift=%d\n", s[9]);
	}
	pr_csc("\n");
}

static int *cur_osd_mtx = RGB709_to_YUV709l_coeff;
void set_vpp_matrix(int m_select, int *s, int on)
{
	int *m = NULL;
	int size = 0;
	int i;
	if (debug_csc && print_lut_mtx)
		print_vpp_matrix(m_select, s, on);
	if (m_select == VPP_MATRIX_OSD) {
		m = osd_matrix_coeff;
		size = MATRIX_5x3_COEF_SIZE;
		cur_osd_mtx = s;
	} else if (m_select == VPP_MATRIX_POST)	{
		m = post_matrix_coeff;
		size = MATRIX_5x3_COEF_SIZE;
	} else if (m_select == VPP_MATRIX_VD1) {
		m = vd1_matrix_coeff;
		size = MATRIX_5x3_COEF_SIZE;
	} else if (m_select == VPP_MATRIX_VD2) {
		m = vd2_matrix_coeff;
		size = MATRIX_5x3_COEF_SIZE;
	} else if (m_select == VPP_MATRIX_XVYCC) {
		m = xvycc_matrix_coeff;
		size = MATRIX_5x3_COEF_SIZE;
	} else if (m_select == VPP_MATRIX_EOTF) {
		m = video_eotf_coeff;
		size = EOTF_COEFF_SIZE;
	} else if (m_select == VPP_MATRIX_OSD_EOTF) {
		m = osd_eotf_coeff;
		size = EOTF_COEFF_SIZE;
	} else
		return;

	if (s)
		for (i = 0; i < size; i++)
			m[i] = s[i];
	else
		reload_mtx &= ~(1 << m_select);

	if (m_select == VPP_MATRIX_OSD) {
		/* osd matrix, VPP_MATRIX_0 */
		/* not enable latched */
		hdr_osd_reg.viu_osd1_matrix_pre_offset0_1 =
			((m[0] & 0xfff) << 16) | (m[1] & 0xfff);
		hdr_osd_reg.viu_osd1_matrix_pre_offset2 =
			m[2] & 0xfff;
		hdr_osd_reg.viu_osd1_matrix_coef00_01 =
			((m[3] & 0x1fff) << 16) | (m[4] & 0x1fff);
		hdr_osd_reg.viu_osd1_matrix_coef02_10 =
			((m[5] & 0x1fff) << 16) | (m[6] & 0x1fff);
		hdr_osd_reg.viu_osd1_matrix_coef11_12 =
			((m[7] & 0x1fff) << 16) | (m[8] & 0x1fff);
		hdr_osd_reg.viu_osd1_matrix_coef20_21 =
			((m[9] & 0x1fff) << 16) | (m[10] & 0x1fff);
		if (m[21]) {
			hdr_osd_reg.viu_osd1_matrix_coef22_30 =
				((m[11] & 0x1fff) << 16) | (m[12] & 0x1fff);
			hdr_osd_reg.viu_osd1_matrix_coef31_32 =
				((m[13] & 0x1fff) << 16) | (m[14] & 0x1fff);
			hdr_osd_reg.viu_osd1_matrix_coef40_41 =
				((m[15] & 0x1fff) << 16) | (m[16] & 0x1fff);
			hdr_osd_reg.viu_osd1_matrix_colmod_coef42 =
				m[17] & 0x1fff;
		} else {
			hdr_osd_reg.viu_osd1_matrix_coef22_30 =
				(m[11] & 0x1fff) << 16;
		}
		hdr_osd_reg.viu_osd1_matrix_offset0_1 =
			((m[18] & 0xfff) << 16) | (m[19] & 0xfff);
		hdr_osd_reg.viu_osd1_matrix_offset2 =
			m[20] & 0xfff;

		hdr_osd_reg.viu_osd1_matrix_colmod_coef42 &= 0x3ff8ffff;
		hdr_osd_reg.viu_osd1_matrix_colmod_coef42 |=
			(m[21] << 30) | (m[22] << 16);

		/* 23 reserved for clipping control */
		hdr_osd_reg.viu_osd1_matrix_ctrl &= 0xfffffffc;
		hdr_osd_reg.viu_osd1_matrix_ctrl |= on;
	} else if (m_select == VPP_MATRIX_EOTF) {
		/* eotf matrix, VPP_MATRIX_EOTF */
		/* enable latched */
		for (i = 0; i < 5; i++)
			VSYNC_WR_MPEG_REG(VIU_EOTF_CTL + i + 1,
				((m[i * 2] & 0x1fff) << 16)
				| (m[i * 2 + 1] & 0x1fff));
		WRITE_VPP_REG_BITS(VIU_EOTF_CTL, on, 30, 1);
		WRITE_VPP_REG_BITS(VIU_EOTF_CTL, on, 31, 1);
	} else if (m_select == VPP_MATRIX_OSD_EOTF) {
		/* osd eotf matrix, VPP_MATRIX_OSD_EOTF */
		/* enable latched */
		hdr_osd_reg.viu_osd1_eotf_coef00_01 =
			((m[0 * 2] & 0x1fff) << 16)
			| (m[0 * 2 + 1] & 0x1fff);

		hdr_osd_reg.viu_osd1_eotf_coef02_10 =
			((m[1 * 2] & 0x1fff) << 16)
			| (m[1 * 2 + 1] & 0x1fff);

		hdr_osd_reg.viu_osd1_eotf_coef11_12 =
			((m[2 * 2] & 0x1fff) << 16)
			| (m[2 * 2 + 1] & 0x1fff);

		hdr_osd_reg.viu_osd1_eotf_coef20_21 =
			((m[3 * 2] & 0x1fff) << 16)
			| (m[3 * 2 + 1] & 0x1fff);
		hdr_osd_reg.viu_osd1_eotf_coef22_rs =
			((m[4 * 2] & 0x1fff) << 16)
			| (m[4 * 2 + 1] & 0x1fff);

		hdr_osd_reg.viu_osd1_eotf_ctl &= 0x3fffffff;
		hdr_osd_reg.viu_osd1_eotf_ctl |= (on << 30) | (on << 31);
	} else {
		/* vd1 matrix, VPP_MATRIX_1 */
		/* post matrix, VPP_MATRIX_2 */
		/* xvycc matrix, VPP_MATRIX_3 */
		/* vd2 matrix, VPP_MATRIX_6 */
		if (m_select == VPP_MATRIX_POST) {
			/* post matrix */
			m = post_matrix_coeff;
			/* set bit for enable latched */
			WRITE_VPP_REG_BITS(VPP_XVYCC_MISC, 1, 14, 1);
			WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 0, 1);
			if (on) {
				if (rdma_flag & (1 << m_select))
					VSYNC_WR_MPEG_REG_BITS(
						VPP_MATRIX_CTRL, 0, 8, 2);
				else
					WRITE_VPP_REG_BITS(
						VPP_MATRIX_CTRL, 0, 8, 2);
			}
		} else if (m_select == VPP_MATRIX_VD1) {
			/* vd1 matrix, latched */
			m = vd1_matrix_coeff;
			/* set bit for enable latched */
			WRITE_VPP_REG_BITS(VPP_XVYCC_MISC, 1, 9, 1);
			WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 5, 1);
			if (on) {
				if (rdma_flag & (1 << m_select))
					VSYNC_WR_MPEG_REG_BITS(
						VPP_MATRIX_CTRL, 1, 8, 2);
				else
					WRITE_VPP_REG_BITS(
						VPP_MATRIX_CTRL, 1, 8, 2);
			}
		} else if (m_select == VPP_MATRIX_VD2) {
			/* vd2 matrix, not latched */
			m = vd2_matrix_coeff;
			if (rdma_flag & (1 << m_select))
				VSYNC_WR_MPEG_REG_BITS(
					VPP_MATRIX_CTRL, on, 4, 1);
			else
				WRITE_VPP_REG_BITS(
					VPP_MATRIX_CTRL, on, 4, 1);
			if (on) {
				if (rdma_flag & (1 << m_select))
					VSYNC_WR_MPEG_REG_BITS(
						VPP_MATRIX_CTRL, 2, 8, 2);
				else
					WRITE_VPP_REG_BITS(
						VPP_MATRIX_CTRL, 2, 8, 2);
			}
		} else if (m_select == VPP_MATRIX_XVYCC) {
			/* xvycc matrix, not latched */
			m = xvycc_matrix_coeff;
			if (rdma_flag & (1 << m_select))
				VSYNC_WR_MPEG_REG_BITS(
					VPP_MATRIX_CTRL, on, 6, 1);
			else
				WRITE_VPP_REG_BITS(
					VPP_MATRIX_CTRL, on, 6, 1);
			if (on) {
				if (rdma_flag & (1 << m_select))
					VSYNC_WR_MPEG_REG_BITS(
						VPP_MATRIX_CTRL, 3, 8, 2);
				else
					WRITE_VPP_REG_BITS(
						VPP_MATRIX_CTRL, 3, 8, 2);
			}
		}
		if (on) {
			if (rdma_flag & (1 << m_select)) {
				VSYNC_WR_MPEG_REG(VPP_MATRIX_PRE_OFFSET0_1,
					((m[0] & 0xfff) << 16)
					| (m[1] & 0xfff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_PRE_OFFSET2,
					m[2] & 0xfff);
				VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF00_01,
					((m[3] & 0x1fff) << 16)
					| (m[4] & 0x1fff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF02_10,
					((m[5]  & 0x1fff) << 16)
					| (m[6] & 0x1fff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF11_12,
					((m[7] & 0x1fff) << 16)
					| (m[8] & 0x1fff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF20_21,
					((m[9] & 0x1fff) << 16)
					| (m[10] & 0x1fff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF22,
					m[11] & 0x1fff);
				if (m[21]) {
					VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF13_14,
						((m[12] & 0x1fff) << 16)
						| (m[13] & 0x1fff));
					VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF15_25,
						((m[14] & 0x1fff) << 16)
						| (m[17] & 0x1fff));
					VSYNC_WR_MPEG_REG(VPP_MATRIX_COEF23_24,
						((m[15] & 0x1fff) << 16)
						| (m[16] & 0x1fff));
				}
				VSYNC_WR_MPEG_REG(VPP_MATRIX_OFFSET0_1,
					((m[18] & 0xfff) << 16)
					| (m[19] & 0xfff));
				VSYNC_WR_MPEG_REG(VPP_MATRIX_OFFSET2,
					m[20] & 0xfff);
				VSYNC_WR_MPEG_REG_BITS(VPP_MATRIX_CLIP,
					m[21], 3, 2);
				VSYNC_WR_MPEG_REG_BITS(VPP_MATRIX_CLIP,
					m[22], 5, 3);
			} else {
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1,
					((m[0] & 0xfff) << 16)
					| (m[1] & 0xfff));
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2,
					m[2] & 0xfff);
				WRITE_VPP_REG(VPP_MATRIX_COEF00_01,
					((m[3] & 0x1fff) << 16)
					| (m[4] & 0x1fff));
				WRITE_VPP_REG(VPP_MATRIX_COEF02_10,
					((m[5]  & 0x1fff) << 16)
					| (m[6] & 0x1fff));
				WRITE_VPP_REG(VPP_MATRIX_COEF11_12,
					((m[7] & 0x1fff) << 16)
					| (m[8] & 0x1fff));
				WRITE_VPP_REG(VPP_MATRIX_COEF20_21,
					((m[9] & 0x1fff) << 16)
					| (m[10] & 0x1fff));
				WRITE_VPP_REG(VPP_MATRIX_COEF22,
					m[11] & 0x1fff);
				if (m[21]) {
					WRITE_VPP_REG(VPP_MATRIX_COEF13_14,
						((m[12] & 0x1fff) << 16)
						| (m[13] & 0x1fff));
					WRITE_VPP_REG(VPP_MATRIX_COEF15_25,
						((m[14] & 0x1fff) << 16)
						| (m[17] & 0x1fff));
					WRITE_VPP_REG(VPP_MATRIX_COEF23_24,
						((m[15] & 0x1fff) << 16)
						| (m[16] & 0x1fff));
				}
				WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1,
					((m[18] & 0xfff) << 16)
					| (m[19] & 0xfff));
				WRITE_VPP_REG(VPP_MATRIX_OFFSET2,
					m[20] & 0xfff);
				WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP,
					m[21], 3, 2);
				WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP,
					m[22], 5, 3);
			}
		}
	}
}

void enable_osd_path(int on)
{
	static int *osd1_mtx_backup;
	static uint32_t osd1_eotf_ctl_backup;
	static uint32_t osd1_oetf_ctl_backup;
	if (!on) {
		osd1_mtx_backup = cur_osd_mtx;
		osd1_eotf_ctl_backup = hdr_osd_reg.viu_osd1_eotf_ctl;
		osd1_oetf_ctl_backup = hdr_osd_reg.viu_osd1_oetf_ctl;

		set_vpp_matrix(VPP_MATRIX_OSD, bypass_coeff, CSC_ON);
		hdr_osd_reg.viu_osd1_eotf_ctl &= 0x07ffffff;
		hdr_osd_reg.viu_osd1_oetf_ctl &= 0x1fffffff;
	} else {
		set_vpp_matrix(VPP_MATRIX_OSD, osd1_mtx_backup, CSC_ON);
		hdr_osd_reg.viu_osd1_eotf_ctl = osd1_eotf_ctl_backup;
		hdr_osd_reg.viu_osd1_eotf_ctl = osd1_oetf_ctl_backup;
	}
}
EXPORT_SYMBOL(enable_osd_path);

const char lut_name[NUM_LUT][16] = {
	"OSD_EOTF",
	"OSD_OETF",
	"EOTF",
	"OETF",
	"INV_EOTF"
};
/* VIDEO_OETF_LUT_SIZE 289 >
	OSD_OETF_LUT_SIZE 41 >
	OSD_OETF_LUT_SIZE 33 */
static void print_vpp_lut(
	enum vpp_lut_sel_e lut_sel,
	int on)
{
	unsigned short r_map[VIDEO_OETF_LUT_SIZE];
	unsigned short g_map[VIDEO_OETF_LUT_SIZE];
	unsigned short b_map[VIDEO_OETF_LUT_SIZE];
	unsigned int addr_port;
	unsigned int data_port;
	unsigned int ctrl_port;
	unsigned int data;
	int i;

	if (lut_sel == VPP_LUT_OSD_EOTF) {
		addr_port = VIU_OSD1_EOTF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_EOTF_CTL;
		return;
	} else if (lut_sel == VPP_LUT_EOTF) {
		addr_port = VIU_EOTF_LUT_ADDR_PORT;
		data_port = VIU_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_EOTF_CTL;
	} else if (lut_sel == VPP_LUT_OSD_OETF) {
		addr_port = VIU_OSD1_OETF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_OETF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_OETF_CTL;
		return;
	} else if (lut_sel == VPP_LUT_OETF) {
		addr_port = XVYCC_LUT_R_ADDR_PORT;
		data_port = XVYCC_LUT_R_DATA_PORT;
		ctrl_port = XVYCC_LUT_CTL;
	} else if (lut_sel == VPP_LUT_INV_EOTF) {
		addr_port = XVYCC_INV_LUT_Y_ADDR_PORT;
		data_port = XVYCC_INV_LUT_Y_DATA_PORT;
		ctrl_port = XVYCC_INV_LUT_CTL;
	} else
		return;
	if (lut_sel == VPP_LUT_OSD_OETF) {
		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, i);
			data = READ_VPP_REG(data_port);
			r_map[i * 2] = data & 0xffff;
			r_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		WRITE_VPP_REG(addr_port, 20);
		data = READ_VPP_REG(data_port);
		r_map[OSD_OETF_LUT_SIZE - 1] = data & 0xffff;
		g_map[0] = (data >> 16) & 0xffff;
		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, 21 + i);
			data = READ_VPP_REG(data_port);
			g_map[i * 2 + 1] = data & 0xffff;
			g_map[i * 2 + 2] = (data >> 16) & 0xffff;
		}
		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, 41 + i);
			data = READ_VPP_REG(data_port);
			b_map[i * 2] = data & 0xffff;
			b_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		WRITE_VPP_REG(addr_port, 61);
		data = READ_VPP_REG(data_port);
		b_map[OSD_OETF_LUT_SIZE - 1] = data & 0xffff;
		pr_csc("%s lut %s:\n", lut_name[lut_sel], on ? "on" : "off");
		for (i = 0; i < OSD_OETF_LUT_SIZE; i++) {
			pr_csc("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}
		pr_csc("\n");
	} else if (lut_sel == VPP_LUT_OSD_EOTF) {
		WRITE_VPP_REG(addr_port, 0);
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			r_map[i * 2] = data & 0xffff;
			r_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		r_map[EOTF_LUT_SIZE - 1] = data & 0xffff;
		g_map[0] = (data >> 16) & 0xffff;
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			g_map[i * 2 + 1] = data & 0xffff;
			g_map[i * 2 + 2] = (data >> 16) & 0xffff;
		}
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			b_map[i * 2] = data & 0xffff;
			b_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		b_map[EOTF_LUT_SIZE - 1] = data & 0xffff;
		pr_csc("%s lut %s:\n", lut_name[lut_sel], on ? "on" : "off");
		for (i = 0; i < EOTF_LUT_SIZE; i++) {
			pr_csc("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}
		pr_csc("\n");
	} else if (lut_sel == VPP_LUT_EOTF) {
		WRITE_VPP_REG(addr_port, 0);
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			r_map[i * 2] = data & 0xffff;
			r_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		r_map[EOTF_LUT_SIZE - 1] = data & 0xffff;
		g_map[0] = (data >> 16) & 0xffff;
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			g_map[i * 2 + 1] = data & 0xffff;
			g_map[i * 2 + 2] = (data >> 16) & 0xffff;
		}
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			b_map[i * 2] = data & 0xffff;
			b_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		b_map[EOTF_LUT_SIZE - 1] = data & 0xffff;
		pr_csc("%s lut %s:\n", lut_name[lut_sel], on ? "on" : "off");
		for (i = 0; i < EOTF_LUT_SIZE; i++) {
			pr_csc("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}
		pr_csc("\n");
	} else if (lut_sel == VPP_LUT_OETF) {
		WRITE_VPP_REG(ctrl_port, 0x0);
		WRITE_VPP_REG(addr_port, 0);
		for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
			r_map[i] = READ_VPP_REG(data_port) & 0x3ff;
		WRITE_VPP_REG(addr_port + 2, 0);
		for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
			g_map[i] = READ_VPP_REG(data_port + 2) & 0x3ff;
		WRITE_VPP_REG(addr_port + 4, 0);
		for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
			b_map[i] = READ_VPP_REG(data_port + 4) & 0x3ff;
		pr_csc("%s lut %s:\n", lut_name[lut_sel], on ? "on" : "off");
		for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++) {
			pr_csc("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}
		pr_csc("\n");
		if (on)
			WRITE_VPP_REG(ctrl_port, 0x7f);
	} else if (lut_sel == VPP_LUT_INV_EOTF) {
		WRITE_VPP_REG_BITS(ctrl_port, 0, 12, 3);
		pr_csc("%s lut %s:\n", lut_name[lut_sel], on ? "on" : "off");
		for (i = 0;
			i < EOTF_INV_LUT_NEG2048_SIZE +
			EOTF_INV_LUT_SIZE + EOTF_INV_LUT_1024_SIZE;
			i++) {
			WRITE_VPP_REG(addr_port, i);
			data = READ_VPP_REG(data_port) & 0xfff;
			if (data & 0x800)
				pr_csc("\t[%d] = %d\n",
					i, -(~(data|0xfffff000) + 1));
			else
				pr_csc("\t[%d] = %d\n", i, data);
		}
		pr_csc("\n");
		if (on)
			WRITE_VPP_REG_BITS(ctrl_port, 1<<2, 12, 3);
	}
}

void set_vpp_lut(
	enum vpp_lut_sel_e lut_sel,
	unsigned int *r,
	unsigned int *g,
	unsigned int *b,
	int on)
{
	unsigned int *r_map = NULL;
	unsigned int *g_map = NULL;
	unsigned int *b_map = NULL;
	unsigned int addr_port;
	unsigned int data_port;
	unsigned int ctrl_port;
	int i;

	if (reload_lut & (1 << lut_sel))
		reload_lut &= ~(1 << lut_sel);
	if (lut_sel == VPP_LUT_OSD_EOTF) {
		r_map = osd_eotf_r_mapping;
		g_map = osd_eotf_g_mapping;
		b_map = osd_eotf_b_mapping;
		addr_port = VIU_OSD1_EOTF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_EOTF_CTL;
	} else if (lut_sel == VPP_LUT_EOTF) {
		r_map = video_eotf_r_mapping;
		g_map = video_eotf_g_mapping;
		b_map = video_eotf_b_mapping;
		addr_port = VIU_EOTF_LUT_ADDR_PORT;
		data_port = VIU_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_EOTF_CTL;
	} else if (lut_sel == VPP_LUT_OSD_OETF) {
		r_map = osd_oetf_r_mapping;
		g_map = osd_oetf_g_mapping;
		b_map = osd_oetf_b_mapping;
		addr_port = VIU_OSD1_OETF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_OETF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_OETF_CTL;
	} else if (lut_sel == VPP_LUT_OETF) {
#if 0
		load_knee_lut(on);
		return;
#else
		r_map = video_oetf_r_mapping;
		g_map = video_oetf_g_mapping;
		b_map = video_oetf_b_mapping;
		addr_port = XVYCC_LUT_R_ADDR_PORT;
		data_port = XVYCC_LUT_R_DATA_PORT;
		ctrl_port = XVYCC_LUT_CTL;
#endif
	} else if (lut_sel == VPP_LUT_INV_EOTF) {
		addr_port = XVYCC_INV_LUT_Y_ADDR_PORT;
		data_port = XVYCC_INV_LUT_Y_DATA_PORT;
		ctrl_port = XVYCC_INV_LUT_CTL;
	} else
		return;

	if (lut_sel == VPP_LUT_OSD_OETF) {
		/* enable latched */
		WRITE_VPP_REG_BITS(VIU_OSD1_OETF_CTL, 1, 28, 1);
		if (r && r_map)
			for (i = 0; i < OSD_OETF_LUT_SIZE; i++)
				r_map[i] = r[i];
		if (g && g_map)
			for (i = 0; i < OSD_OETF_LUT_SIZE; i++)
				g_map[i] = g[i];
		if (r && r_map)
			for (i = 0; i < OSD_OETF_LUT_SIZE; i++)
				b_map[i] = b[i];

		for (i = 0; i < OSD_OETF_LUT_SIZE; i++) {
			hdr_osd_reg.lut_val.or_map[i] = r_map[i];
			hdr_osd_reg.lut_val.og_map[i] = g_map[i];
			hdr_osd_reg.lut_val.ob_map[i] = b_map[i];
		}
		hdr_osd_reg.viu_osd1_oetf_ctl &= 0x1fffffff;
		hdr_osd_reg.viu_osd1_oetf_ctl |= 7 << 22;
		if (on)
			hdr_osd_reg.viu_osd1_oetf_ctl |= 7 << 29;
	} else if (lut_sel == VPP_LUT_OSD_EOTF) {
		/* enable latched */
		if (r && r_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				r_map[i] = r[i];
		if (g && g_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				g_map[i] = g[i];
		if (r && r_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				b_map[i] = b[i];

		for (i = 0; i < EOTF_LUT_SIZE; i++) {
			hdr_osd_reg.lut_val.r_map[i] = r_map[i];
			hdr_osd_reg.lut_val.g_map[i] = g_map[i];
			hdr_osd_reg.lut_val.b_map[i] = b_map[i];
		}
		hdr_osd_reg.viu_osd1_eotf_ctl &= 0xc7ffffff;
		if (on)
			hdr_osd_reg.viu_osd1_eotf_ctl |= 7 << 27;
		hdr_osd_reg.viu_osd1_eotf_ctl |= 1 << 31;
	} else if (lut_sel == VPP_LUT_EOTF) {
		/* enable latched */
		if (r && r_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				r_map[i] = r[i];
		if (g && g_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				g_map[i] = g[i];
		if (r && r_map)
			for (i = 0; i < EOTF_LUT_SIZE; i++)
				b_map[i] = b[i];
		if (on) {
			for (i = 0; i < 16; i++) {
				VSYNC_WR_MPEG_REG(addr_port, i);
				VSYNC_WR_MPEG_REG(data_port,
					r_map[i * 2]
					| (r_map[i * 2 + 1] << 16));
			}
			VSYNC_WR_MPEG_REG(addr_port, 16);
			VSYNC_WR_MPEG_REG(data_port,
				r_map[EOTF_LUT_SIZE - 1]
				| (g_map[0] << 16));
			for (i = 0; i < 16; i++) {
				VSYNC_WR_MPEG_REG(addr_port, i + 17);
				VSYNC_WR_MPEG_REG(data_port,
					g_map[i * 2 + 1]
					| (g_map[i * 2 + 2] << 16));
			}
			for (i = 0; i < 16; i++) {
				VSYNC_WR_MPEG_REG(addr_port, i + 33);
				VSYNC_WR_MPEG_REG(data_port,
					b_map[i * 2]
					| (b_map[i * 2 + 1] << 16));
			}
			VSYNC_WR_MPEG_REG(addr_port, 49);
			VSYNC_WR_MPEG_REG(data_port, b_map[EOTF_LUT_SIZE - 1]);
			WRITE_VPP_REG_BITS(ctrl_port, 7, 27, 3);
		} else
			WRITE_VPP_REG_BITS(ctrl_port, 0, 27, 3);
		WRITE_VPP_REG_BITS(ctrl_port, 1, 31, 1);
	} else if (lut_sel == VPP_LUT_OETF) {
		/* set bit to disable latched */
		WRITE_VPP_REG_BITS(VPP_XVYCC_MISC, 0, 18, 3);
		if (r && r_map)
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
				r_map[i] = r[i];
		if (g && g_map)
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
				g_map[i] = g[i];
		if (r && r_map)
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++)
				b_map[i] = b[i];
		if (on) {
			VSYNC_WR_MPEG_REG(ctrl_port, 0x0f);
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port, i);
				VSYNC_WR_MPEG_REG(data_port, r_map[i]);
			}
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port + 2, i);
				VSYNC_WR_MPEG_REG(data_port + 2, g_map[i]);
			}
			for (i = 0; i < VIDEO_OETF_LUT_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port + 4, i);
				VSYNC_WR_MPEG_REG(data_port + 4, b_map[i]);
			}
			VSYNC_WR_MPEG_REG(ctrl_port, 0x7f);
			knee_lut_on = 1;
		} else {
			VSYNC_WR_MPEG_REG(ctrl_port, 0x0f);
			knee_lut_on = 0;
		}
		cur_knee_factor = knee_factor;
	} else if (lut_sel == VPP_LUT_INV_EOTF) {
		/* set bit to enable latched */
		WRITE_VPP_REG_BITS(VPP_XVYCC_MISC, 0x7, 4, 3);
		if (on) {
			VSYNC_WR_MPEG_REG(addr_port, 0);
			for (i = 0; i < EOTF_INV_LUT_NEG2048_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port, i);
				VSYNC_WR_MPEG_REG(data_port, invlut_y_neg[i]);
			}
			for (i = 0; i < EOTF_INV_LUT_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port,
					EOTF_INV_LUT_NEG2048_SIZE + i);
				VSYNC_WR_MPEG_REG(data_port, invlut_y[i]);
			}
			for (i = 0; i < EOTF_INV_LUT_1024_SIZE; i++) {
				VSYNC_WR_MPEG_REG(addr_port,
					EOTF_INV_LUT_NEG2048_SIZE +
					EOTF_INV_LUT_SIZE + i);
				VSYNC_WR_MPEG_REG(data_port, invlut_y_1024[i]);
			}
			WRITE_VPP_REG_BITS(ctrl_port, 1<<2, 12, 3);
		} else
			WRITE_VPP_REG_BITS(ctrl_port, 0, 12, 3);
	}
	if (debug_csc && print_lut_mtx)
		print_vpp_lut(lut_sel, on);
}

/***************************** end of gxl hdr **************************/

/* extern unsigned int cm_size; */
/* extern unsigned int ve_size; */
/* extern unsigned int cm2_patch_flag; */
/* extern struct ve_dnlp_s am_ve_dnlp; */
/* extern struct ve_dnlp_table_s am_ve_new_dnlp; */
/* extern int cm_en; //0:disabel;1:enable */
/* extern struct tcon_gamma_table_s video_gamma_table_r; */
/* extern struct tcon_gamma_table_s video_gamma_table_g; */
/* extern struct tcon_gamma_table_s video_gamma_table_b; */
/* extern struct tcon_gamma_table_s video_gamma_table_r_adj; */
/* extern struct tcon_gamma_table_s video_gamma_table_g_adj; */
/* extern struct tcon_gamma_table_s video_gamma_table_b_adj; */
/* extern struct tcon_rgb_ogo_s     video_rgb_ogo; */

/* csc mode:
	0: 601 limit to RGB
		vd1 for ycbcr to rgb
	1: 601 full to RGB
		vd1 for ycbcr to rgb
	2: 709 limit to RGB
		vd1 for ycbcr to rgb
	3: 709 full to RGB
		vd1 for ycbcr to rgb
	4: 2020 limit to RGB
		vd1 for ycbcr to rgb
		post for rgb to r'g'b'
	5: 2020(G:33c2,86c4 B:1d4c,0bb8 R:84d0,3e80) limit to RGB
		vd1 for ycbcr to rgb
		post for rgb to r'g'b'
	6: customer matrix calculation according to src and dest primary
		vd1 for ycbcr to rgb
		post for rgb to r'g'b' */
static void vpp_set_matrix(
		enum vpp_matrix_sel_e vd1_or_vd2_or_post,
		unsigned int on,
		enum vpp_matrix_csc_e csc_mode,
		struct matrix_s *m)
{
	if (force_csc_type != 0xff)
		csc_mode = force_csc_type;

	if (vd1_or_vd2_or_post == VPP_MATRIX_VD1) {
		/* vd1 matrix */
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 5, 1);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, 1, 8, 2);
	} else if (vd1_or_vd2_or_post == VPP_MATRIX_VD2) {
		/* vd2 matrix */
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 4, 1);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, 2, 8, 2);
	} else {
		/* post matrix */
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 0, 1);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, 0, 8, 2);
		/* saturation enable for 601 & 709 limited input */
		WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, 0, 1, 2);
	}
	if (!on)
		return;

/*	TODO: need to adjust +/-64 for VPP_MATRIX_PRE_OFFSET0
	which was set in vpp_vd_adj1_brightness;
	should get current vd1_brightness(-1024~1023) here

	if (vd1_or_vd2_or_post == 0) {
		if ((csc_mode == 0) ||
			(csc_mode == 2) ||
			(csc_mode >= 4))
			WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1,
				(((vd1_brightness - 64) & 0xfff) << 16) |
				0xe00);
		else
			WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1,
				(((vd1_brightness - 64) & 0xfff) << 16) |
				0xe00);
		WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2, 0xe00);
	}
*/

	if (csc_mode == VPP_MATRIX_YUV601_RGB) {
		/* ycbcr limit, 601 to RGB */
		/*  -16  1.164   0       1.596   0
		    -128 1.164   -0.392  -0.813  0
		    -128 1.164   2.017   0       0 */
		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x04A80000);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x066204A8);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1e701cbf);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x04A80812);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x00000000);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x00000000);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x00000000);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
	} else if (csc_mode == VPP_MATRIX_YUV601F_RGB) {
		/* ycbcr full range, 601F to RGB */
		/*  0    1    0           1.402    0
		   -128  1   -0.34414    -0.71414  0
		   -128  1    1.772       0        0 */
		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, (0x400 << 16) | 0);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, (0x59c << 16) | 0x400);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, (0x1ea0 << 16) | 0x1d24);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, (0x400 << 16) | 0x718);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
	} else if (csc_mode == VPP_MATRIX_YUV709_RGB) {
		/* ycbcr limit range, 709 to RGB */
		/* -16      1.164  0      1.793  0 */
		/* -128     1.164 -0.213 -0.534  0 */
		/* -128     1.164  2.115  0      0 */
		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x04A80000);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x072C04A8);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1F261DDD);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x04A80876);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);

		WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
	} else if (csc_mode == VPP_MATRIX_YUV709F_RGB) {
		/* ycbcr full range, 709F to RGB */
		/*  0    1      0       1.575   0
		   -128  1     -0.187  -0.468   0
		   -128  1      1.856   0       0 */
		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x04000000);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x064D0400);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1F411E21);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x0400076D);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
	} else if (csc_mode == VPP_MATRIX_NULL) {
		/* bypass matrix */
		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x04000000);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x04000000);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x00000000);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x00000400);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
		WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
	} else if (csc_mode >= VPP_MATRIX_BT2020YUV_BT2020RGB) {
		if (vd1_or_vd2_or_post == VPP_MATRIX_VD1) {
			/* bt2020 limit to bt2020 RGB  */
			WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x4ad0000);
			WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x6e50492);
			WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1f3f1d63);
			WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x492089a);
			WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x0);
			WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
			WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
			WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 0, 5, 3);
		}
		if (vd1_or_vd2_or_post == VPP_MATRIX_POST) {
			if (csc_mode == VPP_MATRIX_BT2020YUV_BT2020RGB) {
				/* 2020 RGB to R'G'B */
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2, 0x0);
				WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0xd491b4d);
				WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x1f6b1f01);
				WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x9101fef);
				WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x1fdb1f32);
				WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x108f3);
				WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
				WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
				WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 1, 5, 3);
#if 0 /* disable this case after calculate mtx on the fly*/
			} else if (csc_mode == VPP_MATRIX_BT2020RGB_709RGB) {
				/* 2020 RGB(G:33c2,86c4 B:1d4c,0bb8 R:84d0,3e80)
				to R'G'B' */
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2, 0x0);
				/* from Jason */
				WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x9cd1e33);
				WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x00001faa);
				WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x8560000);
				WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x1fd81f5f);
				WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x108c9);
				WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x0);
				WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x0);
				WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP, 1, 5, 3);
#endif
			} else if (csc_mode == VPP_MATRIX_BT2020RGB_CUSRGB) {
				/* customer matrix 2020 RGB to R'G'B' */
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1,
					(m->pre_offset[0] << 16)
					| (m->pre_offset[1] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2,
					m->pre_offset[2] & 0xffff);
				WRITE_VPP_REG(VPP_MATRIX_COEF00_01,
					(m->matrix[0][0] << 16)
					| (m->matrix[0][1] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_COEF02_10,
					(m->matrix[0][2] << 16)
					| (m->matrix[1][0] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_COEF11_12,
					(m->matrix[1][1] << 16)
					| (m->matrix[1][2] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_COEF20_21,
					(m->matrix[2][0] << 16)
					| (m->matrix[2][1] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_COEF22,
					(m->right_shift << 16)
					| (m->matrix[2][2] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1,
					(m->offset[0] << 16)
					| (m->offset[1] & 0xffff));
				WRITE_VPP_REG(VPP_MATRIX_OFFSET2,
					m->offset[2] & 0xffff);
				WRITE_VPP_REG_BITS(VPP_MATRIX_CLIP,
					m->right_shift, 5, 3);
			}
		}
	}
}

/* matrix betreen xvycclut and linebuffer*/
static uint cur_csc_mode = 0xff;
static void vpp_set_matrix3(
		unsigned int on,
		enum vpp_matrix_csc_e csc_mode)
{
	WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, on, 6, 1);
	if (!on)
		return;

	if (cur_csc_mode == csc_mode)
		return;

	WRITE_VPP_REG_BITS(VPP_MATRIX_CTRL, 3, 8, 2);
	if (csc_mode == VPP_MATRIX_RGB_YUV709F) {
		/* RGB -> 709F*/
		/*WRITE_VPP_REG(VPP_MATRIX_CTRL, 0x7360);*/

		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0xda02dc);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x4a1f8a);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1e760200);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x2001e2f);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x1fd1);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x200);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x200);
		WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2, 0x0);
	} else if (csc_mode == VPP_MATRIX_RGB_YUV709) {
		/* RGB -> 709 limit */
		/*WRITE_VPP_REG(VPP_MATRIX_CTRL, 0x7360);*/

		WRITE_VPP_REG(VPP_MATRIX_COEF00_01, 0x00bb0275);
		WRITE_VPP_REG(VPP_MATRIX_COEF02_10, 0x003f1f99);
		WRITE_VPP_REG(VPP_MATRIX_COEF11_12, 0x1ea601c2);
		WRITE_VPP_REG(VPP_MATRIX_COEF20_21, 0x01c21e67);
		WRITE_VPP_REG(VPP_MATRIX_COEF22, 0x00001fd7);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET0_1, 0x00400200);
		WRITE_VPP_REG(VPP_MATRIX_OFFSET2, 0x00000200);
		WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET0_1, 0x0);
		WRITE_VPP_REG(VPP_MATRIX_PRE_OFFSET2, 0x0);
	}
	cur_csc_mode = csc_mode;
}

static uint cur_signal_type = 0xffffffff;
module_param(cur_signal_type, uint, 0664);
MODULE_PARM_DESC(cur_signal_type, "\n cur_signal_type\n");

static struct vframe_master_display_colour_s cur_master_display_colour = {
	0,
	{
		{0, 0},
		{0, 0},
		{0, 0},
	},
	{0, 0},
	{0, 0}
};

#define SIG_CS_CHG	0x01
#define SIG_SRC_CHG	0x02
#define SIG_PRI_INFO	0x04
#define SIG_KNEE_FACTOR	0x08
#define SIG_HDR_MODE	0x10
#define SIG_HDR_SUPPORT	0x20
#define SIG_WB_CHG	0x40

int signal_type_changed(struct vframe_s *vf, struct vinfo_s *vinfo)
{
	u32 signal_type = 0;
	u32 default_signal_type;
	int change_flag = 0;
	int i, j;
	struct vframe_master_display_colour_s *p_cur;
	struct vframe_master_display_colour_s *p_new;
	struct vframe_master_display_colour_s cus;

	if ((vf->source_type == VFRAME_SOURCE_TYPE_TUNER) ||
		(vf->source_type == VFRAME_SOURCE_TYPE_CVBS) ||
		(vf->source_type == VFRAME_SOURCE_TYPE_COMP) ||
		(vf->source_type == VFRAME_SOURCE_TYPE_HDMI)) {
		if (get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB)
			default_signal_type =
				/* default 709 full */
				  (1 << 29)	/* video available */
				| (5 << 26)	/* unspecified */
				| (1 << 25)	/* limit */
				| (1 << 24)	/* color available */
				| (1 << 16)	/* bt709 */
				| (1 << 8)	/* bt709 */
				| (1 << 0);	/* bt709 */
		else
			default_signal_type =
				/* default 709 limit */
				  (1 << 29)	/* video available */
				| (5 << 26)	/* unspecified */
				| (0 << 25)	/* limit */
				| (1 << 24)	/* color available */
				| (1 << 16)	/* bt709 */
				| (1 << 8)	/* bt709 */
				| (1 << 0);	/* bt709 */
	} else { /* for local play */
		if (vf->height >= 720)
			default_signal_type =
				/* HD default 709 limit */
				  (1 << 29)	/* video available */
				| (5 << 26)	/* unspecified */
				| (0 << 25)	/* limit */
				| (1 << 24)	/* color available */
				| (1 << 16)	/* bt709 */
				| (1 << 8)	/* bt709 */
				| (1 << 0);	/* bt709 */
		else
			default_signal_type =
				/* SD default 601 limited */
				  (1 << 29)	/* video available */
				| (5 << 26)	/* unspecified */
				| (0 << 25)	/* limited */
				| (1 << 24)	/* color available */
				| (3 << 16)	/* bt601 */
				| (3 << 8)	/* bt601 */
				| (3 << 0);	/* bt601 */
	}
	if (vf->signal_type & (1 << 29))
		signal_type = vf->signal_type;
	else
		signal_type = default_signal_type;

	p_new = &vf->prop.master_display_colour;
	p_cur = &cur_master_display_colour;
	/* customer overwrite */
	if (customer_master_display_en
	&& ((p_new->present_flag & 0x80000000) == 0)) {
		signal_type =	  (1 << 29)	/* video available */
				| (5 << 26)	/* unspecified */
				| (0 << 25)	/* limit */
				| (1 << 24)	/* color available */
				| (9 << 16)	/* 2020 */
				| (16 << 8)	/* 2084 */
				| (9 << 0);	/* 2020 */
		for (i = 0; i < 3; i++)
			for (j = 0; j < 2; j++)
				cus.primaries[i][j] =
					customer_master_display_param[i*2+j];
		for (i = 0; i < 2; i++)
			cus.white_point[i] =
				customer_master_display_param[6+i];
		for (i = 0; i < 2; i++)
			cus.luminance[i] =
				customer_master_display_param[8+i];
		cus.present_flag = 1;
		p_new = &cus;
	}
	if (p_new->present_flag & 1) {
		for (i = 0; i < 3; i++)
			for (j = 0; j < 2; j++) {
				if (p_cur->primaries[i][j]
				 != p_new->primaries[i][j])
					change_flag |= SIG_PRI_INFO;
				p_cur->primaries[i][j]
					= p_new->primaries[i][j];
			}
		for (i = 0; i < 2; i++) {
			if (p_cur->white_point[i]
			 != p_new->white_point[i])
				change_flag |= SIG_PRI_INFO;
			p_cur->white_point[i]
				= p_new->white_point[i];
		}
		for (i = 0; i < 2; i++) {
			if (p_cur->luminance[i]
			 != p_new->luminance[i])
				change_flag |= SIG_PRI_INFO;
			p_cur->luminance[i]
				= p_new->luminance[i];
		}
		if (!p_cur->present_flag) {
			p_cur->present_flag = 1;
			change_flag |= SIG_PRI_INFO;
		}
	} else if (p_cur->present_flag) {
		p_cur->present_flag = 0;
		change_flag |= SIG_PRI_INFO;
	}
	if (change_flag & SIG_PRI_INFO)
		pr_csc("Master_display_colour changed.\n");

	if (signal_type != cur_signal_type) {
		pr_csc("Signal type changed from 0x%x to 0x%x.\n",
			cur_signal_type, signal_type);
		change_flag |= SIG_CS_CHG;
		cur_signal_type = signal_type;
	}
	if (range_control != cur_range_control) {
		pr_csc("Range control changed from 0x%x to 0x%x.\n",
			cur_range_control, range_control);
		change_flag |= SIG_CS_CHG;
		cur_range_control = range_control;
	}
	if (pre_src_type != vf->source_type) {
		pr_csc("Signal source changed from 0x%x to 0x%x.\n",
			pre_src_type, vf->source_type);
		change_flag |= SIG_SRC_CHG;
		pre_src_type = vf->source_type;
	}
	if (cur_knee_factor != knee_factor) {
		pr_csc("Knee factor changed.\n");
		change_flag |= SIG_KNEE_FACTOR;
	}
	if (cur_hdr_process_mode != hdr_process_mode) {
		pr_csc("HDR mode changed.\n");
		change_flag |= SIG_HDR_MODE;
		cur_hdr_process_mode = hdr_process_mode;
	}
	if (cur_sdr_process_mode != sdr_process_mode) {
		pr_csc("SDR mode changed.\n");
		change_flag |= SIG_HDR_MODE;
		cur_sdr_process_mode = sdr_process_mode;
	}
	if (cur_hdr_support != (vinfo->hdr_info.hdr_support & 0x4)) {
		pr_csc("Tx HDR support changed.\n");
		change_flag |= SIG_HDR_SUPPORT;
		cur_hdr_support = vinfo->hdr_info.hdr_support & 0x4;
	}

	if ((cur_eye_protect_mode != wb_val[0]) ||
		(cur_eye_protect_mode == 1)) {
		pr_csc(" eye protect mode changed.\n");
		change_flag |= SIG_WB_CHG;
	}

	return change_flag;
}

#define signal_range ((cur_signal_type >> 25) & 1)
#define signal_color_primaries ((cur_signal_type >> 16) & 0xff)
#define signal_transfer_characteristic ((cur_signal_type >> 8) & 0xff)
enum vpp_matrix_csc_e get_csc_type(void)
{
	enum vpp_matrix_csc_e csc_type = VPP_MATRIX_NULL;
	if ((signal_color_primaries == 1) &&
		(signal_transfer_characteristic < 14)) {
		if (signal_range == 0)
			csc_type = VPP_MATRIX_YUV709_RGB;
		else
			csc_type = VPP_MATRIX_YUV709F_RGB;
	} else if ((signal_color_primaries == 3) &&
			(signal_transfer_characteristic < 14)) {
		if (signal_range == 0)
			csc_type = VPP_MATRIX_YUV601_RGB;
		else
			csc_type = VPP_MATRIX_YUV601F_RGB;
	} else if ((signal_color_primaries == 9) ||
			(signal_transfer_characteristic == 16)) {
		if (signal_transfer_characteristic == 16) {
			/* smpte st-2084 */
			if (signal_color_primaries != 9)
				pr_csc("\tWARNING: non-standard HDR!!!\n");

			if (signal_range == 0)
				csc_type = VPP_MATRIX_BT2020YUV_BT2020RGB;
			else {
				pr_csc("\tWARNING: full range HDR!!!\n");
				csc_type = VPP_MATRIX_BT2020YUV_BT2020RGB;
			}
		} else if (signal_transfer_characteristic == 14) {
			/* bt2020-10 */
			pr_csc("\tWARNING: bt2020-10 HDR!!!\n");
			if (signal_range == 0)
				csc_type = VPP_MATRIX_YUV709_RGB;
			else
				csc_type = VPP_MATRIX_YUV709F_RGB;
		} else if (signal_transfer_characteristic == 15) {
			/* bt2020-12 */
			pr_csc("\tWARNING: bt2020-12 HDR!!!\n");
			if (signal_range == 0)
				csc_type = VPP_MATRIX_YUV709_RGB;
			else
				csc_type = VPP_MATRIX_YUV709F_RGB;
		} else {
			/* unknown transfer characteristic */
			pr_csc("\tWARNING: unknown HDR!!!\n");
			if (signal_range == 0)
				csc_type = VPP_MATRIX_YUV709_RGB;
			else
				csc_type = VPP_MATRIX_YUV709F_RGB;
		}
	} else {
		pr_csc("\tWARNING: unsupported colour space!!!\n");
		if (signal_range == 0)
			csc_type = VPP_MATRIX_YUV601_RGB;
		else
			csc_type = VPP_MATRIX_YUV601F_RGB;
	}
	return csc_type;
}

static void cal_out_curve(uint panel_luma)
{
	int index;
	if (panel_luma == 0)
		return;

	if (panel_luma <= 500) {
		if (panel_luma < 250)
			panel_luma = 250;
		index = (panel_luma - 250) / 20;
	} else {
		if (panel_luma > 1000)
			panel_luma = 1000;
		index = ((500 - 240) / 20) + (panel_luma - 500) / 100;
	}
	memcpy(eotf_33_2084_mapping,
		eotf_33_2084_table[index], sizeof(int) * 33);
	memcpy(oetf_289_gamma22_mapping,
		oetf_289_gamma22_table[index], sizeof(int) * 289);
}
static void mtx_dot_mul(
	int64_t (*a)[3], int64_t (*b)[3],
	int64_t (*out)[3], int32_t norm)
{
	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			out[i][j] = (a[i][j] * b[i][j] + (norm >> 1)) / norm;
}

static void mtx_mul(int64_t (*a)[3], int64_t *b, int64_t *out, int32_t norm)
{
	int j, k;

	for (j = 0; j < 3; j++) {
		out[j] = 0;
		for (k = 0; k < 3; k++)
			out[j] += a[k][j] * b[k];
		out[j] = (out[j] + (norm >> 1)) / norm;
	}
}

static void mtx_mul_mtx(
	int64_t (*a)[3], int64_t (*b)[3],
	int64_t (*out)[3], int32_t norm)
{
	int i, j, k;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			out[i][j] = 0;
			for (k = 0; k < 3; k++)
				out[i][j] += a[k][j] * b[i][k];
			out[i][j] = (out[i][j] + (norm >> 1)) / norm;
		}
}

static void inverse_3x3(
	int64_t (*in)[3], int64_t (*out)[3],
	int32_t norm, int32_t obl)
{
	int i, j;
	int64_t determinant = 0;

	for (i = 0; i < 3; i++)
		determinant +=
			in[0][i] * (in[1][(i + 1) % 3] * in[2][(i + 2) % 3]
			- in[1][(i + 2) % 3] * in[2][(i + 1) % 3]);
	determinant = (determinant + 1) >> 1;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			out[j][i] = (in[(i + 1) % 3][(j + 1) % 3]
				* in[(i + 2) % 3][(j + 2) % 3]);
			out[j][i] -= (in[(i + 1) % 3][(j + 2) % 3]
				* in[(i + 2) % 3][(j + 1) % 3]);
			out[j][i] = (out[j][i] * norm) << (obl - 1);
			out[j][i] =
				(out[j][i] + (determinant >> 1)) / determinant;
		}
	}
}

static void calc_T(
	int32_t (*prmy)[2], int64_t (*Tout)[3],
	int32_t norm, int32_t obl)
{
	int i, j;
	int64_t z[4];
	int64_t A[3][3];
	int64_t B[3];
	int64_t C[3];
	int64_t D[3][3];
	int64_t E[3][3];

	for (i = 0; i < 4; i++)
		z[i] = norm - prmy[i][0] - prmy[i][1];

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 2; j++)
			A[i][j] = prmy[i][j];
		A[i][2] = z[i];
	}
	B[0] = (norm * prmy[3][0] * 2 / prmy[3][1] + 1) >> 1;
	B[1] = norm;
	B[2] = (norm * z[3] * 2 / prmy[3][1] + 1) >> 1;
	inverse_3x3(A, D, norm, obl);
	mtx_mul(D, B, C, norm);
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			E[i][j] = C[i];
	mtx_dot_mul(A, E, Tout, norm);
}

static void gamut_mtx(
	int32_t (*src_prmy)[2], int32_t (*dst_prmy)[2],
	int64_t (*Tout)[3], int32_t norm, int32_t obl)
{
	int64_t Tsrc[3][3];
	int64_t Tdst[3][3];
	int64_t out[3][3];

	calc_T(src_prmy, Tsrc, norm, obl);
	calc_T(dst_prmy, Tdst, norm, obl);
	inverse_3x3(Tdst, out, 1 << obl, obl);
	mtx_mul_mtx(out, Tsrc, Tout, 1 << obl);
}

static void apply_scale_factor(int64_t (*in)[3], int32_t *rs)
{
	int i, j;
	int32_t right_shift;

	if (display_scale_factor > 2 * 2048)
		right_shift = -2;
	else if (display_scale_factor > 2048)
		right_shift = -1;
	else
		right_shift = 0;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			in[i][j] *= display_scale_factor;
			in[i][j] >>= 11 - right_shift;
		}
	right_shift += 1;
	if (right_shift < 0)
		*rs = 8 + right_shift;
	else
		*rs = right_shift;
}

static void N2C(int64_t (*in)[3], int32_t ibl, int32_t obl)
{
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			in[i][j] =
				(in[i][j] + (1 << (ibl - 12))) >> (ibl - 11);
			if (in[i][j] < 0)
				in[i][j] += 1 << obl;
		}
}

static void cal_mtx_seting(
	int64_t (*in)[3],
	int32_t ibl, int32_t obl,
	struct matrix_s *m)
{
	int i, j;
	int32_t right_shift;
	if (get_cpu_type() > MESON_CPU_MAJOR_ID_GXTVBB) {
		apply_scale_factor(in, &right_shift);
		m->right_shift = right_shift;
	}
	N2C(in, ibl, obl);
	pr_csc("\tHDR color convert matrix:\n");
	for (i = 0; i < 3; i++) {
		m->pre_offset[i] = 0;
		for (j = 0; j < 3; j++)
			m->matrix[i][j] = in[j][i];
		m->offset[i] = 0;
		pr_csc("\t\t%04x %04x %04x\n",
			(int)(m->matrix[i][0] & 0xffff),
			(int)(m->matrix[i][1] & 0xffff),
			(int)(m->matrix[i][2] & 0xffff));
	}
}

static int check_primaries(
	/* src primaries and white point */
	uint (*p)[3][2],
	uint (*w)[2],
	/* dst display info from vinfo */
	const struct vinfo_s *v,
	/* prepare src and dst color info array */
	int32_t (*si)[4][2], int32_t (*di)[4][2])
{
	int i, j;
	/* always calculate to apply scale factor */
	int need_calculate_mtx = 1;
	const struct master_display_info_s *d;

	/* check and copy primaries */
	if (hdr_flag & 1) {
		if (((*p)[0][1] > (*p)[1][1])
			&& ((*p)[0][1] > (*p)[2][1])
			&& ((*p)[2][0] > (*p)[0][0])
			&& ((*p)[2][0] > (*p)[1][0])) {
			/* reasonable g,b,r */
			for (i = 0; i < 3; i++)
				for (j = 0; j < 2; j++) {
					(*si)[i][j] = (*p)[(i + 2) % 3][j];
				if ((*si)[i][j] !=
				bt2020_primaries[(i + 2) % 3][j])
					need_calculate_mtx = 1;
				}
		} else if (((*p)[0][0] > (*p)[1][0])
			&& ((*p)[0][0] > (*p)[2][0])
			&& ((*p)[1][1] > (*p)[0][1])
			&& ((*p)[1][1] > (*p)[2][1])) {
			/* reasonable r,g,b */
			for (i = 0; i < 3; i++)
				for (j = 0; j < 2; j++) {
					(*si)[i][j] = (*p)[i][j];
					if ((*si)[i][j] !=
					bt2020_primaries[(i + 2) % 3][j])
						need_calculate_mtx = 1;
				}
		} else {
			/* source not usable, use standard bt2020 */
			for (i = 0; i < 3; i++)
				for (j = 0; j < 2; j++)
					(*si)[i][j] =
						bt2020_primaries
						[(i + 2) % 3][j];
		}
		/* check white point */
		if (need_calculate_mtx == 1) {
			if (((*w)[0] > (*si)[2][0]) &&
				((*w)[0] < (*si)[0][0]) &&
				((*w)[1] > (*si)[2][1]) &&
				((*w)[1] < (*si)[1][1])) {
					for (i = 0; i < 2; i++)
						(*si)[3][i] = (*w)[i];
			} else {
				for (i = 0; i < 3; i++)
					for (j = 0; j < 2; j++)
						(*si)[i][j] =
					bt2020_primaries[(i + 2) % 3][j];

				for (i = 0; i < 2; i++)
					(*si)[3][i] = bt2020_white_point[i];
				/* need_calculate_mtx = 0; */
			}
		} else {
			if (((*w)[0] > (*si)[2][0]) &&
				((*w)[0] < (*si)[0][0]) &&
				((*w)[1] > (*si)[2][1]) &&
				((*w)[1] < (*si)[1][1])) {
				for (i = 0; i < 2; i++) {
					(*si)[3][i] = (*w)[i];
					if ((*si)[3][i] !=
					bt2020_white_point[i])
						need_calculate_mtx = 1;
				}
			} else {
				for (i = 0; i < 2; i++)
					(*si)[3][i] = bt2020_white_point[i];
			}
		}
	} else {
		/* use standard bt2020 */
		for (i = 0; i < 3; i++)
			for (j = 0; j < 2; j++)
				(*si)[i][j] = bt2020_primaries[(i + 2) % 3][j];
		for (i = 0; i < 2; i++)
			(*si)[3][i] = bt2020_white_point[i];
	}

	/* check display */
	if ((v->master_display_info.present_flag) && (hdr_flag & 2)) {
		d = &v->master_display_info;
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 2; j++) {
				(*di)[i][j] = d->primaries[(i + 2) % 3][j];
				if ((*di)[i][j] !=
				bt709_primaries[(i + 2) % 3][j])
					need_calculate_mtx = 1;
			}
		}
		for (i = 0; i < 2; i++) {
			(*di)[3][i] = d->white_point[i];
			if ((*di)[3][i] != bt709_white_point[i])
				need_calculate_mtx = 1;
		}
		if (v->hdr_info.sink_flag)
			cal_out_curve(v->hdr_info.lumi_max);
	} else {
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 2; j++)
				(*di)[i][j] = bt709_primaries[(i + 2) % 3][j];
		}
		for (i = 0; i < 2; i++)
			(*di)[3][i] = bt709_white_point[i];
	}
	return need_calculate_mtx;
}

enum vpp_matrix_csc_e prepare_customer_matrix(
	u32 (*s)[3][2],	/* source primaries */
	u32 (*w)[2],	/* source white point */
	const struct vinfo_s *v, /* vinfo carry display primaries */
	struct matrix_s *m,
	bool inverse_flag)
{
	int32_t prmy_src[4][2];
	int32_t prmy_dst[4][2];
	int64_t out[3][3];
	int i, j;

	if ((customer_matrix_en &&
		get_cpu_type() <= MESON_CPU_MAJOR_ID_GXTVBB)) {
		for (i = 0; i < 3; i++) {
			m->pre_offset[i] =
				customer_matrix_param[i];
			for (j = 0; j < 3; j++)
				m->matrix[i][j] =
					customer_matrix_param[3 + i * 3 + j];
			m->offset[i] =
				customer_matrix_param[12 + i];
		}
		m->right_shift =
			customer_matrix_param[15];
		return VPP_MATRIX_BT2020RGB_CUSRGB;
	} else {
		if (inverse_flag) {
			if (check_primaries(s, w, v, &prmy_src, &prmy_dst)) {
				gamut_mtx(prmy_dst, prmy_src, out, INORM, BL);
				cal_mtx_seting(out, BL, 13, m);
			}
		} else {
			if (check_primaries(s, w, v, &prmy_src, &prmy_dst)) {
				gamut_mtx(prmy_src, prmy_dst, out, INORM, BL);
				cal_mtx_seting(out, BL, 13, m);
			}
		}
		return VPP_MATRIX_BT2020RGB_CUSRGB;
	}
	return VPP_MATRIX_BT2020YUV_BT2020RGB;
}

/* Max luminance lookup table for contrast */
static const int maxLuma_thrd[5] = {512, 1024, 2048, 4096, 8192};
static int calculate_contrast_adj(int max_lumin)
{
	int k;
	int left, right, norm, alph;
	int ratio, target_contrast;

	if (max_lumin < maxLuma_thrd[0])
		k = 0;
	else if (max_lumin < maxLuma_thrd[1])
		k = 1;
	else if (max_lumin < maxLuma_thrd[2])
		k = 2;
	else if (max_lumin < maxLuma_thrd[3])
		k = 3;
	else if (max_lumin < maxLuma_thrd[4])
		k = 4;
	else
		k = 5;

	if (k == 0)
		ratio = extra_con_lut[0];
	else if (k == 5)
		ratio = extra_con_lut[4];
	else {
		left = extra_con_lut[k - 1];
		right = extra_con_lut[k];
		norm = maxLuma_thrd[k] - maxLuma_thrd[k - 1];
		alph = max_lumin - maxLuma_thrd[k - 1];
		ratio = left + (alph * (right - left) + (norm >> 1)) / norm;
	}
	target_contrast = ((vd1_contrast + 1024) * ratio + 64) >> 7;
	target_contrast = clip(target_contrast, 0, 2047);
	target_contrast -= 1024;
	return target_contrast - vd1_contrast;
}

static void print_primaries_info(struct vframe_master_display_colour_s *p)
{
	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 2; j++)
			pr_csc(
				"\t\tprimaries[%1d][%1d] = %04x\n",
				i, j,
				p->primaries[i][j]);
	pr_csc("\t\twhite_point = (%04x, %04x)\n",
		p->white_point[0], p->white_point[1]);
	pr_csc("\t\tmax,min luminance = %08x, %08x\n",
		p->luminance[0], p->luminance[1]);
}

static void amvecm_cp_hdr_info(struct master_display_info_s *hdr_data,
		struct vframe_master_display_colour_s *p)
{
	int i, j;
	if (customer_hdmi_display_en) {
		hdr_data->features =
			  (1 << 29)	/* video available */
			| (5 << 26)	/* unspecified */
			| (0 << 25)	/* limit */
			| (1 << 24)	/* color available */
			| (customer_hdmi_display_param[0] << 16) /* bt2020 */
			| (customer_hdmi_display_param[1] << 8)	/* 2084 */
			| (10 << 0);	/* bt2020c */
		memcpy(hdr_data->primaries,
			&customer_hdmi_display_param[2],
			sizeof(u32)*6);
		memcpy(hdr_data->white_point,
			&customer_hdmi_display_param[8],
			sizeof(u32)*2);
		hdr_data->luminance[0] =
				customer_hdmi_display_param[10];
		hdr_data->luminance[1] =
				customer_hdmi_display_param[11];
		hdr_data->max_content =
				customer_hdmi_display_param[12];
		hdr_data->max_frame_average =
				customer_hdmi_display_param[13];
	} else if (((hdr_data->features >> 16) & 0xff) == 9) {
		if (p->present_flag & 1) {
			memcpy(hdr_data->primaries,
				p->primaries,
				sizeof(u32)*6);
			memcpy(hdr_data->white_point,
				p->white_point,
				sizeof(u32)*2);
			hdr_data->luminance[0] =
				p->luminance[0];
			hdr_data->luminance[1] =
				p->luminance[1];
			if (p->content_light_level.present_flag == 1) {
				hdr_data->max_content =
					p->content_light_level.max_content;
				hdr_data->max_frame_average =
					p->content_light_level.max_pic_average;
			} else {
				hdr_data->max_content = 0;
				hdr_data->max_frame_average = 0;
			}
		} else {
			for (i = 0; i < 3; i++)
				for (j = 0; j < 2; j++)
					hdr_data->primaries[i][j] =
							bt2020_primaries[i][j];
			hdr_data->white_point[0] = bt709_white_point[0];
			hdr_data->white_point[1] = bt709_white_point[1];
			/* default luminance */
			hdr_data->luminance[0] = 1000 * 10000;
			hdr_data->luminance[1] = 50;

			/* content_light_level */
			hdr_data->max_content = 0;
			hdr_data->max_frame_average = 0;
		}
		hdr_data->luminance[0] = hdr_data->luminance[0] / 10000;
		hdr_data->present_flag = 1;
	} else
		memset(hdr_data->primaries, 0, 10 * sizeof(unsigned int));

	/* hdr send information debug */
	memcpy(&dbg_hdr_send, hdr_data,
			sizeof(struct master_display_info_s));

}

static void hdr_process_pq_enable(int enable)
{
	dnlp_en = enable;
	/*cm_en = enable;*/
}

static void vpp_lut_curve_set(enum vpp_lut_sel_e lut_sel)
{
	if (lut_sel == VPP_LUT_EOTF) {
		/* eotf lut 2048 */
		if ((get_cpu_type() == MESON_CPU_MAJOR_ID_GXL) ||
			(get_cpu_type() == MESON_CPU_MAJOR_ID_GXM)) {
			if (video_lut_swtich == 1)
				/*350nit alpha_low = 0.12; */
				set_vpp_lut(VPP_LUT_EOTF,
					eotf_33_2084_mapping_level1_box, /* R */
					eotf_33_2084_mapping_level1_box, /* G */
					eotf_33_2084_mapping_level1_box, /* B */
					CSC_ON);
			else if (video_lut_swtich == 2)
				/*800nit alpha_low = 0.12; */
				set_vpp_lut(VPP_LUT_EOTF,
					eotf_33_2084_mapping_level2_box, /* R */
					eotf_33_2084_mapping_level2_box, /* G */
					eotf_33_2084_mapping_level2_box, /* B */
					CSC_ON);
			else if (video_lut_swtich == 3)
				/*400nit alpha_low = 0.20; */
				set_vpp_lut(VPP_LUT_EOTF,
					eotf_33_2084_mapping_level3_box, /* R */
					eotf_33_2084_mapping_level3_box, /* G */
					eotf_33_2084_mapping_level3_box, /* B */
					CSC_ON);
			else if (video_lut_swtich == 4)
				/*450nit alpha_low = 0.12; */
				set_vpp_lut(VPP_LUT_EOTF,
					eotf_33_2084_mapping_level4_box, /* R */
					eotf_33_2084_mapping_level4_box, /* G */
					eotf_33_2084_mapping_level4_box, /* B */
					CSC_ON);
			else
				/* eotf lut 2048 */
				/*600nit  alpha_low = 0.12;*/
				set_vpp_lut(VPP_LUT_EOTF,
					eotf_33_2084_mapping_box, /* R */
					eotf_33_2084_mapping_box, /* G */
					eotf_33_2084_mapping_box, /* B */
					CSC_ON);
		} else
			set_vpp_lut(VPP_LUT_EOTF,
				eotf_33_2084_mapping, /* R */
				eotf_33_2084_mapping, /* G */
				eotf_33_2084_mapping, /* B */
				CSC_ON);
	} else if (lut_sel == VPP_LUT_OETF) {
		/* oetf lut bypass */
		if ((get_cpu_type() == MESON_CPU_MAJOR_ID_GXL) ||
			(get_cpu_type() == MESON_CPU_MAJOR_ID_GXM)) {
			if (video_lut_swtich == 1)
				set_vpp_lut(VPP_LUT_OETF,
					oetf_289_gamma22_mapping_level1_box,
					oetf_289_gamma22_mapping_level1_box,
					oetf_289_gamma22_mapping_level1_box,
					CSC_ON);
			else if (video_lut_swtich == 2)
				set_vpp_lut(VPP_LUT_OETF,
					oetf_289_gamma22_mapping_level2_box,
					oetf_289_gamma22_mapping_level2_box,
					oetf_289_gamma22_mapping_level2_box,
					CSC_ON);
			else if (video_lut_swtich == 3)
				set_vpp_lut(VPP_LUT_OETF,
					oetf_289_gamma22_mapping_level3_box,
					oetf_289_gamma22_mapping_level3_box,
					oetf_289_gamma22_mapping_level3_box,
					CSC_ON);
			else if (video_lut_swtich == 4)
				set_vpp_lut(VPP_LUT_OETF,
					oetf_289_gamma22_mapping_level4_box,
					oetf_289_gamma22_mapping_level4_box,
					oetf_289_gamma22_mapping_level4_box,
					CSC_ON);
			else
				/* oetf lut bypass */
				set_vpp_lut(VPP_LUT_OETF,
					oetf_289_gamma22_mapping_box,
					oetf_289_gamma22_mapping_box,
					oetf_289_gamma22_mapping_box,
					CSC_ON);
		} else
			set_vpp_lut(VPP_LUT_OETF,
				oetf_289_gamma22_mapping,
				oetf_289_gamma22_mapping,
				oetf_289_gamma22_mapping,
				CSC_ON);
	}
}
static int hdr_process(
	enum vpp_matrix_csc_e csc_type,
	struct vinfo_s *vinfo,
	struct vframe_master_display_colour_s *master_info)
{
	int need_adjust_contrast_saturation = 0;
	int max_lumin = 10000;
	struct matrix_s m = {
		{0, 0, 0},
		{
			{0x0d49, 0x1b4d, 0x1f6b},
			{0x1f01, 0x0910, 0x1fef},
			{0x1fdb, 0x1f32, 0x08f3},
		},
		{0, 0, 0},
		1
	};
	struct matrix_s osd_m = {
		{0, 0, 0},
		{
			{0x505, 0x2A2, 0x059},
			{0x08E, 0x75B, 0x017},
			{0x022, 0x0B4, 0x72A},
		},
		{0, 0, 0},
		1
	};
	int mtx[EOTF_COEFF_SIZE] = {
		EOTF_COEFF_NORM(1.6607056/2), EOTF_COEFF_NORM(-0.5877533/2),
		EOTF_COEFF_NORM(-0.0729065/2),
		EOTF_COEFF_NORM(-0.1245575/2), EOTF_COEFF_NORM(1.1329346/2),
		EOTF_COEFF_NORM(-0.0083771/2),
		EOTF_COEFF_NORM(-0.0181122/2), EOTF_COEFF_NORM(-0.1005249/2),
		EOTF_COEFF_NORM(1.1186371/2),
		EOTF_COEFF_RIGHTSHIFT,
	};
	int osd_mtx[EOTF_COEFF_SIZE] = {
		EOTF_COEFF_NORM(0.627441),	EOTF_COEFF_NORM(0.329285),
		EOTF_COEFF_NORM(0.043274),
		EOTF_COEFF_NORM(0.069092),	EOTF_COEFF_NORM(0.919556),
		EOTF_COEFF_NORM(0.011322),
		EOTF_COEFF_NORM(0.016418),	EOTF_COEFF_NORM(0.088058),
		EOTF_COEFF_NORM(0.895554),
		EOTF_COEFF_RIGHTSHIFT
	};
	int i, j;

	if (master_info->present_flag & 1) {
		pr_csc("\tMaster_display_colour available.\n");
		print_primaries_info(master_info);
		/* for VIDEO */
		csc_type =
			prepare_customer_matrix(
				&master_info->primaries,
				&master_info->white_point,
				vinfo, &m, 0);
		/* for OSD */
		if (get_cpu_type() > MESON_CPU_MAJOR_ID_GXTVBB)
			prepare_customer_matrix(
				&master_info->primaries,
				&master_info->white_point,
				vinfo, &osd_m, 1);
		need_adjust_contrast_saturation |= 1;
	} else {
		/* use bt2020 primaries */
		pr_csc("\tNo master_display_colour.\n");
		/* for VIDEO */
		csc_type =
			prepare_customer_matrix(
			&bt2020_primaries,
			&bt2020_white_point,
			vinfo, &m, 0);
		/* for OSD */
		if (get_cpu_type() > MESON_CPU_MAJOR_ID_GXTVBB)
			prepare_customer_matrix(
				&bt2020_primaries,
				&bt2020_white_point,
				vinfo, &osd_m, 1);
	}

	if (get_cpu_type() > MESON_CPU_MAJOR_ID_GXTVBB) {
		/************** OSD ***************/
		/* RGB to YUV */
		/* not using old RGB2YUV convert HW */
		/* use new 10bit OSD convert matrix */
		/* WRITE_VPP_REG_BITS(VIU_OSD1_BLK0_CFG_W0,
			0, 7, 1);
		*/

		/* eotf lut 709 */
		set_vpp_lut(VPP_LUT_OSD_EOTF,
			osd_eotf_33_709_mapping, /* R */
			osd_eotf_33_709_mapping, /* G */
			osd_eotf_33_709_mapping, /* B */
			CSC_ON);

		/* eotf matrix 709->2020 */
		osd_mtx[EOTF_COEFF_SIZE - 1] = osd_m.right_shift;
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++) {
				if (osd_m.matrix[i][j] & 0x1000)
					osd_mtx[i * 3 + j] =
					-(((~osd_m.matrix[i][j]) & 0xfff) + 1);
				else
					osd_mtx[i * 3 + j] = osd_m.matrix[i][j];
			}
		set_vpp_matrix(VPP_MATRIX_OSD_EOTF,
			osd_mtx,
			CSC_ON);

		/* oetf lut 2084 */
		set_vpp_lut(VPP_LUT_OSD_OETF,
			osd_oetf_41_2084_mapping, /* R */
			osd_oetf_41_2084_mapping, /* G */
			osd_oetf_41_2084_mapping, /* B */
			CSC_ON);

		/* osd matrix RGB2020 to YUV2020 limit */
		set_vpp_matrix(VPP_MATRIX_OSD,
			RGB2020_to_YUV2020l_coeff,
			CSC_ON);

		/************** VIDEO **************/
		/* vd1 matrix bypass */
		set_vpp_matrix(VPP_MATRIX_VD1,
			bypass_coeff,
			CSC_OFF);

		/* post matrix YUV2020 to RGB2020 */
		set_vpp_matrix(VPP_MATRIX_POST,
			YUV2020l_to_RGB2020_coeff,
			CSC_ON);

		/* eotf lut 2048 */
		vpp_lut_curve_set(VPP_LUT_EOTF);

		need_adjust_contrast_saturation = 0;
		saturation_offset =	0;
		if (hdr_flag & 8) {
			need_adjust_contrast_saturation |= 2;
			saturation_offset =	extra_sat_lut[0];
		}
		if (master_info->present_flag & 1) {
			max_lumin = master_info->luminance[0]
				/ 10000;
			if ((max_lumin <= 1200) && (max_lumin > 0)) {
				if (hdr_flag & 4)
					need_adjust_contrast_saturation |= 1;
				if (hdr_flag & 8)
					saturation_offset =
						extra_sat_lut[1];
			}
		}
		/* eotf matrix RGB2020 to RGB709 */
		mtx[EOTF_COEFF_SIZE - 1] = m.right_shift;
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++) {
				if (m.matrix[i][j] & 0x1000)
					mtx[i * 3 + j] =
					-(((~m.matrix[i][j]) & 0xfff) + 1);
				else
					mtx[i * 3 + j] = m.matrix[i][j];
			}
		set_vpp_matrix(VPP_MATRIX_EOTF,
			mtx,
			CSC_ON);

		vpp_lut_curve_set(VPP_LUT_OETF);

		/* xvyccc matrix3: bypass */
		if (vinfo->viu_color_fmt != TVIN_RGB444)
			set_vpp_matrix(VPP_MATRIX_XVYCC,
				RGB709_to_YUV709l_coeff,
				CSC_ON);
	} else {

		/* turn vd1 matrix on */
		vpp_set_matrix(VPP_MATRIX_VD1, CSC_ON,
			csc_type, NULL);
		/* turn post matrix on */
		vpp_set_matrix(VPP_MATRIX_POST, CSC_ON,
			csc_type, &m);
		/* xvycc lut on */
		load_knee_lut(CSC_ON);

		vecm_latch_flag |= FLAG_VADJ1_BRI;
		hdr_process_pq_enable(0);
		/* if GXTVBB HDMI output(YUV) case */
		/* xvyccc matrix3: RGB to YUV */
		/* other cases */
		/* xvyccc matrix3: bypass */
		if ((vinfo->viu_color_fmt != TVIN_RGB444) &&
			(get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB))
			vpp_set_matrix3(CSC_ON, VPP_MATRIX_RGB_YUV709);
		else
			vpp_set_matrix3(CSC_OFF, VPP_MATRIX_NULL);
	}
	return need_adjust_contrast_saturation;
}

static void bypass_hdr_process(
	enum vpp_matrix_csc_e csc_type,
	struct vinfo_s *vinfo,
	struct vframe_master_display_colour_s *master_info)
{
	struct matrix_s osd_m = {
		{0, 0, 0},
		{
			{0x505, 0x2A2, 0x059},
			{0x08E, 0x75B, 0x017},
			{0x022, 0x0B4, 0x72A},
		},
		{0, 0, 0},
		1
	};
	int osd_mtx[EOTF_COEFF_SIZE] = {
		EOTF_COEFF_NORM(0.627441),	EOTF_COEFF_NORM(0.329285),
		EOTF_COEFF_NORM(0.043274),
		EOTF_COEFF_NORM(0.069092),	EOTF_COEFF_NORM(0.919556),
		EOTF_COEFF_NORM(0.011322),
		EOTF_COEFF_NORM(0.016418),	EOTF_COEFF_NORM(0.088058),
		EOTF_COEFF_NORM(0.895554),
		EOTF_COEFF_RIGHTSHIFT
	};
	int i, j;
	if (get_cpu_type() > MESON_CPU_MAJOR_ID_GXTVBB) {
		/************** OSD ***************/
		/* RGB to YUV */
		/* not using old RGB2YUV convert HW */
		/* use new 10bit OSD convert matrix */
		/* WRITE_VPP_REG_BITS(VIU_OSD1_BLK0_CFG_W0,
			0, 7, 1);
		*/
		if ((csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB) &&
			(((vinfo->hdr_info.hdr_support & 0x4) &&
			(vinfo->viu_color_fmt != TVIN_RGB444)) ||
			(vinfo->viu_color_fmt == TVIN_RGB444))) {
			/* OSD convert to HDR to match HDR video */
			/* osd eotf lut 709 */
			set_vpp_lut(VPP_LUT_OSD_EOTF,
				osd_eotf_33_709_mapping, /* R */
				osd_eotf_33_709_mapping, /* G */
				osd_eotf_33_709_mapping, /* B */
				CSC_ON);

			/* osd eotf matrix 709->2020 */
			if (master_info->present_flag & 1) {
				pr_csc("\tMaster_display_colour available.\n");
				print_primaries_info(master_info);
				prepare_customer_matrix(
					&master_info->primaries,
					&master_info->white_point,
					vinfo, &osd_m, 1);
			} else {
				pr_csc("\tNo master_display_colour.\n");
				prepare_customer_matrix(
					&bt2020_primaries,
					&bt2020_white_point,
					vinfo, &osd_m, 1);
			}
			osd_mtx[EOTF_COEFF_SIZE - 1] = osd_m.right_shift;
			for (i = 0; i < 3; i++)
				for (j = 0; j < 3; j++) {
					if (osd_m.matrix[i][j] & 0x1000) {
						osd_mtx[i * 3 + j] =
						(~osd_m.matrix[i][j]) & 0xfff;
						osd_mtx[i * 3 + j] =
						-(1 + osd_mtx[i * 3 + j]);
					} else
						osd_mtx[i * 3 + j] =
							osd_m.matrix[i][j];
				}
			set_vpp_matrix(VPP_MATRIX_OSD_EOTF,
				osd_mtx,
				CSC_ON);

			/* osd oetf lut 2084 */
			set_vpp_lut(VPP_LUT_OSD_OETF,
				osd_oetf_41_2084_mapping, /* R */
				osd_oetf_41_2084_mapping, /* G */
				osd_oetf_41_2084_mapping, /* B */
				CSC_ON);

			/* osd matrix RGB2020 to YUV2020 limit */
			set_vpp_matrix(VPP_MATRIX_OSD,
				RGB2020_to_YUV2020l_coeff,
				CSC_ON);
		} else {
			/* OSD convert to 709 limited to match SDR video */
			/* eotf lut bypass */
			set_vpp_lut(VPP_LUT_OSD_EOTF,
				eotf_33_linear_mapping, /* R */
				eotf_33_linear_mapping, /* G */
				eotf_33_linear_mapping, /* B */
				CSC_OFF);

			/* eotf matrix bypass */
			set_vpp_matrix(VPP_MATRIX_OSD_EOTF,
				eotf_bypass_coeff,
				CSC_OFF);

			/* oetf lut bypass */
			set_vpp_lut(VPP_LUT_OSD_OETF,
				oetf_41_linear_mapping, /* R */
				oetf_41_linear_mapping, /* G */
				oetf_41_linear_mapping, /* B */
				CSC_OFF);

			/* osd matrix RGB709 to YUV709 limit/full */
			if (range_control & 2){
				set_vpp_matrix(VPP_MATRIX_OSD,
					RGB709_to_YUV709_coeff,
					CSC_ON);	/* use full range */
				pr_info("Full range RGB-YUV");
			}
			else {
				set_vpp_matrix(VPP_MATRIX_OSD,
					RGB709_to_YUV709l_coeff,
					CSC_ON);	/* use limit range */
				pr_info("Limited range RGB-YUV");
			}
		}

		/************** VIDEO **************/
		/* vd1 matrix: bypass */
		if (csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB)
			set_vpp_matrix(VPP_MATRIX_VD1,
				bypass_coeff,
				CSC_OFF);	/* limit->limit range */
		else {
			pr_info("Setting VD1 for BT709");
			switch (range_control) {
				case 1:
					/* input is full-range, output is limited */
					set_vpp_matrix(VPP_MATRIX_VD1,
						YUV709f_to_YUV709l_coeff,
						CSC_ON);
					break;
				case 2:
					/* input is limited, output is full-range */
					set_vpp_matrix(VPP_MATRIX_VD1,
						YUV709l_to_YUV709f_coeff,
						CSC_ON);
					break;
				case 0:
				case 3:
				default:
					/* input is same as output */
					set_vpp_matrix(VPP_MATRIX_VD1,
							bypass_coeff,
							CSC_OFF);
					break;
			}
		}

		/* post matrix bypass */
		if (vinfo->viu_color_fmt != TVIN_RGB444)
			/* yuv2rgb for eye protect mode */
			set_vpp_matrix(VPP_MATRIX_POST,
				bypass_coeff,
				CSC_OFF);
		else /* matrix yuv2rgb for LCD */
			set_vpp_matrix(VPP_MATRIX_POST,
				YUV709l_to_RGB709_coeff,
				CSC_ON);
		/* xvycc inv lut */
		if (sdr_process_mode &&
		(csc_type < VPP_MATRIX_BT2020YUV_BT2020RGB) &&
		((get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB) ||
		 (get_cpu_type() == MESON_CPU_MAJOR_ID_TXL)))
			set_vpp_lut(VPP_LUT_INV_EOTF,
				NULL,
				NULL,
				NULL,
				CSC_ON);
		else
			set_vpp_lut(VPP_LUT_INV_EOTF,
				NULL,
				NULL,
				NULL,
				CSC_OFF);

		/* eotf lut bypass */
		set_vpp_lut(VPP_LUT_EOTF,
			NULL, /* R */
			NULL, /* G */
			NULL, /* B */
			CSC_OFF);

		/* eotf matrix bypass */
		set_vpp_matrix(VPP_MATRIX_EOTF,
			eotf_bypass_coeff,
			CSC_OFF);

		/* oetf lut bypass */
		set_vpp_lut(VPP_LUT_OETF,
			NULL,
			NULL,
			NULL,
			CSC_OFF);

		/* xvycc matrix full2limit or bypass */
/*		if (vinfo->viu_color_fmt != TVIN_RGB444) {
			if (csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB)
				set_vpp_matrix(VPP_MATRIX_XVYCC,
					bypass_coeff,
					CSC_OFF);
			else {
				if (range_control)
					set_vpp_matrix(VPP_MATRIX_XVYCC,
						YUV709f_to_YUV709l_coeff,
						CSC_ON);
				else
					set_vpp_matrix(VPP_MATRIX_XVYCC,
						bypass_coeff,
						CSC_OFF);
			}
		}*/
		set_vpp_matrix(VPP_MATRIX_XVYCC,
				bypass_coeff,
				CSC_OFF);

	} else {
		/* OSD */
		/* keep RGB */

		/* VIDEO */
		if (csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB) {
			/* vd1 matrix: convert YUV to RGB */
			csc_type = VPP_MATRIX_YUV709_RGB;
		}
		/* vd1 matrix on to convert YUV to RGB */
		vpp_set_matrix(VPP_MATRIX_VD1, CSC_ON,
			csc_type, NULL);
		/* post matrix off */
		vpp_set_matrix(VPP_MATRIX_POST, CSC_OFF,
			csc_type, NULL);
		/* xvycc lut off */
		load_knee_lut(CSC_OFF);
		/* xvycc inv lut */

		if (sdr_process_mode)
			set_vpp_lut(VPP_LUT_INV_EOTF,
				NULL,
				NULL,
				NULL,
				CSC_ON);
		else
			set_vpp_lut(VPP_LUT_INV_EOTF,
				NULL,
				NULL,
				NULL,
				CSC_OFF);

		vecm_latch_flag |= FLAG_VADJ1_BRI;
		hdr_process_pq_enable(1);
		/* if GXTVBB HDMI output(YUV) case */
		/* xvyccc matrix3: RGB to YUV */
		/* other cases */
		/* xvyccc matrix3: bypass */
		if ((vinfo->viu_color_fmt != TVIN_RGB444) &&
			(get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB))
			vpp_set_matrix3(CSC_ON, VPP_MATRIX_RGB_YUV709);
		else
			vpp_set_matrix3(CSC_OFF, VPP_MATRIX_NULL);
	}
}

static void sdr_hdr_process(
	enum vpp_matrix_csc_e csc_type,
	struct vinfo_s *vinfo,
	struct vframe_master_display_colour_s *master_info)
{
	if ((get_cpu_type() == MESON_CPU_MAJOR_ID_GXL) ||
		(get_cpu_type() == MESON_CPU_MAJOR_ID_GXM)) {
		/* OSD convert to 709 limited to match SDR video */
		/* eotf lut bypass */
		set_vpp_lut(VPP_LUT_OSD_EOTF,
			eotf_33_linear_mapping, /* R */
			eotf_33_linear_mapping, /* G */
			eotf_33_linear_mapping, /* B */
			CSC_OFF);

		/* eotf matrix bypass */
		set_vpp_matrix(VPP_MATRIX_OSD_EOTF,
			eotf_bypass_coeff,
			CSC_OFF);

		/* oetf lut bypass */
		set_vpp_lut(VPP_LUT_OSD_OETF,
			oetf_41_linear_mapping, /* R */
			oetf_41_linear_mapping, /* G */
			oetf_41_linear_mapping, /* B */
			CSC_OFF);

		/* osd matrix RGB709 to YUV709 limit/full */
		if (range_control & 2)
			set_vpp_matrix(VPP_MATRIX_OSD,
				RGB709_to_YUV709_coeff,
				CSC_ON);	/* use full range */
		else
			set_vpp_matrix(VPP_MATRIX_OSD,
				RGB709_to_YUV709l_coeff,
				CSC_ON);	/* use limit range */

		/************** VIDEO **************/
		/* convert SDR Video to HDR */
		if (range_control == 1) 
			set_vpp_matrix(VPP_MATRIX_VD1,
					YUV709f_to_YUV709l_coeff,
					CSC_ON);	/* full->limit range */
		else 
			set_vpp_matrix(VPP_MATRIX_VD1,
					bypass_coeff,
					CSC_OFF);


		set_vpp_matrix(VPP_MATRIX_POST,
			YUV709l_to_RGB709_coeff,
			CSC_ON);

		/* eotf lut bypass */
		set_vpp_lut(VPP_LUT_EOTF,
			eotf_33_sdr_709_mapping, /* R */
			eotf_33_sdr_709_mapping, /* G */
			eotf_33_sdr_709_mapping, /* B */
			CSC_ON);

		/* eotf matrix bypass */
		set_vpp_matrix(VPP_MATRIX_EOTF,
			eotf_RGB709_to_RGB2020_coeff,
			CSC_ON);

		/* oetf lut bypass */
		set_vpp_lut(VPP_LUT_OETF,
			oetf_sdr_2084_mapping,
			oetf_sdr_2084_mapping,
			oetf_sdr_2084_mapping,
			CSC_ON);

		/* xvycc matrix bypass */
		set_vpp_matrix(VPP_MATRIX_XVYCC,
			RGB2020_to_YUV2020l_coeff,
			CSC_ON);
	} else if ((get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB) ||
				(get_cpu_type() == MESON_CPU_MAJOR_ID_TXL)) {
		bypass_hdr_process(csc_type, vinfo, master_info);
	}
}

static int vpp_eye_protection_process(
	enum vpp_matrix_csc_e csc_type,
	struct vinfo_s *vinfo)
{
	cur_eye_protect_mode = wb_val[0];
	memcpy(&video_rgb_ogo, wb_val,
		sizeof(struct tcon_rgb_ogo_s));
	ve_ogo_param_update();

	/* only SDR need switch csc */
	if ((csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB) &&
			hdr_process_mode)
		return 0;
	if ((csc_type < VPP_MATRIX_BT2020YUV_BT2020RGB) &&
			sdr_process_mode)
		return 0;

	/* post matrix bypass */
	if ((vinfo->viu_color_fmt != TVIN_RGB444) &&
		(cur_eye_protect_mode == 0))
		/* yuv2rgb for eye protect mode */
		set_vpp_matrix(VPP_MATRIX_POST,
			bypass_coeff,
			CSC_ON);
	else /* matrix yuv2rgb for LCD */
		set_vpp_matrix(VPP_MATRIX_POST,
			YUV709l_to_RGB709_coeff,
			CSC_ON);

	/* xvycc matrix bypass */
	if ((vinfo->viu_color_fmt != TVIN_RGB444) &&
		(cur_eye_protect_mode == 1))
		/*  for eye protect mode */
		video_rgb_ogo_xvy_mtx_latch &= MTX_RGB2YUVL_RGB_OGO;
	else /* matrix yuv2rgb for LCD */
		set_vpp_matrix(VPP_MATRIX_XVYCC,
			bypass_coeff,
			CSC_ON);

	return 0;
}

static void vpp_matrix_update(struct vframe_s *vf, struct vinfo_s *vinfo)
{
	enum vpp_matrix_csc_e csc_type = VPP_MATRIX_NULL;
	int signal_change_flag = 0;
	struct vframe_master_display_colour_s *p = &cur_master_display_colour;
	struct master_display_info_s send_info;
	int need_adjust_contrast_saturation = 0;

	/* Tx hdr information */
	memcpy(&receiver_hdr_info, &vinfo->hdr_info,
			sizeof(struct hdr_info));

	/* check hdr support info from Tx or Panel */
	if (hdr_mode == 2) { /* auto */
		if (vinfo->hdr_info.hdr_support & 0x4)
			hdr_process_mode = 0; /* hdr->hdr*/
		else
			hdr_process_mode = 1; /* hdr->sdr*/
	} else
		hdr_process_mode = hdr_mode;

	if (sdr_mode == 2) { /* auto */
		if ((vinfo->hdr_info.hdr_support & 0x4) &&
		((get_cpu_type() == MESON_CPU_MAJOR_ID_GXL) ||
		 (get_cpu_type() == MESON_CPU_MAJOR_ID_GXM)))
			sdr_process_mode = 1; /*box sdr->hdr*/
		else if ((vinfo->viu_color_fmt == TVIN_RGB444) &&
			((get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB) ||
			(get_cpu_type() == MESON_CPU_MAJOR_ID_TXL)))
			sdr_process_mode = 1; /*tv sdr->hdr*/
		else
			sdr_process_mode = 0; /* sdr->sdr*/
	} else
		sdr_process_mode = sdr_mode; /* force sdr->hdr */

	signal_change_flag = signal_type_changed(vf, vinfo);

	if ((!signal_change_flag) && (force_csc_type == 0xff))
		return;

	vecm_latch_flag |= FLAG_MATRIX_UPDATE;

	if (force_csc_type != 0xff)
		csc_type = force_csc_type;
	else
		csc_type = get_csc_type();

	if ((vinfo->viu_color_fmt != TVIN_RGB444) &&
		((vinfo->hdr_info.hdr_support & 0x4) ||
		(signal_change_flag & SIG_HDR_SUPPORT))) {
		if (sdr_process_mode &&
			(csc_type < VPP_MATRIX_BT2020YUV_BT2020RGB)) {
			/* sdr source convert to hdr */
			/* send hdr info */
			/* use the features to discribe source info */
			send_info.features =
					  (1 << 29)	/* video available */
					| (5 << 26)	/* unspecified */
					| (0 << 25)	/* limit */
					| (1 << 24)	/* color available */
					| (9 << 16)	/* bt2020 */
					| (14 << 8)	/* bt2020-10 */
					| (10 << 0);	/* bt2020c */
		} else if ((hdr_process_mode == 0) &&
			(csc_type >= VPP_MATRIX_BT2020YUV_BT2020RGB)) {
			/* source is hdr, send hdr info */
			/* use the features to discribe source info */
			send_info.features =
					  (1 << 29)	/* video available */
					| (5 << 26)	/* unspecified */
					| (0 << 25)	/* limit */
					| (1 << 24)	/* color available */
					/* bt2020 */
					| (signal_color_primaries << 16)
					/* bt2020-10 */
					| (signal_transfer_characteristic << 8);
		} else {
			/* sdr source send normal info
			 * use the features to discribe source info */
			send_info.features =
					/* default 709 full */
					  (1 << 29)	/* video available */
					| (5 << 26)	/* unspecified */
					| (1 << 25)	/* full */
					| (1 << 24)	/* color available */
					| (1 << 16)	/* bt709 */
					| (1 << 8)	/* bt709 */
					| (1 << 0);	/* bt709 */
		}
		amvecm_cp_hdr_info(&send_info, p);
		if (vinfo->fresh_tx_hdr_pkt)
			vinfo->fresh_tx_hdr_pkt(&send_info);
	}

	if ((cur_csc_type != csc_type)
	|| (signal_change_flag
	& (SIG_PRI_INFO | SIG_KNEE_FACTOR | SIG_HDR_MODE |
		SIG_HDR_SUPPORT | SIG_CS_CHG))) {
		/* decided by edid or panel info or user setting */
		if ((csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB) &&
			hdr_process_mode) {
			/* hdr->sdr */
			if ((signal_change_flag &
					(SIG_PRI_INFO |
					SIG_KNEE_FACTOR |
					SIG_HDR_MODE |
					SIG_HDR_SUPPORT)
				) ||
				(cur_csc_type <
					VPP_MATRIX_BT2020YUV_BT2020RGB)) {
				need_adjust_contrast_saturation =
					hdr_process(csc_type, vinfo, p);
			}
		} else {
			if ((csc_type < VPP_MATRIX_BT2020YUV_BT2020RGB) &&
				sdr_process_mode)
				/* for gxl and gxm SDR to HDR process */
				sdr_hdr_process(csc_type, vinfo, p);
			else
				/* for gxtvbb and gxl HDR bypass process */
				bypass_hdr_process(csc_type, vinfo, p);

			if ((csc_type == VPP_MATRIX_BT2020YUV_BT2020RGB) &&
				(get_cpu_type() <= MESON_CPU_MAJOR_ID_GXTVBB))
				csc_type = VPP_MATRIX_YUV709_RGB;
		}
		if (need_adjust_contrast_saturation & 1) {
			if (lut_289_en &&
				(get_cpu_type() <= MESON_CPU_MAJOR_ID_GXTVBB))
				vd1_contrast_offset = 0;
			else
				vd1_contrast_offset =
				calculate_contrast_adj(p->luminance[0] / 10000);
			vecm_latch_flag |= FLAG_VADJ1_CON;
		} else {
			vd1_contrast_offset = 0;
			vecm_latch_flag |= FLAG_VADJ1_CON;
		}
		if (need_adjust_contrast_saturation & 2) {
			vecm_latch_flag |= FLAG_VADJ1_COLOR;
		} else {
			if (((get_cpu_type() == MESON_CPU_MAJOR_ID_GXTVBB) ||
				(get_cpu_type() == MESON_CPU_MAJOR_ID_TXL)) &&
				(sdr_process_mode == 1))
				saturation_offset = sdr_saturation_offset;
			else
				saturation_offset =	0;
			vecm_latch_flag |= FLAG_VADJ1_COLOR;
		}
		if (cur_csc_type != csc_type) {
			pr_csc("CSC from 0x%x to 0x%x.\n",
				cur_csc_type, csc_type);
			pr_csc("contrast offset = %d.\n",
				vd1_contrast_offset);
			pr_csc("saturation offset = %d.\n",
				saturation_offset);
			cur_csc_type = csc_type;
		}
	}

	/* eye protection mode */
	if (signal_change_flag & SIG_WB_CHG)
		vpp_eye_protection_process(csc_type, vinfo);

	vecm_latch_flag &= ~FLAG_MATRIX_UPDATE;
}

static struct vframe_s *last_vf;
static int null_vf_cnt;

static unsigned int fg_vf_sw_dbg;
unsigned int null_vf_max = 5;
module_param(null_vf_max, uint, 0664);
MODULE_PARM_DESC(null_vf_max, "\n null_vf_max\n");
void amvecm_matrix_process(struct vframe_s *vf)
{
	struct vframe_s fake_vframe;
	struct vinfo_s *vinfo = get_current_vinfo();
	int i;

	if ((get_cpu_type() < MESON_CPU_MAJOR_ID_GXTVBB) ||
		is_meson_gxl_package_905M2() ||
		skip_csc_en)
		return;

	if (reload_mtx) {
		for (i = 0; i < NUM_MATRIX; i++)
			if (reload_mtx & (1 << i))
				set_vpp_matrix(i, NULL, CSC_ON);
	}

	if (reload_lut) {
		for (i = 0; i < NUM_LUT; i++)
			if (reload_lut & (1 << i))
				set_vpp_lut(i,
					NULL, /* R */
					NULL, /* G */
					NULL, /* B */
					CSC_ON);
	}

	/* OSD sdr -> hdr in first vsync when hdr output */
	if ((vinfo->hdr_info.hdr_support & 0x4) && (vf == NULL) &&
		((get_cpu_type() == MESON_CPU_MAJOR_ID_GXL) ||
		 (get_cpu_type() == MESON_CPU_MAJOR_ID_GXM))) {
		if (((sdr_process_mode != 1) && (sdr_mode > 0))
		|| ((sdr_process_mode > 0) && (sdr_mode == 0)))
			null_vf_cnt = null_vf_max + 1;
	}

	if ((vf == last_vf) && (null_vf_cnt <= null_vf_max) &&
		(cur_eye_protect_mode == wb_val[0]))
		return;

	if (is_dolby_vision_on())
		return;

	if (vf != NULL) {
		vpp_matrix_update(vf, vinfo);
		last_vf = vf;
		null_vf_cnt = 0;
		fg_vf_sw_dbg = 1;

		/* debug vframe info backup */
		dbg_vf = vf;
	} else {
		/* check last signal type */
		if ((last_vf != NULL) /*&&
			((((last_vf->signal_type >> 16) & 0xff) == 9)
			|| customer_master_display_en
			|| (((last_vf->signal_type >> 8) & 0xff) >= 14))*/)
			null_vf_cnt++;

		if ((((READ_VPP_REG(VPP_MISC) & (1<<10)) == 0)
			&& (null_vf_cnt > null_vf_max)) ||
			(cur_eye_protect_mode != wb_val[0])) {
			/* send a faked vframe to switch matrix
			   from 2020 to 601 when video disabled */
			fake_vframe.source_type = VFRAME_SOURCE_TYPE_OTHERS;
			fake_vframe.signal_type = 0;
			fake_vframe.width = 720;
			fake_vframe.height = 480;
			fake_vframe.prop.master_display_colour.present_flag
				= 0x80000000;
			vpp_matrix_update(&fake_vframe, vinfo);
			pr_csc("change CSC when disable video.\n");
			last_vf = vf;
			null_vf_cnt = 0;
			fg_vf_sw_dbg = 2;
		} else
			fg_vf_sw_dbg = 3;
	}
}

int amvecm_hdr_dbg(u32 sel)
{
	int i, j;

	/* select debug information */
	if (sel == 1) /* dump reg */
		goto reg_dump;

	if (dbg_vf == NULL)
		goto hdr_dump;

	/*
	pr_err("----vframe info----\n");
	pr_err("index:%d, type:0x%x, type_backup:0x%x, blend_mode:%d\n",
		dbg_vf->index, dbg_vf->type,
		dbg_vf->type_backup, dbg_vf->blend_mode);
	pr_err("duration:%d, duration_pulldown:%d, pts:%d, flag:0x%x\n",
		dbg_vf->duration, dbg_vf->duration_pulldown,
		dbg_vf->pts, dbg_vf->flag);
	pr_err("canvas0Addr:0x%x, canvas1Addr:0x%x, bufWidth:%d\n",
		dbg_vf->canvas0Addr, dbg_vf->canvas1Addr,
		dbg_vf->bufWidth);
	pr_err("width:%d, height:%d, ratio_control:0x%x, bitdepth:%d\n",
		dbg_vf->width, dbg_vf->height,
		dbg_vf->ratio_control, dbg_vf->bitdepth);
	pr_err("signal_type:%x, orientation:%d, video_angle:0x%x\n",
		dbg_vf->signal_type, dbg_vf->orientation,
		dbg_vf->video_angle);
	pr_err("source_type:%d, phase:%d, soruce_mode:%d, sig_fmt:0x%x\n",
		dbg_vf->source_type, dbg_vf->phase,
		dbg_vf->source_mode, dbg_vf->sig_fmt);
	pr_err(
		"trans_fmt 0x%x, lefteye(%d %d %d %d), righteye(%d %d %d %d)\n",
		vf->trans_fmt, vf->left_eye.start_x, vf->left_eye.start_y,
		vf->left_eye.width, vf->left_eye.height,
		vf->right_eye.start_x, vf->right_eye.start_y,
		vf->right_eye.width, vf->right_eye.height);
	pr_err("mode_3d_enable %d",
		vf->mode_3d_enable);
	pr_err("early_process_fun 0x%p, process_fun 0x%p,
	private_data %p\n",
		vf->early_process_fun, vf->process_fun, vf->private_data);

	pr_err("hist_pow %d, luma_sum %d, chroma_sum %d, pixel_sum %d\n",
		vf->prop.hist.hist_pow, vf->prop.hist.luma_sum,
		vf->prop.hist.chroma_sum, vf->prop.hist.pixel_sum);

	pr_err("height %d, width %d, luma_max %d, luma_min %d\n",
		vf->prop.hist.hist_pow, vf->prop.hist.hist_pow,
		vf->prop.hist.hist_pow, vf->prop.hist.hist_pow);

	pr_err("vpp_luma_sum %d, vpp_chroma_sum %d, vpp_pixel_sum %d\n",
		vf->prop.hist.vpp_luma_sum, vf->prop.hist.vpp_chroma_sum,
		vf->prop.hist.vpp_pixel_sum);

	pr_err("vpp_height %d, vpp_width %d, vpp_luma_max %d,
	 vpp_luma_min %d\n",
		vf->prop.hist.vpp_height, vf->prop.hist.vpp_width,
		vf->prop.hist.vpp_luma_max, vf->prop.hist.vpp_luma_min);

	pr_err("vs_span_cnt %d, vs_cnt %d, hs_cnt0 %d, hs_cnt1 %d\n",
		vf->prop.meas.vs_span_cnt, vf->prop.meas.vs_cnt,
		vf->prop.meas.hs_cnt0, vf->prop.meas.hs_cnt1);

	pr_err("hs_cnt2 %d, vs_cnt %d, hs_cnt3 %d,
	 vs_cycle %d, vs_stamp %d\n",
		vf->prop.meas.hs_cnt2, vf->prop.meas.hs_cnt3,
		vf->prop.meas.vs_cycle, vf->prop.meas.vs_stamp);

	pr_err("pixel_ratio:%d list:%p ready_jiffies64:%lld, frame_dirty %d\n",
		dbg_vf->pixel_ratio, &dbg_vf->list,
		dbg_vf->ready_jiffies64, dbg_vf->frame_dirty);
	*/

	pr_err("----Video frame info----\n");
	pr_err("bitdepth:0x%x, signal_type:0x%x, present_flag:0x%x\n",
		dbg_vf->bitdepth,
		dbg_vf->signal_type,
		dbg_vf->prop.master_display_colour.present_flag);

	if (((dbg_vf->signal_type >> 16) & 0xff) == 9) {
		pr_err("HDR color primaries:0x%x\n",
			((dbg_vf->signal_type >> 16) & 0xff));
		pr_err("HDR transfer_characteristic:0x%x\n",
			((dbg_vf->signal_type >> 8) & 0xff));
	} else
		pr_err("SDR color primaries:0x%x\n", signal_color_primaries);

	if (dbg_vf->prop.master_display_colour.present_flag == 1) {
		pr_err("----SEI info----\n");
		for (i = 0; i < 3; i++)
			for (j = 0; j < 2; j++)
				pr_err(
					"\tprimaries[%1d][%1d] = %04x\n",
				i, j,
			dbg_vf->prop.master_display_colour.primaries[i][j]);
		pr_err("\twhite_point = (%04x, %04x)\n",
			dbg_vf->prop.master_display_colour.white_point[0],
			dbg_vf->prop.master_display_colour.white_point[1]);
		pr_err("\tmax,min luminance = %08x, %08x\n",
			dbg_vf->prop.master_display_colour.luminance[0],
			dbg_vf->prop.master_display_colour.luminance[1]);
	}

hdr_dump:
	pr_err("----HDR process info----\n");
	pr_err("customer_master_display_en:0x%x\n", customer_master_display_en);

	pr_err("hdr_mode:0x%x, hdr_process_mode:0x%x, cur_hdr_process_mode:0x%x\n",
		hdr_mode, hdr_process_mode, cur_hdr_process_mode);

	pr_err("sdr_mode:0x%x, sdr_process_mode:0x%x, cur_sdr_process_mode:0x%x\n",
		sdr_mode, sdr_process_mode, cur_sdr_process_mode);

	pr_err("hdr_flag:0x%x,     fg_vf_sw_dbg:0x%x\n",
		hdr_flag, fg_vf_sw_dbg);
	pr_err("cur_signal_type:0x%x, cur_csc_mode:0x%x, cur_csc_type:0x%x\n",
		cur_signal_type, cur_csc_mode, cur_csc_type);

	pr_err("knee_lut_on:0x%x,knee_interpolation_mode:0x%x,cur_knee_factor:0x%x\n",
		knee_lut_on, knee_interpolation_mode, cur_knee_factor);

	if ((receiver_hdr_info.hdr_support & 0x4) == 0)
		goto dbg_end;
	pr_err("----TV EDID info----\n");
	pr_err("hdr_support:0x%x, lumi_max:%d, lumi_avg:%d, lumi_min:%d\n",
		receiver_hdr_info.hdr_support,
		receiver_hdr_info.lumi_max,
		receiver_hdr_info.lumi_avg,
		receiver_hdr_info.lumi_min);

	pr_err("----Tx HDR package info----\n");
	pr_err("\tfeatures = 0x%08x\n", dbg_hdr_send.features);
	for (i = 0; i < 3; i++)
		for (j = 0; j < 2; j++)
			pr_err(
				"\tprimaries[%1d][%1d] = %04x\n",
				i, j,
				dbg_hdr_send.primaries[i][j]);
	pr_err("\twhite_point = (%04x, %04x)\n",
		dbg_hdr_send.white_point[0],
		dbg_hdr_send.white_point[1]);
	pr_err("\tmax,min luminance = %08x, %08x\n",
		dbg_hdr_send.luminance[0], dbg_hdr_send.luminance[1]);

	goto dbg_end;

	/************************dump reg start***************************/
reg_dump:

	/* osd matrix, VPP_MATRIX_0 */
	pr_err("----dump regs VPP_MATRIX_OSD----\n");
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_PRE_OFFSET0_1,
		READ_VPP_REG(VIU_OSD1_MATRIX_PRE_OFFSET0_1));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_PRE_OFFSET2,
		READ_VPP_REG(VIU_OSD1_MATRIX_PRE_OFFSET2));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF00_01,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF00_01));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF02_10,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF02_10));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF11_12,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF11_12));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF20_21,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF20_21));

	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF22_30,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF22_30));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF31_32,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF31_32));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF40_41,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF40_41));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COLMOD_COEF42,
		READ_VPP_REG(VIU_OSD1_MATRIX_COLMOD_COEF42));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COEF22_30,
		READ_VPP_REG(VIU_OSD1_MATRIX_COEF22_30));

	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_OFFSET0_1,
		READ_VPP_REG(VIU_OSD1_MATRIX_OFFSET0_1));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_OFFSET2,
		READ_VPP_REG(VIU_OSD1_MATRIX_OFFSET2));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COLMOD_COEF42,
		READ_VPP_REG(VIU_OSD1_MATRIX_COLMOD_COEF42));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_COLMOD_COEF42,
		READ_VPP_REG(VIU_OSD1_MATRIX_COLMOD_COEF42));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_PRE_OFFSET0_1,
		READ_VPP_REG(VIU_OSD1_MATRIX_PRE_OFFSET0_1));
	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_MATRIX_CTRL,
		READ_VPP_REG(VIU_OSD1_MATRIX_CTRL));

	/* osd eotf matrix, VPP_MATRIX_OSD_EOTF */
	pr_err("----dump regs VPP_MATRIX_OSD_EOTF----\n");

	for (i = 0; i < 5; i++)
		pr_err("\taddr = %08x, val = %08x\n",
			(VIU_OSD1_EOTF_CTL + i + 1),
			READ_VPP_REG(VIU_OSD1_EOTF_CTL + i + 1));

	pr_err("\taddr = %08x, val = %08x\n",
		VIU_OSD1_EOTF_CTL,
		READ_VPP_REG(VIU_OSD1_EOTF_CTL));

	{
		unsigned short r_map[VIDEO_OETF_LUT_SIZE];
		unsigned short g_map[VIDEO_OETF_LUT_SIZE];
		unsigned short b_map[VIDEO_OETF_LUT_SIZE];
		unsigned int addr_port;
		unsigned int data_port;
		unsigned int ctrl_port;
		unsigned int data;
		int i;

		pr_err("----dump regs VPP_LUT_OSD_OETF----\n");

		addr_port = VIU_OSD1_OETF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_OETF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_OETF_CTL;

		pr_err("\taddr = %08x, val = %08x\n",
			ctrl_port, READ_VPP_REG(ctrl_port));

		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, i);
			data = READ_VPP_REG(data_port);
			r_map[i * 2] = data & 0xffff;
			r_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		WRITE_VPP_REG(addr_port, 20);
		data = READ_VPP_REG(data_port);
		r_map[OSD_OETF_LUT_SIZE - 1] = data & 0xffff;
		g_map[0] = (data >> 16) & 0xffff;
		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, 21 + i);
			data = READ_VPP_REG(data_port);
			g_map[i * 2 + 1] = data & 0xffff;
			g_map[i * 2 + 2] = (data >> 16) & 0xffff;
		}
		for (i = 0; i < 20; i++) {
			WRITE_VPP_REG(addr_port, 41 + i);
			data = READ_VPP_REG(data_port);
			b_map[i * 2] = data & 0xffff;
			b_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		WRITE_VPP_REG(addr_port, 61);
		data = READ_VPP_REG(data_port);
		b_map[OSD_OETF_LUT_SIZE - 1] = data & 0xffff;

		for (i = 0; i < OSD_OETF_LUT_SIZE; i++) {
			pr_err("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}

		addr_port = VIU_OSD1_EOTF_LUT_ADDR_PORT;
		data_port = VIU_OSD1_EOTF_LUT_DATA_PORT;
		ctrl_port = VIU_OSD1_EOTF_CTL;
		pr_err("----dump regs VPP_LUT_OSD_EOTF----\n");
		WRITE_VPP_REG(addr_port, 0);
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			r_map[i * 2] = data & 0xffff;
			r_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		r_map[EOTF_LUT_SIZE - 1] = data & 0xffff;
		g_map[0] = (data >> 16) & 0xffff;
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			g_map[i * 2 + 1] = data & 0xffff;
			g_map[i * 2 + 2] = (data >> 16) & 0xffff;
		}
		for (i = 0; i < 16; i++) {
			data = READ_VPP_REG(data_port);
			b_map[i * 2] = data & 0xffff;
			b_map[i * 2 + 1] = (data >> 16) & 0xffff;
		}
		data = READ_VPP_REG(data_port);
		b_map[EOTF_LUT_SIZE - 1] = data & 0xffff;

		for (i = 0; i < EOTF_LUT_SIZE; i++) {
			pr_err("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i, r_map[i], g_map[i], b_map[i]);
		}

		pr_err("----dump hdr_osd_reg structure ----\n");

		pr_err("\tviu_osd1_matrix_ctrl = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_ctrl);
		pr_err("\tviu_osd1_matrix_coef00_01 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef00_01);
		pr_err("\tviu_osd1_matrix_coef02_10 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef02_10);
		pr_err("\tviu_osd1_matrix_coef11_12 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef11_12);
		pr_err("\tviu_osd1_matrix_coef20_21 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef20_21);
		pr_err("\tviu_osd1_matrix_colmod_coef42 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_colmod_coef42);
		pr_err("\tviu_osd1_matrix_offset0_1 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_offset0_1);
		pr_err("\tviu_osd1_matrix_offset2 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_offset2);
		pr_err("\tviu_osd1_matrix_pre_offset0_1 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_pre_offset0_1);
		pr_err("\tviu_osd1_matrix_pre_offset2 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_pre_offset2);
		pr_err("\tviu_osd1_matrix_coef22_30 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef22_30);
		pr_err("\tviu_osd1_matrix_coef31_32 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef31_32);
		pr_err("\tviu_osd1_matrix_coef40_41 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_matrix_coef40_41);
		pr_err("\tviu_osd1_eotf_ctl = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_ctl);
		pr_err("\tviu_osd1_eotf_coef00_01 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_coef00_01);
		pr_err("\tviu_osd1_eotf_coef02_10 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_coef02_10);
		pr_err("\tviu_osd1_eotf_coef11_12 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_coef11_12);
		pr_err("\tviu_osd1_eotf_coef20_21 = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_coef20_21);
		pr_err("\tviu_osd1_eotf_coef22_rs = 0x%04x\n",
				hdr_osd_reg.viu_osd1_eotf_coef22_rs);
		pr_err("\tviu_osd1_oetf_ctl = 0x%04x\n",
				hdr_osd_reg.viu_osd1_oetf_ctl);

		for (i = 0; i < EOTF_LUT_SIZE; i++) {
			pr_err("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i,
				hdr_osd_reg.lut_val.r_map[i],
				hdr_osd_reg.lut_val.g_map[i],
				hdr_osd_reg.lut_val.b_map[i]);
		}
		for (i = 0; i < OSD_OETF_LUT_SIZE; i++) {
			pr_err("\t[%d] = 0x%04x 0x%04x 0x%04x\n",
				i,
				hdr_osd_reg.lut_val.or_map[i],
				hdr_osd_reg.lut_val.og_map[i],
				hdr_osd_reg.lut_val.ob_map[i]);
		}
		pr_err("\n");
	}
	pr_err("----dump regs VPP_LUT_EOTF----\n");
	print_vpp_lut(VPP_LUT_EOTF, READ_VPP_REG(VIU_EOTF_CTL) & (7 << 27));
	pr_err("----dump regs VPP_LUT_OETF----\n");
	print_vpp_lut(VPP_LUT_OETF, READ_VPP_REG(XVYCC_LUT_CTL) & 0x70 & 0xf);
	/*********************dump reg end*********************/
dbg_end:

	return 0;
}
