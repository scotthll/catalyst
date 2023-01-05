/*
 * @Description: 
 * @Author: Huang Leilei
 * @Verion: v0.0.1
 * @Date: 2023-01-04 11:07:26
 * @LastEditTime: 2023-01-05 13:51:34
 * @LastEditors: Huang Leilei
 * @FilePath: \catalyst\compress\xz.h
 */
#ifndef __COMP_LZMA_H__
#define __COMP_LZMA_H__

#ifdef __cplusplus
extern "C" {
#endif

int compress_lzma(void *src, int slen, void *dst, int *dlen, int compress_ratio);
int decompress_lzma(void *src, int slen, void *dst, int *dlen);

#ifdef __cplusplus
}
#endif

#endif
