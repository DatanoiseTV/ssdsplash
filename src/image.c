#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ssdsplash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void ssd1306_draw_pixel(int x, int y, bool on);
extern display_config_t current_config;

static uint8_t rgb_to_gray(uint8_t r, uint8_t g, uint8_t b) {
    return (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
}

static uint8_t dither_threshold(int x, int y, uint8_t gray) {
    static const uint8_t bayer_4x4[4][4] = {
        {  0, 128,  32, 160 },
        { 192,  64, 224,  96 },
        {  48, 176,  16, 144 },
        { 240, 112, 208,  80 }
    };
    
    uint8_t threshold = bayer_4x4[y & 3][x & 3];
    return gray > threshold ? 255 : 0;
}

int ssd1306_load_and_display_image(const char *filename) {
    int width, height, channels;
    unsigned char *img_data = stbi_load(filename, &width, &height, &channels, 0);
    
    if (!img_data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return -1;
    }
    
    printf("Loaded image: %dx%d, channels: %d\n", width, height, channels);
    
    ssd1306_clear();
    
    int start_x = (current_config.width - width) / 2;
    int start_y = (current_config.height - height) / 2;
    
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    
    for (int y = 0; y < height && (start_y + y) < current_config.height; y++) {
        for (int x = 0; x < width && (start_x + x) < current_config.width; x++) {
            int pixel_idx = (y * width + x) * channels;
            uint8_t r, g, b;
            
            if (channels == 1) {
                r = g = b = img_data[pixel_idx];
            } else if (channels == 3) {
                r = img_data[pixel_idx];
                g = img_data[pixel_idx + 1];
                b = img_data[pixel_idx + 2];
            } else if (channels == 4) {
                r = img_data[pixel_idx];
                g = img_data[pixel_idx + 1];
                b = img_data[pixel_idx + 2];
            } else {
                r = g = b = 128;
            }
            
            uint8_t gray = rgb_to_gray(r, g, b);
            uint8_t dithered = dither_threshold(start_x + x, start_y + y, gray);
            
            ssd1306_draw_pixel(start_x + x, start_y + y, dithered > 127);
        }
    }
    
    stbi_image_free(img_data);
    ssd1306_display();
    
    return 0;
}

int ssd1306_load_and_display_image_scaled(const char *filename) {
    int width, height, channels;
    unsigned char *img_data = stbi_load(filename, &width, &height, &channels, 0);
    
    if (!img_data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return -1;
    }
    
    printf("Loaded image: %dx%d, channels: %d\n", width, height, channels);
    
    ssd1306_clear();
    
    float scale_x = (float)current_config.width / width;
    float scale_y = (float)current_config.height / height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    
    if (scale > 1.0f) scale = 1.0f;
    
    int scaled_width = (int)(width * scale);
    int scaled_height = (int)(height * scale);
    
    int start_x = (current_config.width - scaled_width) / 2;
    int start_y = (current_config.height - scaled_height) / 2;
    
    for (int y = 0; y < scaled_height; y++) {
        for (int x = 0; x < scaled_width; x++) {
            int src_x = (int)(x / scale);
            int src_y = (int)(y / scale);
            
            if (src_x >= width) src_x = width - 1;
            if (src_y >= height) src_y = height - 1;
            
            int pixel_idx = (src_y * width + src_x) * channels;
            uint8_t r, g, b;
            
            if (channels == 1) {
                r = g = b = img_data[pixel_idx];
            } else if (channels == 3) {
                r = img_data[pixel_idx];
                g = img_data[pixel_idx + 1];
                b = img_data[pixel_idx + 2];
            } else if (channels == 4) {
                r = img_data[pixel_idx];
                g = img_data[pixel_idx + 1];
                b = img_data[pixel_idx + 2];
            } else {
                r = g = b = 128;
            }
            
            uint8_t gray = rgb_to_gray(r, g, b);
            uint8_t dithered = dither_threshold(start_x + x, start_y + y, gray);
            
            ssd1306_draw_pixel(start_x + x, start_y + y, dithered > 127);
        }
    }
    
    stbi_image_free(img_data);
    ssd1306_display();
    
    return 0;
}