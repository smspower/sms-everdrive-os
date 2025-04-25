/* 
 * File:   everdrive.h
 * Author: KRIK
 *
 * Created on 22 Декабрь 2010 г., 15:03
 */

#ifndef _EVERDRIVE_H
#define	_EVERDRIVE_H

#define CFG_REG *((u8 *)0xfff8)
#define KEY_REG *((u8 *)0xfff9)
#define BANK_CFG0_REG *((u8 *)0xfffa)
#define BANK_CFG1_REG *((u8 *)0xfffb)
#define BANK_CFG2_REG *((u8 *)0xfffc)

#define SPI_PORT *((u8 *)0x7f00)
#define STATE_PORT *((volatile u8 *)0x7f01)
#define FIFO_PORT *((u8 *)0x7f02)
#define FIRM_VERSION *((u8 *)0x7f03)

#define _BCFG_ROM_BANK 1
#define _BCFG_RAM_BANK 2
#define _BCFG_RAM_NROM 3

//cfg bits
#define _CFG_ROM_REGS_ON 0
#define _CFG_SPI_SS 1
#define _CFG_SPI_FULL_SPEED 2
#define _CFG_CDM_MAP 3
#define _CFG_AUTO_BUSY 4
#define _CFG_ROM_WE_OFF 5
#define _CFG_SMS_MODE 6

//state bits
#define _ST_SPI_BUSY 7
#define _ST_ROM_BUSY 6
#define _ST_FIFO_RD_BUSY 5
#define _ST_FIFO_WR_BUSY 4
#define _ST_DEV_GG 3

#define IS_GG_CART (STATE_PORT & (1 << _ST_DEV_GG ))
#define IS_FIFO_WR_BUSY (STATE_PORT & (1 << _ST_FIFO_WR_BUSY))
#define IS_FIFO_RD_BUSY (STATE_PORT & (1 << _ST_FIFO_RD_BUSY))
#define FIFO_RD_BUSY while (IS_FIFO_RD_BUSY);
#define FIFO_WR_BUSY while (IS_FIFO_WR_BUSY);

//#define AUTO_BUSY_ON CFG_REG |= (1 << _CFG_AUTO_BUSY)
#define AUTO_BUSY_OFF CFG_REG &= ~(1 << _CFG_AUTO_BUSY)
//#define AUTO_BUSY_ON CFG_REG &= ~(1 << _CFG_AUTO_BUSY)

#define RAM_REGS_ON \
    KEY_REG = 0;    \
    KEY_REG = 0x52; \
    KEY_REG = 0x45; \
    KEY_REG = 0x4e;

#define RAM_REGS_OFF KEY_REG = 0

#define ROM_REGS_OFF CFG_REG &= ~(1 << _CFG_ROM_REGS_ON)
#define ROM_REGS_ON CFG_REG |= (1 << _CFG_ROM_REGS_ON)

#define SS_OFF CFG_REG &= ~(1 << _CFG_SPI_SS)
#define SS_ON CFG_REG |= (1 << _CFG_SPI_SS)

#define SPI_SPEED_OFF CFG_REG &= ~(1 << _CFG_SPI_FULL_SPEED)
#define SPI_SPEED_ON CFG_REG |= (1 << _CFG_SPI_FULL_SPEED)

#define OS_VERSION 6
#endif	/* _EVERDRIVE_H */

