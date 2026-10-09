# hw definition file for Real Ice CAN Node G474
#
# Derived from (but not `include`d - see below) the upstream
# libraries/AP_HAL_ChibiOS/hwdef/MatekG474/hwdef.inc. This board keeps the
# same MCU, flash layout, clock, CAN buses, I2C buses, console UART and
# status LED as the Matek CAN-G474 it's based on, but remaps every PWM
# output and removes USART2/USART3/SPI2 - all pins the upstream .inc
# claims differently - so it's copied here rather than included.

# MCU class and specific type
MCU STM32G474 STM32G474xx

# bootloader starts firmware at 32k + 4k (STORAGE_FLASH)
FLASH_RESERVE_START_KB 36
FLASH_SIZE_KB 512

# store parameters in pages
STORAGE_FLASH_PAGE 16
define HAL_STORAGE_SIZE 1800

# ChibiOS system timer
STM32_ST_USE_TIMER 15
define CH_CFG_ST_RESOLUTION 16

# board ID. See Tools/AP_Bootloader/board_types.txt
# PROVISIONAL - AP_HW_RealIceG474 is a locally-picked placeholder (not
# reserved upstream). Replace it with a properly allocated ID (via a PR
# to board_types.txt, max 10 IDs per request) before this board ships or
# goes near a shared/production fleet - see board_types.txt's own header
# for the allocation process.
APJ_BOARD_ID AP_HW_RealIceG474

# crystal frequency
OSCILLATOR_HZ 8000000

env AP_PERIPH 1

define SERIAL_BUFFERS_SIZE 512
# stack for fast interrupts
define PORT_INT_REQUIRED_STACK 64

define DMA_RESERVE_SIZE 2048

# don't share any DMA channels (there are enough for everyone)
DMA_NOSHARE *

# keep ROMFS uncompressed as we don't have enough RAM
# to uncompress the bootloader at runtime
env ROMFS_UNCOMPRESSED True

define AP_PARAM_MAX_EMBEDDED_PARAM 512

# red LED, active high (PC13, 2.2k, LED, GND per the schematic -
# different polarity convention from Matek's blue LED, but same pin)
PC13 LED OUTPUT HIGH
define HAL_LED_ON 1

# ---------------------- I2C bus ------------------------
I2C_ORDER I2C1 I2C2

# SWD debugging, disabled for I2C1 (shared pins) - same as Matek.
# Uncomment these two and comment out the I2C1 lines below for an app
# debug build.
# PA13 JTMS-SWDIO SWD
# PA14 JTCK-SWCLK SWD

PA13 I2C1_SCL I2C1
PA14 I2C1_SDA I2C1

# I2C2: GPS connector (J9) plus the onboard SHT45 at 7-bit address 0x44
PC4 I2C2_SCL I2C2
PA8 I2C2_SDA I2C2

define HAL_I2C_INTERNAL_MASK 0

define HAL_USE_I2C TRUE

# ---------------------- CAN bus -------------------------
PA11 CAN1_RX CAN1
PA12 CAN1_TX CAN1
PB5  CAN2_RX CAN2
PB6  CAN2_TX CAN2

define HAL_CAN_POOL_SIZE 6000

# ---------------------- UARTs ---------------------------
# USART2 and USART3 are gone on this board (their pins are now PWM
# outputs M12, M10 and M11) - only USART1 (console, J6) and UART4 (GPS
# port J9) remain. EMPTY placeholders keep SERIALn numbering matching
# the upstream UART number, same convention Matek uses.
#           | sr0  | sr1   | sr2  | sr3  | sr4  |
SERIAL_ORDER EMPTY  USART1  EMPTY  EMPTY  UART4

PA9  USART1_TX USART1 SPEED_HIGH
PA10 USART1_RX USART1 SPEED_HIGH

# UART4 is on the GPS connector (J9), alongside I2C2
PC10  UART4_TX UART4 SPEED_HIGH
PC11  UART4_RX UART4 SPEED_HIGH

STDOUT_SERIAL SD1
STDOUT_BAUDRATE 57600

# ----------- ADC ------------------------------------------
# Disabled, same as Matek: PA4/PB4 (J8, the spare I/O pins) were
# intended for analog use, but ArduPilot's STM32G474 support in this
# tree has no ADC2 channel map, and PA4 isn't in the ADC1 map either -
# see the comment in RealIceG474-DShot-SHT/hwdef.dat for the full story.
# Both pins are left as plain GPIO instead.
define HAL_USE_ADC FALSE
define STM32_ADC_USE_ADC1 FALSE
define HAL_DISABLE_ADC_DRIVER TRUE
