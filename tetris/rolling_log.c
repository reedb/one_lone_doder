//*****************************************************************************
//
// rolling_log.c - Implementation of a rolling logs. Linux hosted.
//
// Author: Reed Bement, Sat 28 Jan 2023
//
//    Notes:
//    ----
//    Build with:
//    gcc -std=c99 -Werror -Wall -Wextra -Wconversion -g -c rolling_log.c 
//   
//******************************************************************************

#define _POSIX_C_SOURCE 199309L		// Expose clock_gettime and friends using feature_test_macros

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>

#include "rolling_log.h"

#define MAX_LOG_FILES 4
#define MAX_LOG_SIZE  64 * 1024

char g_szLogFileName[MAX_LOG_FILE_NAME] = "default_log_file_name";

// Log current time in seconds since start of Epoch with millisecond resolution.
// When reading log, use Epoch converter to get GMT (example: epochconverter.com)
//
void log_current_time_with_ms(FILE *log_fp)
{
    long   ms; // Milliseconds
    time_t s;  // Seconds

    struct timespec spec;
    char sz[64];
    
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = (long int)round((double)spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) { s++; ms = 0; }
    snprintf(sz, sizeof(sz)-1, "%ld.%03ld - ", s, ms);
    fputs(sz, log_fp);
}

FILE *log_printf_open_helper(int i)
{
    static char sz[MAX_LOG_FILE_NAME + 16];
    snprintf(sz, sizeof(sz)-1, "%s%d.log", g_szLogFileName, i);
    return (fopen(sz, "a"));
}

void log_printf_rename_helper(int i)
{
    char sz0[MAX_LOG_FILE_NAME + 16], sz1[MAX_LOG_FILE_NAME + 16];
    snprintf(sz0, sizeof(sz0)-1, "%s%d.log", g_szLogFileName, i);
    snprintf(sz1, sizeof(sz1)-1, "%s%d.log", g_szLogFileName, i+1);
    rename(sz0, sz1);
}

// Rolling logging with N log files, where syslog0.log contains the
// most recent data and syslog(N-1).log contains the oldest.
// 
void log_printf(const char *fmt, ...)
{   
  va_list args;
  static char szBuf[128];
  static FILE *log_fp = NULL;

  if (*fmt == '\0') return;
  va_start(args, fmt);
  
  if (!log_fp) log_fp = log_printf_open_helper(0);
 
  if (log_fp) {
    if (ftell(log_fp) > MAX_LOG_SIZE) {
      fclose(log_fp);
      log_fp = NULL;

      for (int i = MAX_LOG_FILES - 2; i >= 0; i--) {
        log_printf_rename_helper(i);  
      }
      log_fp = log_printf_open_helper(0);
    }

    log_current_time_with_ms(log_fp);
    vsnprintf(szBuf, sizeof(szBuf)-1, fmt, args);
    va_end(args);
    fputs(szBuf, log_fp);
    fflush(log_fp);
  }
}
