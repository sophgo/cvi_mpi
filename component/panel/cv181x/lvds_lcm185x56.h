#ifndef _LVDS_PARAM_LCM185X56_H_
#define _LVDS_PARAM_LCM185X56_H_

#include <cvi_comm_vo.h>

VO_LVDS_ATTR_S lvds_lcm185x56_cfg = {
	.lvds_vesa_mode = VO_LVDS_MODE_VESA,
	.out_bits = VO_LVDS_OUT_8BIT,
	.chn_num = 1,
	.data_big_endian = 0,
	.lane_id = {VO_LVDS_LANE_0, VO_LVDS_LANE_1, VO_LVDS_LANE_CLK, VO_LVDS_LANE_2, VO_LVDS_LANE_3},
	.lane_pn_swap = {true, true, true, true, true},
	.sync_info = {
		.vid_hsa_pixels = 10,
		.vid_hbp_pixels = 88,
		.vid_hfp_pixels = 62,
		.vid_hline_pixels = 1280,
		.vid_vsa_lines = 4,
		.vid_vbp_lines = 23,
		.vid_vfp_lines = 11,
		.vid_active_lines = 800,
		.vid_vsa_pos_polarity = 0,
		.vid_hsa_pos_polarity = 0,
	},
	.u16FrameRate = 60,
};

#endif // _LVDS_PARAM_LCM185X56_H_
