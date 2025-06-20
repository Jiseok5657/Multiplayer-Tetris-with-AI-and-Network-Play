/**
 * @file error.c
 * @brief エラー処理とログ機能実装
 * 
 */

#include "error.h"
#include <stdarg.h>
#include <stdio.h>

/**
 * @brief ログメッセージを出力する
 * @param level ログレベル
 * @param fmt フォーマット文字列
 */
void log_message(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    const char* level_str;
    FILE* output = stdout;
    
    switch (level) {
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_WARNING:
            level_str = "WARN";
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            output = stderr;
            break;
        default:
            level_str = "UNKNOWN";
    }
    
    fprintf(output, "[%s] ", level_str);
    vfprintf(output, fmt, args);
    fprintf(output, "\n");
    
    va_end(args);
}