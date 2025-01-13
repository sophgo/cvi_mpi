/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: awb_project_param.h
 * Description:
 *
 */

#ifndef _AWB_PROJECT_PARAM_H_
#define _AWB_PROJECT_PARAM_H_



#define AWB_BOUND_LOW_R 800
#define AWB_BOUND_HIGH_R 4095
#define AWB_BOUND_LOW_B 1400
#define AWB_BOUND_HIGH_B 3200

#define P1_RGAIN_RATIO		(1.378906)
#define P1_BGAIN_RATIO		(3.036133)
#define P1_COLOR_TEMPERATURE	(2850)
#define P2_RGAIN_RATIO		(1.496094)
#define P2_BGAIN_RATIO		(2.450195)
#define P2_COLOR_TEMPERATURE	(3900)
#define P3_RGAIN_RATIO		(2.296875)
#define P3_BGAIN_RATIO		(1.571289)
#define P3_COLOR_TEMPERATURE	(6500)


#define CT_TO_RGAIN_CURVE_K1	(0.000055)
#define CT_TO_RGAIN_CURVE_K2	(-0.254903)
#define CT_TO_RGAIN_CURVE_K3	(1692.074097)

#define CT_SHIFT_LIMIT_REGION1	(3900)//ColorTemp
#define CT_SHIFT_LIMIT_REGION2	(5300)
#define CT_SHIFT_LIMIT_REGION3	(6600)



#define DEFAULT_RGAIN_RATIO	(1.789062)
#define DEFAULT_BGAIN_RATIO	(1.639648)


#define MIN_COLOR_TEMPERATURE			1500
#define MAX_COLOR_TEMPERATURE			10000

#define AWB_BOUNDARY_START_CT			2500
#define AWB_BOUNDARY_END_CT				8000


#endif // _AWB_PROJECT_PARAM_H_