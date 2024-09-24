/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: sample_dsi_panel.h
 * Description:
 */

#ifndef __SAMPLE_DSI_PANEL_H__
#define __SAMPLE_DSI_PANEL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum {
	PANEL_MODE_DSI,
	PANEL_MODE_LVDS,
	PANEL_MODE_BT,
	PANEL_MODE_MCU,
} PANEL_TYPE;

typedef struct dsi_panel_desc_s {
	struct combo_dev_cfg_s *dev_cfg;
	const struct hs_settle_s *hs_timing_cfg;
	const struct dsc_instr *dsi_init_cmds;
	int dsi_init_cmds_size;
} dsi_panel_desc;

struct vo_panel_desc_s {
	char *panel_mode;
	PANEL_TYPE panel_type;
	union {
		dsi_panel_desc stdsicfg;
		VO_PUB_ATTR_S stVoPubAttr;
		HW_I80_CFG_S stHwI80Attr;
	};
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
