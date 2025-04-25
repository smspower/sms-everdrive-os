/* 
 * File:   str.h
 * Author: krik
 *
 * Created on November 11, 2013, 10:59 PM
 */

#ifndef STR_H
#define	STR_H

#include "types.h"

u8 str_cmp_len(u8 *str1, u8 *str2, u8 len);
u8 str_length(u8 *str);
u8 str_extension(u8 *target, u8 *str);
u8 str_eq_ncase(u8 *str1, u8 *str2);
u8 str_contains(u8 *target, u8 *str);
u8 str_extension_list(u8 **ext_list, u8 *name);
void str_copy(u8 *src, u8 *dst);
void str_append(u8 *dst, u8 *src);
void str_append_num(u32 num, u8 *dst);
void str_append_hex8(u8 num, u8 *dst);
void str_append_hex16(u16 num, u8 *dst);
void str_append_hex32(u32 num, u8 *dst);
 
#endif	/* STR_H */

