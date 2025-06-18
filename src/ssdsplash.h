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
    DISPLAY_128x32 = 1,
    DISPLAY_ILI9341_240x320 = 2,
    DISPLAY_SSH1106_128x64 = 3
} display_type_t;

typedef struct {
    int width;
    int height;
    int pages;
} display_config_t;

extern const display_config_t display_configs[];

int display_init(display_type_t type, const char *device, uint8_t addr);
void display_cleanup(void);
void display_clear(void);
void display_update(void);
void display_draw_text(const char *text, int x, int y);
void display_draw_progress_bar(int value, int max_value, int x, int y, int width, int height);

int display_load_and_display_image(const char *filename);
int display_load_and_display_image_scaled(const char *filename);

void display_draw_text_truetype(const char *text, int x, int y, const char *font_path, int font_size);
void display_cleanup_truetype(void);

#endif