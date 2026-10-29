# ACE Mega

**A 1 kg autonomous sumobot built around aluminum, high-current motor control, and the pursuit of better pushing performance.**

ACE Mega is part of the ACE competitive robotics lineup. It pairs a custom aluminum chassis with DIY silicone wheels, a Cytron Maker RP2040 controller, and dual BTS7960 motor drivers.

Unlike ACE Micro's compact 3D-printed construction, ACE Mega explores a different approach to robot design: working with metal, managing higher-current drive electronics, and investigating how chassis construction and wheel traction affect performance in the dohyo.

The project wasn't easy. From machining aluminum with a heavy-duty drill to troubleshooting failed wheel casts and power-system problems, every stage introduced a new engineering challenge.

Got any questions? Get in touch with me:
Email: daphneuniceacena@gmail.com
Instagram: @d_4cena

---

## Overview

| | |
|---|---|
| **Project** | ACE Mega |
| **Category** | Autonomous sumobot |
| **Weight class** | 1 kg |
| **Chassis** | Aluminum |
| **Controller** | Cytron Maker RP2040 |
| **Design focus** | Traction, strength, and power delivery |

## Hardware Specifications

| Component | Specification |
|:--|:--|
| Microcontroller | Cytron Maker RP2040 |
| Motor drivers | 2 × BTS7960 modules |
| Drive motors | To be documented |
| Battery | 11.1 V LiPo, 3S |
| Main protection | 15 A automotive fuse |
| Main power control | Power switch |
| Opponent detection | 5 × Sharp IR sensors |
| Front sensors | 3 × Sharp IR |
| Side sensors | 2 × Sharp IR, one per side |
| Edge detection | 2 × IR line sensors |
| Start module | JSumo start module |
| Wireless interface | Bluetooth module |
| Logic power | LM2596 buck converter |
| Wheels | DIY silicone |
| Target weight | 1 kg |

*Motor specifications, chassis dimensions, and measured performance will be added as testing progresses.*

## Chassis Design: Why Aluminum?

ACE Mega uses aluminum instead of a 3D-printed chassis. The decision came down to three things:

- **Structure:** A metal chassis offers a different balance of stiffness, strength, and durability than a printed plastic frame.
- **Experimentation:** Working with aluminum was an opportunity to develop fabrication skills and explore another approach to robot construction.
- **Available materials:** The aluminum was already sitting in the attic. Sometimes, engineering starts with what you have.

The trade-off was fabrication difficulty. Drilling, aligning, and working with the aluminum required more effort than printing a chassis. The experience made material selection feel less like a specification on paper and more like a decision that affects the entire build process.

## Build Challenges

### 1. Aluminum Fabrication

Working with aluminum using a heavy-duty drill was one of the most challenging parts of the project. Compared with 3D printing, metal fabrication requires careful attention to hole placement, alignment, and the physical work involved in producing the chassis.

The experience highlighted an important lesson: choosing a material means committing to the manufacturing process that comes with it.

### 2. Wheel Casting

The first blue silicone wheel was unsuccessful. Air bubbles formed in the cast, and the resulting wheel had poor traction.

This introduced two problems to investigate: casting quality and the relationship between the wheel material and the competition surface.

Future iterations will compare wheel consistency and measured grip rather than relying on appearance alone.

### 3. Battery and Power-System Failure

During development, the LiPo battery smoked, and its outer black covering tore open. This was a serious power-system failure, not merely a component inconvenience.

**Safety lesson:** A fuse is an important part of battery-system protection, but it is not a guarantee against every battery failure. A fuse must be correctly selected and installed, and it cannot replace proper wiring, insulation, charging practices, or battery inspection.

Before powering a new robot:

- Inspect the battery, connectors, insulation, and wiring.
- Verify power polarity and the intended current paths.
- Check the ratings of the battery, motor drivers, wiring, and protection devices.
- Confirm that the fuse is appropriately rated for the circuit and its wiring.
- Keep damaged or swollen batteries out of service and follow appropriate battery-safety procedures.

The 15 A automotive fuse is part of ACE Mega's power-distribution design. Its suitability must be verified against the actual circuit and component ratings; it should not be assumed to make every fault safe.

## Engineering Notes

### 1. Traction and Pushing Force

The maximum theoretical friction force available at the wheels can be approximated by

\[
F_{\max}=\mu mg
\]

For a 1 kg robot:

\[
F_{\max}=\mu(1)(9.81)
=9.81\mu\text{ N}
\]

where:

- \(F_{\max}\) is the theoretical traction limit.
- \(\mu\) is the effective coefficient of friction.
- \(m\) is the robot's mass in kilograms.
- \(g\) is gravitational acceleration.

This means a 1 kg robot has a theoretical traction limit of approximately \(9.81\mu\) newtons under the simplified assumptions.

Actual pushing performance also depends on weight distribution, the number of driven wheels, motor torque, drivetrain losses, and the competition surface.

**Experiment:** Measure the grip of the DIY silicone wheels on the intended dohyo surface, then compare it with observed pushing performance.

See [`docs/traction_and_speed.md`](docs/traction_and_speed.md).

### 2. Front Sensor Coverage

ACE Mega uses three forward-facing Sharp IR sensors and one sensor on each side. The three front sensors are intended to improve opponent coverage during a match.

Sensor spacing matters. An opponent may fall between the effective detection regions of adjacent sensors, particularly at greater distances.

A simplified geometric estimate for the distance beyond which an opponent of width \(w\) could fit between two idealized sensor beams separated by angle \(\alpha\) is

\[
d_{\text{blind}}\approx\frac{w}{\tan(\alpha)}
\]

This relationship assumes a particular beam geometry and is not a complete model of Sharp IR detection. Real sensors have finite fields of view, nonlinear distance responses, and detection thresholds.

The useful next step is to map the actual detection regions of all three front sensors and identify gaps for representative opponent widths and distances.

See [`docs/sharp_ir_distance.md`](docs/sharp_ir_distance.md).

### 3. Motor Drivers, Stall Current, and Protection

ACE Mega uses two BTS7960 motor-driver modules to control its drive motors.

During a hard push, a motor may approach stall, causing current to rise significantly. A simplified winding-current estimate is

\[
I_{\text{stall}}\approx\frac{V}{R_{\text{winding}}}
\]

For two motors, their currents must be considered alongside the driver's actual current capability, supply behavior, wiring, connectors, and thermal limits.

The 15 A fuse must be evaluated as part of the complete protection system. Its rating alone does not establish that the motors, driver modules, or wiring are adequately protected. Fuse selection depends on the circuit, expected operating current, wire ampacity, fault conditions, and the fuse's time-current characteristics.

See [`docs/power_budget.md`](docs/power_budget.md).

### 4. Buck Converter Efficiency

The LM2596 buck converter supplies the robot's regulated logic power from the battery.

For a hypothetical 5 V, 1 A load, the output power is

\[
P_{\text{out}}=VI=5(1)=5\text{ W}
\]

Assuming 80% conversion efficiency:

\[
P_{\text{loss}}
=P_{\text{out}}\left(\frac{1}{\eta}-1\right)
\]

\[
P_{\text{loss}}
=5\left(\frac{1}{0.8}-1\right)
=1.25\text{ W}
\]

For comparison, an idealized linear regulator supplying the same load from 11.1 V would dissipate approximately

\[
P_{\text{loss}}=(11.1-5)(1)=6.1\text{ W}
\]

These figures are illustrative estimates, not measured ACE Mega power consumption. Actual buck-converter efficiency depends on input voltage, output current, module design, and operating conditions.

## Firmware

The main Arduino sketch is located at:

[`firmware/ace_mega/ace_mega.ino`](firmware/ace_mega/ace_mega.ino)

The firmware follows a priority-based match strategy:

1. **Edge avoidance:** React to boundary detection before other behaviors.
2. **Opponent engagement:** Charge toward a detected opponent.
3. **Search behavior:** Execute the selected strategy when no opponent is detected.

### Strategy Selection

| Command | Strategy |
|:--|:--|
| `0` | Spin |
| `1` | Charge |
| `2` | Arc right |
| `r` | Print sensor readings while waiting for the start module |

The current pin assignments and thresholds are placeholders and must be verified against the physical build and calibrated sensor readings.

### Analog Input Limitation

The RP2040 provides three external ADC inputs on the standard RP2040 pinout, while ACE Mega has five analog Sharp IR sensors.

The current sketch reads the three front sensors using GP26, GP27, and GP28. The two side sensors are not yet implemented: `readExtraSharp()` currently returns zero.

Possible hardware approaches include:

- **External I²C ADC:** An ADS1115 can provide additional analog inputs, subject to its voltage range and interface requirements.
- **Analog multiplexer:** A suitable multiplexer can route several sensor outputs to fewer ADC inputs, with additional switching and settling considerations.
- **Digital-output sensors:** Compatible sensor modules can reduce demand for analog inputs, depending on the available detection and calibration features.

The selected approach must be integrated into both the wiring and firmware before the side sensors can contribute to match decisions.

## Repository Structure

```text
ACE-Mega/
├── firmware/
│   └── ace_mega/
│       └── ace_mega.ino
├── docs/
│   ├── traction_and_speed.md
│   ├── power_budget.md
│   └── sharp_ir_distance.md
├── cad/                  # Planned: chassis drawings and CAD files
├── photos/               # Planned: build and testing photographs
└── README.md
```


---

*ACE Mega — part of the ACE competitive robotics lineup.*
