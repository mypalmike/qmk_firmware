# MCU name
MCU = atmega32u4

# Bootloader selection
#   Teensy       halfkay
#   Pro Micro    caterina
#   Atmel DFU    atmel-dfu
#   LUFA DFU     lufa-dfu
#   QMK DFU      qmk-dfu
#   ATmega32A    bootloadHID
#   ATmega328P   USBasp
# BOOTLOADER = atmel-dfu
BOOTLOADER = caterina

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = lite      # Virtual DIP switch configuration
MOUSEKEY_ENABLE = no       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = yes        # Console for debug
COMMAND_ENABLE = yes        # Commands for debug and configuration
CUSTOM_MATRIX = lite        # Custom matrix file
NKRO_ENABLE = yes            # USB Nkey Rollover
RGBLIGHT_ENABLE = no # yes        # Enable keyboard RGB underglow

SRC += matrix.c
QUANTUM_LIB_SRC += i2c_master.c
