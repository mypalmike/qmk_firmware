/*
Copyright 2021 minke.io

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */

// TODO: Get actual assigned ID
// https://beta.docs.qmk.fm/faqs/faq_build
// http://www.obdev.at/products/vusb/license.html
// http://www.mcselec.com/index.php?page=shop.product_details&flypage=shop.flypage&product_id=92&option=com_phpshop&Itemid=1

#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x9A2B  // Randomish
#define DEVICE_VER      0x0002
#define MANUFACTURER    minke.io
#define PRODUCT         b1t
#define DESCRIPTION     A tented ergonomic 60% keyboard

/* key matrix size */
#define MATRIX_ROWS 5
#define MATRIX_COLS 15

#define UNUSED_PINS

/* COL2ROW, ROW2COL*/
#define DIODE_DIRECTION COL2ROW

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

/* number of backlight levels */

/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
#define LOCKING_SUPPORT_ENABLE
/* Locking resynchronize hack */
#define LOCKING_RESYNC_ENABLE
