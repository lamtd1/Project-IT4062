#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/utils.h"

static FILE *log_file = NULL;

void log_init() {
    // Ensure logs directory exists
    struct stat st = {0};
    if (stat("server/logs", &st) == -1) {
        #ifdef _WIN32
        mkdir("server/logs");
        #else
        mkdir("server/logs", 0700);
        #endif
    }

    log_file = fopen("server/logs/server.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
    }
}

void log_message(LogLevel level, const char *format, ...) {
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0'; // Remove newline

    const char *level_str;
    switch (level) {
        case LOG_INFO:  level_str = "INFO"; break;
        case LOG_WARN:  level_str = "WARN"; break;
        case LOG_ERROR: level_str = "ERROR"; break;
        case LOG_DEBUG: level_str = "DEBUG"; break;
        default:        level_str = "UNKNOWN"; break;
    }

    // Format message
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print to console
    printf("[%s] [%s] %s\n", date, level_str, buffer);

    // Write to file
    if (log_file) {
        fprintf(log_file, "[%s] [%s] %s\n", date, level_str, buffer);
        fflush(log_file);
    }
}

void log_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
