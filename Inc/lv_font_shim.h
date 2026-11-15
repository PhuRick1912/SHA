#ifndef LV_FONT_SHIM_H_
#define LV_FONT_SHIM_H_

#include <stdint.h>
#include <stdbool.h>

#define LV_ATTRIBUTE_LARGE_CONST
#define LV_VERSION_CHECK(x, y, z) 1
#define lv_font_get_bitmap_fmt_txt ((void*)0)
#define lv_font_get_glyph_dsc_fmt_txt ((void*)0)
#define LV_FONT_SUBPX_NONE 0

typedef struct {
    uint32_t bitmap_index : 20;
    uint32_t adv_w : 12;
    uint8_t box_w;
    uint8_t box_h;
    int8_t ofs_x;
    int8_t ofs_y;
} lv_font_fmt_txt_glyph_dsc_t;

enum {
    LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY = 0,
    LV_FONT_FMT_TXT_CMAP_SPARSE_TINY = 2
};

typedef struct {
    uint32_t range_start;
    uint16_t range_length;
    uint16_t glyph_id_start;
    const uint16_t *unicode_list;
    const void *glyph_id_ofs_list;
    uint16_t list_length;
    uint8_t type;
} lv_font_fmt_txt_cmap_t;

typedef struct {
    const uint8_t *glyph_bitmap;
    const lv_font_fmt_txt_glyph_dsc_t *glyph_dsc;
    const lv_font_fmt_txt_cmap_t *cmaps;
    const void *kern_dsc;
    uint16_t kern_scale;
    uint16_t cmap_num : 9;
    uint16_t bpp : 4;
    uint16_t kern_classes : 1;
    uint16_t bitmap_format : 2;
} lv_font_fmt_txt_dsc_t;

typedef struct _lv_font_t {
    const void *get_glyph_dsc;
    const void *get_glyph_bitmap;
    int16_t line_height;
    int16_t base_line;
    uint8_t subpx;
    int8_t underline_position;
    int8_t underline_thickness;
    const lv_font_fmt_txt_dsc_t *dsc;
    const void *fallback;
    void *user_data;
} lv_font_t;

void SHA_Font_DrawString(uint16_t x, uint16_t y, const char *utf8_str, const lv_font_t *font, uint16_t fg_rgb565, uint16_t bg_rgb565);

#endif /* LV_FONT_SHIM_H_ */
