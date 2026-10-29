# ACE Mega

1 kg sumobot with an aluminum chassis and DIY silicone wheels.

## Specs

| | |
|---|---|
| Weight | 1 kg |
| Chassis | Aluminum |
| Controller | Cytron Maker RP2040 |
| Motor drivers | 2x BTS7960 |
| Battery | 11.1 V LiPo (3S) |
| Protection | 15 A automotive fuse, main switch |
| Opponent sensors | 5x SHARP IR: 3 in front, 1 on each side |
| Edge sensors | 2x IR line sensors |
| Start | JSumo start module |
| Wireless | Bluetooth module |
| Logic power | LM2596 buck converter |
| Motors | (fill in) |
| Wheels | DIY silicone |

## Why an aluminum chassis

- More weight and strength than a printed chassis.
- A chance to experiment with a new material.
- The aluminum was already in the attic.

## What was hard

Everything about it was hard, including working the aluminum with a heavy-duty drill.

## What went wrong

**Wheels.** The first blue silicone wheel (attempt 1) had bubbles and no traction.

**LiPo.** The battery smoked once and its black cover ripped open. Fix: a 15A fuse in line, on every LiPo build. Add the fuse before the first power-on of any new robot.

## A bit of math

**Push force.** At 1 kg the traction limit is `F_max = mu * m * g = 9.8 N * mu`. Past that point the wheels slip, so wheel grip decides pushing matches as much as motor power. Measure `mu` on the dohyo. See [`traction_and_speed.md`](../docs/traction_and_speed.md).

**Three front sensors.** An opponent of width `w` can sit between two front beams `alpha` apart when it is farther than `d_blind = w / tan(alpha)`. The 3 front sensors can be checked against this: for the opponent widths you expect, work out `d_blind` from the beam angle and make sure it is beyond the range that matters. See [`sharp_ir_distance.md`](../docs/sharp_ir_distance.md).

**Fuse and stall current.** In a hard push both sides stall, and the current is `I_stall = V / R_winding` for each motor, added together. That total has to stay under the BTS7960 and wiring limits, and the 15 A fuse has to sit above the running current so it doesn't trip mid-match. See [`power_budget.md`](../docs/power_budget.md).

**Buck converter heat.** Logic at 5 V and 1 A through the LM2596 at `eta = 0.8` loses `5 * (1/0.8 - 1) = 1.25 W`, against 6.1 W for a linear regulator from 11.1 V.

## Firmware

`firmware/ace_mega/ace_mega.ino` runs the match logic: avoid the edge first, then charge the opponent (faster as the reading rises), otherwise open with the chosen strategy and search. Bluetooth selects the strategy (`0` spin, `1` charge, `2` arc right) and `r` prints the sensor readings while waiting for the start module. The pins and thresholds are placeholders.

**ADC limit.** The RP2040 has only 3 usable analog inputs, and the Mega has 5 SHARP sensors. The sketch reads the 3 front sensors on GP26 to GP28 and leaves the 2 side sensors as a stub (`readExtraSharp()`, reads 0) until you add an I2C ADC (ADS1115), an analog multiplexer, or digital SHARP modules.

## Still to add

- `cad/` with the chassis drawings
- `photos/`
- Motor model, and the measured `mu` and sensor calibration tables from the docs
