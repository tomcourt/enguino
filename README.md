# Enguino

## General Description

Enguino (a portmanteau of engine and Arduino) is an inexpensive _(about $100)_, lightweight _(2 ounces w/o case or cables)_, small _(about the size of cigarette pack)_, [open source] engine monitor for experimental aircraft. The engine monitor is displayed on a tablet as a web page. Here is an [example] of a typical Enguino display. The tablet can be an iPad, an Android or any other tablet that includes a modern web browser and has wifi.

Enguino is intended to work with the [Stratux] ADS-B receiver which is also open source. The Stratux acts as a Wifi router for Enguino. For airplanes not equipped with a Stratux it is still possible to create an Enguino system with slightly different hardware and a small change to the configuration.

The hardware consists of a tiny single board computer called an [Arduino]. This board is similar to the Raspberry Pi used in the Stratux, although the Arduino has a much simpler computer on it. Despite being a simple computer, the Arduino is much better at connecting to the real world.

Because Enguino is experimental, it is recommended that you don't replace your legally required gauges with Enguino. Also tablets and and wifi communication alone shouldn't be counted upon for critical flight information. Furthermore you may not have a dedicated tablet for Enguino. For these reasons an auxiliary display is an optional part of Enguino. It consists of a simple LED display that normally displays tachometer and fuel gauges but can also display engine alerts and warnings.

## Hardware

The main board is a particular type of Arduino called the [Leonardo ETH]. This Arduino contains an  ethernet adapter which is then attached to the Stratux via an RJ-45 cable.

Arduino expansion boards are referred as _shields_. Attached to the Leonardo is an 8 input [thermocouple shield]. This assumes you want CHT/EGT temperatures. If not a simple [proto shield] can be used instead.

The prototyping area of the shield will have a number of resistors and other components added to it to complete the Enguino. Some basic soldering will be required to complete this project. A parts list can be found further down. **TBD** - Instructions for assembly.

The engine sensors themselves are fairly generic. The default configuration uses Vans Aircraft engine sensors. Although the thermocouple board is designed for the K-style thermocouples that Van's uses for CHT and EGT, J-style can also be used with it with a configuration change. Most other sensors are of the 'resistive' type, typically 33-240 ohms. **TBD** - The tach and MP sensors currently need further study.

## Software

On the tablet, attach to the Stratux network, use the browser to navigate to 192.168.0.111, save the link to the home page. Go to the home screen and press the Enguino icon, the web page will open up in full screen.

The firmware is installed on the Arduino using the Arduino IDE. First install the [Arduino IDE]. Add the Ethernet 2 libary **TBD** add detailed instructions. Then download the [Enguino source code]. Any configuration must be done before installing the firmware. Configuration involves editing the config.h file. After editing the configuration, connect the Arduino to your PC via a USB cable. This will also power up the Arduino and a green light will appear next to the USB connector. Start up the Arduino software on your PC. Go `File`, `Open` and select the file `enigno.ino`. Then go `Sketch` and `Upload`. The other green LED next to the power LED will flash until the file is uploaded. Done Uploading will appear near the bottom of the window.

## Configuration

### Sensors

### Layout
During design and testing of the layout it may be helpful to connect the Enguino to your local network instead of the Stratux. You may need to update the IP address in the Enguino.ino file. If you do do so, remember to change it back after finalizing the layout. To see values on the gauges while testing, uncomment out the line `#define RANDOM_SENSORS 1`

### Ranges

Setting the ranges of the gauges (green, yellow, red regions) will require updating a spreadsheet and then pasting the values into a file. Details are **TBD**.

## Stratux
The Stratux has to be configured to route network traffic between the wired ethernet to the Enguino and the wifi connecting to a tablet. Note - these instructions are for Stratux v0.8r2, newer versions may require modification.

* Boot the Stratux. For first boot, wait 3 minutes, for rebooting wait 30 seconds.
* Either connect a monitor and keyboard to the Rapberry Pi and login with un: `pi`, pw: `raspberry` then skip ahead in these instructions to sudo nano...
* Or connect to the Stratux wifi network (this will need to be repeated when rebooting)
* On the Mac, start terminal. On a PC, run PuTTY.
* Once connected type `ssh pi@192.168.10.1`
* You may get a message "Are you sure you want to continue connecting (yes/no)?" `yes`
* Password: `raspberry`

`sudo nano /etc/dhcp/dhcpd.conf`

Scroll to the bottom and right after the _subnet..._  and _range..._ lines add

        option routers 192.168.0.1;

Ctrl-o, enter, ctrl-x

`sudo nano /etc/network/interfaces`

Change _iface eth0 inet dhcp_ to the following:

        auto eth0
        iface eth0 inet static
          address 192.168.0.1
          netmask 255.255.255.0

Ctrl-o, enter, ctrl-x

`sudo nano /etc/sysctl.conf`

Scroll to bottom and add

        net.ipv4.ip_forward=1

Ctrl-o, enter, ctrl-x

Reboot.  
Connect to Stratux wireless network again.  
Browse to 192.168.10.1, confirm Stratux control panel shows up.  
Browse to 192.168.0.111, confirm Engiuno panel shows up.

## Design Details

### Arduino

The Leonardo is rated to run from 6-20 volts although they suggest keeping it between 7-12 volts. They claim to worry about the voltage regulator overheating and 'damaging the board', but the regulator chip (88% efficient) does have a thermal shutdown feature at 150 deg C. The text appears to be a carry over from previous Arduino's which used a linear supply. **TBD** if heating is a problem. **TBD** if voltage sag during starting is a problem.

The supply is rated for 1000 ma. The Leonardo uses 82 mA. Testing will need to be done to determine actual power used.

**Leonardo Pins Mapping to ATmega 32U4**

| Arduino | 32U4  | Use         |IRQ|Analog|Counter|
|---------|-------|-------------|---|------|-------|
| D0      | PD2   | Serial RX   | * |      |       |
| D1      | PD3   | Serial TX   | * |      |       |
| D2      | PD1   | SDA         | * |      |       |
| D3      | PD0   | SCL/PWM     | * |      |       |
| D4      | PD4   | TC Mux A0   |   |   *  |   *   |
| D5      | PC6   | TC Mux A1   |   |      |       |
| D6      | PD7   | TC Mux A2   |   |   *  |   *   |
| D7      | PE6   | TC Mux EN   | * |      |       |
| D8      | PB4   | TC CS       |   |   *  |       |
| D9      | PB5   | Analog 9    |   |   *  |       |
| D10     | PB6   | Ether CS    |   |   *  |       |
| D11     | PB7   | PWM         |   |      |       |
| D12     | PD6   | TC MISO     |   |   *  |       |
| D13     | PC7   | TC SCLK/LED |   |      |       ||

The D6 and D12 pins support counters which would be useful for the tach and fuel flow. But the thermocouple board also conflicts with this.

Interrupt pins will be used instead. With the tach, assume 2 pulses per revolution at 2700RPM the tach will max out at 90/cps so as long as IRQ disable time is <11ms no error should be expected. For the fuel flow with a k-factor of 68,000 and 13GPH it will max out at 250/cps so IRQ disable time must be <4ms.

The SD card on the Leonardo can not be used with the thermocouple board attached as they both use pin D4.


### Software

The Arduino serves up several web pages which are used to create the display.
* Main page (http://) contains the static elements of the engine display.
* Dynamic page (http://d) contains the dynamic web content that displays the gauges.
* Setup page (http://s) contains a simple web form for setting the fuel totalizer.

The main webpage has a timer running in Javascript on the tablet that invokes a reload of the dynamic portion of the page using an AJAX request. The dynamic portion is drawn using SVG (scaled vectored graphics) that is created on the fly by the Arduino. The configuration tables in config.h control how the SVG is drawn, for example the layout of the gauges on the screen.

### Sensors

The sensor system is fairly generic but currently only Van's Aircraft engine sensors have predefined configurations. The resistive sensors will have a 240 ohm(1%) pull up to Vcc. This will require up to 18 ma per sensor, or for the typical 5 sensors (fuel x 2, oil-p, oil-t, fuel-p) 90 ma total. This provides a good compromise between power usage, heat and loss of resolution. This provides 9 bits of resolution. To limit resistor heating to reasonable levels, .5 watt resistors should be used. Resistor temp. rise should be no more than 100 deg. C in free air. A resistance < 16 ohms will register as a short failure. > 480 ohms will register as an open failure. To convert from ADC units to ohms use this formula `ohms = 240 * (adc / (1024-adc))`, this will require long divided by long division unfortunately or a lookup/interpolation table.

All but the oil-temp sensor are 33.5-240 ohm sensors. These usually scale linearly by resistance. Some may scale closer to ADC counts as follow: zero=240ohm/511adc, half=103ohm/716adc, full=33.5ohm/898adc. The oil temperature sensor (for a Rochester 3080-37) as follows (degF=ohms): 100=497, 150=179, 200=72, 250=34. This is a thermistor. Using a Steinhar-Hart calculator the conversion formula becomes `degrees Kelvin = 1 / (0.0016207535760566691 + 0.0002609330007304247 * log(R) + -1.0278556187396396e-7 * log(R)^3)`. A lookup table will certainly be required.

For resistive sensors still attached to the gauge, the gauge itself provides the pull up resistance and voltage. For Vans Aircraft engine instruments the pull up is 5 volts and the resistor is about 227 ohms(measured externally, internally the resistor appears to be 240 ohms, 5%). The pin can be directly connected if the voltage can't exceed Vcc by more than .5v. Otherwise a series resistor as discussed later could be attached to isolate the pin.

For thermocouple sensors the board will detect both open and short. It directly reads out in degrees C (.25 resolution). This will optionally be converted to F. To support J style thermocouples, the C output will be adjusted by taking the thermocouple reading (before the CJT adjustment) `newC = oldC * 46677 / 65536`

For the voltage sensor a 4:1 voltage divider consisting of a 1k and 3.01k (1%) resistor is used. This limits the draw to .1 watt at 20 volts or .05 at 14 volts.

Typical sensors are Stewart Warner. The tachometer is supplied with 12 volts, a hall effect sensor that returns 5 volt pulses, 8/16 PPR, TBD. The manifold pressure sensor is believed to be 0-100mV ratio-metric.

For a sensor that might contain a voltage higher than Vcc using a voltage divider as used on the voltage sensor will cause a significant load which could create issues if still attached to a backup gauge. Alternatively a 15K ohm resistor between the sensor and the pin will safely clip voltages between 20.5 and -15.5 volts. The goal here is to limit the internal clamping diodes to no more than 1ma after the .7 volt diode drop.

### Auxiliary Display

The auxiliary display consists of two lines of a 4 digit [7 segment LED displays]. Limited text would be shown on the s. Some letters are displayed in the wrong case, for example 't' instead of 'T'. The letters 'M', 'W' and 'X' can not be displayed in an any form. Others like 'V' end up looking the same as 'U'.

A caution/warning [bicolor LED] is reworked by soldering onto the 'colon' column of the top display. This would be easier to see and interpret than the decimal point indicators. Red - warning, yellow - alert, green - ok.

An acknowledge pushbutton is also part of the display.

In normal operation the tachometer and fuel for left and right tank in gallons would be shown as `2300` / `15:10`. Excessive RPM's would cause the tachometer to blink. On power up the following sequence would be shown:
* `Hobb` / `123.4`    only last 4 digits of hobbs shown
* `bAt` / `12.2`      alternator-battery voltage
* `   0` / `15:10`

Whenever out of range indicators happen the display would switch to showing the condition and the value, for example `OPLo` / `2.4`. A 'warning' (red) out of range will be indicated by the first line blinking quickly and the warning LED also blinking red. Pressing the 'Acknowledge' button would return the display to showing tachometer and fuel. The warning LED would continue to be red. In the case of a caution condition (yellow range) the LED would turn yellow. When the engine isn't turning some warnings are suppressed and 'alternator voltage' becomes 'battery voltage' which has a lower warning level. Holding the acknowledge button for longer than a second would show previously acknowledged warnings, followed by any outstanding cautions, followed by readings with each press. A long hold again would return the display to tachometer and fuel. A very long press (>4 seconds) toggles a dim mode.

The warning/cautions are prioritized as follows:
* `FPLo`
* `OPLo`
* `Fuel`  low fuel for either tank
* `OPHi`
* `AltH`  > 15 volts
* `OTHi`
* `ChtH`
* `OTLo`

Other messages would include:
* `AltL`
* `bAt`   normal battery when not charging   
* `batL`
* `Alt`   normal charge system voltage
* `OP`
* `Ot`
* `FP`
* `Cht`
* `Shrt`  short circuit, shown on second line
* `OPEn`  open circuit, shown on second line

To prevent having to re-acknowledge warnings there would be both temporal and range hysteresis built in.

### Parts list
* --- Arduino Leonardo ETH - Digikey 1050-1007-ND
* --- Thermocouple Multiplexer Shield ($50 electronics123)
* Bicolor LED - Digikey 754-1232-ND
* T 1 3/4 LED holder for panel - Digikey 67-1332-ND
* 2 Adafruit 7 segment displays - Digikey 1528-1473-ND
* 10 240 ohm 1% resistors - Digikey A121513CT-ND
* 15K ohm 5% resistors - Digikey
* 3K ohm 1% resistor - Digikey 3.01KXBK-ND
* 1k ohm 1% resistor - Digikey 1.00KXBK-ND
* 10 input screw terminal block - Digikey ED10567-ND
* .025 square breakaway headers - Digikey 929647-04-35-ND - will probably work well on the thermocouple board w/o a header extension.
* jack for auxiliary display 6 pin - Digikey 455-2271-ND
* header for auxiliary display 6 pin - Digikey 455-2218-ND
* contacts for header x 10 - Digikey 455-1135-1-ND
* pushbutton switch - Digikey EG2015-ND
* K style thermocouple wire, 24-26 gauge, EBay

### Future stuff
* It may be possible to support 2 thermocouples without the thermocouple multiplexer shield by using the differential mode ADC, 40x gain and a thermistor. Only 8 bits are usable with 40x. 488 uV per count works out to 21.5 deg. F resolution with a K type thermocouple. The 2.56 volt internal reference would double the resolution and oversampling could probably double it again.
* The Arduino Yun would support airplanes lacking a Stratux. The code would need to use the 'bridge' objects instead of the ethernet objects. Use #ifdef AVR_YUN to flex the code.
* Themes - a dark theme could be created easily enough by adjusting the styles. A larger text theme for the gauges would involve more defines and stringizing them for the SVG.
* Warning lights instead of auxillary display? A board with 4 caution/warning LEDs (red/green common cathode [bicolor LED]). Turning red and green on produces yellow. Alternatively use a module like the [BOB-13884] to provide 3 RGB LED's.
* Use digitalFastWrite for smaller code - https://github.com/NicksonYap/digitalWriteFast
* Create mult16x16to32 function for smaller code - https://github.com/rekka/avrmultiplication


[open source]:https://en.wikipedia.org/wiki/Open-source_model
[Stratux]:http://stratux.me
[example]:http://www.tcourt.net/efis
[Arduino]:https://www.arduino.cc
[Leonardo ETH]:http://www.arduino.org/products/boards/arduino-leonardo-eth
[thermocouple shield]:https://oceancontrols.com.au/KTA-259.html
[proto shield]:https://www.amazon.com/Arduino-Proto-Shield-R3-Assembled/dp/B007QXTRNA
[Arduino IDE]:https://www.arduino.cc/en/Main/Software
[bicolor LED]:http://www.kingbrightusa.com/images/catalog/SPEC/WP59EGW.pdf
[MCP23008]:http://ww1.microchip.com/downloads/en/DeviceDoc/21919e.pdf
[BOB-13884]:https://www.sparkfun.com/products/13884
[7 segment LED displays]:https://www.adafruit.com/product/878
[14 segment LED display]:https://www.adafruit.com/product/1911
