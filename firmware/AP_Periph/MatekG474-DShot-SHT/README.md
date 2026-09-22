# MatekG474-DShot-SHT AP_Periph firmware

Prebuilt AP_Periph firmware for the Mateksys CAN G474, combining PWM/DShot
ESC output with SHT3x/SHT4x temperature+humidity sensing over I2C. Board
definition: `libraries/AP_HAL_ChibiOS/hwdef/MatekG474-DShot-SHT/`.

This is not an official ArduPilot release channel build (it won't appear on
firmware.ardupilot.org) - it's built from this fork and checked in here so it
can be flashed without a local build environment.

## Built from

- Commit: `35630eb51f73395cc0551eb1683c6948f0137162`
- Branch: `claude/magical-shannon-nnaeuy`
- Built: 2026-09-22
- git_identity embedded in the .apj: `35630eb5`
- board_id: 1170 (`AP_HW_MatekG474`, shared with the stock `MatekG474-DShot`/
  `MatekG474-Periph`/`MatekG474-GPS` firmwares - any of them can be replaced
  with this one over CAN without a bootloader change)

Flash used: 168,539 / 487,424 B.

This build also requests the I2C bus at standard mode (100kHz) instead of
the 400kHz fast-mode default when talking to the SHT3x/SHT4x sensor
(`AP_TemperatureSensor_Sensirion.cpp`) - a breadboard-wired SHT45 that
detected fine on a Raspberry Pi's I2C bus was not ACKing at all at 400kHz
on this board, which points at wiring/breadboard signal integrity at the
higher clock rather than a wrong bus/address/param. This is a permanent
change (not debug-only), since these sensors don't need fast-mode
throughput at a few Hz of polling.

It still includes three temporary hardware bring-up aids, all removable
once the SHT3x/SHT4x sensor is confirmed working:

- Extra `printf()` debug output in the SHT3x/SHT4x driver init sequence
  (`libraries/AP_TemperatureSensor/AP_TemperatureSensor_Sensirion.cpp`),
  visible on `TX1`/`RX1` (USART1, 57600 baud 8N1) at boot - since
  `GCS_SEND_TEXT` on this build routes to a CAN debug LogMessage broadcast
  instead of the console.
- A full I2C bus scan (both buses, addresses 0x08-0x77) also printed on
  `TX1`/`RX1` at boot, gated behind `AP_PERIPH_I2C_SCAN_DEBUG` in this
  board's hwdef (`Tools/AP_Periph/AP_Periph.cpp`), to independently confirm
  what's actually responding on the bus regardless of the SHT3x/SHT4x
  command sequence.
- A raw GPIO toggle of I2C1_SCL (PA13) and I2C2_SCL (PC4), bypassing the
  I2C peripheral entirely: each pin is held HIGH (3.3V) for 4s then LOW
  (0V) for 4s at boot, so it can be checked directly with a multimeter -
  useful when the bus scan finds nothing at all, to prove whether the MCU
  can control these physical pins independent of the I2C driver stack.

**If the hwdef, the SHT3x/SHT4x driver, or anything else this firmware
depends on changes, these files go stale.** Rebuild and replace them (see
below) rather than trusting the commit hash above once source has moved on.

## Flashing over CAN with Mission Planner

The node already needs to be running any prior AP_Periph firmware (DShot,
Periph or GPS variant) with its bootloader intact - this only replaces the
application, not the bootloader.

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

Since this is the same physical board/bootloader as the stock Matek G474
firmwares, this is the same procedure you'd use to flash any of those.

## Rebuilding

From the root of this repo, with the ArduPilot build toolchain and
submodules (`ChibiOS`, `DroneCAN/DSDL`, `DroneCAN/dronecan_dsdlc`,
`DroneCAN/libcanard`, `mavlink`, `lwip`, `waf`) available:

```
./waf configure --board MatekG474-DShot-SHT
./waf AP_Periph
cp build/MatekG474-DShot-SHT/bin/AP_Periph.apj firmware/AP_Periph/MatekG474-DShot-SHT/
cp build/MatekG474-DShot-SHT/bin/AP_Periph.bin firmware/AP_Periph/MatekG474-DShot-SHT/
```

Update the commit/build info at the top of this file when you do.
