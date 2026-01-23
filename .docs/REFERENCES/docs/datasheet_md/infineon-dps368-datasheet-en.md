DPS368
Digital XENSIVTM barometric pressure sensor

Product Description

The DPS368 is a miniaturized Digital
Barometric Air Pressure Sensor robust
against water, dust and humidity. It offers a
high accuracy and a low current
consumption, capable of measuring both
pressure and temperature. The pressure
sensor element is based on a capacitive
sensing principle which guarantees high
precision during temperature changes. The
small package makes the DPS368 ideal for
mobile applications and wearable devices.
Due to its robustness it can be used in harsh
environment.

The internal signal processor converts the output from the pressure and temperature sensor elements to 24 bit
results. Each unit is individually calibrated, the calibration coefficients calculated during this process are stored
in the calibration registers. The coefficients are used in the application to convert the measurement results to
high accuracy pressure and temperature values.
The result FIFO can store up to 32 measurement results, allowing for a reduced host processor polling rate.
Sensor measurements and calibration coefficients are available through the serial I²C or SPI interface. The
measurement status is indicated by status bits or interrupts on the SDO pin.

Features

Pressure sensor with environmentally resistant package
Operation range: Pressure: 300 –1200 hPa. Temperature: -40 – 85 °C.
Pressure sensor precision: ± 0.002 hPa (or ±0.02 m) (high precision mode).
Relative accuracy: ± 0.06 hPa (or ±0.5 m)
Absolute accuracy: ± 1 hPa (or ±8 m)
IPx8 certified: Temporary immersion of 50m for 1 hour
Temperature accuracy: ± 0.5°C.
Pressure temperature sensitivity: 0.5Pa/K

•
•
•
•
•
•
•
•
• Measurement time: Typical 27.6 ms for standard mode (16x). Minimum: 3.6 ms for low precision mode.
Average current consumption: 1.7 µA for pressure measurement, 1.5 µA for temperature measurement
•
@1Hz sampling rate, standby: 0.5 µA.
Supply voltage: VDDIO: 1.2 – 3.6 V, VDD: 1.7 – 3.6 V.
Operating modes: Command (manual), Background (automatic), and Standby.
Calibration: Individually calibrated with coefficients for measurement correction.
FIFO: Stores up to 32 pressure or temperature measurements.
Interface: I2C and SPI (both with optional interrupt)
Package dimensions: 8-pin PG-VLGA-8-2 , 2.0 mm x 2.5 mm x 1.1 mm.
Green Product (RoHS) Compliant

•
•
•
•
•
•
•

Datasheet
www.infineon.com

Please read the Important Notice and Warnings at the end of this document

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Typical applications

Typical applications

•
•
•
•

Smart watches, wearable & smart phone ( e.g. altitude, fitness tracking, step counting, fall detection)
Home appliances (e.g. air flow control in HVAC / vacuum cleaner, water level detection, intruder detection)
Drones ( e.g. flight stability, height control)
Health care(e.g. fall detection, air flow monitoring, smart inhaler)

Product Validation

Qualified for industrial applications according to the relevant tests of JEDEC47/20/22.

Datasheet

2

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Table of contents

Table of contents

Product Description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1

Features . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 1

Typical applications . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 2

Product Validation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .2

Table of contents . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 3

Definitions, acronyms and abbreviations . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5

Definitions . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5

Pin Configuration and Block Diagram . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6

Pin Configuration and Description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .6

Block Diagram . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7

Specifications . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7

Operating Range . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7

Absolute Maximum Ratings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .8

Current Consumption . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8

Temperature Transfer Function . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 9

Pressure Transfer Function . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10

Timing Characteristics . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11

Functional Description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .11

Operating Modes . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11

Mode transition diagram . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11

Start-up sequence . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12

Measurement Precision and Rate . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12

Sensor Interface . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13

Interrupt . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13

Result Register Operation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

FIFO Operation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

Calibration and Measurement Compensation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14

How to Calculate Compensated Pressure Values . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

How to Calculate Compensated Temperature Values . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15

Compensation Scale Factors . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16

Pressure and Temperature calculation flow . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16

Applications . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .18

Measurement Settings and Use Case Examples . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18

Application Circuit Example . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19

IIR filtering . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .20

Digital interfaces . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20

1

1.1

2

2.1

2.2

3

3.1

3.2

3.3

3.4

3.5

3.6

4

4.1

4.2

4.3

4.4

4.5

4.6

4.7

4.8

4.9

4.9.1

4.9.2

4.9.3

4.9.4

5

5.1

5.2

5.3

6

Datasheet

3

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Table of contents

6.1

6.2

6.3

6.3.1

6.3.1.1

6.3.1.2

I2C Interface . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21

SPI Interface . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21

Interface parameters specification . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 22

General interface parameters  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23

I2C timings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23

SPI timings . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25

7

8

8.1

8.1.1

8.1.2

8.1.3

8.2

8.2.1

8.2.2

8.2.3

8.3

8.4

8.5

8.6

8.7

8.8

8.9

8.10

8.11

8.12

9

9.1

10

11

12

Register Map  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .26

Register description . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26

Pressure Data (PRS_Bn) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26

PRS_B2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27

PRS_B1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27

PRS_B0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28

Temperature Data (TMP_Tn) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28

TMP_B2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 28

TMP_B1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29

TMP_B0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29

Pressure Configuration (PRS_CFG) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30

Temperature Configuration(TMP_CFG) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 32

Sensor Operating Mode and Status (MEAS_CFG) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 33

Interrupt and FIFO configuration (CFG_REG) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 34

Interrupt Status (INT_STS) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .35

FIFO Status (FIFO_STS) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .36

Soft Reset and FIFO flush (RESET) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 37

Product and Revision ID (ID) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 37

Calibration Coefficients (COEF) . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 38

Coefficient Source . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 39

Package Dimensions . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 39

Package drawing . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 39

Footprint and stencil recommendation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 40

Reflow soldering and board assembly . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 41

Package Handling . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43

Revision history . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43

Disclaimer . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 44

Datasheet

4

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

1 Definitions, acronyms and abbreviations

1

Definitions, acronyms and abbreviations

1.1
An explanation of terms and definitions used in this datasheet.

Definitions

Table 1

Term

Absolute accuracy

Digital bit depth

Digital resolution

Definition/explanation

The absolute measurement accuracy over the entire measurement
range.

The total bit depth used for conversion of the sensor input to the
digital output. Measured in bits.

The pressure value represented by the LSB change in output. This
value should be much smaller than the sensor noise.

Full Scale Range (FSR)

The peak-to-peak measurement range of the sensor.

LSB

Measurement time

MSB

Non-linearity

Output compensation

Precision (noise)

Pressure temperature coefficient

Sensor calibration

Least Significant Bit

The time required to acquire one sensor output result. This value
determines the maximum measurement rate.

Most Significant Bit

The deviation of measured output from the best-fit straight line,
relative to 1000 hPa and 25 °C.

The process of applying an algorithm to the sensor output to improve
the absolute accuracy of the sensor across temperature and to
minimize unit to unit output variation. This algorithm makes use of
both the temperature sensor readings and the individual calibration
coefficients.

The smallest measurable change, expressed as rms, after sensor
oversampling.

The pressure measurement deviation, after compensation, from
expected measurement value due to temperature change from 25 °C.
Measured in Pa/K.

The process, during the production test, where the sensor's
measurement results are compared against reference values, and a set
of calibration coefficients are calculated from the deviation. The
coefficients are stored in the sensor's memory and are used in the
output compensation.

Sensor oversampling rate (OSR)

Specifies the number of sensor measurements used internally to
generate one sensor output result.

Datasheet

5

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

2 Pin Configuration and Block Diagram

2

2.1

Pin Configuration and Block Diagram

Pin Configuration and Description

Figure 1

Pin configuration (top view, figure not to scale)

Table 2

Pin description

Pin

Name

SPI 3-wire

SPI 3-wire with
interrupt

SPI 4-wire

I2C

I2C with
interrupt

1

2

3

4

5

6

7

8

GND

CSB

Chip select - active
low

Chip select -
active low

Ground

Chip select -
active low

Not used - open
(internal pull-up)
or tie to VDDIO

SDI

Serial data in/out

Serial data in/out Serial data in

Serial data in/out

SCK

SDO

VDDIO

GND

VDD

Not used

Interrupt

Serial data out

Serial Clock

Least significant
bit in the device
address.

Digital supply voltage for digital blocks and I/O interface

Ground

Supply voltage for analog blocks

Not used - open
(internal pull-
up) or tie to
VDDIO

Serial data
in/out

Interrupt pin
and least
significant bit in
the device
address.

Datasheet

6

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

3 Specifications

2.2

Block Diagram

Figure 2

3

Specifications

Operating Range

3.1
The following operating conditions must not be exceeded in order to ensure correct operation of the device. All
parameters specified in the following sections refer to these operating conditions, unless noted otherwise.

Table 3

Operating Range

Parameter

Symbol

Values

Unit

Note / Test Condition

Pressure

Temperature

Supply voltage

Supply voltage IO

Supply voltage ramp-up time

Min.

300

-40

1.7

1.2

0.001

Pa
Ta
VDD
VDDIO
tvddup

Solder drift1)

Long term stability

Typ.

0.8

±1

Max.

1200

85

3.6

3.6

5

hPa

°C

V

V

ms

hPa

hPa

Time for supply voltage
to reach 90% of final
value.

Minimum solder height
50um.

Depending on
environmental
conditions.

1

Effects of solder drift can be eliminated by one point calibration. See AN487.

Datasheet

7

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

3 Specifications

Absolute Maximum Ratings

3.2
Maximum ratings are absolute ratings. Exceeding any one of these values may cause irreversible damage to the
integrated circuit.

Attention: Stresses above the values listed as "Absolute Maximum Ratings" may cause permanent damage
to the devices. Exposure to absolute maximum rating conditions for extended periods may affect
device reliability.

Table 4

Absolute Maximum Ratings

Parameter

Symbol

Values

Min.

Typ. Max.

Unit Note / Test
Condition

VDD and VDDIO
Voltage on any pin

Storage temperature

Pressure

ESD

VDDxx_max
Vmax
Ts
Pmax
VESD_HBM

-40

-2

4

4

125

10,000

2

V

V

°C

hPa

KV

HBM (JS001)

Current Consumption

3.3
Test conditions (unless otherwise specified in the table): VDD= 1.8V and VDDIO=1.8V. Typ. values (PA=1000hPa and
TA=25°C). Max./Min. values (PA= 950-1050hPa and TA=0...+65°C).

Table 5

Current Consumption

Parameter

Symbol

Values

Unit Note / Test Condition

Peak Current Consumption

Ipeak

Min.

Standby Current Consumption

Current Consumption.
( 1 pressure and temperature
measurements per second.)

Istb
I1Hz

Max.

Typ.

345

280

0.5

2.1

11

38

µA

µA

µA

µA

during Pressure
measurement

during Temperature
measurement

Low precision

Standard precision

High precision

Note: The current consumption depends on both pressure measurement precision and rate. Please refer to the
Pressure Configuration (PRS_CFG) register description for an overview of the current consumption in different
combinations of measurement precision and rate.

Datasheet

8

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

3 Specifications

Temperature Transfer Function

3.4
Test conditions (unless otherwise specified in the table): VDD= 1.8V and VDDIO=1.8V. Typ. values (PA=1000hPa and
TA=25°C). Max./Min. values (PA= 950-1050hPa and TA=0...+65°C).

Table 6

Temperature Transfer Function

Parameter

Symbol

Values

Unit Note / Test Condition

Temperature accuracy

Temperature data resolution

At
At_res

Min.

Max.

Typ.

+/-0.5

0.01

Temperature measurement rate f

1

128

°C

°C

Hz

Datasheet

9

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

3 Specifications

Pressure Transfer Function

3.5
Test conditions (unless otherwise specified in the table): VDD= 1.8V and VDDIO=1.8V. Typ. values (PA=1000hPa and
TA=25°C).

Table 7

Pressure Transfer Function

Parameter

Symbol

Values

Unit

Note / Test Condition

Absolute pressure accuracy

Ap_abs

+/-100

Min.

Typ.

Max.

Relative pressure accuracy

Ap_rel

Pressure precision

Ap_prc

+/-6

1.0

0.35

0.2

Pa

Pa

PaRMS

Pa=300 - 1200hPa;
TA=0..+65°C; Excluding
solder effects

Any Δ1hPa in the range
Pa=800 - 1200hP; Any
constant temperature in
the range TA=20..+60°C
Low Power

Standard

High Precision

Note:

Pressure precision is measured as the average standard deviation. Please refer to the Pressure
Configuration (PRS_CFG) register description for all precision mode options.

Power supply rejection

Ap_psr

0.063

PaRMS

Pressure temperature sensitivity
of calibrated measurements

Ap_tmp

Pressure data resolution

Pressure measurement rate

Ap_res
f

1

Pressure measurement time

t

Pa/K

PaRMS
Hz

ms

0.06

128

0.5

5.2

27.6

105

Measured with 217Hz
square wave and broad
band noise, 100mVpp
1000hPa, 25...+65°C.

Low Power

Standard

High Precision

Note:

The pressure measurement time (and thus the maximum rate) depends on the pressure
measurement precision. Please refer to the Pressure Configuration (PRS_CFG) register description
for an overview of the possible combinations of measurement precision and rate.

Datasheet

10

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

3.6

Timing Characteristics

Table 8

Timing Characteristics

Parameter

Symbol

Values

Unit Note / Test Condition

Start-up timing

Time to sensor ready

TSensor_rdy

12

ms

Min.

Typ.

Max.

Time to coefficients are
available.

TCoef_rdy

40

ms

The SENSOR_RDY bit in the
Measurement Configuration
register will be set when the
sensor is ready.

The COEF_RDY bit in the
Measurement Configuration
register will be set when the
coefficients can be read out.

Note:

Start-up timing is measured from VDD > 1.2V & VDDIO > 0.6V or Soft Reset.

I2C Clock.

SPI Clock

fI2C
fSPI

3.4

10

MHz

MHz

4

Functional Description

4.1
The supports 3 different modes of operation: Standby, Command, and Background mode.

Operating Modes

Default mode after power on or reset. No measurements are performed.
All registers and compensation coefficients are accessible.

One temperature or pressure measurement is performed according to the selected precision.
The sensor will return to Standby Mode when the measurement is finished, and the measurement
result will be available in the data registers.

•

•

•

Standby Mode
-
-
Command Mode
-
-

Background Mode
-

Pressure and/or temperature measurements are performed continuously according to the selected
measurement precision and rate. The temperature measurement is performed immediately after the
pressure measurement.
The FIFO can be used to store 32 measurement results and minimize the number of times the sensor
must be accessed to read out the results.

Operation mode and measurement type are set in the Sensor Operating Mode and Status
(MEAS_CFG) register.

-

Note:

4.2
The mode transition diagram is shown below.

Mode transition diagram

Datasheet

11

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

Figure 3

Mode transition diagram

Start-up sequence

4.3
The start-up sequence is shown below. This diagram shows when the registers are accessible for read and/or
write and also when the Pressure/Temperature measurements can start.

Figure 4

Start-up sequence

Measurement Precision and Rate

4.4
Different applications require different measurement precision and measurement rates. Some applications,
such as weather stations, require lower precision and measurement rates than for instance indoor navigation
and sports applications.

When the sensor is in Background Mode, the measurement precision and rate can be configured to match the
requirements of the application. This reduces current consumption of the sensor and the system.

Datasheet

12

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

In order to achieve a higher precision, the sensor will read multiple times ( oversampling ), and combine the
readings into one result. This increases the current consumption and also the measurement time, reducing the
maximum possible measurement rate. It is necessary to balance the accuracy and data rate required for each
application with the allowable current consumption.
The measurement precision, rate and time is set in the Pressure Configuration (PRS_CFG) and Temperature
Configuration(TMP_CFG) registers. The register descriptions contain information about the current
consumption and the possible combinations of measurement precision, time, and rate.
Enabling temperature measurements allows for compensation of temperature drift in the pressure
measurement. The rates of these measurements can be set independently, but temperature compensation is
more accurate when temperature and pressure measurements are taken together. This reduces the maximum
pressure measurement rate, since: Ratetemperature*Timetemperature + Ratepressure*Timepressure< 1 second.
Measurement Settings and Use Case Examples contains a table with examples of combinations of pressure
and temperature precision and rates for different use cases.
In the figure below is described the Temperature and Pressure measurements sequence in background mode.

Figure 5

Background mode temperature and pressure measurements sequence

Sensor Interface

4.5
The DPS368 can be accessed as a slave device through mode '11' SPI 3-wire, SPI 4-wire, or I2C serial interface.
•

The sensor's default interface.
The sensor's address is 0x77 (default) or 0x76 (if the SDO pin is pulled-down to GND).

I2C interface
-
-
SPI interface
-

•

The sensor will switch to SPI configuration if it detects an active low on the CSB pin. SPI 4-wire is the
default SPI interface.
To enable SPI 3-wire configuration, a bit must be set in the Interrupt and FIFO configuration
(CFG_REG) register after start up.

-

More details about digital interfaces are available in the Digital interfaces.

Interrupt

4.6
The sensor can generate an interrupt when a new measurement result is available and/or when the FIFO is full.
The sensor uses the SDO pin for the interrupt signal, and interrupt is therefore not supported if the interface is 4-
wire SPI.
The interrupt is enabled and configured in the Interrupt and FIFO configuration (CFG_REG) register. In I2C
configuration the SDO pin serves as both interrupt and as the least significant bit in the device address. If the
SDO pin is pulled low the interrupt polarity must be set to active high and vice-versa.
The interrupt status can be read from the Interrupt Status (INT_STS) register.

Datasheet

13

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

Result Register Operation

4.7
After starting the measurements, the latest pressure and temperature raw data will be available in their
respective result registers. Temperature measurement can be skipped. The temperature measurements can be
disabled if there is a requirement to measure pressure rapidly, but it will make accurate temperature drift
compensation impossible.
All measurement data can be read in a single command using auto-increment read. When FIFO is disabled,
reading the result register will not affect the register value, it will only be updated when a new measurement is
completed. When FIFO is enabled, the pressure result register will update to the next value in the FIFO after each
read. When all of the FIFO values have been read, the result register will be set to 0x800000.

FIFO Operation

4.8
The FIFO can store the last 32 measurements of pressure or temperature. This reduces the overall system power
consumption as the host processor does not need to continuously poll data from the sensor but can go into
standby mode for longer periods of time.

The FIFO can store any combination of pressure and temperature results, according to the background mode
measurement rate settings. The pressure rate can for instance be set 4 times higher than the temperature rate
and thus only every fifth result will be a temperature result. The measurement type can be seen in the result
data. The sensor will set the least significant bit to:
'1' if the result is a pressure measurement.
•
'0' if it is a temperature measurement.
•
-

The sensor uses 24 bits to store the measurement result. Because this is more bits than is needed to
cover the full dynamic range of the pressure sensor, using the least significant bit to label the
measurement type will not affect the precision of the result.

The FIFO can be enabled in the Interrupt and FIFO configuration register. The data from the FIFO is read out
from the Pressure Data (PRS_Bn) registers regardless of whether the next result in the FIFO is a temperature or a
pressure measurement.
When a measurement has been read out, the FIFO will auto increment and place the next result in the data
register. A flag will be set in the FIFO Status register when the FIFO is empty and all following reads will return
0x800000.
If the FIFO is full, the FIFO_FULL bit in the FIFO Status (FIFO_STS)will be set. If the INT_FIFO bit in the Interrupt
and FIFO configuration register ( CFG_REG) is set, an interrupt will also be generated when the FIFO is full.
The FIFO will stop recording measurements results when it is full.

Calibration and Measurement Compensation

4.9
The sensor is a calibrated sensor and contains calibration coefficients. These are used in the application (for
instance by the host processor) to compensate the measurement results for sensor non-linearities.

The sections that follow describe how to calculate the compensated results and convert them into Pa and °C
values.

Datasheet

14

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

4.9.1

Steps

1.

How to Calculate Compensated Pressure Values

Read the pressure calibration coefficients (c00, c10, c20, c30, c01, c11, and c21) from the Calibration
Coefficient register.

Note:

The coefficients read from the coefficient register are 2's complement numbers.

2.

3.

Choose scaling factors kT (for temperature) and kP (for pressure) based on the chosen precision rate. The
scaling factors are listed in Table 9.
Read the pressure and temperature result from the registers or FIFO.

Note:

The measurements read from the result registers (or FIFO) are 24 bit 2´s complement numbers.
Depending on the chosen measurement rates, the temperature may not have been measured
since the last pressure measurement.

4.

Calculate scaled measurement results.

Traw_sc = Traw/kT
Praw_sc = Praw/kP

5.

Calculate compensated measurement results.

Pcomp(Pa) = c00  + Praw_sc*(c10 + Praw_sc *(c20+ Praw_sc *c30)) + Traw_sc *c01 +
Traw_sc *Praw_sc *(c11+Praw_sc*c21)

4.9.2

Steps

1.

How to Calculate Compensated Temperature Values

Read the temperature calibration coefficients ( c0 and c1 ) from the Calibration Coefficients (COEF)
register.

Note:

The coefficients read from the coefficient register are 12 bit 2´s complement numbers.

2.

3.

Choose scaling factor kT (for temperature) based on the chosen precision rate. The scaling factors are
listed in Table 9.
Read the temperature result from the temperature register or FIFO.

Note:

The temperature measurements read from the temperature result register (or FIFO) are 24 bit 2
´s complement numbers.

Datasheet

15

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

4.

Calculate scaled measurement results.

Traw_sc = Traw/kT

5.

Calculate compensated measurement results.

Tcomp (°C) = c0*0.5 + c1*Traw_sc

4.9.3

Compensation Scale Factors

Table 9

Compensation Scale Factors

Oversampling Rate

Scale Factor (kP or kT)

1 (single)

2 times (Low Power)

4 times

8 times

16 times (Standard)

32 times

524288

1572864

3670016

7864320

253952

516096

64 times (High Precision)

1040384

128 times

2088960

Result shift ( bit 2and 3 address
0x09)

0

0

0

0

enable pressure or temperature
shift

enable pressure or temperature
shift

enable pressure or temperature
shift

enable pressure or temperature
shift

4.9.4
The flow chart below describes the Pressure and Temperature calculate

Pressure and Temperature calculation flow

Figure 6

Pressure and temperature calculation flow

Datasheet

16

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

4 Functional Description

See also How to Calculate Compensated Pressure Values and How to Calculate Compensated Temperature
Values

Datasheet

17

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

5 Applications

5

5.1

Table 10

Use Case

Applications

Measurement Settings and Use Case Examples

Measurement Settings and Use Case Examples (TBD)

Weather Station (Low power)

Indoor navigation (Standard
precision, background mode)

Performance

5 Pa precision.
1 pr sec.
3 uA

10 cm precision.
2 pr sec.
22 uA

Pressure Register
Configuration
Address: 0x06

Temperature
Register
Configuration
Address: 0x07

0x01

0x80

0x14

0x90

Sports (High precision, high
rate, background mode)

5 cm precision
4 pr sec.
200 uA

0x26

0xA0

Other

Start
background
measurements
(addr 0x08)

Enable P shift
(addr 0x09)
Start
background
measurements
(addr 0x08)

Enable P shift
(addr 0x09)
Start
background
measurements
(addr 0x08)

Datasheet

18

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

5 Applications

Application Circuit Example

5.2
The examples application circuit uses the I2C and SPI serial interface. The SDO pin can be used for interrupt or
to set least significant bit of the device address. The DPS368 analog core supply voltage is internally regulated,
guaranteeing robustness to external VDD supply variations within the specified range. The simplest voltage
supply solution is to connect VDD and VDDIO to 1.8V supply and add a suitable decoupling capacitor to reduce
VDD ripple below 50mVpp.

Figure 7

Application Circuit Example using the I2C serial interface.

Figure 8

Application Circuit Example using the SPI 4-wires serial interface

Figure 9

Application Circuit Example using the SPI 3-wire serial interface

Datasheet

19

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

Figure 10

Application Circuit Example using the SPI 3-wire with interrupt interface

Table 11

Component Values

Component

Symbol

Values

Unit Note / Test Condition

Min.

Typ.

Max.

Pull-up/down Resistor

R1, R2
R3

Supply Blocking Capacitor

C1, C2

100

100

10

100

KΩ

KΩ

nF

R3 is optional and will set
the address to 0x76 instead
of 0x77.

The blocking capacitors
should be placed as close
to the package pins as
possible.

IIR filtering

5.3
The air pressure is slowly changing due to weather conditions or short term changing like air turbulence created
by a fan, slamming a door or window. All these disturbances can be suppressed or triggered on the software
application level by implementing different IIR filtering.
Same sensor can be used by different software applications applying different IIR filtering to the raw data like
low pass, high pass or band pass filtering.

Digital interfaces

6
The measurement data, calibration coefficients, Product ID and configuration registers can be accessed through
both the I2C and SPI serial interfaces.
The SPI interface can configured to operate in 3-wire or 4-wire mode. In I2C and SPI 3-wire, an interrupt output
can be implemented on the SDO pin. The SPI interface support mode '11' only ( CPOL=CPHA='1' ) in 4-wire and
3-wire configuration. The following commands are supported: single byte write, single byte read and multiple
byte read using auto increment from a specified start address. The interface selection is done based on CSB pin
status. If CSB is connected to VDDIO, the I2C interface is active. If CSB is low, the SPI interface is active. After the
CSB has been pulled down once the I2C interface is disabled until the next power-on-reset.

Datasheet

20

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

I2C Interface

6.1
The I2C slave interface is compatible with Philips I2C Specification version 2.1. The I2C interface supports
standard, fast and high speed mode.
The sensor's address is 0x77 (if SDO pin is left floating or pulled-up to VDDIO) or 0x76 (if the SDO pin is pulled-
down to GND). The I2C interface uses the pins described in Table 2
The basic timing is shown in the diagram below:

Figure 11

I2C timing diagram

In one access, without stop, incremental read (address is auto increment) and auto-incremental write is
supported. The read and write access is described below:

Figure 12

I2C write and read commands

SPI Interface

6.2
The SPI interface is compatible with SPI mode '11' ( CPOL = CPHA = '1'. The SPI interface has two modes: 4-wire
and 3-wire.
The protocol is the same for both. The 3-wire mode is selected by setting '1' to the register Interrupt and FIFO
configuration (CFG_REG)
The SPI interface uses the pins like in theTable 2 Refere toApplication Circuit Example for connections
instructions. The SPI protocol is shown in the diagram below:

Figure 13

SPI protocol, 4-wire without interrupt

Datasheet

21

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

A SPI write is carried out by setting CSB low and sending a control byte followed by register data. The control
byte consist of the SPI register address ( full register address without bit 7) and the write command ( bit7 = RW =
'0'). Setting CSB high ends the transaction. The SPI write protocol is described in the diagram below.
A SPI read is initiated by setting CSB low and sending a single control byte. The control byte consist of the SPI
register address ( = full register address without bit 7) and the read command ( bit7 = RW = '1'). After writing the
control byte, data is sent out of the SDO pin ( SDI in 3-wire mode); the register address is automatically
incremented. Sending CSB high ends the SPI read transaction. The SPI read protocol is shown in the diagram
below:

Figure 14

SPI write, read protocol diagrams

6.3

Interface parameters specification

Datasheet

22

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

6.3.1
The general interface parameters are given in the table below:

General interface parameters

Table 12

Interface parameters

Parameter

Symbol

Values

Unit Note or Test

Min.

Typ.

Max.

Input voltage for low logic
level at input pins

Input voltage for high logic
level at input pins

Vlow_in

Vhigh_in

Output - low level for I2C

Vlow_SDI

Output voltage for low level
at pin SDI for I2C

Vlow_SDI_1.2

Output voltage for high level
at pins SDO, SDI

Vhigh_out

Output voltage for high level
at pins SDO, SDI

Vhigh_out_1.2

0.7 *
VDDIO

0.8 *
VDDIO

0.6 *
VDDIO

0.3 *
VDDIO

0.1 *
VDDIO

0.2*
VDDIO

V

V

V

V

V

V

Condition

VDDIO=1.2V to 3.6V

VDDIO=1.2V to 3.6V

VDDIO=1.8V,
iol=2mA

VDDIO=1.20V,
iol=1.3mA

VDDIO=1.8V,
iol=1mA (SDO,
SDI)

VDDIO=1.2V,
iol=1mA (SDO,
SDI)

Pull-up resistor

Rpull

60

120

180

kohm Internal pull-up

I2C bus load capacitor

Cb

resistance to VDDIO

400

pF

On SDI and SCK

I2C timings

6.3.1.1
The I2C timing is shown in the diagram below and corresponding values are given in the table below. The
naming refers to I2C Specification version 2.1, the abbreviations used "S&F mode" = standard and fast mode,
"HS mode" = high speed mode, Cb = bus capacitance on SDA line.

Datasheet

23

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

Figure 15

I2C timing diagram

Table 13

I2C timings

Parameter

Symbol

Values

Unit Note or Test

Data setup time on SDI pin

tSetup

Data hold time on SDI pin

Duty Cycle

tHold

DC

Min.

Typ.

Max.

20

5

0

70

55

Condition

S&F mode

HS mode

S&F&HSmode,

S&F mode,

HS mode,

ns

ns

ns

%

%

Datasheet

24

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Digital interfaces

SPI timings

6.3.1.2
The SPI timing diagram is shown in the figure below and the corresponding values are given in the table below.
All timings apply both to 4-wire and 3-wire SPI.

Figure 16

SPI timing diagram

Table 14

SPI timings

Parameter

Symbol

Values

Unit Note or Test

Min.

Typ.

Max.

Duty Cycle (Thigh%)

SPI_DC

SDI setup time

SDI hold time

Clock

CSB setup time

CSB hold time

T_setup_sdi

T_hold_sdi

SPI_CLK

T_setup_csb

30

20

2

2

15

15

%

%

ns

ns

10

MHz

ns

ns

Condition

VDDIO = 1.2V

VDDIO = 1.8V/3.6V

Datasheet

25

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register Map

7

Register Map

Table 15

Register
Name

PSR_B2

PSR_B1

PSR_B0

TMP_B2

TMP_B1

TMP_B0

PRS_CFG

TMP_CFG

Register Map

Addr.

bit7

bit6

bit5

bit4

bit3

bit2

bit1

bit0

0x00

0x01

0x02

0x03

0x04

0x05

0x06

0x07

PSR[23:16] (r)

PSR[15:8](r)

PSR[7:0](r)

TMP[23:16] (r)

TMP[15:8] (r)

TMP[7:0] (r)

PM_RATE [2:0] (rw)

PM_PRC [3:0] (rw)

TMP_RATE [2:0] (rw)

TM_PRC [3:0] (rw)

-

TMP_
EXT
(rw)

MEAS_CFG

0x08

COEF_
RDY (r)

SENSOR
_ RDY (r)

TMP_
RDY (r)

PRS_
RDY (r)

-

MEAS_CRTL [2:0] (rw)

CFG_REG

0x09

INT_ HL
(rw)

INT_ SEL [2:0] (rw)

INT_STS

0x0A

FIFO_STS

0x0B

RESET

0x0C

-

-

-

-

-

FIFO_
FLUSH
(w)

-

-

-

-

-

-

TMP_
SHIFT_
EN (rw)

-

-

PRS_
SHIFT_
EN (rw)

INT_
FIFO_
FULL (r)

-

SOFT_RST [3:0] (w)

FIFO_
EN (rw)

INT_
TMP(r)

SPI_
MODE
(rw)

INT_
PRS(r)

FIFO_
FULL(r)

FIFO_
EMPTY(r)

Product ID

0x0D

REV_ID [3:0] (r)

PROD_ID [3:0] (r)

COEF

Reserved

0x10-
0x21

0x22-
0x27

COEF_SRCE 0x28

< see register description >

Reserved

TMP_C
OEF_SR
CE (r)

Reserved

8

8.1

Register description

Pressure Data (PRS_Bn)

Reset
State

00h
00h
00h
00h
00h
00h
00h
00h

C0h

00h

00h

00h

00h

10h
XXh

XXh

XXh

The Pressure Data registers contains the 24 bit (3 bytes) 2's complement pressure measurement value.
If the FIFO is enabled, the register will contain the FIFO pressure and/or temperature results. Otherwise, the
register contains the pressure measurement results and will not be cleared after read.

Datasheet

26

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.1.1
The highest byte of the three bytes measured pressure value.

PRS_B2

PRS_B2

Pressure (MSB data)

Address:

Reset value:

00H
00H

7

6

5

4

3

2

1

0

PRS23

PRS22

PRS21

PRS20

PRS19

PRS18

PRS17

PRS16

r

Field

PRS[23:16]

Bits

7:0

Type

Description

r

MSB of 24 bit 2´s complement pressure data.

8.1.2
The middle byte of the three bytes measured pressure value.

PRS_B1

PRS_B1

Pressure (LSB data)

Address:

Reset value:

01H
00H

7

6

5

4

3

2

PRS15

PRS14

PRS13

PRS12

PRS11

PRS10

1

PRS9

0

PRS8-

r

Field

PRS[15:8]

Bits

7:0

Type

Description

r

LSB of 24 bit 2´s complement pressure data.

Datasheet

27

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.1.3
The lowest byte of the three bytes measured pressure value.

PRS_B0

PRS_B0

Pressure (XLSB data)

Address:

Reset value:

02H
00H

7

PRS7

6

PRS6

5

PRS5

4

PRS4

3

PRS3

2

PRS2

1

PRS1

0

PRS0

r

Field

PRS[7:0]

Bits

7:0

Type

Description

r

XLSB of 24 bit 2´s complement pressure data.

8.2

Temperature Data (TMP_Tn)

The Temperature Data registers contain the 24 bit (3 bytes) 2's complement temperature measurement value
( unless the FIFO is enabled, please see FIFO operation ) and will not be cleared after the read.

8.2.1
The highest byte of the three bytes measured temperature value.

TMP_B2

TMP_B2

Temperature (MSB data)

Address:

Reset value:

03H
00H

7

6

5

4

3

2

1

0

TMP23

TMP22

TMP21

TMP20

TMP19

TMP18

TMP17

TMP16

r

Field

TMP[23:16]

Bits

7:0

Type

Description

r

MSB of 24 bit 2´s complement temperature data.

Datasheet

28

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.2.2
The middle byte of the three bytes measured temperature value.

TMP_B1

TMP_B1

Temperature (LSB data)

Address:

Reset value:

04H
00H

7

6

5

4

3

2

1

0

TMP15

TMP14

TMP13

TMP12

TMP11

TMP10

TMP9

TMP8

r

Field

TMP[15:8]

Bits

7:0

Type

Description

r

LSB of 24 bit 2´s complement temperature data.

8.2.3
The lowest part of the three bytes measured temperature value.

TMP_B0

TMP_B0

Temperature (XLSB data)

Address:

Reset value:

05H
00H

7

TMP7

6

5

4

3

2

1

0

TMP6

TMP5

TMP4

TMP3

TMP2

TMP1

TMP0

r

Field

TMP[7:0]

Bits

7:0

Type

Description

r

XLSB of 24 bit 2´s complement temperature data.

Datasheet

29

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.3
Configuration of pressure measurement rate (PM_RATE) and resolution (PM_PRC).

Pressure Configuration (PRS_CFG)

PRS_CFG

Pressure measurement configuration

Address:

Reset value:

06H
00H

7

-

-

6

5

4

3

2

1

0

PM_RATE[2:0]

rw

PM_PRC[3:0]

rw

Field

-

PM_RATE[2:0]

Bits

7

6:4

Type

Description

-

rw

Reserved.

Pressure measurement rate:
000 - 1 measurements pr. sec.
001 - 2 measurements pr. sec.
010 - 4 measurements pr. sec.
011 - 8 measurements pr. sec.
100 - 16 measurements pr. sec.
101 - 32 measurements pr. sec.
110 - 64 measurements pr. sec.
111 - 128 measurements pr. sec.
Applicable for measurements in Background mode only

PM_PRC[3:0]

3:0

rw

Pressure oversampling rate:
0000 - Single. (Low Precision)
0001 - 2 times (Low Power).
0010 - 4 times.
0011 - 8 times.
0100 *)- 16 times (Standard).
0101 *) - 32 times.
0110 *) - 64 times (High Precision).
0111 *) - 128 times.

1xxx - Reserved

*) Note: Use in combination with a bit shift. See Interrupt and FIFO configuration (CFG_REG) register

Datasheet

30

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

Table 16

Precision (PaRMS) and pressure measurement time (ms) versus oversampling rate

Oversampling
(PRC[3:0])

Single
(0000)

2 times
(0001)

4 times
(0010)

8 times
(0011)

16 times
(0100)

32 times
(0101)

64 times
(0110)

Measurement time
(ms)

3.6

Precision (PaRMS)

2.5

5.2

1

8.4

0.5

14.8

0.4

27.6

0.35

53.2

104.4

0.3

0.2

Table 17

Estimated current consumption (uA)

Oversampling
(PRC[3:0])

Single
(0000)

2 times
(0001)

4 times
(0010)

8 times
(0011)

16 times
(0100)

32 times
(0101)

64 times
(0110)

128
times
(0111)

206.8

128
times
(0111)

Measurements pr
sec.
(PM_RATE([2:0])

1 (000)

2 (001)

4 (010)

8 (011)

16 (100)

32 (101)

64 (110)

128 (111)

2.1

2.7

3.8

6.1

11

20

38

75

Note: The current consumption can be calculated as the Measurement Rate *
Current Consumption of 1 measurement per. sec.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

n.a.

Note: The table shows the possible combinations of Pressure Measurement Rate and oversampling when no
temperature measurements are performed. When temperature measurements are performed the possible
combinations are limited to Ratetemperature x Measurement Timetemperature + Ratepressure x Measurement
Timepressure < 1 second.
The temperature measurement time versus temperature oversampling rate is similar with pressure
measurement time versus pressure oversampling rate

Datasheet

31

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

Temperature Configuration(TMP_CFG)
8.4
Configuration of temperature measurement rate (TMP_RATE) and resolution (TMP_PRC).

TMP_CFG

Temperature measurement configuration

Address:

Reset value:

07H
00H

7

6

5

4

TMP_EXT

rw

TMP_RATE[6:4]

rw

3

-

2

1

0

TMP_PRC[3:0]

rw

Field

TMP_EXT

Bits

7

Type

rw

TMP_RATE[2:0]

6:4

rw

TMP_PRC[2:0]

3:0

rw

Description

Temperature measurement
0 - Internal sensor (in ASIC)
1 - External sensor (in pressure sensor MEMS element)
Note: It is highly recommended to use the same temperature
sensor as the source of the calibration coefficients. Please see the
Coefficient Source register

Temperature measurement rate:
000 - 1 measurement pr. sec.
001 - 2 measurements pr. sec.
010 - 4 measurements pr. sec.
011 - 8 measurements pr. sec.
100 - 16 measurements pr. sec.
101 - 32 measurements pr. sec.
110 - 64 measurements pr. sec.
111 - 128 measurements pr. sec..
Applicable for measurements in Background mode only

Temperature oversampling (precision):
0000 - single. (Default) - Measurement time 3.6 ms.
Note: Following are optional, and may not be relevant:
0001 - 2 times.
0010 - 4 times.
0011 - 8 times.
0100 - 16 times.
0101 - 32 times.
0110 - 64 times..
0111 - 128 times.
1xxx - Reserved.

Datasheet

32

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.5
Setup measurement mode.

Sensor Operating Mode and Status (MEAS_CFG)

MEAS_CFG

Measurement configuration

Address:

Reset value:

08H
C0H

7

6

5

4

COEF_RDY

SENSOR_R
DY

TMP_RDY

PRS_RDY

r

r

r

r

3

-

-

2

1

0

MEAS_CTRL

rw

Bits

Type

Description

Field

COEF_RDY

SENSOR_RDY

TMP_RDY

PRS_RDY

7

6

5

4

-

MEAS_CTRL

3

2:0

r

r

r

r

-

rw

Coefficients will be read to the Coefficents Registers after start-
up:
0 - Coefficients are not available yet.
1 - Coefficients are available.

The pressure sensor is running through self initialization after
start-up.
0 - Sensor initialization not complete
1 - Sensor initialization complete
It is recommend not to start measurements until the sensor has
completed the self intialization.

Temperature measurement ready
1 - New temperature measurement is ready. Cleared when
temperature measurement is read.

Pressure measurement ready
1 - New pressure measurement is ready. Cleared when
pressurement measurement is read.

Reserved.

Set measurement mode and type:
Standby Mode
000 - Idle / Stop background measurement
Command Mode
001 - Pressure measurement
010 - Temperature measurement
011 - na.
100 - na.
Background Mode
101 - Continous pressure measurement
110 - Continous temperature measurement
111 - Continous pressure and temperature measurement

Datasheet

33

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.6
Configuration of interupts, measurement data shift, and FIFO enable.

Interrupt and FIFO configuration (CFG_REG)

CFG_REG

Configuration register

Address:

Reset value:

09H
00H

7

6

5

4

3

2

1

0

INT_HL

INT_FIFO

INT_TMP

INT_PRS

T_SHIFT

P_SHIFT

FIFO_EN

SPI_MODE

rw

rw

rw

rw

rw

rw

rw

rw

Field

INT_HL

INT_FIFO

INT_TMP

INT_PRS

T_SHIFT

P_SHIFT

FIFO_EN

SPI_MODE

Bits

7

6

5

4

3

2

1

0

Type

rw

Description

Interupt (on SDO pin) active level:
0 - Active low.
1 - Active high.

rw

rw

rw

rw

rw

rw

rw

Generate interupt when the FIFO is full:
0 - Disable.
1 - Enable.

Generate interupt when a temperature measurement is ready:
0 - Disable.
1 - Enable.

Generate interupt when a pressure measurement is ready:
0 - Disable.
1 - Enable.

Temperature result bit-shift
0 - no shift.
1 - shift result right in data register.
Note: Must be set to '1' when the oversampling rate is >8 times.

Pressure result bit-shift
0 - no shift.
1 - shift result right in data register.
Note: Must be set to '1' when the oversampling rate is >8 times.

Enable the FIFO:
0 - Disable.
1 - Enable.

Set SPI mode:
0 - 4-wire interface.
1 - 3-wire interface.

Datasheet

34

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

Interrupt Status (INT_STS)
8.7
Interrupt status register. The register is cleared on read.

INT_STS

Interrupt status

7

6

Address:

Reset value:

0AH
00H

4

3

2

1

0

INT_FIFO_F
ULL

INT_TMP

INT_PRS

r

r

r

5

-

-

Field

-

INT_FIFO_FULL

INT_TMP

INT_PRS

Bits

7:3

2

1

0

Type

Description

-

r

r

r

Reserved.

Status of FIFO interrupt
0 - Interrupt not active
1 - Interrupt active

Status of temperature measurement interrupt
0 - Interrupt not active
1 - Interrupt active

Status of pressure measurement interrupt
0 - Interrupt not active
1 - Interrupt active

Datasheet

35

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

FIFO Status (FIFO_STS)

8.8
FIFO status register

FIFO_STS

FIFO status register

Address:

Reset value:

0BH
00H

7

6

5

4

3

2

1

0

-

-

FIFO_FULL

FIFO_EMPT
Y

r

r

Field

-

FIFO_FULL

FIFO_EMPTY

Bits

7:2

1

0

Type

Description

-

r

r

Reserved.

0 - The FIFO is not full
1 - The FIFO is full

0 - The FIFO is not empty
1 - The FIFO is empty

Datasheet

36

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

8.9
Flush FIFO or generate soft reset.

Soft Reset and FIFO flush (RESET)

RESET

FIFO flush and soft reset

7

6

FIFO_FLUSH

w

5

-

-

Address:

Reset value:

0CH
00H

4

3

2

1

0

SOFT_RST

w

Field

FIFO_FLUSH

-

SOFT_RST

Bits

7

6:4

3:0

Type

Description

w

-

w

FIFO flush
1 - Empty FIFO
After reading out all data from the FIFO, write '1' to clear all old
data.

Reserved.

Write '1001' to generate a soft reset. A soft reset will run though
the same sequences as in power-on reset.

8.10
Product and Revision ID.

Product and Revision ID (ID)

ID

Product and revision ID

Address:

Reset value:

0DH
0x10H

7

6

5

4

3

2

1

0

REV_ID

r

PROD_ID

r

Field

REV_ID

PROD_ID

Bits

7:4

3:0

Type

Description

r

r

Revision ID

Product ID

Datasheet

37

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Register description

Calibration Coefficients (COEF)

8.11
The Calibration Coefficients register contains the 2´s complement coefficients that are used to calculate the
compensated pressure and temperature values.

Table 18

Calibration Coefficients

Coefficient

Addr.

bit7

bit6

bit5

bit4

bit3

bit2

bit1

bit0

c1 [11:8]

c10 [19:16]

c0

c0/c1

c1

c00

c00

c00/c10

c10

c10

c01

c01

c11

c11

c20

c20

c21

c21

c30

c30

0x10

0x11

0x12

0x13

0x14

0x15

0x16

0x17

0x18

0x19

0x1A

0x1B

0x1C

0x1D

0x1E

0x1F

0x20

0x21

c0 [11:4]

c0 [3:0]

c1[7:0]

c00 [19:12]

c00 [11:4]

c00 [3:0]

c10 [15:8]

c10 [7:0]

c01 [15:8]

c01 [7:0]

c11 [15:8]

c11 [7:0]

c20 [15:8]

c20 [7:0]

c21 [15:8]

c21 [7:0]

c30 [15:8]

c30 [7:0]

Note: Generate the decimal numbers out of the calibration coefficients registers data:

C20 := reg0x1D + reg0x1C * 2^ 8

if (C20 > (2^15 - 1))
   C20 := C20 - 2^16
end if

C0 := (reg0x10 * 2^ 4) + ((reg0x11 / 2^4) & 0x0F)
if (C0 > (2^11 - 1))
   C0 := C0 - 2^12
end if

Datasheet

38

v1.1
2019-07-03

TMP_COEF_
SRCE

r

Field

TMP_COEF_SRCE

-

DPS368
Digital XENSIVTM barometric pressure sensor

Package Dimensions

Coefficient Source

8.12
States which internal temperature sensor the calibration coefficients are based on: the ASIC temperature sensor
or the MEMS element temperature sensor. The coefficients are only valid for one sensor and it is highly
recommended to use the same temperature sensor in the application. This is set-up in the Temperature
Configuration register.

TMP_COEF_SRCE

Temperature Coefficients Source

Address:

Reset value:

28H
XXH

7

6

5

4

2

1

0

3

-

-

Bits

7

6:0

Type

Description

r

-

Temperature coefficients are based on:
0 - Internal temperature sensor (of ASIC)
1 - External temperature sensor (of pressure sensor MEMS
element)

Reserved

9
The sensor package is a 8-pin PG-VLGA-8-2, 2.0 x 2.5 x 1.1 mm3, with 0.65 mm pitch.

Package Dimensions

9.1

Package drawing

Datasheet

39

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Footprint and stencil recommendation

Figure 17

DPS368 package drawing ( side view, top view, bottom view)

Footprint and stencil recommendation

10
The board pad and stencil aperture recommendations shown in figure below are based on Solder Mask Defined
( SMD ) pads. The specific design rules of the board manufacturer should be considered for individual design
optimization or adaptations.

Datasheet

40

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Reflow soldering and board assembly

Figure 18

DPS368 footprint and stencil recommendation

Reflow soldering and board assembly

11
The Infineon pressure sensors are qualified in accordance with the IPC/JEDEC J-STD-020D-01. The moisture
sensitivity level of pressure sensor is rated as MSL1. For PCB assembly of the sensor the widely used reflow
soldering using a forced convection oven is recommended.
The soldering profile should be in accordance with the recommendations of the solder paste manufacturer to
reach an optimal solder joint quality. The reflow profile shown in figure below is recommended for board
manufacturing with Infineon pressure sensors.

Figure 19

Reflow profile

Datasheet

41

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Reflow soldering and board assembly

Table 19

Reflow profile limits

Profile feature

Preheat/Soak

Temperature Min (Tsmin)

Temperature Max (Tsmax)

Time (Tsmin to Tsmax) (ts)

Peak Temperature (Tp)

Time within 5°C of actual peak
temperature (tp)*

150 °C

200 °C

60-120 seconds

260°C +0°C/-5°C

20-40 seconds

Pb-Free assembly

Sn-Pb Eutectic assembly

100 °C

150 °C

60-150 seconds

235°C +0°C/-5°C

10-30 seconds

6 °C/second max.

6 minutes max.

Ramp-down rate

6 °C/second max.

Time 25°C to peak temperature

8 minutes max.

* Tolerance for peak profile temperature (Tp) is defined as a supplier
minimum and a user maximum

Note:

For further information please consult the 'General recommendation for assembly of Infineon
packages' document which is available on the Infineon Technologies web page

Datasheet

42

v1.1
2019-07-03

DPS368
Digital XENSIVTM barometric pressure sensor

Package Handling

The Infineon pressure sensors can be handled using industry standard pick and place equipment. Care should
be taken to avoid damage to the sensor structure as follows:
•
•
•

Do not pick the sensor with vacuum tools which make contact with the sensor vent port hole
The sensor's vent hole should not be exposed to vacuum, this can destroy or damage the MEMS
Do not blow air into the sensor vent hole. If an air blow cleaning process is used, the vent hole must be
sealed to prevent particle contamination.
It is recommended to perform the PCB assembly in a clean room environment in order to avoid sensor
contamination.
Air blow and ultrasonic cleaning procedures shall not be applied to MEMS pressure sensors. A non-clean
paste is recommended for the assembly to avoid subsequent cleaning steps. The MEMS sensor can be
severely damaged by cleaning substances.
To prevent the blocking or partial blocking of vent hole during PCB assembly, it is recommended to cover
the sound port with protective tape during PCB sawing or system assembly.
Do not use excessive force to place the sensor on the PCB. The use of industry standard pick and place tools
is recommended in order to limit the mechanical force exerted to the package.

•

•

•

•

Package Handling

12
Further Information please refer to the attached “Digital Barometric Pressure Sensor_ Package Handling”. It
describes the package handling and delivery format.

Revision history

Document
version

Date of
release

Description of changes

v0.1

v0.2

v0.3

v0.4

v1.0

v1.1

05.05.2017

initial release

11.29.2017

update package dimensions

03.04.2018

Pressure Transfer Function table update

19.10.2018

Pressure noise values update

01.03.2018

Features and Typical applications chapters update; Package drawing,
Footprint and stencil recommendation, Solder reflow chapters added

03.07.2019

Title update

Datasheet

43

v1.1
2019-07-03

Trademarks

All referenced product or service names and trademarks are the property of their respective owners.

Edition 2019-07-03

Published by

Infineon Technologies AG

81726 Munich, Germany

© 2019 Infineon Technologies AG

All Rights Reserved.

Do you have a question about any
aspect of this document?

Email: erratum@infineon.com

Document reference
IFX-duu1493988991355

IMPORTANT NOTICE
The  information  given  in  this  document  shall  in  no
event  be  regarded  as  a  guarantee  of  conditions  or
characteristics (“Beschaffenheitsgarantie”) .
With respect to any examples, hints or any typical values
stated  herein  and/or  any  information  regarding  the
application  of  the  product,
Infineon  Technologies
hereby disclaims any and all warranties and liabilities of
any  kind,  including  without  limitation  warranties  of
non-infringement  of  intellectual  property  rights  of  any
third party.
In  addition,  any  information  given  in  this  document  is
subject  to  customer’s  compliance  with  its  obligations
stated  in  this  document  and  any  applicable  legal
requirements,  norms  and
standards  concerning
customer’s  products  and  any  use  of  the  product  of
Infineon Technologies in customer’s applications.
The  data  contained  in  this  document  is  exclusively
intended
the
responsibility  of  customer’s  technical  departments  to
evaluate  the  suitability  of  the  product  for  the  intended
application  and  the  completeness  of  the  product
information given in this document with respect to such
application.

trained  staff.

technically

for

is

It

WARNINGS
Due to technical requirements products may contain
dangerous substances. For information on the types
in  question  please  contact  your  nearest  Infineon
Technologies office.
Except  as  otherwise  explicitly  approved  by  Infineon
Technologies  in  a  written  document  signed  by
authorized representatives of Infineon Technologies,
Infineon Technologies’ products may not be used in
any  applications  where  a  failure  of  the  product  or
any consequences of the use thereof can reasonably
be expected to result in personal injury


