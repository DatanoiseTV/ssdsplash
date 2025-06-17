#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "ssdsplash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void ssd1306_draw_pixel(int x, int y, bool on);
extern display_config_t current_config;

typedef struct {
    char path[SSDSPLASH_MAX_PATH_LEN];
    int size;
    stbtt_fontinfo font;
    unsigned char *font_data;
    float scale;
    int ascent, descent, line_gap;
} font_cache_t;

static font_cache_t cached_font = {0};

static void free_cached_font(void) {
    if (cached_font.font_data) {
        free(cached_font.font_data);
        cached_font.font_data = NULL;
        memset(&cached_font, 0, sizeof(cached_font));
    }
}

static int load_truetype_font(const char *font_path, int font_size) {
    if (cached_font.font_data && 
        strcmp(cached_font.path, font_path) == 0 && 
        cached_font.size == font_size) {
        return 0;
    }
    
    free_cached_font();
    
    FILE *font_file = fopen(font_path, "rb");
    if (!font_file) {
        printf("Failed to open font file: %s\n", font_path);
        return -1;
    }
    
    fseek(font_file, 0, SEEK_END);
    long font_data_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    
    cached_font.font_data = malloc(font_data_size);
    if (!cached_font.font_data) {
        fclose(font_file);
        return -1;
    }
    
    if (fread(cached_font.font_data, 1, font_data_size, font_file) != (size_t)font_data_size) {
        fclose(font_file);
        free(cached_font.font_data);
        cached_font.font_data = NULL;
        return -1;
    }
    fclose(font_file);
    
    if (!stbtt_InitFont(&cached_font.font, cached_font.font_data, 0)) {
        printf("Failed to initialize font: %s\n", font_path);
        free(cached_font.font_data);
        cached_font.font_data = NULL;
        return -1;
    }
    
    strncpy(cached_font.path, font_path, sizeof(cached_font.path) - 1);
    cached_font.size = font_size;
    cached_font.scale = stbtt_ScaleForPixelHeight(&cached_font.font, font_size);
    
    stbtt_GetFontVMetrics(&cached_font.font, &cached_font.ascent, &cached_font.descent, &cached_font.line_gap);
    cached_font.ascent = (int)(cached_font.ascent * cached_font.scale);
    cached_font.descent = (int)(cached_font.descent * cached_font.scale);
    cached_font.line_gap = (int)(cached_font.line_gap * cached_font.scale);
    
    printf("Loaded font: %s, size: %d, scale: %.3f\n", font_path, font_size, cached_font.scale);
    
    return 0;
}

void ssd1306_draw_text_truetype(const char *text, int x, int y, const char *font_path, int font_size) {
    if (!text || !font_path || strlen(font_path) == 0) {
        ssd1306_draw_text(text, x, y);
        return;
    }
    
    if (load_truetype_font(font_path, font_size) < 0) {
        printf("Fallback to bitmap font\n");
        ssd1306_draw_text(text, x, y);
        return;
    }
    
    int baseline_y = y + cached_font.ascent;
    int advance_x = x;
    
    const char *ch = text;
    while (*ch) {
        if (*ch == '\n') {
            advance_x = x;
            baseline_y += font_size + 2;
            ch++;
            continue;
        }
        
        if (baseline_y >= current_config.height) break;
        
        int glyph_index = stbtt_FindGlyphIndex(&cached_font.font, *ch);
        if (glyph_index == 0) {
            ch++;
            continue;
        }
        
        int width, height, xoff, yoff;
        unsigned char *bitmap = stbtt_GetGlyphBitmap(&cached_font.font, cached_font.scale, cached_font.scale,
                                                     glyph_index, &width, &height, &xoff, &yoff);
        
        if (bitmap) {
            int glyph_x = advance_x + xoff;
            int glyph_y = baseline_y + yoff;
            
            for (int py = 0; py < height; py++) {
                for (int px = 0; px < width; px++) {
                    int screen_x = glyph_x + px;
                    int screen_y = glyph_y + py;
                    
                    if (screen_x >= 0 && screen_x < current_config.width &&
                        screen_y >= 0 && screen_y < current_config.height) {
                        
                        unsigned char alpha = bitmap[py * width + px];
                        if (alpha > 127) {
                            ssd1306_draw_pixel(screen_x, screen_y, true);
                        }
                    }
                }
            }
            
            stbtt_FreeBitmap(bitmap, NULL);
        }
        
        int advance_width, left_side_bearing;
        stbtt_GetGlyphHMetrics(&cached_font.font, glyph_index, &advance_width, &left_side_bearing);
        advance_x += (int)(advance_width * cached_font.scale);
        
        if (advance_x >= current_config.width) break;
        
        ch++;
    }
}

void ssd1306_cleanup_truetype(void) {
    free_cached_font();
}