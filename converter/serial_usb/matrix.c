/*
Copyright 2014 Jun Wako <wakojun@gmail.com>

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

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "print.h"
#include "util.h"
#include "matrix.h"
#include "debug.h"
#include "action_util.h"
#include "host.h"
#include "protocol/serial.h"


/*
 * Not use Matrix.
 *
 * ROW: 16(4bits)
 * COL: 16(4bits)
 *
 *    8bit wide
 *   +---------+
 *  0|00 ... 0F|
 *  1|08 ... 1F|
 *  :|   ...   |
 *  :|   ...   |
 *  E|E0 ... EF|
 *  F|F0 ... FF|
 *   +---------+
 */


inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

void matrix_init(void)
{
    debug_enable = true;
    debug_matrix = true;
    debug_keyboard = true;
    debug_mouse = true;
    serial_init();

    debug("init\n");
    return;
}

#define SERIAL_USB_REPORT_SIZE 9 

static uint8_t current_report[SERIAL_USB_REPORT_SIZE] = {};

static report_mouse_t current_mouse_report = {};

static void send_current_keyboard_report(void);
static void send_current_mouse_report(void);
static void send_current_system_report(void);

static void debug_current_report(void);
static void keyboard_debug(void);
static void mouse_debug(void);
static void consumer_debug(void);

static uint8_t count = 0;

uint8_t matrix_scan(void)
{
    count++;
    if (count % 1000 == 1) {
        dprintf(".");
    }
    if (count % 100000 == 1) {
        dprintf("\n");
    }
    static enum {
        INIT,
        MODIFIERS,
        REPORT_TYPE,
        KBD1, KBD2, KBD3, KBD4, KBD5, KBD6,
        MS_1, MS_2, MS_3, MS_4, MS_5, MS_6,
        SYS1, SYS2, SYS3, SYS4, SYS5, SYS6,
    } state = INIT;

    uint8_t code = serial_recv();
    if (code == -1) {
        return 0;
    } else if (code > 0 || state != INIT) {
        dprintf("Received: ["); debug_hex8(code); dprintf("]\n");
    }

    switch (state) {
        case INIT:
            if (code == 0xFD) {
                current_report[0] = 0xFD;
                int i = 1;
                while (i < 8) {
                    current_report[i++] = 0;
                }
                state = MODIFIERS;
            }
            break;
        case MODIFIERS:
            current_report[1] = code;
            state = REPORT_TYPE;
            break;
        case REPORT_TYPE:
            current_report[2] = code;
            switch (code) { 
                case 0x00 :
                    state = KBD1;
                    break;
                case 0x02 :
                    state = SYS1;
                    break;
                case 0x03 :
                    state = MS_1;
                    break;
                default:
                    state = INIT;
                    break;
            }
        case KBD1:
        case SYS1:
        case MS_1:
            current_report[3] = code;
            break;
        case KBD2:
        case SYS2:
        case MS_2:
            current_report[4] = code;
            break;
        case KBD3:
        case SYS3:
        case MS_3:
            current_report[5] = code;
            break;
        case KBD4:
        case SYS4:
        case MS_4:
            current_report[6] = code;
            break;
        case KBD5:
        case SYS5:
        case MS_5:
            current_report[7] = code;
            break;
        case KBD6:
            current_report[8] = code;
            send_current_keyboard_report();
            state = INIT;
            break;
        case SYS6:
            current_report[8] = code;
            send_current_system_report();
            state = INIT;
            break;
        case MS_6:
            current_report[8] = code;
            send_current_mouse_report();
            state = INIT;
            break;
        default:
            state = INIT;
            break;
    }
    return 0;   
}

static
void send_current_keyboard_report()
{
    keyboard_debug();
    clear_keys();
    clear_mods();
    set_mods(current_report[1]);
    uint8_t i = 2;
    while (i < 8) {
        if (current_report[i] != 0x00) {
            add_key(current_report[i]);
        }
    }
    send_keyboard_report();
}

static void send_current_mouse_report()
{
    mouse_debug();
    current_mouse_report.buttons = current_report[3];
    current_mouse_report.x       = current_report[4];
    current_mouse_report.y       = current_report[5];
    current_mouse_report.v       = current_report[6];
    current_mouse_report.h       = current_report[7];
    host_mouse_send(&current_mouse_report);
    current_mouse_report = (report_mouse_t){};
}

static void send_current_system_report() {
    consumer_debug();

}

inline
bool matrix_has_ghost(void)
{
    return false;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return false;
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return 0;
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < matrix_rows(); row++) {
        phex(row); print(": ");
        pbin_reverse(matrix_get_row(row));
        print("\n");
    }
}

static void keyboard_debug(void)
{
    if (!debug_keyboard) return;
    print("  -- keyboard HID:");
    debug_current_report();
}

static void mouse_debug(void)
{
    if (!debug_mouse) return;
    print("  --    mouse HID:");
    debug_current_report();
}

static void consumer_debug(void)
{
    if (!debug_keyboard) return;
    print("  -- consumer HID:");
    debug_current_report();
}

static void debug_current_report() 
{
    uint8_t i = 0;
    for (; i < SERIAL_USB_REPORT_SIZE; i++) {
        dprintf(" ");
        debug_hex8(current_report[i]);
        dprintf(" ");
    }
    print("--\n");
}

