/* 
 * File:   joy.h
 * Author: KRIK
 *
 * Created on 22 Декабрь 2010 г., 13:41
 */

#ifndef _JOY_H
#define	_JOY_H

#include "types.h"

#define KEY_UP (1 << 0)
#define KEY_DOWN (1 << 1)
#define KEY_LEFT (1 << 2)
#define KEY_RIGHT (1 << 3)
#define KEY_B (1 << 4)
#define KEY_C (1 << 5)

void joy_read();
void joy_wait_read();

extern u8 joy;

#endif	/* _JOY_H */

