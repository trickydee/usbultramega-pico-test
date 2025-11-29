/*

MIT License

Copyright (c) 2021 David Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ssd1306.h"
#include "font.h"

inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, false)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        //printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}

inline static void ssd1306_write(ssd1306_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    fancy_write(p->i2c_i, p->address, d, 2, "ssd1306_write");
}

bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance) {
    p->width=width;
    p->height=height;
    p->pages=height/8;
    p->address=address;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->width);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }

    ++(p->buffer);

	// from https://github.com/makerportal/rpi-pico-ssd1306
    int8_t cmds[]= {
        SET_DISP | 0x00,  // off
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
        // resolution and layout
        SET_DISP_START_LINE | 0x00,
        SET_SEG_REMAP | 0x01,  // column addr 127 mapped to SEG0
        SET_MUX_RATIO,
        height - 1,
        SET_COM_OUT_DIR | 0x08,  // scan from COM[N] to COM0
        SET_DISP_OFFSET,
        0x00,
        SET_COM_PIN_CFG,
        width>2*height?0x02:0x12,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,  // 0.83*Vcc
        // display
        SET_CONTRAST,
        0xFF,  // maximum
        SET_ENTIRE_ON,  // output follows RAM contents
        SET_NORM_INV,  // not inverted
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_DISP | 0x01
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        ssd1306_write(p, cmds[i]);

    return true;
}

inline void ssd1306_deinit(ssd1306_t *p) {
    free(p->buffer-1);
}

inline void ssd1306_poweroff(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x00);
}

inline void ssd1306_poweron(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x01);
}

inline void ssd1306_contrast(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_CONTRAST);
    ssd1306_write(p, val);
}

inline void ssd1306_invert(ssd1306_t *p, uint8_t inv) {
    ssd1306_write(p, SET_NORM_INV | (inv & 1));
}

inline void ssd1306_clear(ssd1306_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
	if(x>=p->width || y>=p->height) return;

    p->buffer[x+p->width*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        uint32_t t=x1;
        x1=x2;
        x2=t;
        t=y1;
        y1=y2;
        y2=t;
    }

    float m=(float) (y2-y1) / (float) (x2-x1);

    for(int32_t i=x1; i<=x2; ++i) {
        float y=m*(float) (i-x1)+(float) y1;
        ssd1306_draw_pixel(p, (uint32_t) i, (uint32_t) y);
    }
}

void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height){
	for(uint32_t i=0;i<width;++i)
		for(uint32_t j=0;j<height;++j)
			ssd1306_draw_pixel(p, x+i, y+j);

}

void ssd13606_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height){
	ssd1306_draw_line(p, x, y, x+width, y);
	ssd1306_draw_line(p, x, y+height, x+width, y+height);
	ssd1306_draw_line(p, x, y, x, y+height);
	ssd1306_draw_line(p, x+width, y, x+width, y+height);
}

void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if(c > '~')
        return;

    for(uint8_t i=0; i<font[1]; ++i) {
        uint8_t line=(uint8_t)(font[(c-0x20)*font[1]+i+2]);

        for(int8_t j=0; j<font[0]; ++j, line>>=1) {
            if(line & 1 ==1)
                ssd1306_draw_square(p, x+i*scale, y+j*scale, scale, scale);
        }
    }
}

void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    for(int32_t x_n=x; *s; x_n+=font[0]*scale) {
        ssd1306_draw_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}

void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c) {
    ssd1306_draw_char_with_font(p, x, y, scale, font_8x5, c);
}

void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    ssd1306_draw_string_with_font(p, x, y, scale, font_8x5, s);
}

static inline uint32_t ssd1306_bmp_get_val(const uint8_t *data, size_t offset, size_t len) {
    uint32_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= ((uint32_t)data[offset + i]) << (i * 8);
    }
    return result;
}

void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset) {
    if(size<54) // data smaller than header
        return;

    const uint32_t bfOffBits=ssd1306_bmp_get_val(data, 10, 4);
    const uint32_t biSize=ssd1306_bmp_get_val(data, 14, 4);
    const uint32_t biWidth=ssd1306_bmp_get_val(data, 18, 4);
    const int32_t biHeight=(int32_t) ssd1306_bmp_get_val(data, 22, 4);
    const uint16_t biBitCount=(uint16_t) ssd1306_bmp_get_val(data, 28, 2);
    const uint32_t biCompression=ssd1306_bmp_get_val(data, 30, 4);

    if(biBitCount!=1) // image not monochrome
        return;

    if(biCompression!=0) // image compressed
        return;

    const int table_start=14+biSize;
    uint8_t white_color_val=1;  // Default to entry 1 for white

    // Find which color table entry is black (0,0,0), white is the other
    for(uint8_t i=0; i<2; ++i) {
        if(!((data[table_start+i*4]<<16)|(data[table_start+i*4+1]<<8)|data[table_start+i*4+2])) {
            // Found black at entry i, so white is the other entry
            white_color_val = 1 - i;
            break;
        }
    }

    uint32_t bytes_per_line=(biWidth/8)+(biWidth&7?1:0);
    if(bytes_per_line&3)
        bytes_per_line=(bytes_per_line^(bytes_per_line&3))+4;

    const uint8_t *img_data=data+bfOffBits;

    // BMP is stored bottom-to-top if height is positive
    // We need to flip the y-coordinate for display
    int32_t step=biHeight>0?-1:1;
    int32_t start_y=biHeight>0?biHeight-1:0;
    int32_t border=biHeight>0?-1:biHeight;

    uint32_t display_y = 0;
    for(int32_t bmp_y=start_y; bmp_y!=border; bmp_y+=step) {
        for(uint32_t x=0; x<biWidth; ++x) {
            // Get the bit value for this pixel
            uint8_t bit = (img_data[x>>3]>>(7-(x&7)))&1;
            // Draw pixel if bit matches white color value
            if(bit == white_color_val)
                ssd1306_draw_pixel(p, x_offset+x, y_offset+display_y);
        }
        img_data+=bytes_per_line;
        display_y++;
    }
}

inline void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size) {
    ssd1306_bmp_show_image_with_offset(p, data, size, 0, 0);
}

/**
 * Draw raw pixel data directly to display buffer
 * @param p Display instance
 * @param data Raw pixel data (each byte = 8 pixels, MSB first)
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param bytes_per_row Bytes per row (can be calculated as (width+7)/8, but may be padded)
 * @param x_offset X offset on display
 * @param y_offset Y offset on display
 */
void ssd1306_draw_raw_image(ssd1306_t *p, const uint8_t *data, uint32_t width, uint32_t height, uint32_t bytes_per_row, uint32_t x_offset, uint32_t y_offset) {
    const uint8_t *row_data = data;
    
    for(uint32_t y = 0; y < height; y++) {
        for(uint32_t x = 0; x < width; x++) {
            uint32_t byte_idx = x / 8;
            uint32_t bit_idx = 7 - (x % 8);  // MSB first
            uint8_t bit = (row_data[byte_idx] >> bit_idx) & 1;
            
            // Draw pixel if bit is set (white pixel)
            if(bit) {
                ssd1306_draw_pixel(p, x_offset + x, y_offset + y);
            }
        }
        row_data += bytes_per_row;
    }
}

/**
 * Draw image data in SSD1306 page format (like Raspberry Pi example)
 * Image data is organized as: for each page (8 rows), then for each column
 * This format matches the SSD1306 display buffer layout exactly
 * Format: data[page * img_width + col] where page=0..img_pages-1, col=0..img_width-1
 * @param p Display instance
 * @param data Image data in page format
 * @param img_width Image width in pixels
 * @param img_height Image height in pixels
 * @param x_offset X offset on display
 * @param y_offset Y offset on display (must be page-aligned, i.e., multiple of 8)
 */
void ssd1306_draw_page_image(ssd1306_t *p, const uint8_t *data, uint32_t img_width, uint32_t img_height, uint32_t x_offset, uint32_t y_offset) {
    uint32_t img_pages = (img_height + 7) / 8;  // Round up to pages
    uint32_t start_page = y_offset / 8;
    
    // For each page (8 rows) in the image
    for(uint32_t page = 0; page < img_pages; page++) {
        uint32_t dst_page = start_page + page;
        
        if(dst_page >= p->pages) continue;  // Skip if outside display height
        
        // For each column in the image
        for(uint32_t col = 0; col < img_width; col++) {
            uint32_t dst_x = x_offset + col;
            
            if(dst_x >= p->width) continue;  // Skip if outside display width
            
            // Calculate source and destination indices
            // Source: image data is organized as [page0_col0, page0_col1, ..., page0_colN, page1_col0, ...]
            uint32_t src_idx = page * img_width + col;
            
            // Destination: buffer format is buffer[x + width * page]
            uint32_t dst_idx = dst_x + p->width * dst_page;
            
            // Copy the byte directly
            p->buffer[dst_idx] = data[src_idx];
        }
    }
}

void ssd1306_show(ssd1306_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->width-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);

    *(p->buffer-1)=0x40;

    fancy_write(p->i2c_i, p->address, p->buffer-1, p->bufsize+1, "ssd1306_show");
}
