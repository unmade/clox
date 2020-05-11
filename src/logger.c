#include <stdarg.h>
#include <stdio.h>

#include "logger.h"

static const char *get_err_name(int type);


void log_error(int type, const char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", get_err_name(type));

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}


static const char *get_err_name(int type)
{
    switch (type) {
        case LOX_RUNTIME_ERR:
            return "RuntimeError";
        case LOX_SYNTAX_ERR:
            return "SyntaxError";
        default:
            return "Error";
    };
}
