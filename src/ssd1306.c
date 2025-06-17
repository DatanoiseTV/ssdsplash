#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include "ssdsplash.h"

#define SSD1306_I2C_ADDRESS_DEFAULT 0x3C
#define SSD1306_I2C_ADDRESS_ALT 0x3D

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D

const display_config_t display_configs[] = {
    [DISPLAY_128x64] = {128, 64, 8},
    [DISPLAY_128x32] = {128, 32, 4}
};

static int i2c_fd = -1;
display_config_t current_config;
static uint8_t *framebuffer = NULL;
static uint8_t i2c_address = SSD1306_I2C_ADDRESS_DEFAULT;

static int ssd1306_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    return write(i2c_fd, buf, 2) == 2 ? 0 : -1;
}

static int ssd1306_data(uint8_t *data, size_t len) {
    uint8_t *buf = malloc(len + 1);
    if (!buf) return -1;
    
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    int ret = write(i2c_fd, buf, len + 1) == (ssize_t)(len + 1) ? 0 : -1;
    free(buf);
    return ret;
}

int ssd1306_init(display_type_t type, const char *i2c_device, uint8_t addr) {
    if (type >= sizeof(display_configs) / sizeof(display_configs[0])) {
        return -1;
    }
    
    current_config = display_configs[type];
    
    if (addr == 0) {
        i2c_address = SSD1306_I2C_ADDRESS_DEFAULT;
    } else {
        i2c_address = addr;
    }
    
    i2c_fd = open(i2c_device ? i2c_device : "/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        perror("Failed to open I2C device");
        return -1;
    }
    
    if (ioctl(i2c_fd, I2C_SLAVE, i2c_address) < 0) {
        perror("Failed to set I2C slave address");
        close(i2c_fd);
        i2c_fd = -1;
        return -1;
    }
    
    framebuffer = calloc(current_config.width * current_config.pages, 1);
    if (!framebuffer) {
        close(i2c_fd);
        i2c_fd = -1;
        return -1;
    }
    
    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);
    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(current_config.height - 1);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SETSTARTLINE | 0x00);
    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SEGREMAP | 0x01);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(current_config.height == 64 ? 0x12 : 0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(current_config.height == 64 ? 0xCF : 0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);
    ssd1306_command(SSD1306_NORMALDISPLAY);
    ssd1306_command(SSD1306_DISPLAYON);
    
    ssd1306_clear();
    ssd1306_display();
    
    return 0;
}

void ssd1306_cleanup(void) {
    if (i2c_fd >= 0) {
        ssd1306_command(SSD1306_DISPLAYOFF);
        close(i2c_fd);
        i2c_fd = -1;
    }
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }
}

void ssd1306_clear(void) {
    if (framebuffer) {
        memset(framebuffer, 0, current_config.width * current_config.pages);
    }
}

void ssd1306_display(void) {
    if (i2c_fd < 0 || !framebuffer) return;
    
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(current_config.width - 1);
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(current_config.pages - 1);
    
    ssd1306_data(framebuffer, current_config.width * current_config.pages);
}

void ssd1306_draw_pixel(int x, int y, bool on) {
    if (x < 0 || x >= current_config.width || y < 0 || y >= current_config.height) {
        return;
    }
    
    int page = y / 8;
    int bit = y % 8;
    int index = page * current_config.width + x;
    
    if (on) {
        framebuffer[index] |= (1 << bit);
    } else {
        framebuffer[index] &= ~(1 << bit);
    }
}

void ssd1306_draw_progress_bar(int value, int max_value, int x, int y, int width, int height) {
    if (max_value <= 0) return;
    
    int fill_width = (value * width) / max_value;
    if (fill_width > width) fill_width = width;
    
    for (int py = y; py < y + height; py++) {
        for (int px = x; px < x + width; px++) {
            bool fill = (px < x + fill_width);
            bool border = (py == y || py == y + height - 1 || px == x || px == x + width - 1);
            ssd1306_draw_pixel(px, py, fill || border);
        }
    }
}