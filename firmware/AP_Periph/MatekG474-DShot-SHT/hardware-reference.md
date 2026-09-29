# Mateksys CAN-G474 hardware reference (for PCB replication)

Pin-level reference for an electronics engineer replicating the Mateksys
CAN-G474 node, compiled from this firmware's board definition
(`libraries/AP_HAL_ChibiOS/hwdef/MatekG474/hwdef.inc` and
`libraries/AP_HAL_ChibiOS/hwdef/MatekG474-DShot-SHT/hwdef.dat`) plus the
product photos Mateksys publishes. Everything under "Sourced from firmware"
is exact, taken directly from the hwdef files that must match the real
silicon for the firmware to work. Everything under "Inferred / unconfirmed"
is a reasoned guess from pinout constraints or the product photo, not a
verified schematic fact - confirm those against Mateksys's own schematic if
you can get one, or by continuity/inspection of a real unit.

## MCU

- **STM32G474xx**, 512KB flash (`FLASH_SIZE_KB 512` in hwdef).
- **Inferred package: LQFP48** (likely order code **STM32G474CxT6**, C = 48-pin).
  Not stated explicitly anywhere in the hwdef; inferred because the full
  pin inventory below only ever uses PA0-15, PB0-15, and a *subset* of
  PC (PC4, PC6, PC10, PC11, PC13, PC14) with no PD/PE/PF/PG pins at all -
  exactly the pin set the 48-pin package exposes. The flash size (512KB)
  corresponds to ST's "E" suffix (STM32G474xE).
- **Clock reference: 8MHz external crystal (HSE)** - `OSCILLATOR_HZ 8000000`,
  set identically in both the bootloader and application hwdef. This value
  is load-bearing: PLL, UART baud, and CAN bit-timing math all derive from
  it, so a real board must use an 8MHz reference for this firmware to run
  at correct rates.
- No `HSE_BYPASS`-style directive is present, so the firmware assumes a
  passive crystal (2-terminal, with your own load capacitors sized to the
  chosen crystal's `CL` spec), not an active oscillator module.
- System tick timer: **TIM15** (`STM32_ST_USE_TIMER 15`) - reserved by
  ChibiOS internally, not available for PWM or anything else.

## Power / CAN bus

- **CAN1**: PA11 (`CAN1_RX`), PA12 (`CAN1_TX`).
- **CAN2**: PB5 (`CAN2_RX`), PB6 (`CAN2_TX`).
- Each CAN connector in the product photo shows a 120Ω resistor pair
  (silkscreened "120Ω") near the transceiver - standard switchable/fixed
  bus termination, one per CAN port. Not defined in the hwdef (that's a
  PCB-level component, invisible to firmware) - inferred from the photo.
- The photo's transceiver ICs are marked something like "1051A/x" per CAN
  port, consistent with a standard 1Mbps CAN transceiver family (e.g.
  NXP/TI TJA1051-style part) - **unconfirmed exact part**, read from a
  low-resolution product photo, not a datasheet.
- Both CAN header groups show a 5V pin alongside the CAN-H/CAN-L pair, for
  bus-powered nodes - the regulator that derives the board's 3.3V rail
  from that 5V input isn't visible/identifiable in the photo or hwdef;
  that's your own design choice (a standard 5V->3.3V LDO or buck is fine
  given AP_Periph's modest current draw).

## I2C buses

- **I2C1**: PA13 (`SCL`), PA14 (`SDA`).
  - These pins are shared with SWDIO/SWCLK on this MCU. The hwdef
    explicitly disables SWD in favor of I2C1
    (`# SWD debugging, disabled for I2C1`) - if you want both, you'd need
    a separate dedicated debug header/pin pair instead.
- **I2C2**: PC4 (`SCL`), PA8 (`SDA`).
- `HAL_I2C_CLEAR_ON_TIMEOUT 0` and `HAL_I2C_INTERNAL_MASK 0` are the base
  board's defaults for both buses (this firmware's `MatekG474-DShot-SHT`
  variant overrides `HAL_I2C_CLEAR_ON_TIMEOUT` back to `1` - a firmware
  setting, not something that affects your schematic).

## UART / Serial ports

Silkscreen numbering matches the firmware's `SERIALn` numbering directly
(`SERIAL_ORDER EMPTY USART1 USART2 USART3 UART4`):

| Silkscreen | Param | MCU pins | Notes |
|---|---|---|---|
| TX1/RX1 | `SERIAL1` | PA9 (`USART1_TX`) / PA10 (`USART1_RX`) | also the firmware's console (`STDOUT_SERIAL SD1`, 57600 baud) |
| TX2/RX2 | `SERIAL2` | PB3 (`USART2_TX`) / PB4 (`USART2_RX`) | |
| TX3/RX3 | `SERIAL3` | PB10 (`USART3_TX`) / PB11 (`USART3_RX`) | `NODMA` in the hwdef |
| TX4/RX4 | `SERIAL4` | PC10 (`UART4_TX`) / PC11 (`UART4_RX`) | |

## SPI (compass, unused on this specific firmware variant)

- **SPI2**: PB13 (`SCK`), PB14 (`MISO`), PB15 (`MOSI`).
- Chip selects: PB12 (`MAG_CS`), PC14 (`SPARE_CS`).
- Labeled in the hwdef as intended for an onboard RM3100 compass. **Not
  used at all by this firmware build** - no `SPIDEV` entry, no compass
  driver enabled. If you're replicating the board without needing a
  compass, this bus (and its footprint) can be omitted entirely - see
  below for reusing these exact pins for extra PWM instead.

## Debug / boot

- **BOOT0**: labeled pin, pairs with reset to enter the MCU's built-in
  system bootloader.
- **NRST**: reset.
- **DFU button** (product photo): most likely a convenience button that
  asserts BOOT0 (possibly combined with reset) for firmware recovery -
  this firmware has no USB peripheral wired (PA11/PA12 are CAN1, not
  USB D-/D+), so field updates go over CAN via `AP_Bootloader`, not USB
  DFU in the traditional sense. Unconfirmed exact button wiring from the
  photo alone.

## Status LED

- **PC13**, active-high (`PC13 LED OUTPUT HIGH`, `HAL_LED_ON 1`).
- Product photo also shows "Red"/"Blue" LEDs near the CAN connectors -
  likely CAN bus activity/status indicators, not defined in this hwdef
  (could be driven by the CAN transceiver itself, or separate GPIOs not
  used by this firmware) - unconfirmed.

## PWM / motor outputs (M1-M11)

| Pad | MCU pin | Timer/Channel |
|---|---|---|
| M1 | PA0 | TIM2_CH1 |
| M2 | PA1 | TIM2_CH2 |
| M3 | PA2 | TIM2_CH3 |
| M4 | PA3 | TIM2_CH4 |
| M5 | PA6 | TIM3_CH1 |
| M6 | PA7 | TIM3_CH2 |
| M7 | PB0 | TIM3_CH3 |
| M8 | PB1 | TIM3_CH4 |
| M9 | PC6 | TIM8_CH1 |
| M10 | PB9 | TIM8_CH3 |
| M11 | PB2 | TIM5_CH1 |

**Unconfirmed from any source available here**: whether these pads have a
series resistor between the MCU pin and the connector (common practice for
signal lines that leave the board via a cable, protecting against ESD and
short-circuit fault current). If you're re-laying this out, I'd include one
(footprint for e.g. 100-470Ω) on every pad that drives an external
cable/connector - cheap insurance, and consistent with what these pads are
actually used for.

## Adding 3 more PWM outputs (M12-M14)

TIM1 is a full advanced-control timer, completely unused anywhere else in
this design. Its complementary channels 1-3 land on the same three pins
currently used for the (unused-on-this-variant) SPI2 bus:

| New pad | MCU pin | Timer/Channel | AF number | Currently |
|---|---|---|---|---|
| M12 | PB13 | TIM1_CH1N | AF6 | SPI2_SCK |
| M13 | PB14 | TIM1_CH2N | AF6 | SPI2_MISO |
| M14 | PB15 | TIM1_CH3N | AF4 | SPI2_MOSI |

Design notes:
- TIM1's *main* channels (CH1/CH2/CH3, normally PA8/PA9/PA10 on this
  package) are already claimed by I2C2_SDA and USART1 respectively and are
  **not** used here - so there's no dead-time/shoot-through concern from
  running the complementary outputs alone; this isn't a half-bridge motor
  driver configuration, just three independent PWM signals on one timer.
- All three share one PWM refresh rate (same timer group) - fine for
  standard servo/ESC PWM, just not independently tunable per channel.
- This exact pattern (a lone complementary timer channel used as a normal
  PWM output, no main channel present) has precedent elsewhere in
  ArduPilot's hwdef library (e.g. KakuteF7 uses `TIM1_CH3N` this way), so
  it's a proven approach on the tooling/firmware side - the pin-level
  correctness for *this* MCU/package is what I've verified above from the
  chip's actual alternate-function table, not assumed by analogy.
- If you drop SPI2/the compass footprint to make room for this, PB12
  (`MAG_CS`) and PC14 (`SPARE_CS`) become free GPIOs with no useful PWM
  alternate function - fine to repurpose for something else (extra GPIO,
  status LED, etc.) or leave unpopulated.
- On the firmware side, this needs three new `PWM(12)`/`PWM(13)`/`PWM(14)`
  lines added to `hwdef.dat` (removing the SPI2 pin claims) plus a
  rebuild - not done in this repo yet since this doc is for a from-scratch
  PCB. Ask if you also want the firmware-side hwdef updated to match once
  the board exists.

## Connectors: Pixhawk connector standard (JST-GH)

Pinouts below follow the standard Pixhawk/Dronecode JST-GH connector
convention (pin 1 = power, last pin = GND, matching e.g.
`libraries/AP_HAL_ChibiOS/hwdef/ARK_PI6X/README.md`'s connector tables in
this same repo) rather than anything specific to the real Matek board's
actual connectors, which aren't identifiable from the product photo. Signal
voltage is 3.3V throughout (this MCU's native I/O level) unless noted;
common JST-GH crimp housings for reference: `GHR-04V-S` (4-pin) /
`GHR-06V-S` (6-pin), mating shrouded headers `SM04B-GHS-TB` /
`SM06B-GHS-TB`.

### CAN1 / CAN2 - 4-pin JST-GH, two connectors per bus (daisy chain)

Wire **both** connectors on a given bus to the *same* four MCU-side nets in
parallel (not through any switch/mux) - that's what makes them a pass-through
daisy chain: a cable in on one connector, a cable continuing to the next
node out the other.

| Pin | Signal | Voltage |
|---|---|---|
| 1 | 5V | 5V |
| 2 | CAN_H | - |
| 3 | CAN_L | - |
| 4 | GND | GND |

CAN1 pair -> PA12 (`CAN1_TX`)/PA11 (`CAN1_RX`) through the CAN1 transceiver;
CAN2 pair -> PB6 (`CAN2_TX`)/PB5 (`CAN2_RX`) through the CAN2 transceiver.
Put the 120Ω termination resistor(s) once per bus (ideally switchable, e.g.
a solder jumper or small switch) between CAN_H/CAN_L, not per connector -
with two daisy-chain connectors per bus this board might sit in the middle
of a chain, where termination should usually be *off*, only enabled by
whichever node is a physical end of the bus.

### I2C (I2C1) - 4-pin JST-GH

| Pin | Signal | Voltage |
|---|---|---|
| 1 | 5V | 5V |
| 2 | SCL | 3.3V |
| 3 | SDA | 3.3V |
| 4 | GND | GND |

-> PA13 (`I2C1_SCL`) / PA14 (`I2C1_SDA`). Remember these pins are shared
with SWDIO/SWCLK on this MCU - this connector and a debug header are
mutually exclusive unless you break I2C1 out to different pins instead.

### UART1, UART4 - 4-pin JST-GH

Plain UART, no flow control (this hwdef never defines CTS/RTS for either
port, unlike a Pixhawk-standard 6-pin TELEM connector which carries them) -
a 4-pin connector is the honest match for what's actually wired, not the
full 6-pin TELEM pattern:

| Pin | Signal | Voltage |
|---|---|---|
| 1 | 5V | 5V |
| 2 | TX | 3.3V |
| 3 | RX | 3.3V |
| 4 | GND | GND |

UART1 -> PA9 (`TX1`)/PA10 (`RX1`). UART4 -> PC10 (`TX4`)/PC11 (`RX4`).

### UART2 + I2C2 - 6-pin JST-GH ("GPS port" style)

Matches the standard combined GPS+compass connector pattern used across
Pixhawk-standard boards for a module carrying both a UART GPS and an I2C
compass on one cable:

| Pin | Signal | Voltage |
|---|---|---|
| 1 | 5V | 5V |
| 2 | TX (UART2) | 3.3V |
| 3 | RX (UART2) | 3.3V |
| 4 | SCL (I2C2) | 3.3V |
| 5 | SDA (I2C2) | 3.3V |
| 6 | GND | GND |

-> PB3 (`TX2`)/PB4 (`RX2`) for UART2, PC4 (`I2C2_SCL`)/PA8 (`I2C2_SDA`)
for I2C2.

## ESC/servo output header (M1-M11, or M1-M14 with the addition above)

Pixhawk-style JST-GH connectors aren't practical here - eleven (or
fourteen) individual 3-pin connectors won't fit a 36x36mm board, and this
isn't the convention real boards with this many outputs use anyway. The
standard approach for a PWM/ESC output bank instead: a single-row 0.1"
(2.54mm) pitch pin header, three rows deep, one column per channel:

```
GND  GND  GND  GND  GND  GND  GND  GND  GND  GND  GND
5V   5V   5V   5V   5V   5V   5V   5V   5V   5V   5V
S1   S2   S3   S4   S5   S6   S7   S8   S9   S10  S11
```

GND and 5V rows are continuous rails (one net each, tied together across
all columns); only the signal row carries eleven distinct nets, one per
motor pad from the table above.

Two things worth getting right on the silkscreen, since both are common,
damaging mistakes when someone plugs in a servo/ESC cable:
- **Pin order convention**: the JR/Hitec convention (Signal-VCC-GND reading
  from one edge) and the Futaba convention (swaps VCC/GND) are both in
  common use and are *not* interchangeable - reversing power and ground
  into an ESC is a real way to damage it. Pick one, mark it clearly and
  consistently across every channel, and call it out explicitly in your
  documentation/silkscreen (e.g. an arrow or dot marking pin 1 = Signal).
- **This 5V rail only needs to supply the signal circuitry inside each ESC**
  (most ESCs are separately powered by the main battery lead for the motor
  itself) - don't size it assuming it needs to deliver motor current, but
  do size it for however many ESCs' BEC/signal-side draw you expect
  in parallel on this one shared rail.

Same series-resistor note as the M1-M11 table above applies to whichever
row carries the signal here - these are still lines leaving the board to
external cables/ESCs.
