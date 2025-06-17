#ifndef SSDSPLASH_H
#define SSDSPLASH_H

#include <stdint.h>
#include <stdbool.h>

#define SSDSPLASH_SOCKET_PATH "/tmp/ssdsplash.sock"
#define SSDSPLASH_MAX_TEXT_LEN 128
#define SSDSPLASH_MAX_PATH_LEN 256

typedef enum {
    MSG_TYPE_TEXT = 1,
    MSG_TYPE_PROGRESS = 2,
    MSG_TYPE_CLEAR = 3,
    MSG_TYPE_QUIT = 4,
    MSG_TYPE_IMAGE = 5
} message_type_t;

typedef struct {
    message_type_t type;
    union {
        struct {
            char text[SSDSPLASH_MAX_TEXT_LEN];
            int line;
            char font_path[SSDSPLASH_MAX_PATH_LEN];
            int font_size;
        } text_msg;
        struct {
            int value;
            int max_value;
        } progress_msg;
        struct {
            char path[SSDSPLASH_MAX_PATH_LEN];
            bool scaled;
        } image_msg;
    } data;
} ssdsplash_message_t;

typedef enum {
    DISPLAY_128x64 = 0,
    DISPLAY_128x32 = 1
} display_type_t;

typedef struct {
    int width;
    int height;
    int pages;
} display_config_t;

extern const display_config_t display_configs[];

int ssd1306_init(display_type_t type, const char *i2c_device, uint8_t addr);
void ssd1306_cleanup(void);
void ssd1306_clear(void);
void ssd1306_display(void);
void ssd1306_draw_text(const char *text, int x, int y);
void ssd1306_draw_progress_bar(int value, int max_value, int x, int y, int width, int height);

int ssd1306_load_and_display_image(const char *filename);
int ssd1306_load_and_display_image_scaled(const char *filename);

void ssd1306_draw_text_truetype(const char *text, int x, int y, const char *font_path, int font_size);
void ssd1306_cleanup_truetype(void);

#endif