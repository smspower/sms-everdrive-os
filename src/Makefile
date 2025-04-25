GDK= ../../../everdrive-sdk/
CC= $(GDK)bin/gcc
OBJC= $(GDK)bin/objcopy
ASMZ80= $(GDK)bin/asmz80
BINTOC= $(GDK)bin/bintoc
NM= $(GDK)bin/nm
NM2WCH= $(GDK)bin/nm2wch
OUTPUT= MDOS
RESOL=BIN



BOOT_SRC_S = bootasm.s

SRC_S = bios_edmd.s
SRC_C = $(wildcard *.c)



OBJ = $(SRC_S:.s=.o)
OBJ += $(SRC_C:.c=.o)


LINKOBJ= $(OBJ)


FLAGS= $(OPTION) -m68000 -Wall -Os -fomit-frame-pointer $(INCS)
FLAGSZ80= -c -i -x1 -x2 -x3 -z -lnul

all:

make_edmd: $(OUTPUT).$(RESOL)


$(OUTPUT).$(RESOL): $(OUTPUT).out
	$(NM) -n -S -t x $(OUTPUT).out >$(OUTPUT).nm
	$(NM2WCH) $(OUTPUT).nm $(OUTPUT).wch
	$(OBJC) --pad-to 0x1010000 -O binary $(OUTPUT).out $(OUTPUT).$(RESOL)

$(OUTPUT).out: $(OBJ)
	$(CC) -T rom.ld -nostdlib $(LINKOBJ) $(GDK)bin/libgcc.a -o $(OUTPUT).out


%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@  -D CART_EDMD

%.o: %.s
	$(CC) $(FLAGS) -c $< -o $@

clean_edmd:
	$(RM) *.o
	$(RM) *.out
	$(RM) *.wch
	$(RM) *.nm

#******************************************************************************* SMS
HOME_Z80=E:/SDCC-3.3/bin
CC_Z80=$(HOME_Z80)/sdcc	-mz80 --xram-loc 0xc000
LINK_Z80=$(HOME_Z80)/sdcc -mz80  --data-loc 0xc000 --no-std-crt0


SRC_S_SMS = bios_msed.s sys_sms_a.s
SRC_C_SMS = $(wildcard *.c)

OBJ_SMS = $(SRC_S_SMS:.s=_sms.rel)
OBJ_SMS += $(SRC_C_SMS:.c=_sms.rel)

make_sms:  MSOS.SMS

MSOS.SMS: $(OBJ_SMS)
	  $(LINK_Z80) $(OBJ_SMS)   -o MSOS.IHX
	  $(HOME_Z80)/makebin -s 31744 <MSOS.IHX > MSOS.SMX
	  cat MSOS.SMX font_sms.bin > MSOS.SMS
	
%_sms.rel : %.c
	$(CC_Z80) -c $< -o $@ -D CART_MSED


%_sms.rel : %.s
	$(HOME_Z80)/sdasz80 -l -o $@ $<

clean_sms:
	$(RM) *.rel
	$(RM) *.sym
	$(RM) *.asm
	$(RM) *.lst
	$(RM) *.o
#******************************************************************************* GBX
HOME_GBX=E:/SDCC-3.3/bin
CC_GBX=$(HOME_GBX)/sdcc	-mgbz80 --xram-loc 0xc000
LINK_GBX=$(HOME_GBX)/sdcc -mgbz80  --data-loc 0xc000 --no-std-crt0 --code-loc 0x800


SRC_S_GBX = bios_gbx.s sys_gbx_a.s
#SRC_C_GBX = $(wildcard *.c)
SRC_C_GBX = main.c disk.c fat.c gg_gbx.c os_gbx.c sys.c sys_gbx.c browser.c str.c

OBJ_GBX = $(SRC_S_GBX:.s=_gbx.rel)
OBJ_GBX += $(SRC_C_GBX:.c=_gbx.rel)

make_gbx:  GBOS.GB

GBOS.GB: $(OBJ_GBX)
	  $(LINK_GBX) $(OBJ_GBX)   -o GBOS.IHX
	  $(HOME_GBX)/makebin -s 32000 <GBOS.IHX > GBOS.SMX
	  cat GBOS.SMX font_gbx.bin > GBOS.GB
	
%_gbx.rel : %.c
	$(CC_GBX) -c $< -o $@ -D CART_GBX


%_gbx.rel : %.s
	$(HOME_GBX)/sdasgb -l -o $@ $<

clean_gbx:
	$(RM) *.rel
	$(RM) *.sym
	$(RM) *.asm
	$(RM) *.lst
	$(RM) *.o

