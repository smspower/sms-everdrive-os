
#include "types.h"
#include "fat.h"
#include "sys.h"
#include "bios.h"
#include "error.h"
#include "disk.h"
#include "str.h"
#include "os.h"

#ifdef CART_MSED


#define OS_SAVE_FILE_SIZE 16384
#define OS_EMPTY_RAM_BYTE 0
#define SAVE_DIR "/SAVE"

#define CFG_CDM_MAP 8
#define CFG_ROM_WE_OFF 32
#define CFG_SMS_MODE 64

typedef struct {
    u8 file_name[256];
    u8 sync_name[256];
    u8 run_cfg;
    u16 crc;
} RomInfo;


void bi_update_os_usb();


void bi_set_rom_bank(u8 bank);
void bi_set_ram_bank(u8 bank);


u8 osSelectGame(FatFullRecord *rec);
u8 osStartGame();
void osSysInfo();
void osAbout();
u8 osSaveSram();
void osCLeanGameSram();
u8 osLoadSram();
u8 osSaveRamMenu(FatFullRecord *rec);
u8 osSramToFile(FatFullRecord *rec);
u8 osFileToSram(FatFullRecord *rec);
u8 osUpdateMenu(FatFullRecord *rec);
u8 osUpdate(FatFullRecord *rec);
void osSelectGameUsb();
void osUpdateUsb();

void bi_install_os();


u8 cart_3e;
extern u8 os_ver;
u16 joy;
u8 bi_card_type;
u16 ram_ptr;

#define OS_RAM_SIZE 4096
u8 os_ram[OS_RAM_SIZE];
u8 flash_erase_code[128];
u8 spi_to_rom_code[128];
u8 usb_to_rom_code[128];
RomInfo *rom_inf;

u8 osInit() {

    u8 resp;
    rom_inf = (RomInfo *) 0x8000;


    bi_init();
    sysInit();

    ram_ptr = OS_RAM_SIZE;

    gSetFont((u8 *) 0x8000 - 1024);

    gCleanScreen();
    gDrawStringCx("OS init...", 12);
    gRepaint();

    bi_set_ram_bank(1);
    if (rom_inf->crc != 0xaa55) {

        rom_inf->run_cfg = 0;
        rom_inf->crc = 0xaa55;
        rom_inf->file_name[0] = 0;
        osCLeanGameSram();

        gCleanScreen();
        gDrawStringCx("Settings reset to default", 10);
        gDrawStringCx("Press any key", 12);
        gRepaint();
        sysJoyWait();
        gCleanScreen();
        gSetXY(0, 0);
    }
    bi_set_rom_bank(1);

    resp = diskInit();
    if (resp)return resp;

    resp = fat_init();
    if (resp)return resp;

    return 0;
}

u8 osUsbListene2r() {
    return 0;
}

void osUsbListener() {

    u8 cmd;



    if (bi_usb_rd_busy())return;


    cmd = bi_usb_rd_byte();
    if (cmd != (u8) '+')return;

    cmd = bi_usb_rd_byte();

    switch (cmd) {

        case 't':
            bi_usb_wr_byte((u8) 'k');
            break;
        case 'g':
            osSelectGameUsb();
            osStartGame();
            break;
        case 'o':
            //usbWriteOS();
            osUpdateUsb();
            break;
        case 's':
            //FIFO_WR_BUSY;
            //FIFO_PORT = OS_VERSION;
            break;
        case 'f':
            //FIFO_WR_BUSY;
            //*fifo_port = FIRM_VERSION;
            break;
    }

}

u8 osRomMenu(FatFullRecord *rec) {

    u8 resp = 0;
    const u8 * rom_menu_str[] = {
        "File Menu", "Load And Start", "Load Only", "Hex View", 0
    };

    resp = guiDrawMenu(rom_menu_str, 0);

    if (resp == 0) {
        resp = osSelectGame(rec);
        if (resp)return resp;
        resp = osStartGame();
        if (resp)return resp;
    } else if (resp == 1) {
        resp = osSelectGame(rec);
        return resp;
    } else if (resp == 2) {
        resp = guiHexView(rec);
        return resp;
    }


    return 0;
}

u8 osFileMenu(FatFullRecord *rec) {

    u8 resp = 0;
    const u8 * zip_list[] = {"zip", "rar", "7z", "7z ", 0};
    const u8 * zip_menu_str[] = {
        "File Menu", "ROM file must be unzipped", 0
    };


    if (str_extension(".mso", rec->name)) {
        return osUpdateMenu(rec);
    } else if (str_extension(".txt", rec->name)) {
        return guiHexView(rec);
    } else if (str_extension_list(zip_list, rec->name)) {
        guiDrawMenu(zip_menu_str, 0);
        return 0;
    } else if (str_extension(".srm", rec->name)) {
        resp = osSaveRamMenu(rec);
    } else {
        resp = osRomMenu(rec);
    }
    return resp;
}

void *osMallocRam(u16 size) {

    if (ram_ptr < size) {
        guiPrintError(ERR_OUT_OF_RAM1);
        for (;;)osUsbListener();
    }

    ram_ptr -= size;
    return (void *) &os_ram[ram_ptr];
}

void osReleaseRam(u16 size) {

    ram_ptr += size;
    if (ram_ptr > OS_RAM_SIZE) {
        guiPrintError(ERR_OUT_OF_RAM2);
        for (;;)osUsbListener();
    }
}

void g_set_vdp_reg(u8 reg, u8 val);

u8 osMainMenu() {

    u8 selector;
    const u8 * menu_str[] = {
        "Main Menu", "Start Game", "Device Info", "About", 0
    };


    selector = 0;
    for (;;) {
        gCleanScreen();
        selector = guiDrawMenu(menu_str, selector);
        if (selector == 0)osStartGame();
        if (selector == 1)osSysInfo();
        if (selector == 2)osAbout();
        if (selector == 0xff)break;
    }


    return 0;
}

u8 osExitBrowser() {
    osMainMenu();
    return 0;
}

u16 osGetDate() {
    return 0;
}

u16 osGetTime() {
    return 0;
}

void osSysInfo() {

    u8 x = 6;
    u8 y = 7;
    u8 *inf_ptr = (u8 *) 0xbf10;

    gCleanScreen();

    gSetPal(0);

    gDrawString("CPLD Version:", x, y);
    gAppendNum(bi_get_cpld_ver());
    y += 2;

    gDrawString("OS Version:  ", x, y);
    gAppendNum(os_ver);
    y += 2;

    gDrawString("Max Dir Size:", x, y);
    gAppendNum(OS_MAX_DIR_SIZE);
    y += 2;

    gDrawString("Cart Type:   ", x, y);
    if (bi_is_gg_cart()) {
        gAppendString("GG");
    } else {
        gAppendString("SMS");
    }
    y += 2;

    bi_set_rom_bank(0);
    *(u8 *) 0xffff = 3;
    gDrawString("Asm date: ", x, y);
    gAppendString(inf_ptr);
    y += 2;
    bi_set_rom_bank(1);

    gRepaint();
    sysJoyWait();
}

void osAbout() {

    u8 gg_cart = bi_is_gg_cart();

    gCleanScreen();
    gSetXY(0, 2);
    gSetPal(4);
    gConsPrint("");
    if (gg_cart) {
        gConsPrint("EverDrive GG ");
        gConsPrint("Developed by:");
        gSetPal(0);
        gConsPrint("I. Golubovskiy");
    } else {
        gConsPrint("Master EverDrive Flash Cart");
        gConsPrint("Developed by I. Golubovskiy");
    }


    gSetPal(4);
    gConsPrint("");

    gConsPrint("Support:");
    gSetPal(0);
    gConsPrint("biokrik@gmail.com");
    gConsPrint("http://krikzz.com");

    gConsPrint("");
    gSetPal(4);
    gConsPrint("Control:");
    gSetPal(0);
    if (gg_cart) {
        gConsPrint("LEF/RIG-switch page");
    } else {
        gConsPrint("LEFT/RIGHT-switch page");
    }
    gConsPrint("1-file menu");
    gConsPrint("2-back");
    gConsPrint("2 in root-main menu");


    gDrawString("SN:", 2, 23);

    gRepaint();

    sysJoyWait();
}

/*
CFG_ROM_REGS_ON = 1
CFG_SPI_SS = 2
CFG_SPI_FULL_SPEED = 4
CFG_CDM_MAP = 8
CFG_AUTO_BUSY = 16
CFG_ROM_WE_OFF = 32
CFG_SMS_MODE = 64*/

#define REG_CFG *((u8 *)0xfff8)

u8 osStartGame() {

    u8 cfg;
    g_set_vdp_reg(1, 0);

    if ((rom_inf->run_cfg & CFG_SMS_MODE)) {
        REG_CFG |= CFG_SMS_MODE;
    }

    if (bi_is_gg_cart()) {
        gSetSGpal();
    }



    bi_set_ram_bank(1);
    cfg = CFG_ROM_WE_OFF | rom_inf->run_cfg;
    bi_set_rom_bank(1);



    bi_start_game(cfg);
    return 0;
}

void osSetRomCfg(u8 *name) {

    volatile u8 norm_size;
    volatile u8 codemasters = 1;
    volatile u8 *rom_ptr = (u8*) 0xbfe9;
    u8 i;
    u8 run_cfg;

    run_cfg = 0;

    *(u8 *) 0xffff = 1;
    bi_set_rom_bank(1);
    norm_size = *((u8 *) 0xbfff);
    norm_size &= 0xf;



    if (norm_size == 0x0c)norm_size = 2;
    else
        if (norm_size == 0x0e)norm_size = 4;
    else
        if (norm_size == 0x0f)norm_size = 8;
    else
        if (norm_size == 0x00)norm_size = 16;
    else
        if (norm_size == 0x01)norm_size = 32;
    else
        norm_size = 64;

    if (norm_size != *((u8 *) 0xbfe0) && norm_size != 2)codemasters = 0;

    if (*rom_ptr++ == 0)codemasters = 0;
    for (i = 0; i < 6; i++) {
        if (*rom_ptr++ != 0)codemasters = 0;
    }

    if (*((u8 *) 0xbfe0) < 8 || *((u8 *) 0xbfe0) > 32)codemasters = 0;


    if (str_extension(".gg", name) == 0) {
        run_cfg |= CFG_SMS_MODE;
    }


    if (codemasters) {
        run_cfg |= CFG_CDM_MAP;
    }

    bi_set_ram_bank(1);
    rom_inf->run_cfg = run_cfg;
    bi_set_rom_bank(1);

    /*
    if ((run_cfg & CFG_CDM_MAP)) {
        gConsPrint("CDM MAPPER");
    }

    if ((run_cfg & CFG_SMS_MODE)) {
        gConsPrint("SMS MODE");
    } else {
        gConsPrint("GG MODE");
    }*/

}

u8 osSelectGame(FatFullRecord *rec) {

    u8 resp;
    u16 addr;
    u8 slen;
    u8 err_len;

    if (rec->size > 0x100000)return ERR_ROM_SIZE;

    gCleanScreen();
    gSetXY(1, 1);
    gSetPal(0);
    gConsPrint("");
    gConsPrint("");
    if (bi_is_gg_cart()) {
        gConsPrint("!!DO NOT TURN OFF!!");
        gSetPal(0);
        gConsPrint("-------------------");
    } else {
        gConsPrint("!DO NOT TURN OFF THE SYSTEM!");
        gSetPal(0);
        gConsPrint("----------------------------");
    }
    gRepaint();



    resp = osSaveSram();
    if (resp)return resp;

    resp = fat_open_file(rec, 0);
    if (resp)return resp;

    if ((file.sec_available & 1) != 0) {
        resp = fat_skip_sectors(1);
        if (resp)return resp;
    }

    err_len = file.sec_available / 128;
    if ((file.sec_available & 127) != 0)err_len++;

    gConsPrint("Erase...");
    gRepaint();
    bi_flash_erase(0, err_len);

    gConsPrint("Write.");
    gRepaint();
    addr = 0;
    while (file.sec_available) {
        slen = file.sec_available > 128 ? 128 : file.sec_available;
        resp = fat_read((u8*) & addr, slen, ROM);
        if (resp)return resp;
        gAppendChar((u8) '.');
        gRepaint();
    }

    bi_set_ram_bank(1);
    str_copy(rec->name, rom_inf->file_name);
    bi_set_rom_bank(1);
    resp = osLoadSram();
    if (resp)return resp;

    osSetRomCfg(rec->name);



    //gConsPrint("ok");
    gRepaint();


    return 0;
}

u8 _osSaveSram(FatFullRecord *rec) {

    u16 resp;
    u16 i;
    u8 *sram_ptr = (u8 *) 0x8000;

    bi_set_ram_bank(1);
    if (rom_inf->file_name[0] == 0)return 0;

    bi_set_ram_bank(0);
    for (i = 0; i < OS_SAVE_FILE_SIZE; i++) {
        if (*sram_ptr++ != OS_EMPTY_RAM_BYTE)break;
    }
    sram_ptr = (u8 *) 0x8000;

    if (i == OS_SAVE_FILE_SIZE)return 0;

    gConsPrint("Backup save RAM...");
    gRepaint();

    bi_set_ram_bank(1);
    fat_make_sync_name(SAVE_DIR, rom_inf->file_name, "srm", rom_inf->sync_name);


    resp = fat_open_file_by_name(rom_inf->sync_name, rec, OS_SAVE_FILE_SIZE / 512);

    if (resp == FAT_ERR_PATH_NOT_EXIST) {
        resp = fat_make_dir(rec, SAVE_DIR);
        if (resp)return resp;
        resp = fat_open_file_by_name(rom_inf->sync_name, rec, OS_SAVE_FILE_SIZE / 512);
    }
    if (resp)return resp;


    bi_set_ram_bank(0);
    resp = fat_write_file(sram_ptr, OS_SAVE_FILE_SIZE / 512);
    if (resp)return resp;


    return 0;
}

u8 osSaveSram() {

    u8 resp;
    FatFullRecord *rec = (FatFullRecord *) osMallocRam(sizeof (FatFullRecord));
    resp = _osSaveSram(rec);
    osReleaseRam(sizeof (FatFullRecord));
    bi_set_rom_bank(1);

    return resp;
}

u8 _osLoadSram(FatFullRecord *rec) {

    u16 resp;
    u32 size;

    osCLeanGameSram();


    bi_set_ram_bank(1);
    fat_make_sync_name(SAVE_DIR, rom_inf->file_name, "srm", rom_inf->sync_name);


    resp = fat_open_file_by_name(rom_inf->sync_name, rec, 0);
    if (resp == FAT_ERR_NOT_EXIST)return 0;
    if (resp)return resp;

    gConsPrint("Load save RAM...");
    gRepaint();

    size = OS_SAVE_FILE_SIZE / 512;
    if (size > file.sec_available)size = file.sec_available;

    bi_set_ram_bank(0);
    resp = fat_read((void *) 0x8000, size, RAM);
    if (resp)return resp;

    return 0;
}

u8 osLoadSram() {

    u8 resp;
    FatFullRecord *rec = (FatFullRecord *) osMallocRam(sizeof (FatFullRecord));
    resp = _osLoadSram(rec);
    osReleaseRam(sizeof (FatFullRecord));
    bi_set_rom_bank(1);
    return resp;
}

void osCLeanGameSram() {

    u16 i;
    u8 *sram_ptr = (u8 *) 0x8000;

    bi_set_ram_bank(0);
    for (i = 0; i < OS_SAVE_FILE_SIZE; i++)*sram_ptr++ = OS_EMPTY_RAM_BYTE;
    bi_set_rom_bank(1);
}

u8 osSaveRamMenu(FatFullRecord *rec) {

    u8 resp;
    const u8 * sram_menu_str[] = {
        "File Menu", "Cancel", "Copy File To SRAM", "Copy SRAM To File", "Hex View", 0
    };
    resp = guiDrawMenu(sram_menu_str, 0);

    if (resp == 0) {
        return 0;
    } else if (resp == 1) {
        resp = osFileToSram(rec);
        return resp;
    } else if (resp == 2) {
        resp = osSramToFile(rec);
        return resp;
    } else if (resp == 3) {
        resp = guiHexView(rec);
        return resp;
    }
    return 0;
}

u8 osFileToSram(FatFullRecord *rec) {

    u16 resp;
    u16 len;

    resp = fat_open_file(rec, 0);
    if (resp)return resp;

    len = rec->size / 512;
    if (rec->size % 512 != 0)len++;
    if (len > OS_SAVE_FILE_SIZE / 512)len = OS_SAVE_FILE_SIZE / 512;

    bi_set_ram_bank(0);
    resp = fat_read((u8 *) 0x8000, len, RAM);
    bi_set_rom_bank(1);
    if (resp)return resp;

    return 0;
}

u8 osSramToFile(FatFullRecord *rec) {

    u16 resp;
    u16 len;


    if (rec->size < OS_SAVE_FILE_SIZE) {
        len = rec->size / 512;
        if (rec->size % 512 != 0)len++;
    } else {
        len = OS_SAVE_FILE_SIZE / 512;
    }

    resp = fat_open_file(rec, len);
    if (resp)return resp;

    bi_set_ram_bank(0);
    resp = fat_write_file((u8 *) 0x8000, len);
    bi_set_rom_bank(1);
    if (resp)return resp;

    return 0;
}

u8 osUpdateMenu(FatFullRecord *rec) {

    u8 resp;
    const u8 * menu_str[] = {
        "File Menu", "Cancel", "Update OS", 0
    };
    resp = guiDrawMenu(menu_str, 0);

    if (resp == 1)return osUpdate(rec);

    return 0;
}

u8 osUpdate(FatFullRecord *rec) {

    u8 resp;
    u16 addr;
    u8 *buff;
    u8 edos;

    gCleanScreen();
    gSetPal(0);
    gDrawStringCx("!DO NOT TURN OFF THE SYSTEM!", 11);
    gRepaint();

    if (rec->size != 0x8000)return ERR_OS_SIZE;

    resp = fat_open_file(rec, 0);
    if (resp)return resp;

    buff = osMallocRam(512);
    resp = fat_read(buff, 1, RAM);
    edos = str_cmp_len(&buff[0x80], "EDOS", 4);
    osReleaseRam(512);
    if (resp)return resp;
    if (!edos)return ERR_OS_FILE;

    resp = fat_open_file(rec, 0);
    if (resp)return resp;

    bi_flash_erase(0x10000, 1);

    addr = 128;
    resp = fat_read((u8*) & addr, 64, ROM);
    if (resp)return resp;

    bi_install_os();


    return 0;
}

void osSelectGameUsb() {

    u8 wr_len;
    u16 addr;

    gSetPal(0);
    gCleanScreen();
    gSetXY(0, 2);
    gConsPrint("USB loading...");
    gRepaint();
    wr_len = bi_usb_rd_byte();
    if (wr_len > 16 || wr_len == 0)return;

    //osSaveSram();


    gConsPrint("Flash Erase...");
    gRepaint();

    bi_flash_erase(0, wr_len);
    bi_usb_wr_byte((u8) 'k');

    gConsPrint("Copy ROM.");
    gRepaint();

    addr = 0;
    while (wr_len) {

        bi_usb_to_rom(&addr, 64);
        bi_usb_wr_byte((u8) 'k');
        addr += 64;
        bi_usb_to_rom(&addr, 64);
        bi_usb_wr_byte((u8) 'k');
        addr += 64;

        wr_len--;
        gAppendChar((u8) '.');
        gRepaint();
    }

    bi_set_ram_bank(1);
    str_copy("usb-game-savedata.sms", rom_inf->file_name);
    rom_inf->run_cfg = 0;
    bi_set_rom_bank(1);
    osLoadSram();


    /*
        vdpDrawConsStr("game upload", PAL1);
        vdp_repaint();
        FIFO_RD_BUSY;
        r_write_len = FIFO_PORT;
        if (r_write_len > 16) {
            FIFO_WR_BUSY;
            FIFO_PORT = 'e';
        }
        vdpDrawConsStr("erase...", PAL1);
        vdp_repaint();
        eprErase(0x100000, r_write_len);
        FIFO_WR_BUSY;
        FIFO_PORT = 'k';
        vdpDrawConsStr("write...", PAL1);
        vdp_repaint();
        r_usb_game_load();
        startGame();*/
}

void osUpdateUsb() {

    u16 addr;
    gSetPal(0);
    gCleanScreen();
    gSetXY(0, 2);
    gConsPrint("USB loading...");
    gRepaint();


    bi_flash_erase(0x10000, 1);
    bi_usb_wr_byte((u8) 'k');
    addr = 128;
    bi_usb_to_rom(&addr, 64);
    bi_usb_wr_byte((u8) 'k');
    bi_install_os();
}

#endif
