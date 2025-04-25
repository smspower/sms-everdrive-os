
#include "sys.h"

void gDrawString(u8 *str, u8 x, u8 y) {

    gSetXY(x, y);
    gAppendString(str);
}

void gDrawStringCx(u8 *str, u8 y) {

    u16 x = 0;
    while (str[x] != 0)x++;
    x = (gScreenW() - x) / 2;
    gDrawString(str, x, y);
}

void gAppendNum(u32 num) {

    u16 i;
    u8 buff[11];
    u8 *str = (u8 *) & buff[10];


    *str = 0;
    if (num == 0)*--str = '0';
    for (i = 0; num != 0; i++) {
        *--str = num % 10 + '0';
        num /= 10;
    }

    gAppendString(str);

}

void gAppendHex8(u8 num) {

    u8 val;
    u8 buff[8];
    buff[2] = 0;

    val = num >> 4;
    if (val > 9)val += 7;
    buff[0] = val + '0';
    val = num & 0x0f;
    if (val > 9)val += 7;
    buff[1] = val + '0';

    gAppendString(buff);
}

void gAppendHex16(u16 num) {

    gAppendHex8(num >> 8);
    gAppendHex8(num & 0xff);
}

void gAppendHex16SW(u16 num) {

    gAppendHex8(num & 0xff);
    gAppendHex8(num >> 8);
}

void gAppendHex32(u32 num) {

    gAppendHex16(num >> 16);
    gAppendHex16(num & 0xffff);
}

void gDrawStringMl(u8 *str, u8 x, u8 y, u16 len) {

    gSetXY(x, y);
    gAppendStringMl(str, len);
}

void gDrawNum(u32 num, u8 x, u8 y) {

    gSetXY(x, y);
    gAppendNum(num);
}

void sysDrawFileSize(u32 size, u16 x, u16 y) {

    if (size < 1024) {
        gDrawNum(size, x, y);
        gAppendChar((u8) 'B');
    } else if (size < 0x100000) {
        gDrawNum(size / 1024, x, y);
        gAppendChar((u8) 'K');
    } else {
        gDrawNum(size / 0x100000, x, y);
        gAppendChar((u8) 'M');
    }
}

void gDrawCursor(u8 *str, u16 x, u16 y) {

    static u16 cursor_strobe;

    cursor_strobe++;

    if ((cursor_strobe & 15) < 8) {
        gDrawString(str, x, y);
    } else {
        gDrawString(" ", x, y);
    }

}

