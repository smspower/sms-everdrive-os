
#include "types.h"
#include "str.h"
#include "sys.h"

u8 str_cmp_len(u8 *str1, u8 *str2, u8 len) {

    while (len--)if (*str1++ != *str2++)return 0;

    return 1;
}

u8 str_length(u8 *str) {

    u8 len = 0;
    while (*str++ != 0)len++;
    return len;
}

u8 str_extension(u8 *target, u8 *str) {

    u8 str_len;
    u8 targ_len;

    if (*target == (u8) '.')target++;
    str_len = str_length(str);
    targ_len = str_length(target);
    if (str_len < targ_len)return 0;


    return str_eq_ncase(target, &str[str_len - targ_len]);

}

u8 str_extension_list(u8 **ext_list, u8 *name) {

    while (*ext_list != 0) {
        if (str_extension(*ext_list, name))return 1;
        ext_list++;
    }

    return 0;
}

u8 str_eq_ncase(u8 *str1, u8 *str2) {

    u8 val1;
    u8 val2;

    for (;;) {

        if (*str1 == 0 && *str2 == 0)return 1;
        val1 = *str1++;
        val2 = *str2++;
        if (val1 >= (u8) 'A' && val1 <= (u8) 'Z')val1 |= 0x20;
        if (val2 >= (u8) 'A' && val2 <= (u8) 'Z')val2 |= 0x20;
        if (val1 != val2)return 0;
    }

}

void str_copy(u8 *src, u8 *dst) {

    while (*src != 0)*dst++ = *src++;
    *dst = 0;

}

void str_append(u8 *dst, u8 *src) {

    while (*dst != 0)dst++;
    while (*src != 0)*dst++ = *src++;
    *dst = 0;
}

void str_append_hex8(u8 num, u8 *dst) {

    u8 val;
    u8 str_len;

    str_len = str_length(dst);
    dst = (u8 *) & dst[str_len];
    dst[2] = 0;

    val = num >> 4;
    if (val > 9)val += 7;
    dst[0] = val + '0';
    val = num & 0x0f;
    if (val > 9)val += 7;
    dst[1] = val + '0';

}

void str_append_hex16(u16 num, u8 *dst) {

    str_append_hex8(num >> 8, dst);
    str_append_hex8(num & 0xff, dst);
}

#ifndef SYS_GBX

void str_append_hex32(u32 num, u8 *dst) {

    str_append_hex16(num >> 16, dst);
    str_append_hex16(num & 0xffff, dst);
}



u8 str_contains(u8 *target, u8 *str) {

    u16 targ_len = str_length(target);
    u16 eq_len;


    for (eq_len = 0; eq_len < targ_len;) {

        if (*str == 0)return 0;
        if (*str++ == target[eq_len]) {
            eq_len++;
        } else {
            eq_len = 0;
        }
    }

    if (eq_len != targ_len)return 0;

    return 1;

}


void str_append_num(u32 num, u8 *dst) {

    u16 i;
    u8 buff[11]; 
    u8 *str = (u8 *) & buff[10];


    *str = 0;
    if (num == 0)*--str = '0';
    for (i = 0; num != 0; i++) {
        *--str = num % 10 + '0';
        num /= 10;
    }

    str_append(dst, str);

}

#endif
