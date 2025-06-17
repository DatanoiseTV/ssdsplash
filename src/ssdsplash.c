#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include "ssdsplash.h"

static volatile bool running = true;
static int server_fd = -1;
static display_type_t display_type = DISPLAY_128x64;
static char *i2c_device = NULL;
static uint8_t i2c_address = 0;

static void signal_handler(int sig) {
    (void)sig;
    running = false;
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
}

static void show_help(const char *progname) {
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("SSD1306 OLED splash screen daemon\n\n");
    printf("Options:\n");
    printf("  -d, --device DEVICE    I2C device (default: /dev/i2c-1)\n");
    printf("  -a, --address ADDR     I2C address: 0x3C or 0x3D (default: 0x3C)\n");
    printf("  -t, --type TYPE        Display type: 128x64 or 128x32 (default: 128x64)\n");
    printf("  -h, --help             Show this help\n");
    printf("\nCommands via ssdsplash-send:\n");
    printf("  ssdsplash-send -t text \"Boot message\"\n");
    printf("  ssdsplash-send -t text -f font.ttf -z 16 \"TrueType text\"\n");
    printf("  ssdsplash-send -t progress -v 50\n");
    printf("  ssdsplash-send -t img /path/to/logo.png\n");
    printf("  ssdsplash-send -t img -s /path/to/splash.jpg\n");
    printf("  ssdsplash-send -t clear\n");
    printf("  ssdsplash-send -t quit\n");
}

static int setup_server_socket(void) {
    struct sockaddr_un addr;
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    
    unlink(SSDSPLASH_SOCKET_PATH);
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SSDSPLASH_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    
    return 0;
}

static void handle_message(const ssdsplash_message_t *msg) {
    switch (msg->type) {
        case MSG_TYPE_TEXT:
            ssd1306_clear();
            
            if (strlen(msg->data.text_msg.font_path) > 0) {
                printf("Text: %s (line %d, font: %s, size: %d)\n", 
                       msg->data.text_msg.text, msg->data.text_msg.line,
                       msg->data.text_msg.font_path, msg->data.text_msg.font_size);
                ssd1306_draw_text_truetype(msg->data.text_msg.text, 0, msg->data.text_msg.line * msg->data.text_msg.font_size,
                                         msg->data.text_msg.font_path, msg->data.text_msg.font_size);
            } else {
                printf("Text: %s (line %d, bitmap font)\n", msg->data.text_msg.text, msg->data.text_msg.line);
                ssd1306_draw_text(msg->data.text_msg.text, 0, msg->data.text_msg.line * 8);
            }
            
            ssd1306_display();
            break;
            
        case MSG_TYPE_PROGRESS:
            ssd1306_clear();
            ssd1306_draw_text("Loading...", 0, 0);
            ssd1306_draw_progress_bar(msg->data.progress_msg.value, 
                                    msg->data.progress_msg.max_value, 
                                    0, 16, 128, 8);
            
            char progress_text[32];
            snprintf(progress_text, sizeof(progress_text), "%d%%", 
                    (msg->data.progress_msg.value * 100) / msg->data.progress_msg.max_value);
            ssd1306_draw_text(progress_text, 90, 26);
            ssd1306_display();
            printf("Progress: %d/%d\n", msg->data.progress_msg.value, msg->data.progress_msg.max_value);
            break;
            
        case MSG_TYPE_CLEAR:
            ssd1306_clear();
            ssd1306_display();
            printf("Screen cleared\n");
            break;
            
        case MSG_TYPE_QUIT:
            printf("Quit requested\n");
            running = false;
            break;
            
        case MSG_TYPE_IMAGE:
            printf("Loading image: %s (scaled: %s)\n", 
                   msg->data.image_msg.path, 
                   msg->data.image_msg.scaled ? "yes" : "no");
            
            if (msg->data.image_msg.scaled) {
                if (ssd1306_load_and_display_image_scaled(msg->data.image_msg.path) < 0) {
                    printf("Failed to load image: %s\n", msg->data.image_msg.path);
                }
            } else {
                if (ssd1306_load_and_display_image(msg->data.image_msg.path) < 0) {
                    printf("Failed to load image: %s\n", msg->data.image_msg.path);
                }
            }
            break;
    }
}

static void* client_handler(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    ssdsplash_message_t msg;
    ssize_t bytes_read = recv(client_fd, &msg, sizeof(msg), 0);
    
    if (bytes_read == sizeof(msg)) {
        handle_message(&msg);
    }
    
    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    int opt;
    struct option long_options[] = {
        {"device", required_argument, 0, 'd'},
        {"address", required_argument, 0, 'a'},
        {"type", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "d:a:t:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                i2c_device = strdup(optarg);
                break;
            case 'a':
                if (strcmp(optarg, "0x3C") == 0 || strcmp(optarg, "0x3c") == 0) {
                    i2c_address = 0x3C;
                } else if (strcmp(optarg, "0x3D") == 0 || strcmp(optarg, "0x3d") == 0) {
                    i2c_address = 0x3D;
                } else {
                    unsigned int addr;
                    if (sscanf(optarg, "%x", &addr) == 1 && (addr == 0x3C || addr == 0x3D)) {
                        i2c_address = addr;
                    } else {
                        fprintf(stderr, "Invalid I2C address: %s (must be 0x3C or 0x3D)\n", optarg);
                        return 1;
                    }
                }
                break;
            case 't':
                if (strcmp(optarg, "128x32") == 0) {
                    display_type = DISPLAY_128x32;
                } else if (strcmp(optarg, "128x64") == 0) {
                    display_type = DISPLAY_128x64;
                } else {
                    fprintf(stderr, "Invalid display type: %s\n", optarg);
                    return 1;
                }
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (ssd1306_init(display_type, i2c_device, i2c_address) < 0) {
        fprintf(stderr, "Failed to initialize SSD1306 display\n");
        return 1;
    }
    
    printf("SSD1306 splash daemon started (display: %s, device: %s, address: 0x%02X)\n",
           display_type == DISPLAY_128x64 ? "128x64" : "128x32",
           i2c_device ? i2c_device : "/dev/i2c-1",
           i2c_address ? i2c_address : 0x3C);
    
    ssd1306_draw_text("ssdsplash ready", 0, 0);
    ssd1306_display();
    
    if (setup_server_socket() < 0) {
        ssd1306_cleanup();
        return 1;
    }
    
    while (running) {
        fd_set read_fds;
        struct timeval timeout;
        
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }
        
        if (activity > 0 && FD_ISSET(server_fd, &read_fds)) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd >= 0) {
                int *client_fd_ptr = malloc(sizeof(int));
                *client_fd_ptr = client_fd;
                
                pthread_t thread;
                if (pthread_create(&thread, NULL, client_handler, client_fd_ptr) == 0) {
                    pthread_detach(thread);
                } else {
                    close(client_fd);
                    free(client_fd_ptr);
                }
            }
        }
    }
    
    printf("Shutting down...\n");
    ssd1306_clear();
    ssd1306_display();
    ssd1306_cleanup();
    ssd1306_cleanup_truetype();
    
    if (server_fd >= 0) {
        close(server_fd);
        unlink(SSDSPLASH_SOCKET_PATH);
    }
    
    if (i2c_device) {
        free(i2c_device);
    }
    
    return 0;
}