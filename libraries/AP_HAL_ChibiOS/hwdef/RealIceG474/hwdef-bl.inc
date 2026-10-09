# hw definition file for Real Ice CAN Node G474 bootloader
#
# Copied from (not included from) MatekG474/hwdef-bl.inc: this board's
# PB12/PC14 chip-select pins don't exist (SPI2/compass removed) and the
# status LED's active level is inverted relative to Matek's.

# MCU class and specific type
MCU STM32G474 STM32G474xx

# crystal frequency
OSCILLATOR_HZ 8000000

# board ID. See Tools/AP_Bootloader/board_types.txt
# PROVISIONAL - see the note in ../RealIceG474/hwdef.inc
APJ_BOARD_ID AP_HW_RealIceG474

# setup build for a peripheral firmware
env AP_PERIPH 1

FLASH_RESERVE_START_KB 0
FLASH_BOOTLOADER_LOAD_KB 32
FLASH_SIZE_KB 512

# reserve some space for params
APP_START_OFFSET_KB 4

# ---------------------------------------------
# red LED, active high (see hwdef.inc) - Matek's bootloader uses
# HAL_LED_ON 0 for its active-low blue LED; this board's LED is active
# high, same polarity as the app side.
PC13 LED_BOOTLOADER OUTPUT LOW
define HAL_LED_ON 1

# SWD debugging
PA13 JTMS-SWDIO SWD
PA14 JTCK-SWCLK SWD

# enable CAN support
PA11 CAN1_RX CAN1
PA12 CAN1_TX CAN1

PB5  CAN2_RX CAN2
PB6  CAN2_TX CAN2

# ---------------------------------------------

# make bl baudrate match debug baudrate for easier debugging
define BOOTLOADER_BAUDRATE 57600

# use a small bootloader timeout
define HAL_BOOTLOADER_TIMEOUT 2500

define HAL_USE_EMPTY_IO TRUE
define DMA_RESERVE_SIZE 0

define HAL_USE_SERIAL FALSE

# No PB12 MAG_CS / PC14 SPARE_CS here - SPI2 and the RM3100 compass
# footprint don't exist on this board, so those pins are left
# unconnected rather than driven high as Matek's bootloader does.
