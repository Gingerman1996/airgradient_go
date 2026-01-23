as# Datasheet ‒ STCC4 CO 2 Sensor

**Minimal Size. Maximize the Potential.**

**Highlights**

- Thermal conductivity measurement principle
- CO 2 measurement accuracy ±(100 ppm + 10%)
    from 400 – 5’000 ppm
- Digial I^2 C interface and calibrated output signal

(^) • Avg. current consumption below 100 μA in
single shot measurement mode

- Configurable I^2 C address
- Tape and reel package, reflow solderable

The STCC4 is Sensirion’s next generation miniature CO 2 sensor for indoor air quality applications. Based on the
thermal conductivity sensing principle and Sensirion’s CMOSens® technology, the STCC4 enables monitoring
of CO 2 gas concentration in ambient indoor air conditions at an unmatched cost-effectiveness and form factor.
SMD assembly as well as tape & reel packaging allows cost- and space-effective integration of the STCC4 for
high-volume applications. The STCC4 is 100% factory-calibrated and enables automatic on-board
compensation of the CO 2 output for humidity and temperature through an external SHT4x sensor.

**Functional Block Diagram**


## Contents

- 1 Measurement Specifications
   - 1.1 CO 2 Sensing Performance.....................................................................................................................................................
- 2 Sensor Specifications
   - 2.1 Recommended Environmental Conditions
   - 2.2 Electrical Specifications
   - 2.3 Absolute Maximum Ratings
   - 2.4 Hardware Interface Specifications
   - 2.5 Temperature and Humidity Compensation
   - 2.6 Timing Specifications
   - 2.7 Mechanical Specifications
   - 2.8 Material Specifications
- 3 Sensor Operation
   - 3.1 I^2 C Address..................................................................................................................................................................................
   - 3.2 I^2 C Data Transfer Formats
   - 3.3 Checksum Calculation
   - 3.4 I^2 C Commands
   - 3.5 Conversion of Signal Input and Output
- 4 Physical Specification
   - 4.1 Package Description..............................................................................................................................................................
   - 4.2 Package Outline
   - 4.3 Land Pattern
   - 4.4 Traceability and Identification
   - 4.5 Soldering Instructions
- 5 Tape and Reel Packaging
- 6 Ordering Information
- 7 Revision History


## 1 Measurement Specifications

### 1.1 CO 2 Sensing Performance.....................................................................................................................................................

The CO 2 sensing performance specifications in **Table 1** are valid for default conditions of 25 °C, 50 %RH,
1013 hPa, continuous measurement mode with 1 s sampling interval and 3.3 V supply voltage.

```
Parameter Conditions Min. Typ. Max. Unit
CO 2 output range 380 32’000 ppm
CO 2 measurement
accuracy^1 , continuous
measurement mode
```
```
400 – 5’000 ppm, SHT4x temperature
and humidity compensation
```
```
± (100 ppm +
10% of reading) ppm^
```
```
CO 2 measurement
accuracy^1 , single shot
measurement mode
```
```
400 – 5’000 ppm, SHT4x temperature
and humidity compensation, 10 s
sampling interval
```
```
± (100 ppm +
10% of reading) ppm^
```
```
Digital resolution 1 ppm
Response time^2 τ63%, step change 2’000 – 400 ppm 20 s
```
**Table 1.** CO 2 sensing performance

**1.1.1 Thermal Conductivity Measurement Principle**

The sensor measures the thermal conductivity of the gas surrounding the sensing element. Any variation in the
gas composition results in the thermal conductivity deviating from fresh air (defined as a composition of
78% N 2 , 21% O 2 , 0.93% Ar, 400 ppm CO 2 and a variable content of H 2 O depending on the relative humidity)
and is interpreted as a change in CO 2 concentration. Compositions of artificial reference gas mixtures, such as
100% N 2 or 80% N 2 /20% O 2 , deviate substantially from fresh air and can lead to measurement errors.

**1.1.2 Automatic Self-Calibration**

The sensor is operated with a built-in automatic self-calibration (ASC) algorithm to maintain long-term stability
of the CO 2 sensing performance. The algorithm assumes that the sensor during operation is exposed to fresh
air containing approximately 400 ppm CO 2 concentration at least once per week. Exposure to
CO 2 concentrations lower than 400 ppm can affect the sensor accuracy. The ASC state is stored in non-volatile
memory at a maximum interval of 2 hours in continuous measurement mode or after 720 single shot
measurements, corresponding to 2 hours in single shot measurement mode with a 10 s sampling interval.

**1.1.3 Warm-Up**

After idle or power-off periods longer than 3 hours, the CO 2 measurement accuracy is achieved after a
maximum of 1 hour of operation in continuous measurement mode or 360 single shot measurements,
corresponding to 1 hour in single shot measurement mode with a 10 s sampling interval. The
_perform_conditioning_ command ( **Section 3.4.9)** is recommended to improve the CO 2 sensing performance after
long idle or power-off periods.

**1.1.4 Initial Operation**

Initial operation occurs after the first sensor power-up. CO 2 measurement accuracy is achieved after a maximum
of 12 hours of operation in continuous measurement mode^3 , followed by exposure to fresh air containing
approximately 400 ppm CO 2 concentration. The first ASC state is saved after sensor operation for 1 hour in
continuous measurement mode or 360 single shot measurements. The sensor will complete a bypass phase
with a CO 2 concentration output of 390 ppm during the first 20 s of continuous measurement mode or the first
2 single shot measurements. Restarting the continuous measurement mode or power-cycling within the first
hour of sensor operation will reinitialize the bypass phase and timer for the first ASC state save. The
_perform_factory_reset_ command ( **Section 3.4.11)** reinitializes the bypass phase and resets the ASC history.

(^1) Deviation from a high-precision reference with gas mixtures having a ±2% tolerance.
(^2) Response time depends on design-in, signal update rate and environment of the sensor in the final application.
(^3) Initial operation duration increases for sampling intervals greater than 1 s.


## 2 Sensor Specifications

### 2.1 Recommended Environmental Conditions

The sensor is optimized for operation and storage within the environmental conditions listed in **Table 2**. For
an extended operating range, external pressure input through the _set_pressure_compensation_ command
( **Section 3.4.5** ) can improve CO 2 sensing performance.

```
Parameter Conditions Min. Typ. Max. Unit
Operating temperature 10 25 40 °C
Storage temperature 10 25 50 °C
Relative humidity Non-condensing, max dew point 35 °C 20 50 80 %RH
```
```
Pressure
```
```
Without pressure compensation 85 ’ 000 101 ’ 300 110 ’ 000 Pa
With pressure compensation 70 ’ 000 101 ’ 300 110 ’ 000 Pa
```
**Table 2.** Recommended environmental conditions

### 2.2 Electrical Specifications

Default conditions of 25 °C and 3.3 V supply voltage apply to the values in **Table 3** , unless otherwise stated.

```
Parameter Sym. Conditions Min Typ Max Unit
Supply voltage V DD 2.7 3.3 5.5 V
Unloaded supply voltage
ripple
```
```
V RPP Without the load of the sensor 30 mV
```
```
Peak supply current^4 Ipeak 3.7 4.2 mA
```
```
Average supply current
⟨IDD⟩
```
```
Continuous measurement mode 950 μA
Single shot measurement mode^5 ,
10 s sampling interal
```
```
100 μA
```
```
Idle mode 55 μA
Sleep mode 1 μA
I^2 C communication
voltage
```
#### V I2C 1. 8 V DD V

```
Low level input voltage V IL 0 0.3· V I2C V
High level input voltage V IH 0.7· V I2C V I2C V
Low level output voltage V OL Rp ≥ 1. 5 kΩ, max sink current 3 mA 0 0. 2 · V I2C V
```
```
Capacitive bus load^6 C b
```
```
Rp ≥ 1. 5 kΩ, max sink current 3 mA,
Fast mode Plus -^90 pF^
```
**Table 3.** Electrical specifications

(^4) Higher transient currents on shorter timescales than 10 microseconds may be observed but are typically negligible due to parasitic
R/L/C elements in design-in.
(^5) On-demand measurement with adjustable sampling interval. See Section 3.4.6 for more information.
(^6) Refer to the I (^2) C-bus specification for proper dimensioning to achieve desired communication frequency. NXP I2C-bus specification and
user manual UM10204, Rev.7, 01 November 2021.


### 2.3 Absolute Maximum Ratings

Stress levels beyond those listed in **Table 4** may cause permanent damage to the sensor. Exposure to
minimum/maximum rating conditions for extended time periods may affect sensor performance and reliability.

```
Parameter Conditions Min Max Unit
MSL level 1
Supply voltage - 0.3 5.8 V
Voltage on all pins - 0.3 VDD+0.3 V
Operating temperature - 40 85 °C
Short-term storage
temperature^7
```
#### - 40 85 °C

```
Operating relative humidity Non-condensing 0 95 %RH
ESD HDM JEDEC JS- 001 2 kV
ESD CDM JEDEC JS- 002 500 V
Latch-up JESD78 Class II, 125 °C - 100 100 mA
Sensor lifetime Default operating conditions ( Section 1.1 ) 10 years
```
**Table 4.** Absolute maximum ratings

### 2.4 Hardware Interface Specifications

The STCC4 comes in a metal cap LGA package with a pin assignment as described in **Table 5**.

```
Pin Symbol Description Pin assignment
1 SCL I^2 C serial clock input
```
```
2 SDA_C
```
```
I^2 C serial data input/output to/from
SHT4x^8
```
#### 3 ADDR

```
I^2 C address configuration
ADDR = GND → 0x
ADDR = VDD → 0x
4 VSS Ground
5 SDA I^2 C serial data input/output
```
```
6 VDDIO I
```
(^2) C communication voltage
configuration^9 ,^10
7 VDD Supply voltage
8 SCL_C Serial clock output to SHT4x^8
**Table 5.** Pin assignment (transparent top view). Dashed lines are only visible from the bottom of the sensor.
(^7) Short term storage refers to temporary conditions, _e.g._ , during transport.
(^8) The SDA_C/SCL_C pins must be soldered to a floating pad if not connected to the SHT4x.
(^9) The required I (^2) C communication voltage must be applied to the VDDIO pin during sensor power-up.
(^10) The I (^2) C communication voltage is set to VDD when the voltage applied on the VDDIO pin is less than 0.7 V.


### 2.5 Temperature and Humidity Compensation

The STCC4 requires continuous input from an external SHT4x temperature and humidity sensor for accurate
compensation of the CO 2 concentration output. The recommended application circuit for the STCC4 is shown
in **Figure 1**. The STCC4 has a dedicated I^2 C controller interface for connecting to SHT4x sensors with
I^2 C address 0x44. The STCC4 I^2 C controller interface pads SDA_C and SCL_C have internal pull-up resistors of
10 kΩ. Additional pull-up resistors between the STCC4 and SHT4x are not recommended if the SHT4x is
designed close to the STCC4. The pull-up resistor values (Rp) must be dimensioned based on the bus capacity,
communication frequency and NXP I^2 C-bus specification and user manual^11.

**Figure 1.** Recommended application circuit. The positioning of the pins is only for illustrative purposes.

### 2.6 Timing Specifications

```
Parameter Sym. Conditions Min Typ Max Unit
Power-up time tPU After hard reset, VDD ≥ 2.7 V 10 ms
I^2 C SCL frequency fI2C 400 1’000 kHz
```
**Table 6.** Timing specifications

### 2.7 Mechanical Specifications

```
Parameter Sym. Conditions Min Typ Max Unit
Mass m 0. 02 g
```
**Table 7.** Mechanical specifications

### 2.8 Material Specifications

```
Parameter Conditions
REACH, RoHS, Halogen-Free Compliant
```
**Table 8.** Material specifications

(^11) NXP I (^2) C-bus specification and user manual UM10204, Rev.7, 01 November 2021


## 3 Sensor Operation

The STCC4 uses I^2 C communication based on the NXP I2C-bus specification and user manual^12. Supported
I^2 C modes are Standard-mode, Fast-mode and Fast-mode Plus. Clock stretching is not supported.

### 3.1 I^2 C Address..................................................................................................................................................................................

The I^2 C address (I2C ADDR) for the STCC4 is 0x64. See **Section 2.4** to select the alternative I^2 C address 0x65.

### 3.2 I^2 C Data Transfer Formats

The write, read and combined data transfer formats are visualized in **Figure 2**. Data and commands are
transferred in multiples of 16-bit words, with the most significant byte (MSB) transmitted first. All transfers
must begin with a start condition (S) and terminate with a stop condition (P). A sensor is addressed by sending
its 7-bit I^2 C address, followed by an eighth data direction bit denoting the communication direction (W/R):
“zero” indicates transmission to the target (i.e., “WRITE”) and “one” indicates a request for data (i.e., “READ”).
Data words are succeeded by an 8-bit checksum (cyclic redundancy check CRC-8, see **Section 3.3** ). Command
words (CMD) contain a 3-bit CRC. In the write direction, the checksum must be transmitted. In read direction,
the master may abort transmission after any data byte by sending a not acknowledge (NACK) and a stop
condition. After receiving a command, the sensor requires the specified execution time before responding with
a ACK.

**Figure 2.** Write, read and combined data transfer formats

(^12) NXP I (^2) C-bus specification and user manual UM10204, Rev.7, 01 November 2021


### 3.3 Checksum Calculation

The 8 - bit checksum is calculated from the two previously transmitted data bytes with the algorithm in **Table 9**.

```
Property Value
Name CRC- 8
Width 8 bit
Protected data read and/or write data
Polynomial 0x31 (x^8 + x^5 + x^4 + 1)
Initialization 0xFF
Reflect input/output False/False
Final XOR 0x
Example CRC (0xBEEF) = 0x
```
**Table 9.** Data checksum properties

### 3.4 I^2 C Commands

```
Section Command Hex Code
```
```
Executable during
Measurement
3.4.1 start_continuous_measurement 0x218B No
3.4.2 stop_continuous_measurement 0x3F86 Yes
3.4.3 read_measurement 0xEC05 Yes
3.4.4 set_rht_compensation 0xE000 Yes
3.4.5 set_pressure_compensation 0xE016 Yes
3.4.6 measure_single_shot 0x219D No
3.4.7 enter_sleep_mode 0x3650 No
3.4.8 exit_sleep_mode 0x00 No
3.4.9 perform_conditioning 0x29BC No
3.4.10 perform_soft_reset 0x06 Yes
3.4.11 perform_factory_reset 0x3632 No
3.4.12 perform_self_test 0x278C No
3.4.13 enable_testing_mode 0x3FBC Yes
3.4.14 disable_testing_mode 0x3F3D Yes
3.4.15 perform_forced_recalibration 0x362F No
3.4.16 get_product_ID 0x365B No
```
**Table 10.** List of I^2 C commands


**3.4.1 start_continuous_measurement**

The _start_continuous_measurement_ command will start a continuous measurement.

```
Command
```
```
Command
Hex Code
```
```
Argument Read Data
```
```
Execution
Time (ms)
```
```
Description
```
```
start_continu
ous_measure
ment
```
```
0x218B - - -
```
```
Starts continuous
measurement with 1 s
sampling interval.
```
Recommended operation sequence:

1. Send the _start_continuous_measurement_ command.
2. Wait 1 s^13.
3. Send the _read_measurement_ command ( **Section 3.4.3** ) and wait the specified execution time.
4. Read out the measurement data.
    4.1. If data is not yet available, wait 1 5 0 ms and retry to read out the measurement data.
5. Wait 1 s^13.
6. Repeat steps 3 - 5 as needed.
7. To stop the continuous measurement, send the _stop_continuous_measurement_ command ( **Section 3.4.2** )
    and wait the specified execution time.

**3.4.2 stop_continuous_measurement**

The _stop_continuous_measurement_ command will finish the currently running measurement before returning
to idle mode. During the execution time, the sensor will not acknowledge its I^2 C address nor accept commands.

```
Command
```
```
Command
Hex Code Argument^ Read Data^
```
```
Execution
Time (ms) Description^
stop_continuo
us_measurem
ent
```
```
0x3F86 - - 1’
```
```
Stops the continuous
measurement and put the
sensor into idle mode.
```
**3.4.3 Read_measurement**

Measurement data is read out through the _read_measurement_ command. The measurement data buffer is
emptied upon read-out. Subsequent measurement data can be readout by sending the sensor’s I^2 C address
and the read direction bit. The sensor will respond with a NACK if no measurement data is available. The
I^2 C master can abort the read transfer with a NACK followed by a STOP condition after any data byte. See
**Section 3.5** for the signal output and input conversion. The sensor status is used to determine if the sensor is
in testing mode (See **Section 3.4.13** ).

```
Command
```
```
Command
Hex Code
```
```
Argument Read Data
```
```
Execution
Time (ms)
```
```
Description
```
```
read_measur
ement
```
```
0xEC05 -
```
```
Byte0: CO 2 concentration 8msb
Byte1: CO 2 concentration 8lsb
Byte2: CRC
Byte3: Temperature 8msb
Byte4: Temperature 8lsb
Byte5: CRC
Byte6: Relative Humidity 8msb
Byte7: Relative Humidity 8lsb
Byte8: CRC
Byte9: Sensor Status 8msb
Byte10: Sensor Status 8lsb
Byte11: CRC
```
#### 1

```
Reads out the
measurement data.
Values for temperature
and humidity are
provided as received from
the external temperature
and humidity sensor.
```
(^13) Note that the sensor and the microcontroller are subject to clock tolerances. The effective sampling interval is 1’000 ms ± 15 0 ms.


**3.4.4 set_rht_compensation**

External relative humidity (RH) and temperature (T) compensation values can be written through the
_set_rht_compensation_ command when the SHT4x is not controlled by STCC4 through the dedicated I^2 C
controller interface (see **Section 2.5** ). The RH and T values must be provided from the same RHT sensor. The
written RHT values are utilized for RHT compensation after a maximum of one measurement interval. The
default or last written values are used for RHT compensation until overwritten. Power cycling resets the values
to the default values of 25 °C and 50 %RH. See **Section 3.5** for the signal output and input conversion. Do not
use the _set_rht_compensation_ command when an SHT4x is controlled by STCC4 through the I^2 C controller
interface.

```
Command Command
Hex Code
```
```
Argument Consecutive
Read
```
```
Execution
Time (ms)
```
```
Description
```
```
set_rht_com
pensation 0xE^
```
```
Byte0: Temperature 8msb
Byte1: Temperature 8lsb
Byte2: CRC
Byte3: Relative humidity 8msb
Byte4: Relative humidity 8lsb
Byte5: CRC
```
#### -

#### 1

```
Writes the RHT values for
compensation.
Example for 25 °C and
50 %RH:
[0xE0, 0x00, 0x66, 0x66,
0x93, 0x72 0xB0, 0xDC]
```
**3.4.5 set_pressure_compensation**

External pressure values can be written through the _set_pressure_compensation_ command. The written pressure
value is applied for pressure compensation after a maximum of one measurement interval. The default or the
last written value is used in pressure compensation until overwritten. Power cycling resets the sensor to the
default value of 10 1 ’ 300 Pa. See **Section 3.5** for the signal output and input conversion. Input values are clipped
to a range between 40’ 000 – 110 ’ 000 Pa.

```
Command
```
```
Command
Hex Code
```
```
Argument
```
```
Consecutive
Read
```
```
Execution
Time (ms)
```
```
Description
```
```
set_pressure
_compensat
oin
```
```
0xE
```
```
Byte0: Pressure 8msb
Byte1: Pressure 8lsb
Byte2: CRC
```
#### - 1

```
Writes the pressure value.
Example for 101’ 300 Pa:
[0xE0, 0x16, 0xC5, 0xDA, 0x83]
```
**3.4.6 measure_single_shot**

The _measure_single_shot_ command conducts an on-demand measurement of CO 2 gas concentration.

```
Command Command
Hex Code
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
measure_sin
gle_shot
```
```
0x219D - - 500 Performs a single shot
measurement.
```
Recommended operation sequence:

1. Wake the sensor up from sleep mode to idle mode with the _exit_sleep_mode_ command ( **Section 3.4.8** ).
2. Send the _measure_single_shot_ command and wait for the specified execution time.
3. Send the _read_measurement_ command ( **Section 3.4.3** ) and wait for the specified execution time.
4. Read out the measurement data.
5. Set the sensor to sleep mode with _the enter_sleep_mode_ command ( **Section 3.4.7** ).
6. Wait before the next measurement. A sampling interval between 5 s and 6 00 s ( 10 minutes) is recommended
    to ensure proper operation of the ASC algorithm.
7. Repeat steps 1-5 as required by the application.


**3.4.7 enter_sleep_mode**

The _enter_sleep_mode_ command sets the sensor from idle to sleep mode through the I^2 C interface. The written
relative humidity, temperature, pressure compensation values and ASC state are retained while in sleep mode.

```
Command Command
Hex Code
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
enter_sleep_
mode
```
```
0x 3650 - - 1 Sets the sensor from idle
mode into sleep mode.
```
**3.4.8 exit_sleep_mode**

The _exit_sleep_mode_ command wakes the sensor up from sleep mode to idle mode upon receiving its
I^2 C address, a write data direction bit and a payload byte 0x00. Note that the payload byte is not acknowledged.
The wake up of the sensor into idle mode can be confirmed by reading out the product ID (see **Section 3.4.16** ).

```
Command Payload
Byte
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
exit_sleep_
mode
```
```
0x 00 - - 5
```
```
Wakes the sensor from sleep
into idle mode.
```
**3.4.9 perform_conditioning**

The _perform_conditioning_ command is recommended to improve the initial CO 2 sensing performance when the
sensor has not completed measurements for more than 3 hours (See **Section 1.1.3** ).

```
Command
```
```
Command
Hex Code Argument^ Consecutive Read^
```
```
Execution
Time (ms) Description^
perform_co
nditioning 0x29BC^ -^ -^ 22’^
```
```
Conditions the sensor with a
set operation profile.
```
Recommended operation sequence:

1. Power up the sensor or wake the sensor up with the _exit_sleep_mode_ command ( **Section 3.4.8** ).
2. Send the _perform_conditioning_ command and wait for the specified execution time.
3. Send a measurement command; _e.g.,_ the _start_continuous_measurement_ command ( **Section 3.4.1** ) or the
    _measure_single_shot_ command ( **Section 3.4.6** ).

**3.4.10 perform_soft_reset**

The _perform_soft_reset_ command triggers a soft reset of the sensor through the I^2 C general call reset as
implemented according to the NXP I^2 C-bus specification and user manual^14. The _perform_soft_reset_ command
is not acknowledged by the sensor. During the execution time, the sensor will not acknowledge its I^2 C address
nor accept commands. The general call I^2 C address is 0x00, the command is 8 bits long and all devices on the
same I^2 C bus designed to respond to a general call I^2 C reset will also complete a soft reset.

```
Command
```
```
Command
Hex Code Argument^ Consecutive Read^
```
```
Execution
Time (ms) Description^
perform_sof
t_reset 0x^06 -^ -^10
```
```
Resets the sensor to the same
state as after a power cycle.
```
(^14) NXP I2C-bus specification and user manual UM10204, Rev.7, 01 November 2021


**3.4.11 perform_factory_reset**

The _perform_factory_reset_ command resets the FRC and ASC algorithm history, as well as reenables the bypass
phase (See **Section 1.1.4** for more information).

```
Command Command
Hex Code
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
perform_fac
tory_reset
```
```
0x 3632 -
```
```
Word[0] = 0 → pass
Word[0] = 0xFFFF →
command failed
```
#### 90

```
Resets the FRC and ASC
algorithm history.
```
**3.4.12 perform_self_test**

The _perform_self_test_ command can be used to check the sensor functionality in an end-of-line test as well as
for design-in and debugging purposes. The self-test should be performed during stable conditions, _i.e.,_ stable
supply voltage, environmental conditions and CO 2 concentration.

```
Command
```
```
Command
Hex Code Argument^ Consecutive Read^
```
```
Execution
Time (ms) Description^
```
```
perform_self
_test
```
```
0x278C -
```
```
Byte0: Self-test 8 msb
Byte1: Self-test 8lsb
Byte2: CRC
```
#### 360

```
Performs an on-chip self-test.
A successful test will return
either 0x0000 or 0x0010.
```
The self-test output is decoded as follows:

```
Bits Nonzero Value
0 Supply voltage is out of specified range
3:1 For debugging purposes; contact Sensirion for support
4 SHT is not connected through STCC4 I^2 C controller interface pad
6:5 Memory error
```
In case of a memory error, the recommended operation sequence is:

1. Send the _perform_soft_reset_ command (See **Section 3.4.10** ) and wait for the specified execution time. Then
    resend the _perform_self_test_ command and wait for the specified execution time.
2. If the memory error persists, power cycle the supply voltage, then resend the _perform_self_test_ command
    and wait for the specified execution time.
3. If the memory error persists, measurement results may be compromised.

**3.4.13 enable_testing_mode**

The _enable_testing_mode_ command is used to pause the ASC algorithm and temporarily disable updates to the
ASC state. It may be used for testing, _e.g._ , the CO 2 sensing performance during sensor qualification. The sensor
is in testing mode when the 2nd most significant bit of Byte 10 in the _read_measurement_ command ( **Section
3.4.3** ) output is equal to 1.

```
Command Command
Hex Code
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
enable_testi
ng_mode
```
```
0x3FBC - - - Enables the testing mode.
```

**3.4.14 disable_testing_mode**

The _disable_testing_mode_ command is used to exit the testing mode. The sensor is not in testing mode when
the 2nd most significant bit of Byte 10 from the _read_measurement_ ( **Section 3.4.3** ) output is equal to 0.

```
Command Command
Hex Code
```
```
Argument Consecutive Read Execution
Time (ms)
```
```
Description
```
```
disable_testi
ng_mode
```
```
0x3F3D - - - Disables the testing mode.
```
**3.4.15 perform_forced_recalibration**

The _perform_forced_recalibration_ command (FRC) is used to correct the sensor’s CO 2 concentration output with
an externally provided target CO 2 concentration. Ensure the sensor reading and environmental conditions,
including CO 2 concentration, are stable for the duration of the recommended operation sequence. See **Section
3.5** for the signal output and input conversion. Accepted input values are between 0 – 32 ’000 ppm.

```
Command
```
```
Command
Hex Code Argument^ Consecutive Read^
```
```
Execution
Time (ms) Description^
```
```
perform_for
ced_recalibr
ation
```
```
0x362F
```
```
Byte0: Target CO 2
concentration 8msb
Byte1: Target CO 2
concentration 8lsb
Byte2: CRC
```
```
Byte0: CO 2 concentration
correction 8msb
Byte1: CO 2 concentration
correction 8lsb
Byte2: CRC
```
#### 90

```
Performs FRC to the
target CO 2
concentration. Failed
command execution
will return 0xFFFF.
```
Recommended operation sequence:

1. Operate the sensor for a minimum of 30 s in continuous mode or 30 single shot measurements,
    corresponding to 5 minutes in single shot measurement mode with a 10 s sampling interval. For sampling
    intervals higher than 10 s, increase the operation time accordingly.
2. If operating in continuous mode, send the _stop_continuous_measurement_ command ( **Section 3.4.2** ) and wait
    for the specified execution time. Else if operating in single shot mode, the sensor must remain in idle mode,
    _i.e._ , must not be sent to sleep mode, after operation.
3. Send the _perform_forced_recalibration_ command with the target CO 2 concentration and optionally read out
    the applied FRC correction.

**3.4.16 get_product_id**

The _get_product_ID_ command can be used to identify the sensor and verify the communication.

```
Command
```
```
Command
Hex Code
```
```
Argument Consecutive Read
```
```
Execution
Time (ms)
```
```
Description
```
```
Get_product
_id 0x365B^ -^ 6 x (2 Bytes + 1 CRC)^1
```
```
Returns a 32 - bit product ID
and 64-bit unique serial
number.
```
```
Product Product ID
```
```
STCC4 0x0901018A
```

### 3.5 Conversion of Signal Input and Output

The conversion formulas for I^2 C command inputs and digitally calibrated outputs can be found in **Table 11**.

```
Signal Sym. Type Int. Conversion Formula Example
CO 2 gas
concentration
```
```
C Output i16 𝐶=Output 500 𝑝𝑝𝑚= 500
```
```
Relative
humidity
```
#### RH

Input u16 (^) 𝐼𝑛𝑝𝑢𝑡=

#### (𝑅𝐻 [%𝑅𝐻]+ 6 )( 216 − 1 )

#### 125

#### 29360 =

#### ( 50 %𝑅𝐻+ 6 )( 216 − 1 )

#### 125

```
Output u16 𝑅𝐻= 125 ∙
```
```
Output
216 − 1
```
#### − 6 50 %𝑅𝐻= 125 ∙

#### 29'

#### 216 − 1

#### − 6

```
Temperature T
```
Input u16 (^) 𝐼𝑛𝑝𝑢𝑡=

#### (𝑇 [°𝐶]+ 45 )( 216 − 1 )

#### 175

#### 26214 =

#### ( 25 °𝐶+ 45 )( 216 − 1 )

#### 175

```
Output u16 𝑇= 175 ∙
```
```
Output
216 − 1
```
#### − 45 25 °𝐶= 175 ∙

#### 26'

#### 216 − 1

#### − 45

Pressure P Input u16 (^) 𝐼𝑛𝑝𝑢𝑡=

#### 𝑃 [𝑃𝑎]

#### 2

#### 50560 =

#### 101′300 𝑃𝑎

#### 2

#### FRC CO 2

```
Concentration
Correction
```
#### CFRC

```
Input u 16 𝐼𝑛𝑝𝑢𝑡=𝐶𝑇𝑎𝑟𝑔𝑒𝑡^500 = 500 𝑝𝑝𝑚
```
```
Output u 16 𝐶𝐹𝑅𝐶=𝑂𝑢𝑡𝑝𝑢𝑡−32′768^ −^100 𝑝𝑝𝑚=32′668−32′^
```
**Table 11.** Signal input and output conversion formulas


## 4 Physical Specification

### 4.1 Package Description..............................................................................................................................................................

STCC4 consists of a metal cap LGA package. The sensor opening is protected by a membrane on the metal
cap. The protective membrane must not be removed or tampered with. Sensors must be treated according to
Moisture Sensitivity Level 1 (MSL1) as per IPC/JEDEC J-STD- 033 D^15. It is recommended to process the sensors
within one year after delivery date.

### 4.2 Package Outline

The STCC4 package outline with nominal dimensions and tolerances can be seen in **Figure 3**.

**Figure 3.** Package outline drawing. All dimensions in millimeters.

### 4.3 Land Pattern

The recommended land pattern for STCC4 can be seen in **Figure 4**. Pin 1 is denoted by a chamfered edge. Pads
on PCB are recommended to be non-solder mask defined (NSMD). The exact mask geometries, distances and
stencil thicknesses must be adapted to the required soldering processes.

**Figure 4.** Recommended land pattern. All dimensions in millimeters.

(^15) IPC/JEDEC J-STD- 033 D, April 2018


### 4.4 Traceability and Identification

All STCC4 sensors include a laser marking for identification and traceability as shown in **Figure 5**. The first line
of the laser marking includes a filled circle as a pin-1 indicator, followed by the product name (i.e. STCC4). The
second line of the laser marking serves as an internal batch tracking code.

**Figure 5.** Laser marking illustration

### 4.5 Soldering Instructions

The STCC4 sensors are designed to withstand a soldering profile for Pb-free assembly based on IPC/JEDEC J-
STD- 02016 in IR/Convection reflow ovens, as seen in **Figure 6**. Recommended conditions are peak temperature
TP = 260 °C, time within actual peak temperature tp = 30 s and a maximum ramp-down rate of < 4 °C/s. The
sensor is not compatible with vapor phase reflow soldering. The membrane on top of the cap must not be
removed or wetted with any liquid. Do not apply any board wash process step subsequently to the reflow
soldering. If the PCB hosting the sensor passes through multiple solder cycles, it is recommended to assemble
the sensor during the last solder cycle.

**Figure 6.** Recommended soldering profile based on IPC/JEDEC J-STD-020.

(^16) IPC/JEDEC J-STD-020E, December 2014


## 5 Tape and Reel Packaging

The specifications for the tape and reel packaging can be found in **Figure 7**. The tape and reel packaging is
based on DIN EN 60286- 317. The reel diameter is 7 inches for the 1’500-piece packaging size.

**Figure 7.** Technical drawing of tape and reel packaging. All dimensions in millimeters.

## 6 Ordering Information

```
Material Description Material Number Details Quantity (pcs)
STCC4-D-R3 3.000. 976 STCC4-D-R3 CO2 I2C, ToR 1500 pcs 1 ’
```
```
SEK-STCC4-Sensor 3.001.009 SEK-STCC4-Sensor Evaluation Kit, STCC
on dev. board w. cables
```
#### 1

**Table 12.** Ordering information

## 7 Revision History

```
Date Version Pages Changes
June 2025 1 all Initial version
```
(^17) DIN EN 60286- 3 :2007, April 2008


### Important Notices

**Warning, Personal Injury**

**Do not use this product as safety or emergency stop devices or in any other application where failure of the product could
result in personal injury. Do not use this product for applications other than its intended and authorized use. Before installing,
handling, using or servicing this product, please consult the data sheet and application notes. Failure to comply with these
instructions could result in death or serious injury.**

If the Buyer shall purchase or use SENSIRION products for any unintended or unauthorized application, Buyer shall defend,
indemnify and hold harmless SENSIRION and its officers, employees, subsidiaries, affiliates and distributors against all claims, costs,
damages and expenses, and reasonable attorney fees arising out of, directly or indirectly, any claim of personal injury or death
associated with such unintended or unauthorized use, even if SENSIRION shall be allegedly negligent with respect to the design
or the manufacture of the product.

**ESD Precautions**

The inherent design of this component causes it to be sensitive to electrostatic discharge (ESD). To prevent ESD-induced damage
and/or degradation, take customary and statutory ESD precautions when handling this product. See application note “ESD, Latchup
and EMC” for more information.

**Warranty**

SENSIRION warrants solely to the original purchaser of this product for a period of 12 months (one year) from the date of delivery
that this product shall be of the quality, material and workmanship defined in SENSIRION’s published specifications of the product.
Within such period, if proven to be defective, SENSIRION shall repair and/or replace this product, in SENSIRION’s discretion, free
of charge to the Buyer, provided that:

- notice in writing describing the defects shall be given to SENSIRION within fourteen (14) days after their appearance;
- such defects shall be found, to SENSIRION’s reasonable satisfaction, to have arisen from SENSIRION’s faulty design, material,
    or workmanship;
- the defective product shall be returned to SENSIRION’s factory at the Buyer’s expense; and
- the warranty period for any repaired or replaced product shall be limited to the unexpired portion of the original period.
This warranty does not apply to any equipment which has not been installed and used within the specifications recommended by
SENSIRION for the intended and proper use of the equipment. EXCEPT FOR THE WARRANTIES EXPRESSLY SET FORTH HEREIN,
SENSIRION MAKES NO WARRANTIES, EITHER EXPRESS OR IMPLIED, WITH RESPECT TO THE PRODUCT. ANY AND ALL
WARRANTIES, INCLUDING WITHOUT LIMITATION, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
PURPOSE, ARE EXPRESSLY EXCLUDED AND DECLINED.
SENSIRION is only liable for defects of this product arising under the conditions of operation provided for in the data sheet and
proper use of the goods. SENSIRION explicitly disclaims all warranties, express or implied, for any period during which the goods
are operated or stored not in accordance with the technical specifications.
SENSIRION does not assume any liability arising out of any application or use of any product or circuit and specifically disclaims
any and all liability, including without limitation consequential or incidental damages. All operating parameters, including without
limitation recommended parameters, must be validated for each customer’s applications by customer’s technical experts.
Recommended parameters can and do vary in different applications.
SENSIRION reserves the right, without further notice, (i) to change the product specifications and/or the information in this
document and (ii) to improve reliability, functions and design of this product.
**Headquarters and Subsidiaries**

```
Sensirion AG
Laubisruetistr. 50
CH-8712 Staefa ZH
Switzerland
```
```
phone: +41 44 306 40 00
fax: +41 44 306 40 30
info@sensirion.com
http://www.sensirion.com
```
```
Sensirion Inc., USA
phone: +1 312 690 5858
info-us@sensirion.com
http://www.sensirion.com
```
```
Sensirion Korea Co. Ltd.
phone: +82 31 337 7700~
info-kr@sensirion.com
http://www.sensirion.com/kr
```
```
Sensirion Japan Co. Ltd.
phone: +81 45 270 4506
info-jp@sensirion.com
http://www.sensirion.com/jp
```
```
Sensirion China Co. Ltd.
phone: +86 755 8252 1501
info-cn@sensirion.com
http://www.sensirion.com/cn
```
```
Sensirion Taiwan Co. Ltd
phone: +886 2 2218- 6779
info@sensirion.com
http://www.sensirion.com
```
```
To find your local representative, please visit
http://www.sensirion.com/distributors
```
```
Copyright^ © 202 4 , by SENSIRION. CMOSens® is a trademark of Sensirion. All rights reserved
```

