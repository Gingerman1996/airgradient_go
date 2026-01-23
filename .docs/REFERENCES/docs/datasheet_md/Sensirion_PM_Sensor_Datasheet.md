# Datasheet SPS

## Particulate Matter Sensor for Air Quality Monitoring and Control

## ▪ Unique long-term stability

## ▪ Advanced particle size binning

## ▪ Superior precision in mass

## concentration and number

## concentration sensing

## ▪ Small, ultra-slim package

## ▪ Fully calibrated digital output

##### Product Summary

##### The SPS30 Particulate Matter (PM) sensor is a technological breakthrough in optical PM sensors. Its

##### measurement principle is based on laser scattering and makes use of Sensirion’s innovative contamination-

##### resistance technology. This technology, together with high-quality and long-lasting components, enables precise

##### measurements from its first operation and throughout its lifetime of more than ten years. In addition, Sensirion’s

##### advanced algorithms provide superior precision for different PM types and higher-resolution particle size

##### binning, opening up new possibilities for the detection of different sorts of environmental dust and other particles.

##### With dimensions of only 41 x 41 x 12 mm^3 , it is also the perfect solution for applications where size is of

##### paramount importance, such as wall-mounted or compact air quality devices.

## Content

##### 1 Particulate Matter Sensor Specifications 2

##### 2 Electrical Specifications 3

##### 3 Hardware Interface Specifications 4

##### 4 Functional Overview 5

##### 5 Operation and Communication through the UART Interface 8

##### 6 Operation and Communication through the I^2 C Interface 16

##### 7 Mechanical Specifications 23

##### 8 Shipping Package 24

##### 9 Ordering Information 24

##### 10 Revision History 25


### 1 Particulate Matter Sensor Specifications

##### 1.1 Specification Overview

```
Parameter Conditions Value Units
Mass concentration range - 0 to 1’000 μg/m^3
Mass concentration size range PM1.0 0.3 to 1.0 μm
PM2.5 0.3 to 2.5 μm
PM4 0.3 to 4.0 μm
PM10 0.3 to 10.0 μm
Mass concentration precision^1 ,^2 for PM1 and PM2.5^3 0 to 100 μg/m^3  5 μg/m^3 + 5 % m.v.]
100 to 1000 μg/m^3  10 % m.v.
Mass concentration precision^1 ,^2 for PM4, PM10^4 0 to 100 μg/m^3  25 μg/m^3
100 to 1000 μg/m^3  25 % m.v.
Maximum long-term mass concentration precision
limit drift
```
```
0 to 100 μg/m^3 1.25 μg/m^3 / year
100 to 1000 μg/m^3 1.25 % m.v. / year
Number concentration range - 0 to 3’000 #/cm^3
Number concentration size range PM0.5 0.3 to 0.5 μm
PM1.0 0.3 to 1.0 μm
PM2.5 0.3 to 2.5 μm
PM4 0.3 to 4.0 μm
PM10 0.3 to 10.0 μm
Number concentration precision^1 ,^2 for PM0.5, PM
and PM2. 53
```
```
0 to 1000 #/cm^3  100 #/cm^3
1000 to 3000 #/cm^3  10 % m.v.
Number concentration precision^1 ,^2 for PM4, PM10^4 0 to 1000 #/cm^3  250 #/cm^3
1000 to 3000 #/cm^3  25 % m.v.
Maximum long-term number concentration precision
limit drift^2
```
```
0 to 1000 #/cm^3 12.5 #/cm^3 / year
1000 to 3000 #/cm^3 1.25 % m.v. / year
```
Sampling interval - (^1) 0.04 s
Typical start-up time^5 number
concentration
200 – 3000 #/cm^3 8 s
100 – 200 #/cm^3 16 s
50 – 100 #/cm^3 30 s
Sensor output characteristics PM2.5 mass concentration Calibrated to TSI DustTrak™
DRX 8533 Ambient Mode
PM2.5 number concentration Calibrated to TSI OPS 3330
Lifetime^6 24 h/day operation > 10 years
Acoustic emission level 0.2 m max. 25 dB(A)
Long term acoustic emission level drift 0.2 m max. +0.5 dB(A) / year
Additional T-dependent mass and number
concentration precision limit drift^2
temperature
difference to 25°C
typ. (^) 0.5 % m.v. / °C
Weight - (^26) .3 0.3 g
(^1) Also referred to as “between-parts variation” or “device-to-device variation”.
(^2) For further details, please refer to the document “Sensirion Particulate Matter Sensor Specification Statement”.
(^3) Verification Aerosol for PM2.5 is a 3% atomized KCl solution. Deviation to reference instrument is verified in end-tests for every sensor after calibration.
(^4) PM4 and PM10 output values are calculated based on distribution profile of all measured particles.
(^5) Time after starting Measurement-Mode, until a stable measurement is obtained.
(^6) Lifetime is based on mean-time-to-failure (MTTF) calculation. Lifetime might vary depending on different operating conditions.


```
Laser wavelength
(DIN EN 60825- 1 Class 1)
```
```
typ. 660 nm
```
**Table 1 :** Particulate matter sensor specifications. Default conditions of 25 2 °C, 5010% relative humidity and 5 V supply voltage
apply unless otherwise stated. ‘max.’ means ‘maximum’, ‘typ.’ means ‘typical’, ‘% m.v.’ means ‘% of measured value’.

##### 1.2 Recommended Operating Conditions

##### The sensor shows best performance when operated within recommended normal temperature and humidity range of

##### 10 to 40 °C and 20 to 80 % RH, respectively.

### 2 Electrical Specifications

##### 2.1 Electrical Characteristics

```
Parameter Conditions Min Typ Max Unit
Supply voltage - 4.5 5.0 5.5 V
```
```
Supply current
```
```
Sleep-Mode - 38 50
μA
Idle-Mode 300 330 360
Measurement-Mode 45 55 65
mA
Measurement-Mode, first 200ms (fan start) - - 80
Input high level voltage (VIH) - 2.31 - 5.
```
```
V
```
```
Input low level voltage (VIL) - 0 - 0.
Output high level voltage (VOH) - 2.9 3.3 3.
Output low level voltage (VOL) - 0 0 0.
```
**Table 2 :** Electrical specifications at 25°C.

##### 2.2 Absolute Minimum and Maximum Ratings

##### Stress levels beyond those listed in Table 3 may cause permanent damage to the device. These are stress ratings only

##### and functional operation of the device at these conditions cannot be guaranteed. Exposure to the absolute maximum

##### rating conditions for extended periods may affect the reliability of the device.

```
Parameter Min Max Unit
Supply voltage VDD - 0.3 5.
Interface Select SEL - 0.3 4.0 V
I/O pins (RX/SDA, TX/SCL) - 0.3 5.
Max. current on any I/O pin - 16 16 mA
Operating temperature range - 10 60
°C
Storage temperature range - 40 70
Operating humidity range 0 95 % RH
```
**Table 3 :** Absolute minimum and maximum ratings.


##### 2.3 ESD / EMC Ratings

##### Immunity (Industrial level)

```
Description Standard Rating
Electro Static Discharge IEC 61000 - 4 - 2 ±4 kV contact, ±8 kV air
Power-Frequency Magnetic Field IEC 61000- 4 - 8 30A/m, 50Hz and 60Hz
Radio-Frequency EM-Field AM-modulated IEC 61000- 4 - 3 80MHz - 1000MHz, 10V/m, 80% AM @1kHz
Radio-Frequency EM-Field AM-modulated IEC 61000- 4 - 3 1.4GHz – 6GHz, 3V/m, 80% AM @1kHz
```
##### Emission (Residential level)

```
Description Standard Rating
Emission in SAC for 30MHz to 230MHz IEC/CISPR 16 40dB(μV/m) QP @3m
Emission in SAC for 230MHz to 1000MHz IEC/CISPR 16 4 7dB(μV/m) QP @3m
Emission in SAC for 1GHz to 3GHz IEC/CISPR 16 70dB(μV/m) P, 50dB(μV/m) AP @3m
Emission in SAC for 3GHz to 6GHz IEC/CISPR 16 74dB(μV/m) P, 54dB(μV/m) AP @3m
```
### 3 Hardware Interface Specifications

##### The interface connector is located at the side of the sensor opposite to the air inlet/outlet. Corresponding female plug is

##### ZHR-5 from JST Sales America Inc. In Figure 1 a description of the pin layout is given.

(^) Pin Name Description Comments
1 VDD Supply voltage 5V ± 10%

###### 2

###### RX

```
UART: Receiving pin for
communication
```
```
TTL 5V and
LVTTL 3.3V
SDA I^2 C: Serial data input / output compatible^
```
###### 3

```
TX UART: Transmitting pin for
communication
```
```
TTL 5V and
LVTTL 3.3V
SCL I^2 C: Serial clock input compatible^
```
```
4 SEL Interface select
```
```
Leave floating to
select UART
Pull to GND to
select I^2 C
5 GND Ground Housing on GND
```
**Figure 1** : The communication interface connector is

#### located at the side of the sensor opposite to the air outlet.

```
Table 4 SPS30 pin assignment.
```
##### The SPS30 offers both a UART^7 and an I^2 C interface. For connection cables longer than 20 cm we recommend using

##### the UART interface, due to its intrinsic robustness against electromagnetic interference.

##### Note, that there is an internal electrical connection between GND pin (5) and metal shielding. Keep this metal shielding

##### electrically floating in order to avoid any unintended currents through this internal connection. If this is not an option,

##### proper external potential equalization between GND pin and any potential connected to the shielding is mandatory. Any

##### current though the connection between GND and metal shielding may damage the product and poses a safety risk

##### through overheating.

(^7) Universal Asynchronous Receiver Transmitter.

##### Pin 1 Pin 5


### 4 Functional Overview

##### 4.1 Operating Modes

```
Idle
```
```
~ 330 μA
```
```
Power on / Reset
```
```
Measurement
```
```
45 - 65 mA
```
```
Sleep
```
```
< 50 μA
Stop Measurement Wake-Up
```
```
Start Measurement Sleep
```
```
1 s
```
##### Idle

- After power on or reset the module is in Idle-Mode.
- Most of the internal electronics switched off /reduced power consumption.
- Fan and laser are switched off.
- The module is ready to receive and process any command.

##### Measurement

- The Measurement-Mode can only be entered from Idle-Mode.
- All electronics switched on / max. power consumption.
- The measurement is running and the module is continuously processing measurement data.
- New readings are available every second.

##### Sleep

- The Sleep-Mode can only be entered from Idle-Mode.
- Most of the internal electronics switched off / reduced power consumption.
- Fan and laser are switched off.
- Microcontroller is in Sleep-Mode.
- To minimize power consumption, the UART / I2C interface is also disabled.
- A wake-up sequence is needed to turn the module back on. See Wake-up command in the interface description.

##### 4.2 Fan Auto Cleaning

##### When the module is in Measurement-Mode an automatic fan-cleaning procedure will be triggered periodically following

##### a defined cleaning interval. This will accelerate the fan to maximum speed for 10 seconds in order to blow out the dust

##### accumulated inside the fan.

- Measurement values are not updated while the fan-cleaning is running.
- The default cleaning interval is set to 604’800 seconds (i.e., 168 hours or 1 week) with a tolerance of ±3%.
- The interval can be configured using the Set Automatic Cleaning Interval command.
- Set the interval to 0 to disable the automatic cleaning.
- Once set, the interval is stored permanently in the non-volatile memory.
- If the sensor is switched off, the time counter is reset to 0. Make sure to trigger a cleaning cycle at least every

##### week if the sensor is switched off and on periodically (e.g., once per day).

- The cleaning procedure can also be started manually with the Start Cleaning command.


##### 4.3 Measurement Output Formats

##### The measurement results can be read with the “Read Measured Values” command. The returned data structure depends

##### on the selected output format. The output format must be specified when stating the measurement with the “Start

##### Measurement command”.

##### IEEE754 float values

```
Byte #
Datatype Description
SHDLC I2C
0..3 0..5 big-endian float IEEE
Mass Concentration PM1.0 [μg/m³]
4..7 6..11 Mass Concentration PM2.5 [μg/m³]
8..11 12..17 Mass Concentration PM4.0 [μg/m³]
12..15 18..23 Mass Concentration PM10 [μg/m³]
16..19 24..29 Number Concentration PM0.5 [#/cm³]
20..23 30..35 Number Concentration PM1.0 [#/cm³]
24..27 36..41 Number Concentration PM2.5 [#/cm³]
28..31 42..47 Number Concentration PM4.0 [#/cm³]
32..35 48..53 Number Concentration PM10 [#/cm³]
36..39 54..59 Typical Particle Size^8 [μm]
```
##### Unsigned 16-bit integer values^9

```
Byte #
Datatype Description
SHDLC I2C
0..1 0..2 big-endian unsigned 16-bit integer
Mass Concentration PM1.0 [μg/m³]
2..3 3..5 Mass Concentration PM2.5 [μg/m³]
4..5 6..8 Mass Concentration PM4.0 [μg/m³]
6..7 9..11 Mass Concentration PM10 [μg/m³]
8..9 12..14 Number Concentration PM0.5 [#/cm³]
10..11 15..17 Number Concentration PM1.0 [#/cm³]
12..13 18..20 Number Concentration PM2.5 [#/cm³]
14..15 21..23 Number Concentration PM4.0 [#/cm³]
16..17 24..26 Number Concentration PM10 [#/cm³]
```
18..19 27..29 (^) Typical Particle Size^8 [nm]
(^8) The typical particle size (TPS) gives an indication on the average particle diameter in the sample aerosol. Such output correlates with the weighted average of the
number concentration bins measured with a TSI 3330 optical particle sizer. Consequently, lighter aerosols will have smaller TPS values than heavier aerosols.
(^9) Requires at least firmware version 2.


##### 4.4 Device Status Register

##### The Device Status Register is a 32-bit register that contains information about the internal state of the module.

```
31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
Warning
```
```
res. res. res. res. res. res. res. res. res. res. SPEED res. res. res. res. res.
```
```
15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
Error Error
res. res. res. res. res. res. res. res. res. res. LASER FAN res. res. res. res.
```
##### Note: All “res.” bits are reserved for internal use or future versions. These bits can be both 0 and 1 and should therefore

##### be ignored.

##### Bit 21 SPEED: Fan speed out of range

##### 0: Fan speed is ok.

##### 1: Fan speed is too high or too low.

- During the first 3 seconds after starting the measurement (fan start-up) the fan speed is not checked.
- The fan speed is also not checked during the auto cleaning procedure.
- Apart from the two exceptions mentioned above, the fan speed is checked once per second in the

##### measurement mode. If it is out of range twice in succession, the SPEED-bit is set.

- At very high or low ambient temperatures, the fan may take longer to reach its target speed after start-up. In

##### this case, the bit will be set. As soon as the target speed is reached, this bit is cleared automatically.

- If this bit is constantly set, this indicates a problem with the power supply or that the fan is no longer working

##### properly.

##### Bit 5 LASER: Laser failure

##### 0: Laser current is ok.

##### 1: Laser is switched on and current is out of range.

- The laser current is checked once per second in the measurement mode. If it is out of range twice in

##### succession, the LASER-bit is set.

- If the laser current is back within limits, this bit will be cleared automatically.
- A laser failure can occur at very high temperatures outside of specifications or when the laser module is

##### defective.

##### Bit 4 FAN: Fan failure, fan is mechanically blocked or broken.

##### 0: Fan works as expected.

##### 1: Fan is switched on, but the measured fan speed is 0 RPM.

- The fan is checked once per second in the measurement mode. If 0 RPM is measured twice in succession,

##### the FAN bit is set.

- The FAN-bit will not be cleared automatically.
- A fan failure can occur if the fan is mechanically blocked or broken.


### 5 Operation and Communication through the UART Interface

```
SPS 30
Connector
```
##### VDD ( 1 )

##### RX ( 2 )

##### SEL ( 4 )

##### TX ( 3 )

##### GND ( 5 )

##### VDD

##### Master TX

##### Master RX

##### NC

(^) The following UART settings have to be used:

- Baud Rate: 115’200 bit/s
- Data Bits: 8
- Parity: None
- Stop Bit: 1

#### Figure 2 : Typical UART application circuit.

##### 5.1 Physical Layer

##### The SPS30 has separate RX and TX lines with unipolar logic levels. See Figure 3.

```
StartBit Bit 0 Bit 1 Bit 2 Bit 3 Bit 4 Bit 5 Bit 6 Bit 7 StopBit
```
```
Bit Time
( 1 /Baudrate)
```
#### Figure 3 : Transmitted byte.

##### 5.2 SHDLC Frame Layer

##### On top of the UART interface, the SPS30 uses the very powerful and easy-to-implement SHDLC^10 protocol. It is a serial

##### communication protocol based on a master/slave architecture. The SPS30 acts as the slave device.

##### Data is transferred in logical units called frames. Every transfer is initiated by the master sending a MOSI^11 frame. The

##### slave will respond to the MOSI frame with a slave response, or MISO^12 frame. The two types of frames are shown in

##### Figure 4.

```
St art
( 0 x 7 E)
```
```
ADR
( 1 Byt e)
```
```
CMD
( 1 Byt e)
```
```
L
( 1 Byt e)
```
```
TX Data
0 ... 255 Byt es
```
```
CHK
( 1 Byt e)
```
```
St op
( 0 x 7 E)
```
```
Frame Content
```
```
MOSI Frame
```
```
St art
( 0 x 7 E)
```
```
ADR
( 1 Byt e)
```
```
CMD
( 1 Byt e)
```
```
L
( 1 Byt e)
```
```
RX Data
0 ... 255 Byt es
```
```
CHK
( 1 Byt e)
```
```
St op
( 0 x 7 E)
```
```
Frame Content
```
```
MISO Frame ( 1 St ateByt e)
```
#### Figure 4 : MOSI and MISO frames structure.

(^10) Sensirion High-Level Data Link Control.
(^11) Master Out Slave In. Frame direction from master to slave.
(^12) Master In Slave Out. Frame direction from slave to master.


##### Start/Stop Byte and Byte-Stuffing

##### The 0x7E character is sent at the beginning and at the end of the frame to signalize frame start and stop. If this byte

##### (0x7E) occurs anywhere else in the frame, it must be replaced by two other bytes (byte-stuffing). This also applies to the

##### characters 0x7D, 0x11 and 0x13. Use Table 5 for byte-stuffing.

```
Original data byte Transferred data bytes
0x7E 0x7D, 0x5E
0x7D 0x7D, 0x5D
0x11 0x7D, 0x
0x13 0x7D, 0x
```
#### Table 5 Reference table for byte-stuffing.

##### Example: Data to send = [0x43, 0x11, 0x7F] → Data transmitted = [0x43, 0x7D, 0x31, 0x7F].

##### Address

##### The slave device address is always 0.

##### Command

##### In the MOSI frame the command tells the device what to do with the transmitted data. In the MISO frame, the slave just

##### returns the received command.

##### Length

##### Length of the “TX Data” or “RX Data” field (before byte-stuffing).

##### State

##### The MISO frame contains a state byte, which allows the master to detect communication and execution errors.

```
b7 b6 b
Error-Flag Execution error code
```
#### Figure 5 : Status byte structure.

##### The first bit (b7) indicates that at least one of the error flags is set in the Device Status Register.

##### The “Execution error code” signalizes all errors which occur while processing the frame or executing the command. The

##### following table shows the error codes which can be reported from the device. Note that some of these errors are system

##### internal errors which require additional knowledge to be understood. In case of a problem, they will help Sensirion to

##### localize and solve the issue.

```
Error Code
Meaning
dec hex
0 0x00 No error
1 0x01 Wrong data length for this command (too much or little data)
2 0x02 Unknown command
3 0x03 No access right for command
4 0x04 Illegal command parameter or parameter out of allowed range
40 0x28 Internal function argument out of range
67 0x43 Command not allowed in current state
```
#### Table 6 Reference table for error codes.


##### Data

##### The data has a usable size of [0...255] bytes (original data, before byte-stuffing). The meaning of the data content

##### depends on the command.

##### Checksum

##### The checksum is built before byte-stuffing and checked after removing stuffed bytes from the frame. The checksum is

##### defined as follows:

##### 1. Sum all bytes between start and stop (without start and stop bytes).

##### 2. Take the least significant byte of the result and invert it. This will be the checksum.

##### For a MOSI frame use Address, Command, Length and Data to calculate the checksum.

##### For a MISO frame use Address, Command, State, Length and Data to calculate the checksum.

##### Example (MOSI frame without start/stop and without byte-stuffing):

```
Adr CMD L Tx Data 2 Bytes CHK
0x00 0x00 0x02 0x01, 0x03 0xF
```
##### The checksum is calculated as follows:

##### Adr 0x

##### CMD 0x

##### L 0x

##### Data 0 0x

##### Data 1 0x

##### Sum 0x

##### Least Significant Byte of Sum 0x

##### Inverted (=Checksum) 0xF

##### 5.3 SHDLC Commands

##### The following table shows an overview of the available SHDLC commands.

```
CMD Command Read / Write / Execute max. Response Time min. required Firmware
0x00 Start Measurement Execute 20 ms V1.
0x01 Stop Measurement Execute 20 ms V1.
0x03 Read Measured Value Read 20 ms V1.
0x10 Sleep Execute 5 ms V2.
0x11 Wake-up Execute 5 ms V2.
0x56 Start Fan Cleaning Execute 20 ms V1.
0x80 Read/Write Auto Cleaning Interval Read / Write 20 ms V1.
0xD0 Device Information Read 20 ms V1.
0xD1 Read Version Read 20 ms V1.
0xD2 Read Device Status Register Read 20 ms V2. 2
0xD3 Reset Execute 20 ms V1.
```
#### Table 7 Reference table for SHDLC commands.


##### 5.3.1 Start Measurement (CMD: 0x00)

##### Starts the measurement^13. After power up, the module is in Idle-Mode. Before any measurement values can be read, the

##### Measurement-Mode needs to be started using this command.

**MOSI Data:**

```
Byte # Datatype Description
0 uint8 Subcommand, this value must be set to 0x
1 uint8 Measurement Output Format:
0x03 : Big-endian IEEE754 float values
0x05 : Big-endian unsigned 16 - bit integer values
```
##### MISO Data: No data.

**Example Frames:**

##### MOSI

```
Start measurement with output format “Big-endian IEEE754 float values”:
0x7E 0x00 0x00 0x02 0x01 0x03 0xF9 0x7E
```
##### MISO

```
Empty response frame:
0x7E 0x00 0x00 0x00 0x00 0xFF 0x7E
```
##### 5.3.2 Stop Measurement (CMD: 0x01)

##### Stops the measurement^14. Use this command to return to the initial state (Idle-Mode).

##### MOSI Data: No data.

##### MISO Data: No data.

**Example Frames:**

##### MOSI 0x7E 0x00 0x01 0x00 0xFE 0x7E^

##### MISO

```
Empty response frame:
0x7E 0x00 0x01 0x00 0x00 0xFE 0x7E
```
##### 5.3.3 Read Measured Values (CMD: 0x03)

##### Reads the measured values from the module. This command can be used to poll for new measurement values. The

##### measurement interval is 1 second.

##### MOSI Data: No data.

##### MISO Data:

##### If no new measurement values are available, the module returns an empty response frame.

##### If new measurement values are available, the response frame contains the measurement results. The data format

##### depends on the selected output format, see 4.3 Measurement Output Formats.

**Example Frames:**

##### MOSI 0x7E 0x00 0x03 0x00 0xFC 0x7E^

(^13) This command can only be executed in Idle-Mode.
(^14) This command can only be executed in Measurement-Mode.


##### MISO

```
Empty response frame:
0x7E 0x00 0x03 0x00 0x00 0xFC 0x7E
```
```
Or response frame with new measurement values:
0x7E 0x00 0x03 0x00 0x28 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x
0xD4 0x7E
```
##### 5.3.4 Sleep (CMD: 0x 10 )^15

##### Enters the Sleep-Mode with minimum power consumption. This will also deactivate the UART interface, note the wake-

##### up sequence described at the Wake-up command.

##### MOSI Data: No data.

##### MISO Data: No data.

**Example Frames:**

##### MOSI 0x7E 0x00 0x10 0x00 0xEF 0x7E^

##### MISO 0x7E 0x00 0x1^0 0x00 0x00 0xEF^ 0x7E^

##### 5.3.5 Wake-up (CMD: 0x 11 )

##### Use this command to switch from Sleep-Mode to Idle-Mode. In Sleep-Mode the UART interface is disabled and must

##### first be activated by sending a low pulse on the RX pin. This pulse is generated by sending a single byte with the value

##### 0xFF.

##### If then a Wake-up command follows within 100ms, the module will switch on again and is ready for further commands in

##### the Idle-Mode. If the low pulse is not followed by the Wake-up command, the microcontroller returns to Sleep-Mode after

##### 100ms and the interface is deactivated again.

##### The Wake-up command can be sent directly after the 0xFF, without any delay. However, it is important that no other

##### value than 0xFF is used to generate the low pulse, otherwise it’s not guaranteed the UART interface synchronize

##### correctly.

##### MOSI Data: No data.

##### MISO Data: No data.

**Example Frames:**

##### MOSI

```
Send 0xFF to generate a low pulse in order to wake-up the interface:
0xFF
Wake-up command, within 100ms:
0x7E 0x00 0x11 0x00 0xEE 0x7E
```
##### MISO 0x7E 0x00 0x11 0x00 0x00 0xEE 0x7E^

##### Alternatively, if the software implementation does not allow to send a single byte with the value 0xFF, the Wake-up

##### command can be sent twice in succession. In this case the first Wake-up command is ignored, but causes the interface

##### to be activated.

(^15) This command can only be executed in Idle-Mode.


##### MOSI

```
First Wake-up command (just, activates the interface):
0x7E 0x00 0x11 0x00 0xEE 0x7E
Second Wake-up command, within 100ms (this finally wakes up the module):
0x7E 0x00 0x11 0x00 0xEE 0x7E
```
##### MISO 0x7E 0x00 0x11 0x00 0x00 0xEE 0x7E^

##### 5.3.6 Start Fan Cleaning (CMD: 0x56)

##### Starts the fan-cleaning manually^16. For more details, note the explanations given in 4.2 Fan Auto Cleaning.

##### MOSI Data: No data.

##### MISO Data: No data.

**Example Frames:**

##### MOSI 0x7E 0x00 0x56 0x00 0xA9 0x7E^

##### MISO 0x7E 0x00 0x56 0x00 0x00 0xA9 0x7E^

##### 5.3.7 Read/Write Auto Cleaning Interval (CMD: 0x80)

##### Reads/Writes the interval [s] of the periodic fan-cleaning. For more details, note the explanations given in 4.2 Fan Auto

##### Cleaning.

**MOSI Data:**

##### Read Auto Cleaning Interval:

```
Byte # Datatype Description
0 uint 8 Subcommand, this value must be set to 0x
```
##### Write Auto Cleaning Interval:

```
Byte # Datatype Description
0 uint8 Subcommand, this value must be set to 0x
1..4 uint32 Interval in seconds as big-endian unsigned 32-bit integer value.
```
**MISO Data:**

##### Read Auto Cleaning Interval:

```
Byte # Datatype Description
0..3 uint32 Interval in seconds as big-endian unsigned 32-bit integer value.
```
##### Write Auto Cleaning Interval: No data.

**Example Frames:**

##### MOSI

```
Read Auto Cleaning Interval:
0x7E 0x00 0x80 0x01 0x00 0x7D 0x5E 0x7E
Write Auto Cleaning Interval to 0 (disable):
0x7E 0x00 0x80 0x05 0x00 0x00 0x00 0x00 0x00 0x7A 0x7E
```
##### MISO

```
Response frame for “Read Auto Cleaning Interval”:
0x7E 0x00 0x80 0x00 0x04 0x00 0x00 0x00 0x00 0x7B 0x7E
Response frame for “Write Auto Cleaning Interval”:
0x7E 0x00 0x80 0x00 0x00 0x7F 0x7E
```
(^16) This command can only be executed in Measurement-Mode.


##### 5.3.8 Device Information (CMD 0xD0)

##### This command returns the requested device information. It is defined as a string value with a maximum length of 32

##### ASCII characters (including terminating null character).

**MOSI Data:**

```
Byte # Datatype Description
0 uint8 This parameter defines which information is requested:
0x00 : Product Type
0x01 : Reserved
0x02 : Reserved
0x03 : Serial Number
```
**MISO Data:**

```
Byte # Datatype Description
0...n string Requested Device Information as null-terminated ASCII string. The size of the string is limited to 32 ASCII
characters (including null character).
```
**Example Frames:**

##### Product Type:

##### Recommended to use as product identifier, returns always the string “00080000” on this product.

##### MOSI 0x7E 0x00 0xD0 0x01 0x0^0 0x2E^ 0x7E^

##### MISO

```
0x7E 0x00 0xD0 0x00 0x0 9 0x 30 0x 30 0x 30 0x 38 0x 30 0x 30 0x 30 0x 30 0x00 0x9B
0x7E
```
##### Serial Number:

##### MOSI 0x7E 0x00 0xD0 0x01 0x03 0x2B 0x7E^

##### MISO

```
0x7E 0x00 0xD0 0x00 0x15 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x
0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x30 0x00 0x5A 0x7E
```
##### 5.3.9 Read Version (0xD1)

##### Gets version information about the firmware, hardware, and SHDLC protocol.

##### MOSI Data: No data.

**MISO Data:**

```
Byte # Datatype Description
0 uint8 Firmware major version
1 uint8 Firmware minor version^17
2 uint8 Reserved: always 0
3 uint8 Hardware revision
4 uint8 Reserved: always 0
5 uint8 SHDLC protocol major version
6 uint8 SHDLC protocol minor version
```
(^17) Firmware minor version may change without notice, given full backwards compatibility.


**Example Frame:**

##### MOSI 0x7E 0x00 0xD1 0x00 0x2E 0x7E^

##### MISO

```
Firmware V2.1, Hardware V6, SHDLC V2.0:
0x7E 0x00 0xD1 0x01 0x07 0x02 0x01 0x00 0x06 0x00 0x02 0x00 0x1C 0x7E
```
##### 5.3.10 Read Device Status Register (0xD2)

##### Use this command to read the Device Status Register. For more details, note the explanations given in 4.4 Device Status

##### Register.

##### Note: If one of the device status flags of type “Error” is set, this is also indicated in every SHDLC response frame by

##### the Error-Flag in the state byte.

##### MOSI Data:

```
Byte # Datatype Description
0 uint8 0: Do not clear any bit in the Device Status Register after reading.
1: Clear all bits in the Device Status Register after reading.
```
**MISO Data:**

```
Byte # Datatype Description
0...3 big-endian, uint32 Device Status Register
4 uint8 Reserved for future use
```
**Example Frame:**

##### MOSI 0x7E 0x00 0xD2 0x01 0x00 0x2C 0x7E^

##### MISO 0x7E 0x00 0xD2 0x00 0x05 0x00 0x00 0x00 0x00 0x00 0x28 0x7E^

##### 5.3.11 Device Reset (CMD: 0xD3)

##### Soft reset command. After calling this command, the module is in the same state as after a Power-Reset. The reset is

##### executed after sending the MISO response frame.

##### Note: To perform a reset when the sensor is in sleep mode, it is required to send first a wake-up sequence to activate

##### the interface.

##### MOSI Data: No data.

##### MISO Data: No data.

**Example Frames:**

##### MOSI 0x7E 0x00 0xD3 0x00 0x2C 0x7E^

##### MISO 0x7E 0x00 0xD3 0x00 0x00 0x2C 0x7E^


### 6 Operation and Communication through the I^2 C Interface

```
Rp Rp
```
```
SPS 30
Connector
```
```
VDD ( 1 )
SDA ( 2 )
```
```
SEL ( 4 )
```
```
SCL ( 3 )
```
```
GND ( 5 )
```
```
VDD
```
```
SDA
SCL
```
(^) Usage:

- I^2 C address: 0x
- Max. speed: standard mode, 100 kbit/s
- Clock stretching: not used

##### Both SCL and SDA lines are open drain I/Os. They should

##### be connected to external pull-up resistors (e.g. Rp = 10 kΩ).

##### Important notice: in order to correctly select I^2 C as

##### interface, the interface select (SEL) pin must be pulled to

##### GND before or at the same time the sensor is powered up.

#### Figure 6 : Typical I^2 C application circuit.

##### Some considerations should be made about the use of the I^2 C interface. I^2 C was originally designed to connect two chips

##### on a PCB. When the sensor is connected to the main PCB via a cable, particular attention must be paid to

##### electromagnetic interference and crosstalk. Use as short as possible (< 10 cm) and/or well shielded connection cables.

##### We recommend using the UART interface instead, whenever possible: it is more robust against electromagnetic

##### interference, especially with long connection cables.

##### For detailed information on the I2C protocol, refer to NXP I2C-bus specification^18.

##### 6.1 Transfer Types

##### Set Pointer

##### Sets the 16-bit address pointer without writing data to the sensor module. It is used to execute commands, which do not

##### require additional parameters.

```
SDA
```
```
1 9 1 9 1 99
```
```
A 6 A 5 A 4 A 3 A 2 A 1 A 0
Wr iteACK
```
```
P
14
```
```
P
13
```
```
P
12
```
```
P
11
```
```
P
10
```
```
P
9
```
```
P
8
```
```
P
```
(^15) ACK
P
6
P
5
P
4
P
3
P
2
P
1
P
0
P
(^7) ACK
SCL
Pointer Address
S I 2 C Address
I 2 C Header
WA Pointer MSB A Pointer LSB A P
(^18) [http://www.nxp.com/documents/user_manual/UM10204.pdf](http://www.nxp.com/documents/user_manual/UM10204.pdf)


##### Set Pointer & Read Data

##### Sets the 16-bit address pointer and read data from sensor module. It is used to read sensor module information or

##### measurement results. The data is ready to read immediately after the address pointer is set. The sensor module transmits

##### the data in 2-byte packets, which are protected with a checksum.

```
SDA
```
```
1 91 91 99
```
A 6 A 5 A 4 A 3 A 2 A 1 A (^0) Wr iteACK 15 P 14 P 13 P 12 P 11 P 10 PP 9 P (^8) ACKP 7 P 6 P 5 P 4 P 3 P 2 P 1 P (^0) ACK
SCL
Pointer Address
S I 2 C Address
I 2 C Header
WA Pointer MSB A Pointer LSB AP
1
A 6 A 5 A 4 A 3 A 2 A 1 A (^0) ReadACK
S Slave Address
I 2 C Header
RA
9
9
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
Read Data 0
Data 0 A
Data 1 Checksum 0 A
1 9
C 7 C 6 C 5 C 4 C 3 C 2 C 1 C 0
ACK
Checksum

### ...

### ...

### ...

### ...

```
Read Data 1
```
```
91 9
```
```
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
```
```
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
```
```
Read Data (n- 2 )
```
```
1 9
```
```
C 7 C 6 C 5 C 4 C 3 C 2 C 1 C 0
NACK
```
```
Checksum
```
### ...

### ...

### ...

### ...

### A... Data (n- 2 ) A Data (n- 1 ) A Checksum AP

```
91
```
```
1
```
```
Read Data (n- 1 )
```
##### It is allowed to read several times in succession without setting the address pointer again. This reduces the protocol

##### overhead for periodical reading of the measured values.

##### Set Pointer & Write Data

##### Sets the 16-bit address pointer and writes data to the sensor module. It is used to execute commands, which require

##### additional parameters. The data must be transmitted in 2-byte packets which are protected by a checksum.

```
SDA
```
```
1 91 91 91 91 9
```
A 6 A 5 A 4 A 3 A 2 A 1 A (^0) Wr iteACK 15 P 14 P 13 P 12 P 11 P 10 PP 9 P (^8) ACKP 7 P 6 P 5 P 4 P 3 P 2 P 1 P (^0) ACKD 7 D 6 D 5 D 4 D 3 D 2 D 1 D (^0) ACKD 7 D 6 D 5 D 4 D 3 D 2 D 1 D (^0) ACK
SCL
Pointer Address Wr ite Data 0
S Slave Address
I 2 C Header
WA Pointer MSB A Pointer LSB A Data 0 A Data 1 A Checksum A
1 9
C 7 C 6 C 5 C 4 C 3 C 2 C 1 C 0
ACK
Checksum
1 91 9
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
D 7 D 6 D 5 D 4 D 3 D 2 D 1 D 0
ACK
Wr ite Data (n- 2 )
1 9
C 7 C 6 C 5 C 4 C 3 C 2 C 1 C 0
ACK
Checksum

### ...

### ...

### ...

### ...

```
Data (n- 2 ) A Data (n- 1 ) A Checksum AP
```
### ...

### ...

```
Wr ite Data 1
```
```
Wr ite Data (n- 1 )
```

##### 6.2 Checksum Calculation

##### The Read and Write Commands transmit the data in 2-byte packets, followed by an 8-bit checksum. The checksum is

##### calculated as follows:

```
Property Value
Name CRC- 8
Protected Data read and/or write data
Width 8 bit
Polynomial 0x31 (x^8 + x^5 + x^4 + 1)
Initialization 0xFF
Reflect Input false
Reflect Output false
Final XOR 0x
Example CRC(0xBEEF) = 0x
```
##### Please note that the checksums are used only for the 2-byte data packets. The command code itself already contains a

##### 3 - bit CRC and therefore no checksum must be appended to it.

##### 6.3 I 2 C Commands

##### The following table shows an overview of the available I^2 C commands.

```
Address
Pointer Command Name^ Transfer Type^
```
```
Parameter
length
including
CRC [bytes]
```
```
Response
length
including CRC
[bytes]
```
```
Command
execution time
```
```
min. required
Firmware
```
```
0x0010 Start Measurement Set Pointer & Write Data 3 - < 20 ms V1.
0x0104 Stop Measurement Set Pointer - - < 20 ms V1.
0x0202 Read Data-Ready Flag Set Pointer & Read Data - 3 - V1.
```
```
0x0300 Read Measured Values Set Pointer & Read Data
```
- float: 60
    integer: 30

###### -

###### V1.

```
0x1001 Sleep Set Pointer - - < 5 ms V2.
0x1103 Wake-up Set Pointer - - < 5 ms V2.
0x5607 Start Fan Cleaning Set Pointer - - < 5 ms V1.
```
```
0x8004 Read/Write Auto
Cleaning Interval
```
```
Set Pointer & Read/Write
Data
```
```
read: -
write: 6
```
```
read: 6
write: -
```
```
read: < 5 ms
write: < 20 ms
```
###### V1.

```
read: -
write: < 20ms
```
###### V2.

```
0xD002 Read Product Type Set Pointer & Read Data - 12
0xD033 Read Serial Number Set Pointer & Read Data - max. 48 - V1.
0xD100 Read Version Set Pointer & Read Data - 3 - V1.
```
```
0xD
```
```
Read Device Status
Register
```
```
Set Pointer & Read Data
```
###### - 6 -

###### V2. 2

```
0xD
```
```
Clear Device Status
Register
```
```
Set Pointer - - < 5 ms
V2.
```
```
0xD304 Reset Set Pointer - - < 100 ms V1.
```
**Table 8** Reference table for I^2 C commands.

```
uint8_t CalcCrc(uint8_t data[2]) {
uint8_t crc = 0xFF;
for(int i = 0; i < 2; i++) {
crc ^= data[i];
for(uint8_t bit = 8; bit > 0; --bit) {
if(crc & 0x80) {
crc = (crc << 1) ^ 0x31u;
} else {
crc = (crc << 1);
}
}
}
return crc;
}
```

##### 6.3.1 Start Measurement (0x0010)

##### Starts the measurement. After power up, the module is in Idle-Mode. Before any measurement values can be read, the

##### Measurement-Mode needs to be started using this command.

**Transfer Type: Set Pointer & Write Data
Pointer Address: 0x**

**Write Data:**

```
Byte # Description
0 Measurement Output Format
0x03 : Big-endian IEEE754 float values
0x05 : Big-endian unsigned 16 - bit integer values
1 dummy byte, insert 0x
2 Checksum for bytes 0, 1
```
##### 6.3.2 Stop Measurement (0x0104)

##### Stops the measurement. Use this command to return to the Idle-Mode.

##### Transfer Type: Set Pointer

**Pointer Address: 0x**

##### 6.3.3 Read Data-Ready Flag (0x0202)

##### This command can be used for polling to find out when new measurements are available. The pointer address only has

##### to be set once. Repeated read requests get the status of the Data-Ready Flag.

##### Transfer Type: Set Pointer & Read Data

**Pointer Address: 0x**

**Read Data:**

```
Byte # Description
0 unused, always 0x
1 Data-Ready Flag
0x00: no new measurements available
0x01: new measurements ready to read
2 Checksum for bytes 0, 1
```
##### 6.3.4 Read Measured Values (0x0300)

##### Reads the measured values from the sensor module and resets the “Data-Ready Flag”. If the sensor module is in

##### Measurement-Mode, an updated measurement value is provided every second and the “Data-Ready Flag” is set. If no

##### synchronized readout is desired, the “Data-Ready Flag” can be ignored. The command “Read Measured Values” always

##### returns the latest measured values.

##### Transfer Type: Set Pointer & Read Data

**Pointer Address: 0x**

##### The data format depends on the selected output format, see 4.3 Measurement Output Formats. Note that after every

##### two bytes, the checksum of the previous two bytes is transferred.


**Example Data Structure:**

```
Byte # Description
0, 1 two bytes of measurement data
2 checksum for bytes 0, 1
3, 4 two bytes of measurement data
5 checksum for bytes 3, 4
... ...
```
##### 6.3.5 Sleep (0x 1001 )^19

##### Enters the Sleep-Mode with minimum power consumption. This will also deactivate the I2C interface, note the wake-up

##### sequence described at the Wake-up command.

##### Transfer Type: Set Pointer

**Pointer Address: 0x**

##### 6.3.6 Wake-Up (0x 1103 )

##### Use this command to switch from Sleep-Mode to Idle-Mode. In Sleep-Mode the I2C interface is disabled and must first

##### be activated by sending a low pulse on the SDA line. A low pulse can be generated by sending a I2C-Start-Condition

##### followed by a Stop-Condition.

##### If then a Wake-up command follows within 100ms, the module will switch on again and is ready for further commands in

##### the Idle-Mode. If the low pulse is not followed by the Wake-up command, the microcontroller returns after 100ms to

##### Sleep-Mode and the interface is deactivated again.

```
SDA
```
```
1 9 1 9 1 99
```
A 6 A 5 A 4 A 3 A 2 A 1 A (^0) Wr iteACK^1501401301211101009081 ACK^7060504030201101 ACK
SCL
Pointer Address
S I 2 C Address
I 2 C Header
SP WA 0 x 11 A 0 x 03 A P
Pulse

##### Alternatively, if the software implementation does not allow to send a I2C-Start-Condition followed by a Stop-Condition,

##### the Wake-up command can be sent twice in succession. In this case the first Wake-up command is ignored, but causes

##### the interface to be activated.

##### Transfer Type: 2x Set Pointer

**Pointer Address: 0x**

##### 6.3.7 Start Fan Cleaning (0x5607)^20

##### Starts the fan-cleaning manually. For more details, note the explanations given in 4.2 Fan Auto Cleaning.

##### Transfer Type: Set Pointer

**Pointer Address: 0x**

(^19) This command can only be executed in Idle-Mode.
(^20) This command can only be executed in Measurement-Mode.


##### 6.3.8 Read/Write Auto Cleaning Interval (0x8004)

##### Reads/Writes the interval [s] of the periodic fan-cleaning. For more details, note the explanations given in 4.2 Fan Auto

##### Cleaning.

##### Note for FW Version < 2.2: After writing a new interval, this will be activated immediately. However, if the interval register

##### is read out after setting the new value, the previous value is returned until the next start/reset of the sensor module.

##### Transfer Type: Set Pointer & Read/Write Data

**Pointer Address: 0x8004**

**Read/Write Data:**

```
Byte # Description
0, 1 Most Significant Byte big-endian, unsigned 32-bit integer value:
2 Checksum for bytes 0, 1 Auto Cleaning Interval [s]^
3, 4 Least Significant Byte
5 Checksum for bytes 3, 4
```
##### 6.3.9 Read Device Information (0xD002, 0xD033)

##### This command returns the requested device information. It is defined as a string value with a maximum length of 32 ASCII

##### characters (including terminating null-character).

**Transfer Type: Set Pointer & Read Data
Pointer Address: Product Type: 0xD002** (always “00080000” without terminating null-character, recommended to use as product identifier)
**Serial Number: 0xD033**

**Read Data:**

```
Byte # Description
0 ASCII Character 0
1 ASCII Character 1
2 Checksum for bytes 0, 1
... ...
45 ASCII Character 30
46 ASCII Character 31
47 Checksum for bytes 45, 46
```
##### 6.3.10 Read Firmware Version (0xD100)

##### Gets firmware major.minor version.

##### Transfer Type: Set Pointer & Read Data

**Pointer Address: 0xD100**

**Read Data:**

```
Byte # Description
0 Firmware major version
1 Firmware minor version^21
2 Checksum for bytes 0, 1
```
(^21) Firmware minor version may change without notice, given full backwards compatibility.


##### 6.3.11 Read Device Status Register (0xD206)

##### Use this command to read the Device Status Register. For more details, note the explanations given in 4.4.

##### Transfer Type: Set Pointer & Read Data

**Pointer Address: 0xD206**

**Read Data:**

```
Byte # Description
0, 1 Most Significant Byte big-endian, unsigned 32-bit integer value:
2 Checksum for bytes 0, 1 Device Status Register^
3, 4 Least Significant Byte
5 Checksum for bytes 3, 4
```
##### 6.3.12 Clear Device Status Register (0xD210)

##### Clears the device status register. For more details, note the explanations given in 4.4.

##### Transfer Type: Set Pointer

**Pointer Address: 0xD210**

##### 6.3.13 Device Reset (0xD304)

##### Device software reset command. After calling this command, the module is in the same state as after a power reset.

##### Note: To perform a reset when the sensor is in sleep mode, it is required to send first a wake-up sequence to activate

##### the interface.

##### Transfer Type: Set Pointer

##### Pointer Address: 0xD304


### 7 Mechanical Specifications

##### 7.1 Product Outline Drawings

**Figure 7 :** Package outline dimensions of the SPS30 from different views. Tolerances included. All lengths are given in mm.
Dimensions in brackets include plastic fixation elements.

##### 7.2 Label

##### The SPS30 has a label on one side of the sensor. The label contains information about

##### ▪ Sensirion product name

##### ▪ Production year

##### ▪ Production date (month - day)

##### ▪ Serial number (XXXX-XXXX-XXXX-XXXX)

##### ▪ QR-code containing the information above

**Figure 8 :** Label on SPS30


### 8 Shipping Package

##### The SPS30 is shipped in stackable trays with 56 pieces each. Non-packaged tray dimensions are given in

##### Figure 9. Packaged tray dimensions are 670 mm x 460 mm x 45 mm. The weight of each full packaged tray (including

##### sensors) is 2.4 kg.

**Figure 9 :** 56 - sensor tray dimensions (in mm).

### 9 Ordering Information

##### The SPS30 and its evaluation kit can be ordered via the article numbers listed in Table 9.

```
Product Description Article Number
SPS30 sensor Particulate Matter Sensor 1 - 101638 - 10
SEK-SPS30 evaluation kit SPS30 sensor and USB evaluation kit 3.000.119
```
**Table 9 :** SPS30 and evaluation kit ordering information.

##### 457.00

##### (42.50)

##### (42.50)

##### (15.00)

##### 22.50

##### 379.00


### 10 Revision History

```
Date Version Page(s) Changes
27. March 2020 1.0 All Initial version
```
27. June 2023 2.0 2 Improved mass concentration precision^ specification^
    for PM1 and PM2.5 (Table 1)


## Important Notices

**Warning, Personal Injury**

**Do not use this product as safety or emergency stop devices or in any other application where failure of the product could result
in personal injury. Do not use this product for applications other than its intended and authorized use. Before installing, handling,
using or servicing this product, please consult the data sheet and application notes. Failure to comply with these instructions
could result in death or serious injury.**

If the Buyer shall purchase or use SENSIRION products for any unintended or unauthorized application, Buyer shall defend, indemnify and
hold harmless SENSIRION and its officers, employees, subsidiaries, affiliates and distributors against all claims, costs, damages and
expenses, and reasonable attorney fees arising out of, directly or indirectly, any claim of personal injury or death associated with such
unintended or unauthorized use, even if SENSIRION shall be allegedly negligent with respect to the design or the manufacture of the product.

**ESD Precautions**

The inherent design of this component causes it to be sensitive to electrostatic discharge (ESD). To prevent ESD-induced damage and/or
degradation, take customary and statutory ESD precautions when handling this product. See application note “ESD, Latchup and EMC” for
more information.

**Warranty**

SENSIRION warrants solely to the original purchaser of this product for a period of 12 months (one year) from the date of delivery that this
product shall be of the quality, material and workmanship defined in SENSIRION’s published specifications of the product. Within such
period, if proven to be defective, SENSIRION shall repair and/or replace this product, in SENSIRION’s discretion, free of charge to the Buyer,
provided that:

- notice in writing describing the defects shall be given to SENSIRION within fourteen (14) days after their appearance;
- such defects shall be found, to SENSIRION’s reasonable satisfaction, to have arisen from SENSIRION’s faulty design, material, or
    workmanship;
- the defective product shall be returned to SENSIRION’s factory at the Buyer’s expense; and
- the warranty period for any repaired or replaced product shall be limited to the unexpired portion of the original period.
This warranty does not apply to any equipment which has not been installed and used within the specifications recommended by SENSIRION
for the intended and proper use of the equipment. EXCEPT FOR THE WARRANTIES EXPRESSLY SET FORTH HEREIN, SENSIRION
MAKES NO WARRANTIES, EITHER EXPRESS OR IMPLIED, WITH RESPECT TO THE PRODUCT. ANY AND ALL WARRANTIES,
INCLUDING WITHOUT LIMITATION, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE
EXPRESSLY EXCLUDED AND DECLINED.
SENSIRION is only liable for defects of this product arising under the conditions of operation provided for in the data sheet and proper use
of the goods. SENSIRION explicitly disclaims all warranties, express or implied, for any period during which the goods are operated or stored
not in accordance with the technical specifications.
SENSIRION does not assume any liability arising out of any application or use of any product or circuit and specifically disclaims any and all
liability, including without limitation consequential or incidental damages. All operating parameters, including without limitation recommended
parameters, must be validated for each customer’s applications by customer’s technical experts. Recommended parameters can and do
vary in different applications.
SENSIRION reserves the right, without further notice, (i) to change the product specifications and/or the information in this document and (ii)
to improve reliability, functions and design of this product.
Copyright^ © 202 3 , by SENSIRION. CMOSens® is a trademark of Sensirion. All rights reserved

## Headquarters and Subsidiaries

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
phone: +82 31 337 7700~3
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
phone: +886 2 22 18 - 6779
info@sensirion.com
http://www.sensirion.com
To find your local representative, please visit http://www.sensirion.com/distributors
```

