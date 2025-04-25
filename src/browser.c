
#include "types.h"
#include "os.h"
#include "fat.h"
#include "sys.h"

int main();

#define MAX_SEL_STACK_DEEP 32
FatFullRecord br_rec;
u16 sel_stack[MAX_SEL_STACK_DEEP];

u8 browser() {

    u16 sel_stack_ptr;
    u8 resp;
    u16 selector;
    u16 page;
    u16 sub_sel;
    u8 rows;
    u8 full_repaint;
    u32 current_dir;



    selector = 0;
    rows = guiGetMaxRows();
    full_repaint = 2;
    current_dir = 0;
    sel_stack_ptr = 0;

    for (;;) {

        if (full_repaint == 2) {
            resp = fat_load_dir(current_dir);
            if (resp)return resp;
        }

        resp = guiDrawBrowser(full_repaint, selector);
        if (resp)return resp;
        full_repaint = 0;
        sysJoyWait();

        if ((joy & JOY_U) == JOY_U) {
            page = selector / rows * rows;
            sub_sel = selector - page;
            sub_sel = sub_sel == 0 ? rows - 1 : sub_sel - 1;
            selector = page + sub_sel;
            if (selector >= fat_dir_size && fat_dir_size > 0)selector = fat_dir_size - 1;
            continue;
        }

        if ((joy & JOY_D) == JOY_D) {
            page = selector / rows * rows;
            sub_sel = selector - page;
            sub_sel++;
            if (sub_sel >= rows)sub_sel = 0;
            selector = page + sub_sel;
            if (selector >= fat_dir_size)selector = page;
            continue;
        }

        if ((joy & JOY_R) == JOY_R) {
            if (fat_dir_size <= rows)continue;
            page = selector / rows * rows;
            sub_sel = selector - page;
            page += rows;
            if (page >= fat_dir_size)page = 0;
            selector = page + sub_sel;
            if (selector >= fat_dir_size)selector = fat_dir_size - 1;
            full_repaint = 1;
        }

        

        /*left*/
        if ((joy & JOY_L) == JOY_L) {
            if (fat_dir_size <= rows)continue;
            page = selector / rows * rows;
            sub_sel = selector - page;
            if (selector < rows) {
                selector = fat_dir_size / rows * rows + sub_sel;
                if (selector >= fat_dir_size) selector = fat_dir_size - 1;
            } else {
                selector -= rows;
            }
            full_repaint = 1;
        }

        

        if ((joy & JOY_B) == JOY_B && fat_dir_size != 0) {
            resp = fat_get_full_record(fat_dir[selector], &br_rec);
            if (resp)return resp;

            if (br_rec.is_dir) {
                if (sel_stack_ptr >= MAX_SEL_STACK_DEEP)continue;
                current_dir = br_rec.data_clsut;
                full_repaint = 2;
                sel_stack[sel_stack_ptr++] = selector;
                selector = 0;
            } else {
                full_repaint = 2;
                resp = osFileMenu(&br_rec);
                if (resp)return resp;
            }

            continue;
        }

        if ((joy & JOY_A) == JOY_A && sel_stack_ptr == 0) {
            full_repaint = 2;
            resp = osExitBrowser();
            if (resp)return resp;
        }

        if ((joy & JOY_A) == JOY_A && sel_stack_ptr != 0) {

            current_dir = fat_sub_dir_clust;
            full_repaint = 2;
            selector = sel_stack[--sel_stack_ptr];
            continue;
        }


        


        if ((joy & JOY_SEL) == JOY_SEL) {
            resp = osMainMenu();
            if (resp)return resp;
            full_repaint = 2;
            continue;
        }



        if ((joy & JOY_STA) == JOY_STA) {
            resp = osStartGame();
            return resp;
        }
        
        


    }


}


