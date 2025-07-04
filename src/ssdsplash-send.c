#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>
#include <stdarg.h>
#include "ssdsplash.h"

static void show_help(const char *progname) {
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("Send commands to ssdsplash daemon\n\n");
    printf("Options:\n");
    printf("  -t, --type TYPE        Message type: text, progress, clear, quit, img\n");
    printf("  -f, --font FONT        Font file (.ttf) for text type\n");
    printf("  -z, --size SIZE        Font size in pixels (default: 12)\n");
    printf("  -v, --value VALUE      Progress value (for progress type)\n");
    printf("  -m, --max MAX          Maximum value (for progress type, default: 100)\n");
    printf("  -l, --line LINE        Text line number (for text type, default: 0)\n");
    printf("  -s, --scaled           Scale image to fit screen (for img type)\n");
    printf("  -h, --help             Show this help\n");
    printf("  TEXT/PATH [ARGS...]    Text message (for text type) or image path (for img type)\n");
    printf("                         For text: supports printf-style format strings with args\n\n");
    printf("Examples:\n");
    printf("  %s -t text \"Loading configuration...\"\n", progname);
    printf("  %s -t text -f /path/to/font.ttf -z 16 \"TrueType Text\"\n", progname);
    printf("  %s -t text -l 1 \"Starting services\"\n", progname);
    printf("  %s -t text \"Service %%s returned %%d\" nginx 0\n", progname);
    printf("  %s -t text \"Progress: %%d/%%d (%%0.1f%%)\" 42 100 42.0\n", progname);
    printf("  %s -t progress -v 42\n", progname);
    printf("  %s -t progress -v 50 -m 200\n", progname);
    printf("  %s -t img /path/to/logo.png\n", progname);
    printf("  %s -t img -s /path/to/splash.jpg\n", progname);
    printf("  %s -t clear\n", progname);
    printf("  %s -t quit\n", progname);
}

static int format_text_with_args(char *output, size_t output_size, const char *format, int argc, char *argv[], int start_idx) {
    if (start_idx >= argc) {
        strncpy(output, format, output_size - 1);
        output[output_size - 1] = '\0';
        return 0;
    }
    
    char *temp_format = strdup(format);
    if (!temp_format) {
        return -1;
    }
    
    char *result = malloc(output_size);
    if (!result) {
        free(temp_format);
        return -1;
    }
    
    char *format_ptr = temp_format;
    char *output_ptr = result;
    size_t remaining = output_size - 1;
    int arg_idx = start_idx;
    
    while (*format_ptr && remaining > 0) {
        if (*format_ptr == '%' && *(format_ptr + 1) != '\0') {
            char *spec_start = format_ptr;
            format_ptr++;
            
            while (*format_ptr && strchr("-+ #0123456789.*", *format_ptr)) {
                format_ptr++;
            }
            
            if (*format_ptr == '\0') {
                break;
            }
            
            char spec = *format_ptr;
            format_ptr++;
            
            if (spec == '%') {
                if (remaining > 0) {
                    *output_ptr++ = '%';
                    remaining--;
                }
            } else if (arg_idx < argc) {
                char spec_str[32];
                int spec_len = format_ptr - spec_start;
                if (spec_len < (int)sizeof(spec_str)) {
                    strncpy(spec_str, spec_start, spec_len);
                    spec_str[spec_len] = '\0';
                    
                    char formatted[64];
                    int written = 0;
                    
                    switch (spec) {
                        case 'd':
                        case 'i':
                            written = snprintf(formatted, sizeof(formatted), spec_str, atoi(argv[arg_idx]));
                            break;
                        case 'u':
                        case 'x':
                        case 'X':
                        case 'o':
                            written = snprintf(formatted, sizeof(formatted), spec_str, (unsigned int)atoi(argv[arg_idx]));
                            break;
                        case 'f':
                        case 'F':
                        case 'e':
                        case 'E':
                        case 'g':
                        case 'G':
                            written = snprintf(formatted, sizeof(formatted), spec_str, atof(argv[arg_idx]));
                            break;
                        case 'c':
                            written = snprintf(formatted, sizeof(formatted), spec_str, argv[arg_idx][0]);
                            break;
                        case 's':
                            written = snprintf(formatted, sizeof(formatted), spec_str, argv[arg_idx]);
                            break;
                        default:
                            written = snprintf(formatted, sizeof(formatted), "%%");
                            break;
                    }
                    
                    if (written > 0 && (size_t)written <= remaining) {
                        strncpy(output_ptr, formatted, written);
                        output_ptr += written;
                        remaining -= written;
                    }
                    arg_idx++;
                }
            }
        } else {
            *output_ptr++ = *format_ptr++;
            remaining--;
        }
    }
    
    *output_ptr = '\0';
    strncpy(output, result, output_size - 1);
    output[output_size - 1] = '\0';
    
    free(temp_format);
    free(result);
    return 0;
}

static int send_message(const ssdsplash_message_t *msg) {
    int sock_fd;
    struct sockaddr_un addr;
    
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SSDSPLASH_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        fprintf(stderr, "Is ssdsplash daemon running?\n");
        close(sock_fd);
        return -1;
    }
    
    if (send(sock_fd, msg, sizeof(*msg), 0) != sizeof(*msg)) {
        perror("send");
        close(sock_fd);
        return -1;
    }
    
    close(sock_fd);
    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    char *type = NULL;
    char *font_path = NULL;
    int font_size = 12;
    int value = 0;
    int max_value = 100;
    int line = 0;
    bool scaled = false;
    ssdsplash_message_t msg = {0};
    
    struct option long_options[] = {
        {"type", required_argument, 0, 't'},
        {"font", required_argument, 0, 'f'},
        {"size", required_argument, 0, 'z'},
        {"value", required_argument, 0, 'v'},
        {"max", required_argument, 0, 'm'},
        {"line", required_argument, 0, 'l'},
        {"scaled", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "t:f:z:v:m:l:sh", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                type = optarg;
                break;
            case 'f':
                font_path = optarg;
                break;
            case 'z':
                font_size = atoi(optarg);
                break;
            case 'v':
                value = atoi(optarg);
                break;
            case 'm':
                max_value = atoi(optarg);
                break;
            case 'l':
                line = atoi(optarg);
                break;
            case 's':
                scaled = true;
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    if (!type) {
        fprintf(stderr, "Error: Message type (-t) is required\n");
        show_help(argv[0]);
        return 1;
    }
    
    if (strcmp(type, "text") == 0) {
        if (optind >= argc) {
            fprintf(stderr, "Error: Text message is required for text type\n");
            return 1;
        }
        
        msg.type = MSG_TYPE_TEXT;
        
        char formatted_text[SSDSPLASH_MAX_TEXT_LEN];
        if (format_text_with_args(formatted_text, sizeof(formatted_text), argv[optind], argc, argv, optind + 1) < 0) {
            fprintf(stderr, "Error: Failed to format text\n");
            return 1;
        }
        
        strncpy(msg.data.text_msg.text, formatted_text, SSDSPLASH_MAX_TEXT_LEN - 1);
        msg.data.text_msg.text[SSDSPLASH_MAX_TEXT_LEN - 1] = '\0';
        msg.data.text_msg.line = line;
        
        if (font_path) {
            strncpy(msg.data.text_msg.font_path, font_path, SSDSPLASH_MAX_PATH_LEN - 1);
            msg.data.text_msg.font_path[SSDSPLASH_MAX_PATH_LEN - 1] = '\0';
        } else {
            msg.data.text_msg.font_path[0] = '\0';
        }
        msg.data.text_msg.font_size = font_size;
        
    } else if (strcmp(type, "progress") == 0) {
        msg.type = MSG_TYPE_PROGRESS;
        msg.data.progress_msg.value = value;
        msg.data.progress_msg.max_value = max_value;
        
    } else if (strcmp(type, "clear") == 0) {
        msg.type = MSG_TYPE_CLEAR;
        
    } else if (strcmp(type, "quit") == 0) {
        msg.type = MSG_TYPE_QUIT;
        
    } else if (strcmp(type, "img") == 0) {
        if (optind >= argc) {
            fprintf(stderr, "Error: Image path is required for img type\n");
            return 1;
        }
        
        msg.type = MSG_TYPE_IMAGE;
        strncpy(msg.data.image_msg.path, argv[optind], SSDSPLASH_MAX_PATH_LEN - 1);
        msg.data.image_msg.path[SSDSPLASH_MAX_PATH_LEN - 1] = '\0';
        msg.data.image_msg.scaled = scaled;
        
    } else {
        fprintf(stderr, "Error: Invalid message type: %s\n", type);
        fprintf(stderr, "Valid types: text, progress, clear, quit, img\n");
        return 1;
    }
    
    if (send_message(&msg) < 0) {
        return 1;
    }
    
    return 0;
}