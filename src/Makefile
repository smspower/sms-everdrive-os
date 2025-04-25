HOME=E:/SDCC/bin
CC=$(HOME)/sdcc	-mz80 --xram-loc 0xc000
LINK=$(HOME)/sdcc -mz80  --data-loc 0xc000 --no-std-crt0

###--data-loc 0x1800 --code-loc 0x100
### 

OBJ= entry.rel main.rel vdp_a.rel vdp.rel menu_form.rel toolbox.rel mmc.rel mmc_a.rel select_file.rel fat.rel prog.rel usb.rel ram_app.rel sram_manager.rel

OBJ_BOOT= boot_entry.rel


all:	make_boot make_os


make_os:$(OBJ)
	$(LINK) $(OBJ)   -o os.ihx
	$(HOME)/makebin -s 32768 <os.ihx > os.sms

make_boot:$(OBJ_BOOT)
	$(LINK) $(OBJ_BOOT)   -o boot.ihx
	$(HOME)/makebin -s 32768 <boot.ihx > boot.sms
	


main.rel: main.c
entry.rel: entry.s
vdp_a.rel: vdp_a.s
vdp.rel: vdp.c
menu_form.rel: menu_form.c
toolbox.rel: toolbox.c
mmc.rel: mmc.c
mmc_a.rel: mmc_a.s
select_file.rel: select_file.c
fat.rel: fat.c
prog.rel: prog.c
usb.rel: usb.c
ram_app.rel: ram_app.s
boot_entry.rel: boot_entry.s
sram_manager.rel: sram_manager.c

%.rel : %.c
	$(CC) -c $< -o $@


%.rel : %.s
	$(HOME)/sdasz80 -l -o $@ $<

clean:
	$(RM) *.rel
	$(RM) *.sym
	$(RM) *.noi
	$(RM) *.lnk
	$(RM) *.lst
	$(RM) *.map