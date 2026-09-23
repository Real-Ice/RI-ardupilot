# MatekG474-DShot-SHT AP_Periph firmware

Prebuilt AP_Periph firmware for the Mateksys CAN G474, combining PWM/DShot
ESC output with SHT3x/SHT4x temperature+humidity sensing over I2C. Board
definition: `libraries/AP_HAL_ChibiOS/hwdef/MatekG474-DShot-SHT/`.

This is not an official ArduPilot release channel build (it won't appear on
firmware.ardupilot.org) - it's built from this fork and checked in here so it
can be flashed without a local build environment.

## Built from

- Commit: `ad2cabc20cef8e741209ce142c27a581beeafdf6`
- Branch: `claude/magical-shannon-nnaeuy`
- Built: 2026-09-23
- git_identity embedded in the .apj: `ad2cabc2`
- board_id: 1170 (`AP_HW_MatekG474`, shared with the stock `MatekG474-DShot`/
  `MatekG474-Periph`/`MatekG474-GPS` firmwares - any of them can be replaced
  with this one over CAN without a bootloader change)

Flash used: 167,791 / 487,424 B.

### I2C investigation history

1. **`I2CDevice.cpp` TIMINGR family bug (real bug, fixed, but not the actual
   cause here).** A per-device I2C clock override always wrote the
   F7-specific raw `TIMINGR` register value regardless of actual MCU family.
   Genuinely wrong and fixed (H7/L4/L4PLUS/G4 now get their own correct
   branches), but it turned out this branch was never actually triggered in
   our case: `HAL_I2C_MAX_CLOCK` (the bus's default before any per-device
   override) is itself `100000`, so neither the original driver's 400kHz
   request nor our 100kHz request was ever *below* that default - meaning
   the correct STM32G4 timing value was in effect the whole time.
2. **I2C DMA disabled, run in polling mode instead** (`NODMA I2C*` +
   `STM32_I2C_USE_DMA FALSE`). No shipped MatekG474 firmware has ever
   exercised I2C+DMA on this hwdef to prove it works, and several other
   boards (e.g. KakuteH7-Wing) disable I2C DMA after hitting real issues.
3. **`HAL_I2C_CLEAR_ON_TIMEOUT` re-enabled.** The shared MatekG474 base
   disables this stuck-bus recovery helper (on by default in
   AP_HAL_ChibiOS) with no stated reason.
4. **Debug scan probe fixed** to use the SHT4x's actual "read serial
   number" command (`0x89`) instead of a generic "read register 0", since
   SHT4x is command-based, not register-addressable, and could NACK an
   arbitrary command byte even when present and correctly wired.
5. **Battery monitor and STM32G474 pin/AF mapping independently verified**
   as not the cause: `AP_PERIPH_BATTERY_ENABLED` is `0` for this board and
   its only call site is properly guarded, so no I2C battery backend is
   active; I2C1/I2C2's AF4 assignment on PA13/PA14/PC4/PA8 was cross-checked
   against ST's own per-chip pin database, not just a generic assumption.
6. **Oscilloscope confirmed clean 100kHz SCL/SDA at the sensor** - ruling
   out signal integrity, so this build tests one more hypothesis: the
   scan's write (command `0x89`) and read (6-byte response) are now two
   separate I2C transactions with a real 2ms delay between them, instead
   of one combined transfer with an immediate repeated START, in case the
   SHT4x needs more turnaround time than the datasheet's timing tables
   document for the serial-number command specifically.
7. **Scan debug disabled for this build** (`AP_PERIPH_I2C_SCAN_DEBUG` back
   to `0` in this board's hwdef). The probe code stays in
   `Tools/AP_Periph/AP_Periph.cpp` and can be re-enabled by flipping that
   define back to `1` when it's needed again for the next debugging round.
8. **Root cause found and fixed.** With the scan enabled, the sensor was
   found at `0x44` on bus 1, but `SHT4x read sn failed` still printed.
   Oscilloscope capture on the real hardware confirmed: address+write and
   the `0x89` command byte are ACKed, but the very next bit - the repeated
   START's read-address byte - is NACKed. The driver's
   `read_serial_number()` sent the command and read the 6-byte reply as
   one combined write+repeated-START transfer with no gap; the SHT4x needs
   time to prepare its reply and isn't ready for an immediate repeated
   START. Fixed by splitting it into two I2C transactions with a 2ms delay
   between them, the same pattern the (now-disabled) scan probe used
   successfully. While in that code, also fixed a second, unrelated bug in
   the shared `AP_TemperatureSensor_Sensirion::read_measurements()`
   (inherited unmodified from upstream `AP_TemperatureSensor_SHT3x.cpp`):
   it passed `send_len=1` with a null send pointer, which is not a
   read-only transfer - it transmits one garbage byte read from address
   `0x0` before the repeated-START read, which would have corrupted every
   periodic post-init measurement for both SHT3x and SHT4x. Changed to
   `send_len=0` for a proper read-only transfer.

It still includes two temporary hardware bring-up aids, both removable
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

(A third aid, a raw GPIO toggle of I2C1_SCL/I2C2_SCL bypassing the I2C
peripheral, was added and then removed again during bring-up: it left the
SCL pins forced into push-pull GPIO mode and never handed them back to the
I2C peripheral's alternate-function mode, which hung the very next real
I2C transaction. It served its purpose - confirming the MCU could drive
those pins - before the actual root cause below was found.)

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
