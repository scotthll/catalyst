/*
 * @Description: 
 * @Author: Huang Leilei
 * @Verion: v0.0.1
 * @Date: 2023-01-04 11:08:10
 * @LastEditTime: 2023-01-05 14:00:56
 * @LastEditors: Huang Leilei
 * @FilePath: \add\src\comp\xz.c
 */

// https://blog.csdn.net/jkhere/article/details/16980059
// https://www.cnblogs.com/wangchenggen/p/3602293.html

#include <stdio.h> 
#include <stdint.h> 
#include <inttypes.h> 
#include <stdbool.h> 
#include <lzma.h>
#include <string.h>

#include "log.h"

// -llzma

#if 1

#undef BUFSIZ
#define BUFSIZ      4096

#define LZMA_PRESET_VAL 9

/**
 * @brief Get the preset object
 * 
 * @return uint32_t 
 */
static uint32_t get_preset()
{
    return LZMA_PRESET_VAL | LZMA_PRESET_EXTREME;
}

/**
 * @brief Get the error object
 * 
 * @param ret 
 * @return true 
 * @return false 
 */
static bool get_error(lzma_ret ret)
{
    const char *msg = NULL;
    // Return successfully if the initialization went fine.
    if (LZMA_OK == ret
        || LZMA_STREAM_END == ret)
    {
        return true;
    }

    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).
    switch (ret)
    {
        case LZMA_NO_CHECK:     /* 2 */
            msg = "Input stream has no integrity check";
            break;
        case LZMA_UNSUPPORTED_CHECK:    /* 3 */
            msg = "Cannot calculate the integrity check";
            break;
        case LZMA_GET_CHECK:    /* 4 */
            msg = "Integrity check type is now available";
            break;
        case LZMA_MEM_ERROR:    /* 5 */
            msg = "Cannot allocate memory";
            break;
        case LZMA_MEMLIMIT_ERROR:   /* 6 */
            msg = "Memory usage limit was reached";
            break;
        case LZMA_FORMAT_ERROR:     /* 7 */
            msg = "File format not recognized";
            break;
        case LZMA_OPTIONS_ERROR:    /* 8 */
            msg = "Invalid or unsupported options";
            break;
        case LZMA_DATA_ERROR:       /* 9 */
            msg = "Data is corrupt";
            break;
        case LZMA_BUF_ERROR:        /* 10 */
            msg = "No progress is possible";
            break;
        case LZMA_PROG_ERROR:       /* 11 */
            msg = "Programming error";
            break;
        default:
            // This is most likely LZMA_PROG_ERROR indicating a bug in
            // this program or in liblzma. It is inconvenient to have a
            // separate error message for errors that should be impossible
            // to occur, but knowing the error code is important for
            // debugging. That's why it is good to print the error code
            // at least when there is no good error message to show.
            msg = "Unknown error, possibly a bug";
            break;
    }

    log_notice("Error initializing the encoder: %s (error code %u)\n",
            msg, ret);
    return false;
}

/**
 * @brief 
 * 
 * @param strm 
 * @param preset 
 * @return true 
 * @return false 
 */
static bool init_encoder(lzma_stream *strm, uint32_t preset)
{
    // Initialize the encoder using a preset. Set the integrity to check
    // to CRC64, which is the default in the xz command line tool. If
    // the .xz file needs to be decompressed with XZ Embedded, use
    // LZMA_CHECK_CRC32 instead.
    lzma_ret ret = lzma_easy_encoder(strm, preset, LZMA_CHECK_CRC64);
    log_debug("ret: %d", ret);

    return get_error(ret);
}

/**
 * @brief 
 * 
 * @param strm 
 * @return true 
 * @return false 
 */
static bool init_decoder(lzma_stream *strm)
{
    // Initialize a .xz decoder. The decoder supports a memory usage limit
    // and a set of flags.
    //
    // The memory usage of the decompressor depends on the settings used
    // to compress a .xz file. It can vary from less than a megabyte to
    // a few gigabytes, but in practice (at least for now) it rarely
    // exceeds 65 MiB because that's how much memory is required to
    // decompress files created with "xz -9". Settings requiring more
    // memory take extra effort to use and don't (at least for now)
    // provide significantly better compression in most cases.
    //
    // Memory usage limit is useful if it is important that the
    // decompressor won't consume gigabytes of memory. The need
    // for limiting depends on the application. In this example,
    // no memory usage limiting is used. This is done by setting
    // the limit to UINT64_MAX.
    //
    // The .xz format allows concatenating compressed files as is:
    //
    //     echo foo | xz > foobar.xz
    //     echo bar | xz >> foobar.xz
    //
    // When decompressing normal standalone .xz files, LZMA_CONCATENATED
    // should always be used to support decompression of concatenated
    // .xz files. If LZMA_CONCATENATED isn't used, the decoder will stop
    // after the first .xz stream. This can be useful when .xz data has
    // been embedded inside another file format.
    //
    // Flags other than LZMA_CONCATENATED are supported too, and can
    // be combined with bitwise-or. See lzma/container.h
    // (src/liblzma/api/lzma/container.h in the source package or e.g.
    // /usr/include/lzma/container.h depending on the install prefix)
    // for details.
    lzma_ret ret = lzma_stream_decoder(
            strm, UINT64_MAX, LZMA_CONCATENATED);
    log_debug("ret: %d", ret);

    return get_error(ret);
}

/**
 * @brief 
 * 
 * @param strm 
 * @param in_buff 
 * @param in_size 
 * @param out_buff 
 * @param out_size 
 * @return true 
 * @return false 
 */
static bool do_lzma_code(lzma_stream *strm,
                const unsigned char *in_buff, const int in_size,
                unsigned char *out_buff, int *out_size)
{
    lzma_ret ret = 0;
    uint8_t outbuf[BUFSIZ] = { 0 };
    lzma_action action = LZMA_FINISH;

    strm->next_out = outbuf;
    strm->avail_out = sizeof(outbuf);

    strm->next_in = in_buff;
    strm->avail_in = in_size;

    ret = lzma_code(strm, action);
    log_debug("ret: %d", ret);

    *out_size = sizeof(outbuf) - strm->avail_out;
    memcpy(out_buff, outbuf, *out_size);

    return get_error(ret);
}

/**
 * @brief 
 * 
 * @param in_buff 
 * @param in_size 
 * @param out_buff 
 * @param out_size 
 * @return int 
 */
int _compress(const unsigned char *in_buff, const int in_size,
        unsigned char *out_buff, int *out_size, int compress_ratio)
{
    int ret = 0;
    bool success = true;
    uint32_t preset = get_preset();
    int true_ratio = 0;
    lzma_stream strm = LZMA_STREAM_INIT;

    log_debug("");

    success = init_encoder(&strm, preset);
    log_debug("success: %d", success);
    if (success)
    {
        success = do_lzma_code(&strm, in_buff, in_size, out_buff, out_size);
        log_debug("success: %d", success);
    }

    lzma_end(&strm);

    if (true == success)
        ret = 0;
    else
        ret = 0 - success;

    /**
     * @brief 
     * 判断压缩比例是否与预期相符
     */
    true_ratio = *out_size * 100 / in_size;
    if (true_ratio < compress_ratio)
    {
        log_info("Compress ratio may be smaller: %d%%, expect %d%%",
            true_ratio, compress_ratio);
        ret = -1;
    }

    return ret;
}

/**
 * @brief 
 * 
 * @param in_buff 
 * @param in_size 
 * @param out_buff 
 * @param out_size 
 * @return int 
 */
int _decompress(const unsigned char *in_buff, const int in_size,
        unsigned char *out_buff, int *out_size)
{
    int ret = 0;
    bool success = true;
    lzma_stream strm = LZMA_STREAM_INIT;

    log_debug("");

    if (!init_decoder(&strm))
    {
        // Decoder initialization failed. There's no point
        // to retry it so we need to exit.
        success = false;
    }

    success &= do_lzma_code(&strm, in_buff, in_size, out_buff, out_size);
    log_debug("success: %d", success);

    lzma_end(&strm);

    if (true == success)
        ret = 0;
    else
        ret = 0 - success;

    return ret;
}

/**
 * @brief 
 * 
 * @param src 
 * @param slen 
 * @param dst 
 * @param dlen 
 * @return int 
 */
int compress_lzma(void *src, int slen, void *dst, int *dlen, int compress_ratio)
{
    return _compress(src, slen, dst, dlen, compress_ratio);
}

/**
 * @brief 
 * 
 * @param src 
 * @param slen 
 * @param dst 
 * @param dlen 
 * @return int 
 */
int decompress_lzma(void *src, int slen, void *dst, int *dlen)
{
    return _decompress(src, slen, dst, dlen);
}
#else

#define IN_BUF_MAX    4096 
#define OUT_BUF_MAX   4096 
#define RET_OK        0 
#define RET_ERROR     1 

/**
 * @brief 
 * 
 * @param src 
 * @param slen 
 * @param dst 
 * @param dlen 
 * @return int 
 */
int compress_lzma(void *src, int slen, void *dst, int *dlen)
{
    // FILE *in_file, FILE *out_file;
    lzma_check check = LZMA_CHECK_CRC64;
    /* alloc and init lzma_stream struct */
    /**
     * @brief 
     * 定义 lzma_stream 结构体变量, 并使用 LZMA_STREAM_INIT 进行初始化.
     * 这个 lzma_stream 结构体变量会在整个数据压缩过程中使用, 有点类似于
     * C 标准库中文件处理使用的结构体 FILE.
     */
    lzma_stream strm = LZMA_STREAM_INIT;
    
    uint8_t in_buf [4096];
    uint8_t out_buf [4096];
    bool in_finished = false;
    int _dlen = *dlen;

    /* initialize xz encoder */ 
    /**
     * @brief 做压缩准备工作. 
     * 
     * 该函数的第一个参数是 lzma_stream 结构体变量的指针.
     * 第二个参数则指明了压缩率大小,有效值为 [0, 9], 数字越高, 压缩率越高.
     * 第三个参数是指明数据完整性检查方法, LZMA_CHECK_CRC64 可以满足大多数情况需要, 当然, 还有
     *      LZMA_CHECK_CRC32, LZMA_CHECK_SH256 可供选择, 或者使用 LZMA_CHECK_NONE 不进行数
     *      据完整性检查
     */
    lzma_easy_encoder (&strm, 9, LZMA_CHECK_CRC64); 
    while (! in_finished)
    {
        /* read incoming data */
        // size_t in_len = fread (in_buf, 1, 4096, in_file);

        // if (feof (in_file))
        // {
        //     in_finished = true;
        // }
        in_finished = true;

        /**
         * @brief 
         * 通过设定 lzma_stream 结构体变量中的 next_in 和 avail_in 字段,
         * 指明待压缩的数据开始地址和长度
         */
        // strm.next_in = in_buf;      /* input buffer address */
        // strm.avail_in = in_len;     /* data length */
        strm.next_in = src;      /* input buffer address */
        strm.avail_in = slen;     /* data length */

        /* if no more data from in_buf, flushes the internal xz buffers and
         * closes the xz data with LZMA_FINISH */
        lzma_action action = in_finished 
                                ? LZMA_FINISH   /* 结束数据处理 */
                                : LZMA_RUN;     /* 进行数据处理 */

        /* loop until there's no pending compressed output */
        do {
            /**
             * @brief 
             * 通过设定 lzma_stream 结构体变量中的 next_out 和 avail_out 字段,
             * 指明存放压缩结果 buffer 地址和长度.
             */
            // strm.next_out = out_buf;    /* output buffer address */
            // strm.avail_out = 4096;      /* output buffer length */
            strm.next_out = dst;    /* output buffer address */
            strm.avail_out = *dlen;      /* output buffer length */

            /**
             * @brief 
             * 通过调用 lzma_code() 函数来压缩数据, 或结束压缩数据.
             * lamz_code() 函数有两个参数, 第一个参数是 lzma_stream 指针, 而第二个参数用来指明
             * lzma_code() 函数的动作: LZMA_RUN - 进行数据处理, LZMA_FINISH - 结束数据处理.
             * 
             */
            lzma_code (&strm, action); /* compress data */

            // size_t out_len = 4096 - strm.avail_out;
            _dlen = *dlen - strm.avail_out;
            // fwrite (out_buf, 1, out_len, out_file); /* write compressed data */
        } while (0 == strm.avail_out);
    } 

    /* 调用 lzma_end() 函数释放资源, 退出 */
    lzma_end (&strm);
}

/**
 * @brief 
 * 
 * @param src 
 * @param slen 
 * @param dst 
 * @param dlen 
 * @return int 
 */
int decompress_lzma(void *src, int slen, void *dst, int *dlen)
{
    FILE *in_file, FILE *out_file;

    /* alloc and init lzma_stream struct */
    lzma_stream strm = LZMA_STREAM_INIT;
    uint8_t in_buf [IN_BUF_MAX];
    uint8_t out_buf [OUT_BUF_MAX];
    bool in_finished = false;
    bool out_finished = false;
    lzma_action action;
    lzma_ret ret_xz;
    int ret;

    ret = RET_OK;

    /* initialize xz decoder */
    /**
     * @brief 
     * 进行 decoder 的初始化
     * 第一个参数是 lzma_stream 结构体变量的指针;
     * 第二个参数则指明了在解压缩过程中使用的内存的最大值, 若为 UINT64_MAX 则为不限制内存使用量.
     * 第三个参数用来指明一些其他 flags 值:
     *      LZMA_TELL_NO_CHECK - 如果压缩数据流中未指明数据完整性检查方式, 则函数 lzma_code() 返回
     *          LZMA_TELL_NO_CHECK
     *      LZMA_TELL_UNSUPPORTED_CHECK - 如果压缩数据流使用了不被支持的数据完整性检查方式, 则函数
     *          lzma_code() 返回该枚举值
     *      LZMA_CONCATENATED - 支持多个压缩流连接到一个 xz 文件内
     *      
     */
    ret_xz = lzma_stream_decoder (&strm, UINT64_MAX,
                        LZMA_TELL_UNSUPPORTED_CHECK | LZMA_CONCATENATED );
    if (ret_xz != LZMA_OK)
    {
        return RET_ERROR;
    }

    while ((! in_finished)
        && (! out_finished))
    {
        /* read incoming data */
        size_t in_len = fread (in_buf, 1, IN_BUF_MAX, in_file);

        if (feof (in_file))
        {
            in_finished = true;
        }

        strm.next_in = in_buf;
        strm.avail_in = in_len;

        /* if no more data from in_buf, flushes the
           internal xz buffers and closes the decompressed data
           with LZMA_FINISH */
        action = in_finished ? LZMA_FINISH : LZMA_RUN;

        /* loop until there's no pending decompressed output */
        do {
            /* out_buf is clean at this point */
            strm.next_out = out_buf;
            strm.avail_out = OUT_BUF_MAX;

            /* decompress data */ 
            ret_xz = lzma_code (&strm, action);

            if ((ret_xz != LZMA_OK)
                && (ret_xz != LZMA_STREAM_END))
            {
                out_finished = true;
                ret = RET_ERROR;
            } else {
                /* write decompressed data */
                size_t out_len = OUT_BUF_MAX - strm.avail_out;
                fwrite (out_buf, 1, out_len, out_file);
                if (ferror (out_file))
                {
                    out_finished = true;
                    ret = RET_ERROR;
                }
            }
        } while (strm.avail_out == 0);
    } 

    lzma_end (&strm);
    return ret;
}
#endif
