
#include "types.h"
#include "fat.h"


#include "os.h"
#include "sys.h"
#include "str.h"
#include "disk.h"
#include "bios.h"
#include "error.h"

#ifdef SYS_SMS


#define G_SCREEN_W 32
#define G_SCREEN_H 28
u8 SYS_BR_ROWS;

#define VDP_REG_MODE1 0
#define VDP_REG_MODE2 1
#define VDP_REG_NTAB 2
#define VDP_REG_CTAB 3
#define VDP_REG_BG_PAT 4
#define VDP_REG_SPRITE_ATTR 5
#define VDP_REG_SPRITE_PAT 6
#define VDP_REG_BG_COLOR 7
#define VDP_REG_SCROLL_X 8
#define VDP_REG_SCROLL_Y 9
#define VDP_REG_LINE_CTR 10

/*
 D7 - 1= Disable vertical scrolling for columns 24-31
 D6 - 1= Disable horizontal scrolling for rows 0-1  
 D5 - 1= Mask column 0 with overscan color from register #7
 D4 - (IE1) 1= Line interrupt enable
 D3 - (EC) 1= Shift sprites left by 8 pixels
 D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
 D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4. Otherwise has no effect.
 D0 - 1= No sync, display is monochrome, 0= Normal display*/

#define VDP_MOD1_NO_SYNC 1
#define VDP_MOD1_M2 2
#define VDP_MOD1_M4 4
#define VDP_MOD1_ED 8
#define VDP_MOD1_LE 16
#define VDP_MOD1_MC_OVR 32
#define VDP_MOD1_HS_OFF 64
#define VDP_MOD1_VS_OFF 128

/*
 D7 - No effect
 D6 - (BLK) 1= Display visible, 0= display blanked.
 D5 - (IE0) 1= Frame interrupt enable.
 D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
 D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
 D2 - No effect
 D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
 D0 - Sprite pixels are doubled in size.*/

#define VDP_MOD2_SPR_DP 1
#define VDP_MOD2_SPR_16 2
#define VDP_MOD2_M3 8
#define VDP_MOD2_M1 16
#define VDP_MOD2_FE 32
#define VDP_MOD2_BLK 64

u16 g_ptr;
u16 g_current_pal;
u8 g_cons_border;

void g_set_pal(void *ptr, u8 offset, u8 len);
void g_vram_wr(void *src, u16 dst, u16 len);
void g_set_vdp_reg(u8 reg, u8 val);
void g_vdp_init();
u8 sys_joy_read();



u16 gfx_buff[G_SCREEN_W * G_SCREEN_H];
//extern u8 pal_sms[];

#define CL_BL 0x00
#define CL_GL 0x15
#define CL_GD 0x10
#define CL_WT 0x3F
#define CL_YL 0x0A
#define CL_PI 0x33

#define GL_BL 0x0000
#define GL_GL 0x0666
#define GL_GD 0x0300
#define GL_WT 0x0fff
#define GL_YL 0x00cc
#define GL_PI 0x0f0f


const u8 pal_sms[] = {
    CL_BL, CL_WT, CL_BL, CL_BL, CL_BL, CL_BL, CL_BL, CL_BL,
    CL_BL, CL_GL, CL_BL, CL_YL, CL_BL, CL_BL, CL_BL, CL_BL,
    CL_GL, CL_WT, CL_BL, CL_BL, CL_BL, CL_BL, CL_BL, CL_BL,
    CL_BL, CL_BL, CL_BL, CL_YL, CL_BL, CL_BL, CL_BL, CL_BL
};

const u16 pal_gg[] = {
    GL_BL, GL_WT, GL_BL, GL_BL, GL_BL, GL_BL, GL_BL, GL_BL,
    GL_BL, GL_GL, GL_BL, GL_YL, GL_BL, GL_BL, GL_BL, GL_BL,
    GL_GL, GL_WT, GL_BL, GL_BL, GL_BL, GL_BL, GL_BL, GL_BL,
    GL_BL, GL_BL, GL_BL, GL_YL, GL_BL, GL_BL, GL_BL, GL_BL
};


const u8 pal_sg[64] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x0C, 0x20, 0x30, 0x02, 0x3C, 0x02, 0x03, 0x0A, 0x0F, 0x08, 0x33, 0x2A, 0x3F,
};

void sysInit() {

    g_current_pal = 0;
    g_vdp_init();


    if (bi_is_gg_cart()) {
        SYS_BR_ROWS = 13;
        g_cons_border = 6;
        g_set_pal(pal_gg, 0, 64);
    } else {
        g_cons_border = 2;
        SYS_BR_ROWS = 18;
        g_set_pal(pal_sms, 0, 32);

    }
    gCleanScreen();
    gRepaint();

    g_set_vdp_reg(VDP_REG_SPRITE_PAT, 0);
    g_set_vdp_reg(VDP_REG_MODE2, VDP_MOD2_BLK);

}

void gSetSGpal() {

    g_set_pal(pal_sg, 0, 64);
}

void sysJoyWait() {

    while (sysJoyRead() != 0);
    while (sysJoyRead() == 0);

}

#define JOY_DELAY 16

u16 sysJoyRead() {

    /*joy = sys_joy_read();
    osUsbListener();

    return joy;*/

    static u8 joy_ctr;


    osUsbListener();
    if (joy_ctr < JOY_DELAY - 2)gVsync();


    joy = sys_joy_read();


    if (joy != 0 && (joy & (JOY_L | JOY_R | JOY_U | JOY_D)) != 0) {
        joy_ctr++;
    } else {
        joy_ctr = 0;
    }
    if (joy_ctr > JOY_DELAY) {
        joy_ctr -= 2;
        joy = 0;
    }
    //if (joy_ctr < JOY_DELAY - 2 && joy_ctr > 2)gVsync();




    return joy;
}

void gSetFont(u8 *ptr) {

    u8 buff[64];
    u16 i;
    u8 u;
    u8 *ptr2;

    g_set_vdp_reg(VDP_REG_MODE2, 0);

    for (i = 0; i < 64; i++)buff[i] = 0;
    g_vram_wr(buff, 0, 64);


    ptr2 = ptr;
    for (i = 0; i < 4096; i += 64) {
        for (u = 0; u < 64; u += 4) {
            buff[u] = *ptr2++;
            buff[u + 3] = 0;
            buff[u + 1] = 0;
        }
        g_vram_wr(buff, i + 1024, 64);


        for (u = 0; u < 64; u += 4)buff[u + 3] = buff[u];
        g_vram_wr(buff, i + 1024 + 4096, 64);

        for (u = 0; u < 64; u += 4)buff[u + 1] = buff[u];
        g_vram_wr(buff, i + 1024 + 8192, 64);


    }

    g_set_vdp_reg(VDP_REG_MODE2, VDP_MOD2_BLK);

}

void gCleanScreen() {

    sysMemSet(gfx_buff, 0, G_SCREEN_W * G_SCREEN_H * 2);
}

void gFillRect(u16 data, u8 x, u8 y, u8 w, u8 h) {

    //u8 dat8 = data;
    u8 xx;
    u8 yy;
    u16 *ptr;

    gSetXY(x, y);

    for (yy = 0; yy < h; yy++) {
        ptr = (u16 *) & gfx_buff[g_ptr];
        for (xx = 0; xx < w; xx++) {
            *ptr++ = data | g_current_pal;
        }
        g_ptr += G_SCREEN_W;
    }
}

void gAppendString(u8 *str) {

    u16 *ptr = (u16 *) & gfx_buff[g_ptr];
    u8 str_len = 0;

    while (*str != 0) {
        *ptr++ = *str++ | g_current_pal;
        str_len++;
    }
    g_ptr += str_len;
}

void gConsPrint(u8 *str) {

    g_ptr = g_ptr / G_SCREEN_W * G_SCREEN_W + G_SCREEN_W;
    g_ptr += g_cons_border;
    gAppendString(str);
}

void gSetXY(u8 x, u8 y) {

    g_ptr = x + y * G_SCREEN_W;
}

void gMoveXY(s8 x, s8 y) {

    g_ptr += x + y * G_SCREEN_W;
}

void gAppendChar(u8 str) {

    gfx_buff[g_ptr++] = str | (g_current_pal << 8);
}

void gCopyActiveToRam(u16 *buff) {
    buff;
}

void gCopyRamToBack(u16 *buff) {
    buff;
}

void guiPrintError(u8 code) {

    gCleanScreen();
    gDrawStringCx("ERROR: 0x00", 10);
    gMoveXY(-2, 0);
    gAppendHex8(code);
    if (code == FAT_ERR_FAT16) {
        gDrawStringCx("FAT16 is not supported", 12);
        gDrawStringCx("Please use FAT32", 14);
    }

    gRepaint();
    sysJoyWait();
}

u8 guiDrawBrowser(u8 full_repaint, u16 selector) {

    FatFullRecord *rec;
    u8 gg_cart = bi_is_gg_cart();
    u16 i;
    u16 page = selector / SYS_BR_ROWS * SYS_BR_ROWS;
    static u16 old_selector;
    u8 resp;
    u8 y = gg_cart ? 3 : 1;
    u8 x = gg_cart ? 6 : 1;
    u8 ful_name_y = y + SYS_BR_ROWS + 3;
    u8 scr_w = gScreenW();
    u8 max_name_len = gg_cart ? 20 : scr_w - x * 2;
    u8 pal;

    rec = osMallocRam(sizeof (FatFullRecord));
    //u8 scr_h = gGetScreenh();

    if (full_repaint) {
        gCleanScreen();
    }

    //for(;;);
    gSetPal(1);
    if (full_repaint) {
        gFillRect(0, 0, y, scr_w, 1);
    }

    gFillRect(0, 0, ful_name_y, scr_w, 2);


    gDrawString("page: ", x, y);
    gAppendNum(page / SYS_BR_ROWS + 1);
    gAppendString(" of ");
    i = fat_dir_size / SYS_BR_ROWS;
    if (fat_dir_size % SYS_BR_ROWS != 0)i++;
    gAppendNum(i);
    y += 2;

    resp = 0;
    if (fat_dir_size == 0) {
        gSetPal(0);
        gDrawStringCx("There are no files", 11);
    }
    if (full_repaint && fat_dir_size > 4)gRepaint();

    for (i = page; i < page + SYS_BR_ROWS && i < fat_dir_size; i++, y++) {

        if (i != selector && i != old_selector && !full_repaint)continue;
        resp = fat_get_full_record(fat_dir[i], rec);
        if (resp)break;


        if (selector == i) {

            pal = rec->is_dir ? 5 : 1;
            gSetPal(pal);
            gDrawStringMl(rec->name, x, ful_name_y, max_name_len);

            if (str_length(rec->name) > max_name_len) {
                gDrawStringMl(&rec->name[max_name_len], x, ful_name_y + 1, max_name_len);
            }
            gSetPal(0);
        } else {

            pal = rec->is_dir ? 4 : 2;
            gSetPal(pal);
        }


        gDrawStringMl(rec->name, x, y, max_name_len);

    }



    osReleaseRam(sizeof (FatFullRecord));
    old_selector = selector;
    gRepaint();

    return resp;
}

u8 guiDrawMenu(u8 *str[], u16 def_select) {

    u16 i;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
    u16 selector = def_select;
    u16 items = 0;

    while (str[items] != 0)items++;
    items--;

    w = 0;
    for (i = 0; i < items + 1; i++) {

        h = str_length(str[i]);
        if (h > w)w = h;
    }

    h = items * 2 + 1;
    x = (gScreenW() - w) / 2;
    y = (gScreenH() - h) / 2 - 1;

    guiDrawForm(x, y, w, h);


    gSetPal(3);
    gDrawStringCx(str[0], y - 1);

    for (;;) {

        for (i = 0; i < items; i++) {

            if (selector == i) {
                gSetPal(1);
            } else {
                gSetPal(5);
            }
            gDrawStringCx(str[i + 1], i * 2 + 2 + y);
        }

        gRepaint();

        sysJoyWait();

        if (joy & JOY_U) {
            selector = selector == 0 ? items - 1 : selector - 1;
        }

        if (joy & JOY_D) {
            selector = selector == items - 1 ? 0 : selector + 1;
        }

        if ((joy & JOY_A)) {
            return 0xff;
        }

        if ((joy & JOY_B)) {
            return selector;
        }
    }


}

void guiDrawForm(u16 x, u16 y, u16 w, u16 h) {

    gSetPal(1);
    gFillRect(2, x - 1, y - 1, w + 2, h + 2);
    // gFillRect(1, x - 1, y, w + 2, 1);

    gFillRect('-', x - 1, y, w + 2, 1);

}

u8 guiHexView(FatFullRecord *rec) {

    u8 buff[512];
    u8 str_buff[33];
    u16 resp;
    u32 sector;
    u32 ptr = 0;

    u16 *ptr16;
    u8 *ptr8;
    u32 size;
    u16 i;
    u16 x = 0;
    u16 y = 2;
    u32 old_sec = 0;
    u16 repaint = 1;
    u16 u;
    u16 timer = 0;
    //u16 key_lock = 0;
    //u16 txt_y = y + 17;

    gCleanScreen();
    gSetPal(0);



    sector = fat_cluster_to_sector(rec->data_clsut);
    size = rec->size;
    str_buff[32] = 0;

    if (size % 512 != 0)size = size / 512 * 512 + 512;

    for (;;) {

        if (old_sec != sector + ptr / 512) {
            old_sec = sector + ptr / 512;
            resp = diskReadToRam(old_sec, buff, 1);
            //resp = diskRead(old_sec, buff, 1);
            if (resp)return resp;
        }


        if (repaint) {
            //gLockRepaint(1);
            gSetPal(4);
            gDrawString("Offset: ", x, y - 1);
            gAppendHex32(ptr);

            ptr16 = (u16*) & buff[ptr % 512];
            for (i = 0; i < 16; i++) {

                //gSetPal(2);
                // gDrawHex16(ptr + i * 16, x, y + i);
                // gAppendString(":");
                //gDrawString("", i * 16, y + i);
                gSetXY(x, y + i);
                for (u = 0; u < 4; u++) {
                    gSetPal(2);
                    gAppendHex16SW(*ptr16++);
                    gSetPal(0);
                    gAppendHex16SW(*ptr16++);
                }



            }
            gSetPal(0);
            gDrawString("--------------------------------", x, y + 16);

            ptr8 = (u8*) & buff[ptr % 512];
            for (i = 0; i < 8; i++) {

                gSetPal(0);
                for (u = 0; u < 32; u++) {
                    str_buff[u] = *ptr8++;
                    if (str_buff[u] == 0)str_buff[u] = ' ';
                }

                gDrawString(str_buff, x, y + i + 17);

            }

            repaint = 0;

        }



        gRepaint();
        //osJoyWait();
        sysJoyRead();
        while (joy != 0 && timer < 40) {
            timer++;
            sysJoyRead();
        }
        while (joy == 0) {
            timer = 0;
            sysJoyRead();
        }

        if (timer >= 20)timer -= 3;

        if ((joy & JOY_A))return 0;

        if ((joy & JOY_U)) {
            ptr = ptr == 0 ? size - 256 : ptr - 256;
            repaint = 1;
        }

        if ((joy & JOY_D)) {
            ptr = ptr + 256 >= size ? 0 : ptr + 256;
            repaint = 1;
        }

    }



}

u8 guiGetMaxRows() {
    return SYS_BR_ROWS;
}

u8 sysGetRomRegion(u8 *rom_hdr) {
    rom_hdr;
    return 0;
}

u8 gScreenW() {
    return G_SCREEN_W;
}

u8 gScreenH() {
    return G_SCREEN_H;
}

void gSetPal(u16 pal) {

    g_current_pal = (pal & 1) << 11 | (pal & 6) << 6;

}

void gAppendStringMl(u8 *str, u16 len) {


    u16 *ptr = (u16 *) & gfx_buff[g_ptr];
    u8 str_len = 0;

    while (*str != 0 && len != 0) {
        *ptr++ = *str++ | g_current_pal;
        str_len++;
        len--;
    }
    g_ptr += str_len;
}

#endif
