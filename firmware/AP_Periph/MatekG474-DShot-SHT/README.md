# MatekG474-DShot-SHT AP_Periph firmware

Prebuilt AP_Periph firmware for the Mateksys CAN G474, combining PWM/DShot
ESC output with SHT3x/SHT4x temperature+humidity sensing over I2C. Board
definition: `libraries/AP_HAL_ChibiOS/hwdef/MatekG474-DShot-SHT/`.

This is not an official ArduPilot release channel build (it won't appear on
firmware.ardupilot.org) - it's built from this fork and checked in here so it
can be flashed without a local build environment.

See `ArduSub-example-setup.md` in this folder for a worked multi-node
ArduSub parameter reference (two of these nodes on one vehicle, plus the
flight controller side).

## Built from

- Commit: `ab996ba2b3a83f5edf6bd12656a8364ad92c736f`
- Branch: `claude/magical-shannon-nnaeuy`
- Built: 2026-09-25
- git_identity embedded in the .apj: `ab996ba2`
- board_id: 1170 (`AP_HW_MatekG474`, shared with the stock `MatekG474-DShot`/
  `MatekG474-Periph`/`MatekG474-GPS` firmwares - any of them can be replaced
  with this one over CAN without a bootloader change)

Flash used: 167,744 / 487,424 B.

**If the hwdef, the SHT3x/SHT4x driver, or anything else this firmware
depends on changes, these files go stale.** Rebuild and replace them (see
below) rather than trusting the commit hash above once source has moved on.

## Output map

| Pad | Param | Default function | Protocol |
|---|---|---|---|
| M1-M4 | `OUT1_FUNCTION`-`OUT4_FUNCTION` | Motor1-4 | DShot600 (`ESC_PWM_TYPE 7`), commanded via DroneCAN GUI Tool's **ESC** panel |
| M5-M11 | `OUT5_FUNCTION`-`OUT11_FUNCTION` | RCIN1-7 | plain PWM, commanded via the **Servo** panel (`actuator_id` = pad number - 4) |

`ESC_PWM_TYPE` applies to the whole Motor1-4 bank at once (M1-M4 share timer
TIM2), not per channel - see `SRV_Channel::Function` values in
`libraries/SRV_Channel/SRV_Channel.h` if reassigning a pad's function.

Motor-function channels are forward-only by default (`RawCommand` values are
clamped to zero or above). Set the corresponding bit in `ESC_RV` (bit 0 =
Motor1, bit 1 = Motor2, ...) for any channel wired to a reversible/
bidirectional ESC to get the full signed range instead, centered on the
channel's trim - reboot required.

Command timeouts (`ESC_CMD_TIMO` for Motor channels, `SRV_CMD_TIME_OUT` for
RCIN/actuator channels) fail outputs to `0` PWM by default. For an
`ESC_RV`-flagged Motor channel that already lands back on its trim value
(the scaling naturally does that for zero). For an RCIN/actuator channel,
set `OPTIONS` bit 1 (`SERVO_FAILSAFE_TO_TRIM`) to get the same trim-on-
timeout behavior there instead of `0` PWM.

PWM outputs stay disabled until the node sees `SAFETY_OFF` broadcast from
the flight controller bridging the CAN bus (`AP_PERIPH_SAFETY_SWITCH_ENABLED`
is on for this board). Either press the FC's physical safety switch, or set
`BRD_SAFETY_DEFLT 0` on the FC so it boots with safety already off.

## Temperature/humidity sensor setup

Not enabled by default. Once an SHT3x/SHT4x (e.g. SHT45) is wired to I2C1:

```
param set TEMP1_TYPE 10    # 8:SHT3x, 10:SHT4x
param set TEMP1_BUS 0
param set TEMP1_ADDR 0x44  # 0x44/0x45/0x46 depending on the fitted part
```

Publishes `uavcan.equipment.device.Temperature` (kelvin) always, plus
`dronecan.sensors.hygrometer.Hygrometer` (temperature in degrees C, humidity
in %) when the backend reports humidity, at `TEMP_MSG_RATE` Hz.

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
