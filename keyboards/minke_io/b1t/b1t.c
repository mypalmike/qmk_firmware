/* Copyright 2021 minke.io
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "b1t.h"

extern void set_space_led(bool is_on);

static void init_led_ports(void) {
    setPinOutput(F4);
    setPinOutput(F5);
    keyboard_pre_init_user();
}

void keyboard_pre_init_kb(void) {
    init_led_ports();
    keyboard_pre_init_user();
}

bool led_update_kb(led_t led_state) {
    bool res = led_update_user(led_state);
    if(res) {
        writePin(F5, !led_state.caps_lock);
    }
    return res;
}

layer_state_t layer_state_set_kb(layer_state_t state) {
    switch (get_highest_layer(state)) {
        case 0:
            writePin(F4, 1);
            set_space_led(false);
            break;
        case 1:
            writePin(F4, 0);
            set_space_led(false);
            break;
        case 2:
            writePin(F4, 1);
            set_space_led(true);
            break;
        default:
            // 3 or higher = both lights on.
            writePin(F4, 0);
            set_space_led(true);
            break;
    }

    return layer_state_set_user(state);
}
