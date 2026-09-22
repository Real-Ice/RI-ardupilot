# MatekG474-DShot-SHT AP_Periph firmware

Prebuilt AP_Periph firmware for the Mateksys CAN G474, combining PWM/DShot
ESC output with SHT3x/SHT4x temperature+humidity sensing over I2C. Board
definition: `libraries/AP_HAL_ChibiOS/hwdef/MatekG474-DShot-SHT/`.

This is not an official ArduPilot release channel build (it won't appear on
firmware.ardupilot.org) - it's built from this fork and checked in here so it
can be flashed without a local build environment.

## Built from

- Commit: `3f397ce1368e1a802080bea6e06a94a13a24ec33`
- Branch: `claude/magical-shannon-nnaeuy`
- Built: 2026-09-22
- git_identity embedded in the .apj: `3f397ce1`
- board_id: 1170 (`AP_HW_MatekG474`, shared with the stock `MatekG474-DShot`/
  `MatekG474-Periph`/`MatekG474-GPS` firmwares - any of them can be replaced
  with this one over CAN without a bootloader change)

Flash used: 167,723 / 487,424 B.

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
