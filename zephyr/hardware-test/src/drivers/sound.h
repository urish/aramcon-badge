/*
 * Badge Sound Driver (VS1003B/VS1053B)
 * Author: Uri Shaked
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SOUND_H__
#define __SOUND_H__
#include <zephyr.h>

extern void init_sound();
extern bool sound_sanity();

#endif // __SOUND_H__