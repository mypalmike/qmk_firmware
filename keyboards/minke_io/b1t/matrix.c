
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "wait.h"
#include "action_layer.h"
#include "matrix.h"
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "i2c_master.h"
#include "quantum.h"

#define I2C_ADDR        0b0100000
#define I2C_ADDR_WRITE  ( (I2C_ADDR<<1) | I2C_WRITE )
#define I2C_ADDR_READ   ( (I2C_ADDR<<1) | I2C_READ  )
#define IODIRA          0x00            // i/o direction register
#define IODIRB          0x01
#define GPPUA           0x0C            // GPIO pull-up resistor register
#define GPPUB           0x0D
#define GPIOA           0x12            // general purpose i/o port register (write modifies OLAT)
#define GPIOB           0x13
#define OLATA           0x14            // output latch register
#define OLATB           0x15
#define I2C_TIMEOUT     100

#define MATRIX_COLS_ONBOARD     7
#define MATRIX_COLS_EXPANDER    8

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

static const pin_t onboard_row_pins[MATRIX_ROWS] = { F4, F5, F6, C6, D7 };
static const pin_t onboard_col_pins[MATRIX_COLS_ONBOARD] = { B5, B4, B6, B2, B3, B1, F7 };
static const uint8_t expander_row_registers[MATRIX_ROWS] = { GPIOB, GPIOB, GPIOB, GPIOB, GPIOB };
static const uint8_t expander_row_pins[MATRIX_ROWS] = { 2, 1, 0, 3, 4 };

// TODO : In future, would be nice to use this to generate bit operations.
// static const uint8_t expander_col_registers[MATRIX_COLS_EXPANDER] = { GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB };
// static const uint8_t expander_col_pins[MATRIX_COLS_EXPANDER] = { 7, 6, 5, 4, 3, 2, 7, 6 };

static bool i2c_initialized = false;
static uint8_t mcp23018_reset_loop;
static i2c_status_t mcp23018_status = 0x20;

extern matrix_row_t raw_matrix[MATRIX_ROWS];

static void init_mcp23018(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row_onboard(uint8_t row);
static void select_row_expander(uint8_t row);
static matrix_row_t read_cols_onboard(uint8_t row);
static matrix_row_t read_cols_expander(uint8_t row);

void matrix_init_custom(void) {
    init_mcp23018();

    unselect_rows();
    init_cols();
}

// Reads and stores a row combined from onboard and expander, returning
// whether a change occurred.
static inline bool store_raw_matrix_row(uint8_t row) {
    matrix_row_t onboard_row = read_cols_onboard(row);
    matrix_row_t expander_row = read_cols_expander(row);

    matrix_row_t temp = ( expander_row << MATRIX_COLS_ONBOARD ) | onboard_row;

    if (raw_matrix[row] != temp) {
        raw_matrix[row] = temp;
        return true;
    }
    return false;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (mcp23018_status) {  // if there was an error
        if (++mcp23018_reset_loop == 0) {
            print("trying to reset mcp23018\n");
            init_mcp23018();
            if (mcp23018_status) {
                print("right side not responding\n");
            } else {
                print("right side attached\n");
            }
        }
    }

    bool changed = false;

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        // select rows from both left and right hands
        select_row_expander(i);
        select_row_onboard(i);

        changed |= store_raw_matrix_row(i);

        unselect_rows();
    }

    return changed;
}

static void init_mcp23018(void) {
    mcp23018_status = 0x20;

    // I2C subsystem
    if (!i2c_initialized) {
        print("initializing i2c\n");
        i2c_init();  // on pins D(1,0)
        i2c_initialized = true;
        _delay_ms(1000);
    }

    // Hardcode writes of pins as inputs or outputs. The values could be
    // deduced from expander_* arrays, but hardcoding is easier for now.
    // Note: using automatic toggling between A/B register pairs allows
    // writing to both ports in a single start/end.
    //
    // set pin direction
    // - unused  : input  : 1
    // - input   : input  : 1
    // - driving : output : 0
    print("setting pin direction\n");
    mcp23018_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);    if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(IODIRA, I2C_TIMEOUT);            if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b11111110, I2C_TIMEOUT);        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b11100000, I2C_TIMEOUT);        if (mcp23018_status) goto out;
    i2c_stop();

    // set pull-up
    // - unused  : on  : 1
    // - input   : on  : 1
    // - driving : off : 0
    print("setting pin pull-ups\n");
    mcp23018_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);    if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(GPPUA, I2C_TIMEOUT);             if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b11111110, I2C_TIMEOUT);        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b11100000, I2C_TIMEOUT);        if (mcp23018_status) goto out;
out:
    i2c_stop();
}

static void init_cols(void) {
    // init on mcp23018
    // not needed, already done as part of init_mcp23018()

    // init on avr
    for (uint8_t i = 0; i < MATRIX_COLS_ONBOARD; i++) {
        setPinInputHigh(onboard_col_pins[i]);
    }
}

static void unselect_rows(void) {
    // no need to unselect on mcp23018, because the select step sets all
    // the other row bits high, and it's not changing to a different
    // direction

    // unselect on avr
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        setPinInput(onboard_row_pins[i]);
    }
}

static void select_row_onboard(uint8_t row) {
    pin_t pin = onboard_row_pins[row];

    setPinOutput(pin);
    writePinLow(pin);
}

static void select_row_expander(uint8_t row) {
    if (!mcp23018_status) {
        uint8_t reg = expander_row_registers[row];
        uint8_t pin = expander_row_pins[row];

        // set active row low  : 0
        // set other rows hi-Z : 1
        mcp23018_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);        if (mcp23018_status) goto out;
        mcp23018_status = i2c_write(reg, I2C_TIMEOUT);                   if (mcp23018_status) goto out;
        mcp23018_status = i2c_write(0xFF & ~(1 << pin), I2C_TIMEOUT);    if (mcp23018_status) goto out;
    out:
        i2c_stop();
    }
}

static matrix_row_t read_cols_onboard(uint8_t row) {
    /* read from avr */

    // More hardcoded masking, based on values in onboard_col_pins.
    // B5, B4, B6, B2, B3, B1, F7
    matrix_row_t val = 0xFF80;

    val |= ((PINB & (1 << 5)) >> (5 - 0));
    val |= ((PINB & (1 << 4)) >> (4 - 1));
    val |= ((PINB & (1 << 6)) >> (6 - 2));
    val |= ((PINB & (1 << 2)) << 1); // >> (2 - 3));
    val |= ((PINB & (1 << 3)) << 1); // >> (3 - 4));
    val |= ((PINB & (1 << 1)) << 4); // >> (1 - 5));
    val |= ((PINF & (1 << 7)) >> (7 - 6));

    return ~val;
}

static uint8_t reverseBits(uint8_t num)
{
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 1;
        result |= (num & 1);
        num >>= 1;
    }

    return result;
}

static matrix_row_t read_cols_expander(uint8_t row) {
    if (mcp23018_status) {  // if there was an error
        return 0;
    } else {
        uint8_t vals[2];
        matrix_row_t val = 0;

        // Get both port A and port B efficiently. Note we use I2C_ADDR_WRITE here because the
        // imple,mntation of i2c_readReg expects that rather than the read addr.
        mcp23018_status = i2c_readReg(I2C_ADDR_WRITE, GPIOA, &vals[0], 2, I2C_TIMEOUT); if (mcp23018_status) goto out;

        // Hardcoded based on expander_col_pins and expander_col_registers. Bit order is
        // exactly backwards so we reverse the bits.
        val = 0x00FF & ~reverseBits((vals[0] & 0b11111100) | ((vals[1]) >> 6));
    out:
        return val;
    }
}

void set_space_led(bool is_on) {
    // GPIOA pin 0 controls the LED.
    uint8_t val = is_on ? 0 : 1;

    mcp23018_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(GPIOA, I2C_TIMEOUT);                 if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(val, I2C_TIMEOUT);                   if (mcp23018_status) goto out;

    out:
        i2c_stop();
}
