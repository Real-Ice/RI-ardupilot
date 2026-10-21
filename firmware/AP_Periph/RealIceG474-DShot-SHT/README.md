# RealIceG474-DShot-SHT AP_Periph firmware

Prebuilt AP_Periph firmware for the Real Ice CAN Node G474, a custom board
derived from the Mateksys CAN-G474: 14 PWM/ESC outputs instead of 11, and an
onboard SHT45 temperature/humidity sensor on I2C2 instead of the Matek's
optional off-board SHT3x/SHT4x. Board definitions:
`libraries/AP_HAL_ChibiOS/hwdef/RealIceG474/` (shared) and
`libraries/AP_HAL_ChibiOS/hwdef/RealIceG474-DShot-SHT/` (this variant - the
only one, no separate DShot/Periph/GPS split like stock Matek).

This is not an official ArduPilot release channel build - it's built from
this fork and checked in here so it can be flashed without a local build
environment.

**Board ID is provisional.** `AP_HW_RealIceG474` (7999) in
`Tools/AP_Bootloader/board_types.txt` is a locally-picked placeholder, not
an officially reserved ID - see that file's own allocation process (PR
against it, max 10 IDs per request) before this board goes near a shared
or production fleet.

## Built from

- Commit: `f5750233c5273a02e007bcabe45c32973df78fbd`
- Branch: `realice-g474-board`
- Built: 2026-10-09
- git_identity embedded in the .apj: `f5750233`
- board_id: 7999 (`AP_HW_RealIceG474`, provisional - see above)

Flash used: 168,191 / 487,424 B.

**If the hwdef or anything else this firmware depends on changes, these
files go stale.** Rebuild and replace them (see below) rather than
trusting the commit hash above once source has moved on.

## Output map

14 outputs, all plain PWM (no DShot-capable ESCs on this vehicle),
consolidating the two-node (8 propulsion + 2 pump + 3 payload) Matek
ArduSub layout from `../MatekG474-DShot-SHT/ArduSub-example-setup.md` onto
this single board:

| Pad | Param | Default function | Notes |
|---|---|---|---|
| M1-M8 | `OUT1_FUNCTION`-`OUT8_FUNCTION` | Motor1-8 | propulsion, reversible (`ESC_RV 255`) |
| M9-M10 | `OUT9_FUNCTION`-`OUT10_FUNCTION` | RCIN1-2 | pump |
| M11-M13 | `OUT11_FUNCTION`-`OUT13_FUNCTION` | RCIN3-5 | payload |
| M14 | `OUT14_FUNCTION` | RCIN6 | spare |

`ESC_PWM_TYPE 1` (Normal PWM), `OUT_BLH_MASK 0` (no BLHeli passthrough -
these aren't DShot ESCs). `OPTIONS` bit 1 is set so the pump/payload RCIN
outputs fail safe to their trim value (not 0 PWM) on comms loss - see
`ArduSub-example-setup.md`'s "Pump failsafe on comms loss" section for the
mechanism.

Unlike the Matek board, PWM timer/channel assignment on this board isn't
sequential in places (M9-M12 land on TIM2_CH1/CH3/CH4/CH2, not CH1-CH4 in
order) - see `libraries/AP_HAL_ChibiOS/hwdef/RealIceG474-DShot-SHT/hwdef.dat`
for the verified pin/timer/AF table if reassigning a pad's function.

## Temperature/humidity sensor

Onboard SHT45 on I2C2 at 0x44 - enabled by default (unlike Matek, where
this sensor is optional and off-board):

```
TEMP1_TYPE 10      # SHT4x
TEMP1_BUS 1        # I2C2
TEMP1_ADDR 68      # 0x44
TEMP1_SRC 6        # DroneCAN
TEMP_MSG_RATE 2    # Hz
```

Publishes `uavcan.equipment.device.Temperature` (kelvin) and
`dronecan.sensors.hygrometer.Hygrometer` (temperature in degrees C,
humidity in %).

## Spare I/O (J8)

PA4 and PB4 are both plain GPIO, no default function. PA4 was meant to be
a spare ADC input, but this build confirmed ArduPilot's STM32G474 support
in this tree has no ADC2 channel map at all (only ADC1, which PA4 isn't
part of either) - using it as ADC2 fails `waf configure` outright. See the
comment in `hwdef.dat` for the full story, and ask if extending
`STM32G474xx.py`'s ADC support to cover this pin is worth doing.

## Flashing over CAN with Mission Planner

The node needs its bootloader already flashed (see below for a first
flash) - this only replaces the application, not the bootloader.

1. Connect Mission Planner to the flight controller that bridges the CAN bus
   the node is on.
2. Go to **SETUP > Mandatory Hardware > CAN**, enable CAN if not already
   enabled, and open the DroneCAN device list so the node shows up.
3. Use the CAN firmware update / uploader option, select this node, and
   browse to `AP_Periph.apj` in this folder (preferred - Mission Planner
   checks its embedded `board_id` against the connected node before
   flashing). Use `AP_Periph.bin` instead only if your tool requires a raw
   binary rather than an `.apj`.
4. After the transfer completes the node reboots into the new firmware
   automatically.

## First flash (bootloader + app)

This is a new board - unlike Matek, there's no prior stock firmware
already on it to CAN-update from, so `AP_Periph.apj`/`.bin` (application
only, assumes a bootloader is already present) aren't enough by
themselves for a blank chip.

Use **`AP_Periph_with_bl.hex`** in this folder instead - bootloader and
application combined in one Intel HEX file, with the flash start address
(`0x08000000`) embedded in the file itself. In STM32CubeProgrammer:
connect under reset, open this `.hex` file, and program it - no need to
set a start address manually (that's the point of `.hex` over a raw
`.bin`: the address is self-describing). After this first flash,
subsequent application-only updates can go over CAN using
`AP_Periph.apj` as described above.

`Tools/bootloaders/RealIceG474-DShot-SHT_bl.bin`/`.hex` (the bootloader
alone, no application) are also checked in, built via
`Tools/scripts/build_bootloaders.py RealIceG474-DShot-SHT`.

## Rebuilding

From the root of this repo, with the ArduPilot build toolchain and
submodules available:

```
./waf configure --board RealIceG474-DShot-SHT
./waf AP_Periph
cp build/RealIceG474-DShot-SHT/bin/AP_Periph.apj firmware/AP_Periph/RealIceG474-DShot-SHT/
cp build/RealIceG474-DShot-SHT/bin/AP_Periph.bin firmware/AP_Periph/RealIceG474-DShot-SHT/
cp build/RealIceG474-DShot-SHT/bin/AP_Periph_with_bl.hex firmware/AP_Periph/RealIceG474-DShot-SHT/
```

To rebuild the bootloader (only needed if the bootloader-side hwdef
changes):

```
python3 Tools/scripts/build_bootloaders.py RealIceG474-DShot-SHT
```

Update the commit/build info at the top of this file when you do.
