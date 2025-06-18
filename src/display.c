#define _GNU_SOURCE
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
#define SSH1106_I2C_ADDRESS_DEFAULT 0x3C
#define SSH1106_I2C_ADDRESS_ALT 0x3D

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

// SSH1106 specific commands
#define SSH1106_SETLOWCOLUMN    0x00
#define SSH1106_SETHIGHCOLUMN   0x10
#define SSH1106_SETPAGEADDR     0xB0
#define SSH1106_SETCOMPINS      0xDA
#define SSH1106_SETCONTRAST     0x81
#define SSH1106_SETPRECHARGE    0xD9
#define SSH1106_SETVCOMDETECT   0xDB
#define SSH1106_SETDISPLAYCLOCKDIV 0xD5
#define SSH1106_SETMULTIPLEX    0xA8
#define SSH1106_DISPLAYOFF      0xAE
#define SSH1106_DISPLAYON       0xAF
#define SSH1106_NORMALDISPLAY   0xA6
#define SSH1106_INVERTDISPLAY   0xA7
#define SSH1106_SEGREMAP        0xA0
#define SSH1106_COMSCANDEC      0xC8
#define SSH1106_CHARGEPUMP      0x8D

// ILI9341 commands
#define ILI9341_SWRESET         0x01
#define ILI9341_SLPOUT          0x11
#define ILI9341_DISPON          0x29
#define ILI9341_CASET           0x2A
#define ILI9341_PASET           0x2B
#define ILI9341_RAMWR           0x2C
#define ILI9341_MADCTL          0x36
#define ILI9341_COLMOD          0x3A
#define ILI9341_PWCTR1          0xC0
#define ILI9341_PWCTR2          0xC1
#define ILI9341_VMCTR1          0xC5
#define ILI9341_VMCTR2          0xC7
#define ILI9341_GMCTRP1         0xE0
#define ILI9341_GMCTRN1         0xE1

const display_config_t display_configs[] = {
    [DISPLAY_128x64] = {128, 64, 8},
    [DISPLAY_128x32] = {128, 32, 4},
    [DISPLAY_ILI9341_240x320] = {240, 320, 40},
    [DISPLAY_SSH1106_128x64] = {128, 64, 8}
};

static int device_fd = -1;
display_config_t current_config;
static uint8_t *framebuffer = NULL;
static uint8_t device_address = SSD1306_I2C_ADDRESS_DEFAULT;
static display_type_t current_display_type = DISPLAY_128x64;

static int ssd1306_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    return write(device_fd, buf, 2) == 2 ? 0 : -1;
}

static int ssh1106_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    return write(device_fd, buf, 2) == 2 ? 0 : -1;
}

static int ili9341_command(uint8_t cmd) {
    // For SPI, different implementation would be needed
    // This is a placeholder for I2C-based ILI9341
    uint8_t buf[2] = {0x00, cmd};
    return write(device_fd, buf, 2) == 2 ? 0 : -1;
}

static int ssd1306_data(uint8_t *data, size_t len) {
    uint8_t *buf = malloc(len + 1);
    if (!buf) return -1;
    
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    int ret = write(device_fd, buf, len + 1) == (ssize_t)(len + 1) ? 0 : -1;
    free(buf);
    return ret;
}

static int ssh1106_data(uint8_t *data, size_t len) {
    uint8_t *buf = malloc(len + 1);
    if (!buf) return -1;
    
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    int ret = write(device_fd, buf, len + 1) == (ssize_t)(len + 1) ? 0 : -1;
    free(buf);
    return ret;
}

static int ili9341_data(uint8_t *data, size_t len) {
    uint8_t *buf = malloc(len + 1);
    if (!buf) return -1;
    
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    int ret = write(device_fd, buf, len + 1) == (ssize_t)(len + 1) ? 0 : -1;
    free(buf);
    return ret;
}

static int ssd1306_init_display(void) {
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
    return 0;
}

static int ssh1106_init_display(void) {
    ssh1106_command(SSH1106_DISPLAYOFF);
    ssh1106_command(SSH1106_SETDISPLAYCLOCKDIV);
    ssh1106_command(0x80);
    ssh1106_command(SSH1106_SETMULTIPLEX);
    ssh1106_command(current_config.height - 1);
    ssh1106_command(SSD1306_SETDISPLAYOFFSET);
    ssh1106_command(0x00);
    ssh1106_command(SSD1306_SETSTARTLINE | 0x00);
    ssh1106_command(SSH1106_CHARGEPUMP);
    ssh1106_command(0x14);
    ssh1106_command(SSH1106_SEGREMAP | 0x01);
    ssh1106_command(SSH1106_COMSCANDEC);
    ssh1106_command(SSH1106_SETCOMPINS);
    ssh1106_command(0x12);
    ssh1106_command(SSH1106_SETCONTRAST);
    ssh1106_command(0xCF);
    ssh1106_command(SSH1106_SETPRECHARGE);
    ssh1106_command(0xF1);
    ssh1106_command(SSH1106_SETVCOMDETECT);
    ssh1106_command(0x40);
    ssh1106_command(SSD1306_DISPLAYALLON_RESUME);
    ssh1106_command(SSH1106_NORMALDISPLAY);
    ssh1106_command(SSH1106_DISPLAYON);
    return 0;
}

static int ili9341_init_display(void) {
    ili9341_command(ILI9341_SWRESET);
    usleep(150000);
    ili9341_command(ILI9341_SLPOUT);
    usleep(500000);
    ili9341_command(ILI9341_PWCTR1);
    ili9341_command(0x23);
    ili9341_command(ILI9341_PWCTR2);
    ili9341_command(0x10);
    ili9341_command(ILI9341_VMCTR1);
    ili9341_command(0x3E);
    ili9341_command(0x28);
    ili9341_command(ILI9341_VMCTR2);
    ili9341_command(0x86);
    ili9341_command(ILI9341_MADCTL);
    ili9341_command(0x48);
    ili9341_command(ILI9341_COLMOD);
    ili9341_command(0x55);
    ili9341_command(ILI9341_DISPON);
    return 0;
}

int display_init(display_type_t type, const char *device, uint8_t addr) {
    if (type >= sizeof(display_configs) / sizeof(display_configs[0])) {
        return -1;
    }
    
    current_display_type = type;
    current_config = display_configs[type];
    
    if (addr == 0) {
        if (type == DISPLAY_SSH1106_128x64) {
            device_address = SSH1106_I2C_ADDRESS_DEFAULT;
        } else {
            device_address = SSD1306_I2C_ADDRESS_DEFAULT;
        }
    } else {
        device_address = addr;
    }
    
    const char *default_device = "/dev/i2c-1";
    if (type == DISPLAY_ILI9341_240x320) {
        default_device = "/dev/spidev0.0";
    }
    
    device_fd = open(device ? device : default_device, O_RDWR);
    if (device_fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    
    if (type != DISPLAY_ILI9341_240x320) {
        if (ioctl(device_fd, I2C_SLAVE, device_address) < 0) {
            perror("Failed to set I2C slave address");
            close(device_fd);
            device_fd = -1;
            return -1;
        }
    }
    
    framebuffer = calloc(current_config.width * current_config.pages, 1);
    if (!framebuffer) {
        close(device_fd);
        device_fd = -1;
        return -1;
    }
    
    int ret = 0;
    switch (type) {
        case DISPLAY_128x64:
        case DISPLAY_128x32:
            ret = ssd1306_init_display();
            break;
        case DISPLAY_SSH1106_128x64:
            ret = ssh1106_init_display();
            break;
        case DISPLAY_ILI9341_240x320:
            ret = ili9341_init_display();
            break;
        default:
            ret = -1;
            break;
    }
    
    if (ret == 0) {
        display_clear();
        display_update();
    }
    
    return ret;
}

void display_cleanup(void) {
    if (device_fd >= 0) {
        switch (current_display_type) {
            case DISPLAY_128x64:
            case DISPLAY_128x32:
                ssd1306_command(SSD1306_DISPLAYOFF);
                break;
            case DISPLAY_SSH1106_128x64:
                ssh1106_command(SSH1106_DISPLAYOFF);
                break;
            case DISPLAY_ILI9341_240x320:
                break;
            default:
                break;
        }
        close(device_fd);
        device_fd = -1;
    }
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }
}

void display_clear(void) {
    if (framebuffer) {
        memset(framebuffer, 0, current_config.width * current_config.pages);
    }
}

void display_update(void) {
    if (device_fd < 0 || !framebuffer) return;
    
    switch (current_display_type) {
        case DISPLAY_128x64:
        case DISPLAY_128x32:
            ssd1306_command(SSD1306_COLUMNADDR);
            ssd1306_command(0);
            ssd1306_command(current_config.width - 1);
            ssd1306_command(SSD1306_PAGEADDR);
            ssd1306_command(0);
            ssd1306_command(current_config.pages - 1);
            ssd1306_data(framebuffer, current_config.width * current_config.pages);
            break;
        case DISPLAY_SSH1106_128x64:
            for (int page = 0; page < current_config.pages; page++) {
                ssh1106_command(SSH1106_SETPAGEADDR + page);
                ssh1106_command(SSH1106_SETLOWCOLUMN + 2);
                ssh1106_command(SSH1106_SETHIGHCOLUMN + 0);
                ssh1106_data(framebuffer + page * current_config.width, current_config.width);
            }
            break;
        case DISPLAY_ILI9341_240x320:
            ili9341_command(ILI9341_CASET);
            ili9341_command(0x00);
            ili9341_command(0x00);
            ili9341_command(0x00);
            ili9341_command(current_config.width - 1);
            ili9341_command(ILI9341_PASET);
            ili9341_command(0x00);
            ili9341_command(0x00);
            ili9341_command(0x01);
            ili9341_command(current_config.height - 1);
            ili9341_command(ILI9341_RAMWR);
            ili9341_data(framebuffer, current_config.width * current_config.pages);
            break;
        default:
            break;
    }
}

void display_draw_pixel(int x, int y, bool on) {
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

void display_draw_progress_bar(int value, int max_value, int x, int y, int width, int height) {
    if (max_value <= 0) return;
    
    int fill_width = (value * width) / max_value;
    if (fill_width > width) fill_width = width;
    
    for (int py = y; py < y + height; py++) {
        for (int px = x; px < x + width; px++) {
            bool fill = (px < x + fill_width);
            bool border = (py == y || py == y + height - 1 || px == x || px == x + width - 1);
            display_draw_pixel(px, py, fill || border);
        }
    }
}