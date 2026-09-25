# Example: ArduSub with two MatekG474-DShot-SHT nodes

Reference parameter set for a specific deployment: an ArduSub vehicle with
two Mateksys CAN G474 nodes (this firmware) on one DroneCAN bus, plus a
companion computer commanding the non-propulsion outputs over MAVLink.

- **Node A** (propulsion): 8x ESC for locomotion on pins M1-M8, 2x ESC for a
  pump on M9-M10.
- **Node B** (payload): 3x ESC for other payloads on pins M1-M3.
- All 13 ESCs are **plain PWM, not DShot-capable**.
- Flight controller frame: ArduSub `Vectored_6DOF` (8 thrusters).

## Read this first: no reverse thrust via AP_Periph's ESC path

**This firmware cannot currently drive a bidirectional/reversible ESC
through the DroneCAN ESC RawCommand ("Motor") path.**
`Tools/AP_Periph/rc_out.cpp::rcout_esc()` does:

```cpp
// we don't support motor reversal yet on ESCs in AP_Periph
SRV_Channels::set_output_scaled(SRV_Channels::get_motor_function(i), MAX(0,rc[i]));
```

`RawCommand` values are signed (-8192..8191); every negative value - i.e.
every reverse-thrust command - is clamped to 0 before it reaches the motor
output. The flight controller's side is fine: `AP_DroneCAN::scale_esc_output()`
does send a correctly-signed value for a channel configured as a reversible
ESC. The clamp is entirely on this node's receiving end, and it also matches
`rcout_init()` calling `SRV_Channels::set_range()` (0..8191, throttle-style)
rather than `set_angle()` for every Motor-function channel - the whole Motor
output path is built assuming unidirectional ESCs.

**Impact for this vehicle:** `Vectored_6DOF` needs every one of the 8
propulsion thrusters to run in both directions to produce arbitrary
translation + rotation. With this limitation, Node A's propulsion outputs
are forward-only - the vehicle cannot achieve the maneuverability the frame
class assumes. This is a firmware gap, not a parameter that can be worked
around. The pump (M9-M10) and Node B's payload outputs are **not** affected,
since they go through the separate Actuator ArrayCommand ("RCIN") path,
which does preserve full range and sign.

If you want this fixed, it needs a periph-side change (e.g. a per-node
reversible-ESC bitmask param, using `set_angle()` instead of `set_range()`
for the flagged channels, and passing `rc[i]` through unclamped for them) -
ask and I can implement it. Everything below documents the parameters as the
firmware behaves today.

## Architecture: how one CAN bus serves two nodes

The flight controller does not configure "node A" or "node B" separately.
It has one DroneCAN driver instance (`CAN_D1_*`) that broadcasts:

- one `uavcan.equipment.esc.RawCommand` array, built from **its own** local
  SERVOn channels assigned a `Motor` function and selected by `CAN_D1_UC_ESC_BM`
  (array index = channel number - 1 - `CAN_D1_UC_ESC_OF`)
- one `uavcan.equipment.actuator.ArrayCommand` array, from local SERVOn
  channels assigned an `RCINn` function and selected by `CAN_D1_UC_SRV_BM`
  (`actuator_id` = the RCIN number)

Every node on the bus listens to both broadcasts and picks out only the
slots matching **its own** `OUTn_FUNCTION` values. This is why `Motor1` and
`RCIN5`, for example, must each be claimed by exactly one output across the
*entire* vehicle - it's a shared, vehicle-wide numbering space, not per-node.

## Node A - propulsion + pump

Set with the DroneCAN GUI Tool, connected to Node A specifically (check its
node ID in the tool's node list before changing params, so you don't set
these on Node B by mistake).

| Pin | Param | Value | Function |
|---|---|---|---|
| M1 | `OUT1_FUNCTION` | 33 | Motor1 |
| M2 | `OUT2_FUNCTION` | 34 | Motor2 |
| M3 | `OUT3_FUNCTION` | 35 | Motor3 |
| M4 | `OUT4_FUNCTION` | 36 | Motor4 |
| M5 | `OUT5_FUNCTION` | 37 | Motor5 |
| M6 | `OUT6_FUNCTION` | 38 | Motor6 |
| M7 | `OUT7_FUNCTION` | 39 | Motor7 |
| M8 | `OUT8_FUNCTION` | 40 | Motor8 |
| M9 | `OUT9_FUNCTION` | 55 | RCIN5 (pump 1) - already the board default |
| M10 | `OUT10_FUNCTION` | 56 | RCIN6 (pump 2) - already the board default |
| M11 | `OUT11_FUNCTION` | 57 | RCIN7, unused - leave as-is or set `0` |

Only M5-M8 need changing from the board's shipped default (`RCIN1`-`RCIN4`)
to `Motor5`-`Motor8`; M9-M11 are already RCIN by default.

```
param set ESC_PWM_TYPE 1   # 1:Normal PWM - all 8 thrusters are plain PWM
param set OUT_BLH_MASK 0   # BLHeli32 passthrough is DShot-only; these ESCs
                            # can't respond to it, so disable it (the board
                            # default of 15 assumed DShot ESCs on M1-M4)
```

Leaving `OUT_BLH_MASK` at its default wouldn't break anything (a plain-PWM
ESC just won't answer the passthrough probe), but there's no reason to keep
it enabled, and `HAL_WITH_ESC_TELEM`/`ESC_TELEM_PORT` won't produce anything
either - basic PWM ESCs don't report telemetry back.

If the SHT4x sensor is also on this node, its setup is unchanged - see the
main `README.md` in this folder.

## Node B - payload ESCs

Set with the DroneCAN GUI Tool, connected to Node B specifically.

| Pin | Param | Value | Function |
|---|---|---|---|
| M1 | `OUT1_FUNCTION` | 51 | RCIN1 (payload 1) |
| M2 | `OUT2_FUNCTION` | 52 | RCIN2 (payload 2) |
| M3 | `OUT3_FUNCTION` | 53 | RCIN3 (payload 3) |

All three must change from the board's shipped default (`Motor1`-`Motor3`) -
left as-is, this node would fight Node A for control of Motor1-3.
`ESC_PWM_TYPE` doesn't matter here since none of this node's outputs are a
`Motor` function (RCIN-function outputs are always plain PWM regardless of
that param).

## Flight controller (ArduSub) parameters

```
FRAME_CONFIG        2      # Vectored_6DOF; auto-assigns SERVO1-8_FUNCTION
                            # to Motor1-8 (33-40) for the 8 propulsion
                            # thrusters - normally set via the frame/motor
                            # setup wizard in the GCS rather than by hand

SERVO9_FUNCTION      55    # RCIN5 -> Node A pump 1
SERVO10_FUNCTION     56    # RCIN6 -> Node A pump 2
SERVO11_FUNCTION     51    # RCIN1 -> Node B payload 1
SERVO12_FUNCTION     52    # RCIN2 -> Node B payload 2
SERVO13_FUNCTION     53    # RCIN3 -> Node B payload 3
                            # channel numbers 9-13 are arbitrary - any free
                            # SERVOn slot works, it doesn't need a physical
                            # pin on the FC

CAN_P1_DRIVER        1     # physical CAN port -> driver 1 (skip if already set)
CAN_D1_PROTOCOL      1     # DroneCAN (skip if already set)
CAN_D1_UC_ESC_BM     255   # bits 0-7 = Servo1-8 -> broadcast as ESC RawCommand
CAN_D1_UC_ESC_OF     0
CAN_D1_UC_SRV_BM     7936  # bits 8-12 = Servo9-13 -> broadcast as Actuator ArrayCommand

BRD_SAFETY_DEFLT     0     # boot with safety off; both CAN nodes hold their
                            # PWM outputs disabled until they see SAFETY_OFF
                            # broadcast from the FC (see below), and this is
                            # one flight-controller-wide setting, not per-node
```

`CAN_D1_UC_ESC_BM = 255` = `0b11111111` (bits 0-7). `CAN_D1_UC_SRV_BM = 7936`
= `0b1111100000000` (bits 8-12). Recompute these if you use different
`SERVOn` slot numbers.

Both nodes get their DroneCAN node ID automatically via dynamic allocation
(no `CAN_D1_UC_NODE`-equivalent param to set per node) - just check the
DroneCAN GUI Tool's node list to tell them apart before setting per-node
params above.

## Companion computer control (pump + payloads)

`SERVO9`-`SERVO13` are `RCINn` functions specifically so the companion
computer can drive them directly over MAVLink with **`MAV_CMD_DO_SET_SERVO`**
(`servo1_raw` = target `SERVOn` number, e.g. `9` for the first pump;
`servo2_raw` = PWM in microseconds, 1000-2000, 1500 = stopped for anything
bidirectional). This is the right tool for this: ArduPilot's `DO_SET_SERVO`
handler explicitly only accepts `RCIN1`-`RCIN16` (and a few fixed functions
like gripper/sprayer) - it outright refuses any channel assigned a `Motor`
function - so this also could not be used for the propulsion thrusters even
once reversible-ESC support exists there.

`RC_CHANNELS_OVERRIDE` is the alternative if you want continuous/streamed
control instead of discrete commands - it injects values into the RC input
array, which `RCINn` pass-through mirrors on output, but it comes with RC
failsafe/override-timeout semantics that `DO_SET_SERVO` doesn't, so
`DO_SET_SERVO` is the simpler fit for a companion computer here.

One relevant behavior difference from propulsion: `Motor`-function outputs
are automatically zeroed by the flight controller when disarmed (or when
safety is on); `RCINn`/Actuator outputs are only gated by the safety switch,
not by arm state - so the pump and payloads stay controllable by the
companion computer even while the vehicle is disarmed.
