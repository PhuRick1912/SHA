#include "lv_font_shim.h"
#include "spi1_lcd_st7789.h"

static uint32_t UTF8_DecodeNext(const char **str) {
    const uint8_t *s = (const uint8_t *)(*str);
    if (*s == 0) {
        return 0;
    }
    uint32_t unicode = 0;
    if ((s[0] & 0x80U) == 0U) {
        unicode = s[0];
        *str += 1;
    } else if ((s[0] & 0xE0U) == 0xC0U) {
        unicode = ((uint32_t)(s[0] & 0x1FU) << 6) | (s[1] & 0x3FU);
        *str += 2;
    } else if ((s[0] & 0xF0U) == 0xE0U) {
        unicode = ((uint32_t)(s[0] & 0x0FU) << 12) | ((uint32_t)(s[1] & 0x3FU) << 6) | (s[2] & 0x3FU);
        *str += 3;
    } else {
        *str += 1;
    }
    return unicode;
}

static uint16_t Font_LookupGlyphId(const lv_font_fmt_txt_dsc_t *fdsc, uint32_t unicode) {
    for (uint16_t i = 0; i < fdsc->cmap_num; i++) {
        const lv_font_fmt_txt_cmap_t *cmap = &fdsc->cmaps[i];
        if (unicode < cmap->range_start || unicode >= (cmap->range_start + cmap->range_length)) {
            continue;
        }
        uint32_t r_ofs = unicode - cmap->range_start;
        if (cmap->type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            return (uint16_t)(cmap->glyph_id_start + r_ofs);
        } else if (cmap->type == LV_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
            int32_t left = 0;
            int32_t right = (int32_t)cmap->list_length - 1;
            while (left <= right) {
                int32_t mid = left + ((right - left) >> 1);
                if (cmap->unicode_list[mid] == r_ofs) {
                    return (uint16_t)(cmap->glyph_id_start + mid);
                } else if (cmap->unicode_list[mid] < r_ofs) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
        }
    }
    return 0;
}

static uint16_t Blend_RGB565(uint16_t fg, uint16_t bg, uint8_t alpha4) {
    if (alpha4 == 0U) {
        return bg;
    }
    if (alpha4 >= 15U) {
        return fg;
    }
    uint32_t r = (((fg >> 11) & 0x1FU) * alpha4 + ((bg >> 11) & 0x1FU) * (15U - alpha4)) / 15U;
    uint32_t g = (((fg >> 5) & 0x3FU) * alpha4 + ((bg >> 5) & 0x3FU) * (15U - alpha4)) / 15U;
    uint32_t b = ((fg & 0x1FU) * alpha4 + (bg & 0x1FU) * (15U - alpha4)) / 15U;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void SHA_Font_DrawString(uint16_t x, uint16_t y, const char *utf8_str, const lv_font_t *font, uint16_t fg_rgb565, uint16_t bg_rgb565) {
    const lv_font_fmt_txt_dsc_t *fdsc = font->dsc;
    uint16_t cur_x = x;
    uint32_t unicode;
    static uint8_t s_char_buf[32 * 32 * 2];
    while ((unicode = UTF8_DecodeNext(&utf8_str)) != 0U) {
        uint16_t gid = Font_LookupGlyphId(fdsc, unicode);
        if (gid == 0U && unicode != ' ') {
            continue;
        }
        const lv_font_fmt_txt_glyph_dsc_t *gdsc = &fdsc->glyph_dsc[gid];
        uint16_t adv_px = (uint16_t)((gdsc->adv_w + 8U) >> 4);
        if (gdsc->box_w > 0U && gdsc->box_h > 0U) {
            int16_t draw_x = (int16_t)cur_x + gdsc->ofs_x;
            int16_t draw_y = (int16_t)y + (font->line_height - font->base_line - gdsc->box_h - gdsc->ofs_y);
            const uint8_t *bmp = &fdsc->glyph_bitmap[gdsc->bitmap_index];
            uint32_t total_px = (uint32_t)gdsc->box_w * gdsc->box_h;
            uint32_t bit_pos = 0;
            for (uint32_t p = 0; p < total_px; p++) {
                uint8_t byte_val = bmp[bit_pos >> 3];
                uint8_t alpha4 = 0;
                if (fdsc->bpp == 1U) {
                    uint8_t bit = (byte_val >> (7U - (bit_pos & 7U))) & 0x01U;
                    alpha4 = bit ? 15U : 0U;
                    bit_pos += 1U;
                } else if (fdsc->bpp == 2U) {
                    uint8_t shift = 6U - (uint8_t)(bit_pos & 6U);
                    alpha4 = ((byte_val >> shift) & 0x03U) * 5U;
                    bit_pos += 2U;
                } else if (fdsc->bpp == 4U) {
                    alpha4 = ((bit_pos & 4U) == 0U) ? (byte_val >> 4) : (byte_val & 0x0FU);
                    bit_pos += 4U;
                }
                uint16_t color = Blend_RGB565(fg_rgb565, bg_rgb565, alpha4);
                s_char_buf[p * 2U] = (uint8_t)(color >> 8);
                s_char_buf[p * 2U + 1U] = (uint8_t)(color & 0xFFU);
            }
            ST7789_SetWindow((uint16_t)draw_x, (uint16_t)draw_y, (uint16_t)(draw_x + gdsc->box_w - 1), (uint16_t)(draw_y + gdsc->box_h - 1));
            ST7789_WriteDataBuffer(s_char_buf, total_px * 2UL);
        }
        cur_x += adv_px;
    }
}
