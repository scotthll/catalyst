/*
 * @Description: 
 * @Author: Huang Leilei
 * @Verion: v0.0.1
 * @Date: 2022-12-16 14:13:48
 * @LastEditTime: 2023-01-04 14:14:33
 * @LastEditors: Huang Leilei
 * @FilePath: \add\src\include\log.h
 */
#ifndef __ADD_LOG_H__
#define __ADD_LOG_H__

#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MACSTR
#define MACSTR          "%02x:%02x:%02x:%02x:%02x:%02x"
#endif
#ifndef MAC2STR
#define MAC2STR(a)      (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

#ifndef NIPQUAD
#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]
#endif
#ifndef NIPQUAD_r
#define NIPQUAD_r(addr) \
    ((unsigned char *)&addr)[3], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[0]
#endif
#ifndef NIPQUAD_FMT
#define NIPQUAD_FMT "%u.%u.%u.%u"
#endif

#define _log_printf_flag     1
extern int log_level_default;
extern char log_buff[4096];

enum {
    DUMP_PREFIX_NONE,
    DUMP_PREFIX_ADDRESS,
    DUMP_PREFIX_OFFSET
};

#ifndef _XLOG_BASENAME
#define _XLOG_BASENAME
/**
 * @brief 
 * 
 * @param s 
 * @return char* 
 */
static inline char *_xlog_basename(char *s)
{
    char *stub = (char *)s;
    
    stub = strrchr(s, '/');
    if (NULL == stub)
        return s;
    return stub + 1;
}
#endif

/**
 * @brief 
 * 
 * @param log_level 
 * @param file 
 * @param func 
 * @param line 
 * @param fmt 
 * @param ... 
 * @return int 
 */
static inline int _syslog(int log_level, 
        const char *file, const char *func, int line,
        const char *fmt, ...)
{
    // char _str[164] = { 0 };
    char *_str = log_buff;
    va_list ap;
    int str_len = 0;

    if (log_level_default < log_level)
        return 0;

    memset(log_buff, 0, sizeof(log_buff));

    str_len = sprintf(_str, "[%s %s %d] ", _xlog_basename((char *)file), func, line);

    va_start(ap, fmt);
    str_len += vsprintf(_str + str_len, fmt, ap);
    va_end(ap);

    if ('\n' == _str[strlen(_str) - 1])
    {
        _str[strlen(_str) - 1] = '\0';
    }
    
    syslog(log_level, "%s", _str);

    if (_log_printf_flag)
    {
        printf("%s\n", _str);
    }

    // switch (log_level)
    // {
    //     case LOG_DEBUG:
    //         // ESP_LOGD(tag, "%s", _str);
    //         // break;
    //     case LOG_INFO:
    //         syslog(tag, "%s", _str);
    //         break;
    //     case LOG_NOTICE:
    //     case LOG_WARNING:
    //         ESP_LOGW(tag, "%s", _str);
    //         break;
    //     case LOG_ERR:
    //     case LOG_CRIT:
    //     case LOG_ALERT:
    //     case LOG_EMERG:
    //         ESP_LOGE(tag, "%s", _str);
    //         break;
    // }
    
    return str_len;
}


void _hex_dump(const char *file, const char *func, int line,
        const void *buf, int len);

#define log_debug(fmt, ...)     _syslog(LOG_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)      _syslog(LOG_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_notice(fmt, ...)    _syslog(LOG_NOTICE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)      _syslog(LOG_WARNING, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_err(fmt, ...)       _syslog(LOG_ERR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_crit(fmt, ...)      _syslog(LOG_CRIT, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_alert(fmt, ...)     _syslog(LOG_ALERT, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_emerg(fmt, ...)     _syslog(LOG_EMERG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define log_hex(buffer, len)    _hex_dump(__FILE__, __func__, __LINE__, buffer, len)
#define log_set_level(level)    log_set_level_tag(level)

void log_init(char *ident);

#ifdef __cplusplus
}
#endif

#endif