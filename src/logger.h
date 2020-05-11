#ifndef clox_logger_h
#define clox_logger_h

enum LoxErrorType {
    LOX_ERR = 0,
    LOX_RUNTIME_ERR,
    LOX_SYNTAX_ERR,
};

void log_error(int type, const char *fmt, ...);

#endif
