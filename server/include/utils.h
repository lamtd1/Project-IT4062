#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Log levels
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;

// Initialize logger
void log_init();

// Log a message
void log_message(LogLevel level, const char *format, ...);

// Close logger
void log_close();

#endif // UTILS_H
