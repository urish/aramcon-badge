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
extern void vs1053_set_volume(uint8_t left, uint8_t right);
extern void vs1053_send_data(u8_t *buf, u8_t len);
extern void vs1053_test(uint8_t n, uint16_t ms);

#endif // __SOUND_H__