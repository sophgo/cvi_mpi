#ifndef __OSD_CMPR_H__
#define __OSD_CMPR_H__
#include <stdbool.h>
#include <stdint.h>
#include "cvi_type.h"

// DataType-free color field copy
#define CPY_C(in, out)                                                         \
	{                                                                      \
		out.r = in.r;                                                  \
		out.g = in.g;                                                  \
		out.b = in.b;                                                  \
		out.a = in.a;                                                  \
	}

#define HDR_SZ (8)

typedef enum {
	OSD_ARGB8888 = 0,
	OSD_ARGB4444 = 4,
	OSD_ARGB1555 = 5,
	OSD_LUT8 = 8,
	OSD_LUT4 = 10,
	NUM_OF_FORMAT
} OSD_FORMAT;

typedef struct {
	int img_width;
	int img_height;
	bool palette_mode_en;
	bool zeroize_by_alpha;
	int rgb_trunc_bit;
	int alpha_trunc_bit;
	int run_len_bd;
	int palette_idx_bd;
	OSD_FORMAT osd_format;
	bool hdr_en;
} OSDCmpr_Cfg;

typedef struct {
	CVI_U8 *stream; // stream buffer pointer
	int bit_pos; // current pointer (in bit)
	int buf_size; // in byte
	int status;
} StreamBuffer;

inline int clip(int data, int min, int max)
{
	return (data > max) ? max : (data < min) ? min : data;
}

void init_stream(StreamBuffer *bs, const CVI_U8 *buf, int buf_size,
		 bool read_only);
void write_stream(StreamBuffer *bs, CVI_U8 *src, int bit_len);
void parse_stream(StreamBuffer *bs, CVI_U8 *dest, int bit_len, bool read_only);
void move_stream_ptr(StreamBuffer *bs, int bit_len);

CVI_U8 get_bit_val(CVI_U8 *buf, int byte_idx, int bit_idx);

typedef union {
	struct {
		CVI_U8 g;
		CVI_U8 b;
		CVI_U8 r;
		CVI_U8 a;
	};
	CVI_U32 code;
} RGBA;

typedef union {
	struct {
		CVI_U16 g : 4;
		CVI_U16 b : 4;
		CVI_U16 r : 4;
		CVI_U16 a : 4;
	};
	CVI_U16 code;
} ARGB4444;

typedef union {
	struct {
		CVI_U16 g : 5;
		CVI_U16 b : 5;
		CVI_U16 r : 5;
		CVI_U16 a : 1;
	};
	CVI_U16 code;
} ARGB1555;

typedef union {
	RGBA color;
	int palette_idx;
} CODE;

typedef enum {
	Literal = 0,
	Palette,
	Literal_RL,
	Palette_RL,
	NUM_OF_MODE
} MODE_TYPE;

typedef struct {
	RGBA *color;
	int num;
} PaletteRGBA;

typedef struct {
	int reg_image_width;
	int reg_image_height;
	bool reg_zeroize_by_alpha;
	int reg_rgb_trunc_bit;
	int reg_alpha_trunc_bit;
	bool reg_palette_mode_en;
	int reg_run_len_bd;
	int reg_palette_idx_bd;
	OSD_FORMAT reg_osd_format;
	int pel_sz;
	PaletteRGBA palette_cache;
	int bs_buf_size;
	CVI_U8 *bsbuf; // intermediate bitstream buffer
} OSDCmpr_Ctrl;

// RGBA get_color(CVI_U8 *ptr, OSD_FORMAT format = OSD_ARGB8888);
// void set_color(CVI_U8 *ptr, RGBA color, OSD_FORMAT format = OSD_ARGB8888);
RGBA get_color(CVI_U8 *ptr, OSD_FORMAT format);
void set_color(CVI_U8 *ptr, RGBA color, OSD_FORMAT format);

void osd_cmpr_frame_init(OSDCmpr_Ctrl *p_ctrl);
int osd_cmpr_enc_one_frame(CVI_U8 *ibuf, CVI_U8 *obs, OSDCmpr_Ctrl *p_ctrl);
void osd_cmpr_dec_one_frame(CVI_U8 *bsbuf, size_t bs_size, CVI_U8 *obuf,
			    OSDCmpr_Ctrl *p_ctrl);
void osd_cmpr_enc_header(CVI_U8 *hdrbuf, OSDCmpr_Ctrl *p_ctrl);
void osd_cmpr_dec_header(CVI_U8 *hdrbuf, OSDCmpr_Ctrl *p_ctrl);

void osd_cmpr_setup(OSDCmpr_Ctrl *p_ctrl, OSDCmpr_Cfg *p_cfg);
void osd_cmpr_enc_const_pixel(RGBA cur_c, RGBA *last_c, int *rl_cnt,
			      MODE_TYPE *md, CODE *code, CVI_U16 *length,
			      bool is_force_new_run, CVI_U16 max_run_len,
			      OSDCmpr_Ctrl *p_ctrl, StreamBuffer *bitstream);
void osd_cmpr_enc_followed_run(RGBA cur_c, int *rl_cnt, MODE_TYPE *md,
			       CODE *code, CVI_U16 *length, CVI_U16 max_run_len,
			       OSDCmpr_Ctrl *p_ctrl, StreamBuffer *bitstream);

size_t osd_cmpr_get_pixel_sz(OSD_FORMAT format);
size_t osd_cmpr_get_bs_buf_max_sz(int pel_num, int pel_sz);
size_t osd_cmpr_get_header_sz(void);

void osd_cmpr_debug_frame_compare(OSDCmpr_Ctrl *p_ctrl, CVI_U8 *buf0,
				  CVI_U8 *buf1);

void palette_cache_init(PaletteRGBA *cache, int cache_sz);
int palette_cache_lookup_color(PaletteRGBA *cache, RGBA color);
void palette_cache_lru_update(PaletteRGBA *cache, int index);
void palette_cache_push_color(PaletteRGBA *cache, RGBA color);
#endif /* __OSD_CMPR_H__ */
