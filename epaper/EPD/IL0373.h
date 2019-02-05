/* IL0373 Driver, Copyright (C) 2019, Uri Shaked */

#ifndef _IL0373_H
#define _IL0373_H

#include "DEV_Config.h"

// Display resolution
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

void EPD_Reset();
void EPD_Init(bool partial);
void EPD_Clear(void);
void EPD_Display(const UBYTE *Image);
void EPD_Sleep(void);

#endif /* _IL0373_H */
