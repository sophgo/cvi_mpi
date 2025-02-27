#ifndef __CMPR_CANVAS__
#define __CMPR_CANVAS__

#include <float.h>
#include <string.h>
#include <math.h>
#include "osd_cmpr.h"
#include "osd_list.h"

#define BUF_GUARD_SIZE (1 << 12)
#define OSDEC_RL_BD (6)
#define OSDEC_PAL_BD (3)
#define OSDEC_MAX_RL (1 << OSDEC_RL_BD)

#define MIN_THICKNESS (1)
#define MAX_THICKNESS (32)

#define BG_COLOR_CODE (2)
#define CMPR_CANVAS_DBG (0)

typedef enum {
	RECT = 0,
	STROKE_RECT,
	BIT_MAP,
	LINE,
	CMPR_BIT_MAP,
	NUM_OF_DRAW_OBJ
} DRAW_OBJ_TYPE;

typedef struct {
	int width;
	int height;
	OSD_FORMAT format;
	CVI_U32 bg_color_code;
} Canvas_Attr;

typedef union _COLOR {
	CVI_U32 code;
	CVI_U8 *buf;
} COLOR;

typedef struct {
	int x0; // start position
	int x1; // end position
	CVI_U16 obj_id;
} OBJ_SLICE;

typedef struct {
	OBJ_SLICE slice;
	int num;
	dlist_t item;
} SLICE_LIST;

typedef struct {
	bool is_const;
	bool is_cmpr;
	CVI_U16 width;
	union {
		CVI_U32 stride;
		CVI_U16 *bs_len;
	};
	COLOR color;
	CVI_U16 id;
} SEGMENT;

typedef struct {
	SEGMENT segment;
	int num;
	dlist_t item;
} SEGMENT_LIST;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	int thickness;
} RECT_ATTR;

typedef struct {
	RECT_ATTR rect;
	union {
		CVI_U32 stride;
		CVI_U32 bs_offset;
	};
} BITMAP_ATTR;

typedef struct {
	float _mx; // slope of two end-point vector
	float _bx[2];
	float _by[2];
	float _ex[2];
	float _ey[2];
	float ts_h; // thickness proj. on horizontal slice
} LINE_ATTR;

typedef struct {
	DRAW_OBJ_TYPE type;
	union {
		RECT_ATTR rect;
		LINE_ATTR line;
		BITMAP_ATTR bitmap;
	};
	COLOR color;
	int _max_y;
	int _min_y;
} DRAW_OBJ;

typedef struct {
	OSDCmpr_Ctrl osdCmpr_ctrl;
	StreamBuffer bitstream;
	RGBA last_color;
	int rl_cnt;
	MODE_TYPE md;
	CODE code;
} Cmpr_Canvas_Ctrl;

CVI_U32 est_cmpr_canvas_size(Canvas_Attr *canvas, DRAW_OBJ *objs, CVI_U32 obj_num);

int draw_cmpr_canvas(Canvas_Attr *canvas, DRAW_OBJ *objs, CVI_U32 obj_num,
			CVI_U8 *obuf, int buf_size, CVI_U32 *p_osize);

void set_rect_obj_attr(DRAW_OBJ *obj, Canvas_Attr *canvas, CVI_U32 color_code,
			   int pt_x, int pt_y, int width, int height,
			   bool is_filled, int thickness);
void set_bitmap_obj_attr(DRAW_OBJ *obj_attr, Canvas_Attr *canvas, CVI_U8 *buf,
			 int pt_x, int pt_y, int width, int height,
			 bool is_cmpr);
void set_line_obj_attr(DRAW_OBJ *obj, Canvas_Attr *canvas, CVI_U32 color_code,
			   int pt_x0, int pt_y0, int pt_x1, int pt_y1,
			   int thickness);
int cmpr_bitmap(Canvas_Attr *canvas, CVI_U8 *ibuf, CVI_U8 *obuf, int width,
		int height, int buf_size, CVI_U32 *p_osize);

#if (CMPR_CANVAS_DBG)
int draw_canvas_raw_buffer(Canvas_Attr &canvas, vector<DRAW_OBJ> obj_vec,
			   CVI_U8 *obuf);
int draw_canvas_raw_buffer2(Canvas_Attr &canvas, vector<DRAW_OBJ> obj_vec,
			    CVI_U8 *obuf);
#endif

//==============================================================================================
//OSDC interface
#ifdef __cplusplus
extern "C"
{
#endif

CVI_U32 OSDC_est_cmpr_canvas_size(Canvas_Attr *canvas, DRAW_OBJ *objs, CVI_U32 obj_num);

int OSDC_draw_cmpr_canvas(Canvas_Attr *canvas, DRAW_OBJ *objs, CVI_U32 obj_num,
				CVI_U8 *obuf, CVI_U32 buf_size, CVI_U32 *p_osize);

void OSDC_set_rect_obj_attr(Canvas_Attr *canvas, DRAW_OBJ *obj, CVI_U32 color_code,
				int pt_x, int pt_y, int width, int height, bool is_filled, int thickness);

void OSDC_set_bitmap_obj_attr(Canvas_Attr *canvas, DRAW_OBJ *obj_attr, CVI_U8 *buf,
				  int pt_x, int pt_y, int width, int height, bool is_cmpr);

void OSDC_set_line_obj_attr(Canvas_Attr *canvas, DRAW_OBJ *obj, CVI_U32 color_code,
				int pt_x0, int pt_y0, int pt_x1, int pt_y1, int thickness);

int OSDC_cmpr_bitmap(Canvas_Attr *canvas, CVI_U8 *ibuf, CVI_U8 *obuf, int width, int height,
				int buf_size, CVI_U32 *p_osize);

#if (CMPR_CANVAS_DBG)
int OSDC_draw_canvas_raw_buffer(Canvas_Attr &canvas, vector<DRAW_OBJ> obj_vec, CVI_U8 *obuf);
int OSDC_draw_canvas_raw_buffer2(Canvas_Attr &canvas, vector<DRAW_OBJ> obj_vec, CVI_U8 *obuf);
#endif


#ifdef __cplusplus
}
#endif


#endif /* __CMPR_CANVAS__ */
