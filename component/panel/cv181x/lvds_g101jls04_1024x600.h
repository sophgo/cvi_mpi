#ifndef _LVDS_PARAM_G101JLS04_1024X600_H_
#define _LVDS_PARAM_G101JLS04_1024X600_H_

#include <cvi_comm_vo.h>

VO_LVDS_ATTR_S lvds_g101jls04_1024x600_cfg = {
	.lvds_vesa_mode = VO_LVDS_MODE_VESA,
	.out_bits = VO_LVDS_OUT_8BIT,
	.chn_num = 1,
	.data_big_endian = 0,
	.lane_id = {VO_LVDS_LANE_3, VO_LVDS_LANE_2, VO_LVDS_LANE_CLK, VO_LVDS_LANE_1, VO_LVDS_LANE_0},
	.lane_pn_swap = {false, false, false, false, false},
	.sync_info = {
		.vid_hsa_pixels = 0,
		.vid_hbp_pixels = 160,
		.vid_hfp_pixels = 160,
		.vid_hline_pixels = 1024,
		.vid_vsa_lines = 0,
		.vid_vbp_lines = 23,
		.vid_vfp_lines = 12,
		.vid_active_lines = 600,
		.vid_vsa_pos_polarity = 0,
		.vid_hsa_pos_polarity = 0,
	},
	.u16FrameRate = 60,
};

#endif // _LVDS_PARAM_G101JLS04_1024X600_H_
