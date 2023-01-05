/*
 * @Description: 
 * @Author: Huang Leilei
 * @Verion: v0.0.1
 * @Date: 2022-12-16 15:36:53
 * @LastEditTime: 2023-01-04 10:45:38
 * @LastEditors: Huang Leilei
 * @FilePath: \add\src\log.c
 */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "log.h"

#ifndef _min
#define _min(x, y) (((x) < (y)) ? (x) : (y))
#endif
static const char hex_asc[] = "0123456789abcdef";
#ifndef hex_asc_lo
#define hex_asc_lo(x)       hex_asc[((x) & 0x0f)]
#endif
#ifndef hex_asc_hi
#define hex_asc_hi(x)       hex_asc[((x) & 0xf0) >> 4]
#endif

int log_level_default = LOG_INFO;
char log_buff[4096] = { 0 };


/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @param rowsize 
 * @param groupsize 
 * @param linebuf 
 * @param linebuflen 
 * @param ascii 
 */
static inline void hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
        int groupsize, char *linebuf, size_t linebuflen, int ascii)
{
    uint8_t *ptr = (uint8_t*)buf;
    uint8_t ch;
    unsigned int j, lx = 0;
    int ascii_column;

    if (rowsize != 16
        && rowsize != 32)
    {
        rowsize = 16;
    }

    if (!len)
    {
        goto nil;
    }
    if (len > rowsize)              /* limit to one line at a time */
    {
        len = rowsize;
    }
    if ((len % groupsize) != 0)     /* no mixed size output */
    {
        groupsize = 1;
    }

    switch (groupsize)
    {
        case 8:
        {
            const uint64_t *ptr8 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
            {
                lx += snprintf(linebuf + lx, linebuflen - lx,
                        "%s%16.16llx", j ? "                          " : "",
                        (unsigned long long)*(ptr8 + j));
            }
            ascii_column = 17 * ngroups + 2;
            break;
        }

        case 4:
        {
            const uint32_t *ptr4 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
            {
                lx += snprintf(linebuf + lx, linebuflen - lx,
                        "%s%8.8x", j ? " " : "", *(ptr4 + j));
            }
            ascii_column = 9 * ngroups + 2;
            break;
        }

        case 2:
        {
            const uint16_t *ptr2 = buf;
            int ngroups = len / groupsize;

            for (j = 0; j < ngroups; j++)
            {
                lx += snprintf(linebuf + lx, linebuflen - lx,
                        "%s%4.4x", j ? " " : "", *(ptr2 + j));
            }
            ascii_column = 5 * ngroups + 2;
            break;
        }

        default:
            for (j = 0; (j < len) && (lx + 3) <= linebuflen; j++)
            {
                ch = ptr[j];
                linebuf[lx++] = hex_asc_hi(ch);
                linebuf[lx++] = hex_asc_lo(ch);
                linebuf[lx++] = ' ';
                if (0 == (j + 1) % 8)
                {
                    linebuf[lx++] = ' ';
                }
            }
            if (j)
            {
                lx--;
            }

            ascii_column = 3 * rowsize + 2;
            break;
    }
    if (!ascii)
    {
        goto nil;
    }

    while (lx < (linebuflen - 1)
        && lx < (ascii_column - 1))
    {
        linebuf[lx++] = ' ';
    }
    for (j = 0; (j < len) && (lx + 2) < linebuflen; j++)
    {
        ch = ptr[j];
        linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
    }
nil:
    linebuf[lx++] = '\0';
}

/**
 * @brief 
 * 
 * @param prefix_str 
 * @param prefix_type 
 * @param rowsize 
 * @param groupsize 
 * @param buf 
 * @param len 
 * @param ascii 
 */
static void print_hex_dump(const char *file, const char *func, int line,
            const char *prefix_str, int prefix_type,
            int rowsize, int groupsize,
            const void *buf, size_t len, int ascii)
{
    const uint8_t *ptr = (const uint8_t*)buf;
    int i = 0, linelen = 0, remaining = len;
    char linebuf[32 * 3 + 2 + 32 + 1] = { 0 };

    if (rowsize != 16
        && rowsize != 32)
    {
        rowsize = 16;
    }

    for (i = 0; i < len; i += rowsize)
    {
        linelen = _min(remaining, rowsize);
        remaining -= rowsize;

        hex_dump_to_buffer(ptr + i, linelen, rowsize, groupsize,
                    linebuf, sizeof(linebuf), ascii);

        switch (prefix_type)
        {
            case DUMP_PREFIX_ADDRESS:
                printf("%s %p: %s\n",
                        prefix_str, ptr + i, linebuf);
                syslog(LOG_DEBUG,"%s %p: %s\n",
                        prefix_str, ptr + i, linebuf);
                break;
            case DUMP_PREFIX_OFFSET:
                printf("%s %.8x: %s\n", prefix_str, i, linebuf);
                syslog(LOG_DEBUG,"%s %.8x: %s\n", prefix_str, i, linebuf);
                break;
            default:
                printf("%s %s\n",prefix_str, linebuf);
                syslog(LOG_DEBUG,"%s %s\n",prefix_str, linebuf);
                break;
        }
    }
}

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 */
void _hex_dump(const char *file, const char *func, int line,
        const void *buf, int len)
{
    if (log_level_default < LOG_DEBUG)
        return ;
    print_hex_dump(file, func, line,
        "IWC Hex Dump", DUMP_PREFIX_OFFSET, 16, 1, buf, len, 1);
}

/**
 * @brief 
 * 
 * @param file 
 * @param func 
 * @param line 
 * @param buffer 
 * @param len 
 */
void log_hex_buffer(const char *file, const char *func, int line,
        unsigned char *buffer, int len)
{
    int i = 0;
    int print_flag = 0;
    char _str[128] = { 0 };
    int str_len = 0;

    if (LOG_DEBUG > log_level_default)
        return ;

    for (i = 0; i < len; i++)
    {
        print_flag = 0;
        if (0 == (i + 1) % 16)
        {
            str_len += sprintf(_str + str_len, "%02x", buffer[i]);
            _syslog(LOG_DEBUG, file, func, line, "%s", _str);
            memset(_str, 0, 128);
            print_flag = 1;
            str_len = 0;
        } else if (0 == (i + 1) % 8) {
            str_len += sprintf(_str + str_len, "%02x   ", buffer[i]);
        } else {
            str_len += sprintf(_str + str_len, "%02x ", buffer[i]);
        }
    }
    if (0 == print_flag)
    {
        _syslog(LOG_DEBUG, file, func, line, "%s", _str);
    }
    return ;
}

/**
 * @brief 
 * 
 * @param ident 
 */
void log_init(char *ident)
{
    // printf("[%s %d]\n", __func__, __LINE__);
    openlog(ident, LOG_PID, LOG_DAEMON | LOG_USER);
}