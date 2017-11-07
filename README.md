# Enguino

## General Description

Enguino (a portmanteau of engine and Arduino) is an inexpensive _(as cheap as $100)_, lightweight _(2 ounces w/o case or cables)_, small _(about the size of a bar of bath soap)_, [open source] engine monitor for experimental aircraft. The engine monitor is displayed on a tablet as a web page. Here is an [example] of a typical Enguino display. The tablet can be an iPad, an Android or any other tablet that includes a modern web browser and has wifi.

Enguino is intended to work with the [Stratux] ADS-B receiver which is also open source. The Stratux acts as a Wifi router for Enguino. For airplanes not equipped with a Stratux it is still possible to create an Enguino system with slightly different hardware and a small change to the configuration.

The hardware consists of a tiny single board computer called an [Arduino]. This board is similar to the Raspberry Pi used in the Stratux, although the Arduino has a much simpler computer on it. Despite being a simple computer, the Arduino is much better at connecting to the real world.

Enguino is the combination of the Arduino and and another board that connects to 8 thermocouples for CHT and EGT, a tachometer, a fuel flow transducer and seven generic inputs. Heres a [photo].

Because Enguino is experimental, it is recommended that you don't replace your legally required gauges with Enguino until you've confirmed to yourself that its readings are accurate and reliable. Also tablets and and wifi communication alone shouldn't be counted upon for critical flight information. Furthermore you may not have a dedicated tablet for Enguino. For these reasons an auxiliary display is an optional part of Enguino. It consists of a simple LED display that normally displays tachometer and fuel gauges but can also display engine alarms and readings.

## Aux Display

The auxiliary display will fit in a 3 1/8" cutout. Here is a [photo of the prototype]. It is showing the default page for a fixed pitch prop airplane. It is showing RPM's on top and left and right fuel in gallons on the bottom. The red LED in top shows a warning alarm is active.

In the middle is a two lines of 4 digit 7 segment LED displays. Limited text is also shown on the display.

The master caution/warning annunciator, a multi-colored LED is on top. If any gauge, even those not currently being shown, is in the red range it will show red, if any in is in the yellow range it will show yellow, otherwise it will show green.

A pushbutton is below the display. Tapping the button acknowledges an alert if one is displayed, otherwise it switches to the next page of information. Holding it for at least a second redisplays all alerts that have been suppressed. Holding it for 3 seconds or more toggles dimming the display for night flight.

In normal operation (for a fixed pitch prop) the tachometer and fuel for left and right tank in gallons is shown as `2300` / `15:10` (2300 rpm, 15 gallons left tank, 10 gallons right). Excessive RPM's cause the tachometer to blink. On power up the following sequence would be shown:
* `Hobb` / `123.4`    only last 4 digits of hobbs shown
* `bAt` / `12.2`      alternator-battery voltage
* `   0` / `15:10`

Whenever out of range indicators happen the display switch to showing the condition and the value, for example `OP L` / `10`. A 'warning' (red) out of range will be indicated by the first line blinking and the annunciator LED also blinking red. Pressing the pushbutton stops the blinking. Pressing it again cycles the display to the next information page. The annunciator LED continues to be red as long as the condition persists. In the case of a caution condition (yellow range) the annunciator LED turns yellow. When the engine isn't turning some alerts are suppressed.

## Hardware

The main board is a particular type of Arduino called the [Leonardo ETH]. This Arduino contains an  ethernet adapter which is then attached to the Stratux via an RJ-45 cable.

Arduino expansion boards are referred as _shields_. Attached to the Leonardo is an 8 input [thermocouple shield]. This assumes you want CHT/EGT temperatures. If not a simple [proto shield] can be used instead.

The prototyping area of either of the shields will have a number of resistors and other components added to it to complete the Enguino. Some basic soldering will be required to complete this project. A parts list can be found further down. **TBD** - Instructions for assembly or create circuit boards.

## Software

On the tablet, attach to the Stratux wifi network, use the browser to navigate to 192.168.0.111, save the link to the home page. Go to the home screen and press the Enguino icon, the web page will open up in full screen.

The firmware is installed on the Arduino using the Arduino IDE. First install the [Arduino IDE]. Add the Ethernet 2 library by selecting 'Sketch' in the menu, 'Include Library' and 'Manage Libraries...', scroll down to 'Ethernet2' and press 'Install'. Then download the [Enguino source code] to a folder.

 For now just install the default configuration. Configuration involves editing the config.h file and then reinstalling the firmware. After editing the configuration, connect the Arduino to your PC via a USB cable. This will also power up the Arduino and a green light will appear next to the USB connector. Start up the Arduino software on your PC. Go `File`, `Open` and select the file `enigno.ino`. Then go `Sketch` and `Upload`. The other green LED next to the power LED will flash until the file is uploaded. Done Uploading will appear near the bottom of the window. You may see a message that says "Low memory available, stability problems may occur.", you can ignore this.

## Configuration

Configuring the engine display or new sensors can be a bit involved. Eventually a number of 'stock' config files will be developed and users can choose the one for their engine and sensors.

All configuration is done by editing the config.h file in the Arduino IDE (integrated development environment).

Due to hardware limitations all calculations are done using integers. A decimal point is assumed during the calculation and added when numbers are displayed. Decimal numbers are sometimes used in the config.h file but are converted to integers before being transferred to the Arduino.

### Sensors and scaling

The engine sensor system is fairly universal. The default configuration uses Vans Aircraft engine sensors. At this time the only non-supported sensor type is a millivolt sensor for an amp-meter.

Sensors fall into several categories: digital, such as fuel flow and tachometer or analog, generally all the rest.

The digital sensors are attached to dedicated inputs on the Enguino. The analog sensors are either thermocouples in which case they are attached to the thermocouple side otherwise they are attached to the generic side.

If you have a new sensor type you will first need to determine if the sensor has a *resistive* or *voltage* output.

Many sensors are linear in some sense. This means if you plot the resistance or voltage of a sensor against its output (typically pressure) it will show a straight line. Note - some sensors have a resistive output that is linear in voltage, Van's pressure sensors are this way.

The exception to this is many temperature sensors. These are based on a *thermistor* and these are anything but linear. The *CurveFit.xls* spreadsheet can be used to calculate a conversion table for this type of sensor.

Good calibration data to characterize a sensor can be hard to find. One way to determine configurations for sensors is to disconnect a sensor from its gauge and replace it with a *resistance decade box* (assuming the sensor is resistive). The *CurveFit.xls* spreadsheet can then be used to find the configuration settings.

With this information in hand you can update the *Sensor* table in config.h. Update the sensor-type and the voffset and vfactor to get correct numberic readings from the sensor. The goffset and gfactor are used to set the range displayed visually on gauges. The caution and warning levels are specific for the engine.

### Labels and graduations

Only vertical and horizontal gauges can have graduations. The label value (suffix LV) containing the label for the graduation is the first line. The label's position (suffix LP) contains the labels position. Use the VSEG and HSEG functions to convert the values to 'gauge' coordinates.   

### Ranges

This sets the color ranges of the gauges (green, yellow, red regions). The vertical and horizontal gauges are straight forward. The color is the first line (suffix of RC), the limit's position is the second line (suffix of RP). The starting limit of 0 is not included in the table.

The round gauges require 3 lines to describe the limit's position. The first line is the X coordinate for the position so use the ARCX function to convert the value to gauge coordinates. The second line is the Y coordinate so use the ARCY function. The third line is either 0 or 1, use 1 if the length of this color arc is more than 180 degrees. If not sure, just try it both ways and see which way draws correctly.

### Layout

During design and testing of the layout it may be helpful to connect the Enguino to your local network instead of the Stratux. You may need to update the IP address in the Enguino.ino file. If you do do so, remember to change it back after finalizing the layout. To see values on the gauges while testing, uncomment out the line `#define SIMULATE_SENSORS...`

Most gauges are of the vertical (gs_vert) type. Gauges that are paired left and right such as fuel gauges are of the paired vertical type (gs_pair). In this case the left sensor is connected to *pin* and the right sensor to *pin*+1.

The horizontal gauges (gs_horiz) are typically for handling 3 or more CHT gauges. Set CYLINDERS to the number of CHT sensors. The first cylinder is attached to *pin*, the second to *pin*+1, etc. The EGT gauges (gs_egt) overlay the CHT gauges and support a peaking mode.

### Aux display

The aux display can display a number of pages during startup. After that it displays a default page. Following that are a number of information pages.

A page can display text on the first line, 4 characters, however not all letters are supported. Some letters are displayed in the wrong case, for example 't' instead of 'T'. The letters 'M', 'W' and 'X' can not be displayed in any form. Others like 'V' end up looking the same as 'U'.

Each line can be associated with a sensor. If the first line has text and is associated with a sensor it will show caution (the letter L or H will replace the last character) or alarm information (the line will blink).

Otherwise the sensors numeric value will be shown. The default page is tachometer on the top and fuel gauges on the bottom. Avoid having more than one unlabelled page as the numbers may be ambiguous.

## Stratux
The Stratux is configured to route network traffic between the wired ethernet to the Enguino and the wifi connecting to a tablet.

* Boot the Stratux. For first boot after installing a fresh micro-SD card wait 3 minutes, for rebooting wait 30 seconds.
* Either connect a monitor and keyboard to the Rapberry Pi and login with un: `pi`, pw: `raspberry` then skip ahead in these instructions to sudo nano...
* Or connect to the Stratux wifi network (this will need to be repeated when rebooting)
* On the Mac, start terminal. On a Windows PC, run PuTTY.
* To connected type `ssh pi@192.168.10.1`
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
Browse to 192.168.10.1, confirm the Stratux control panel shows up.  
Browse to 192.168.0.111, confirm the Engiuno panel shows up.

## Design Details

### Arduino

The Leonardo is rated to run from 6-20 volts although they suggest keeping it between 7-12 volts. They claim to worry about the voltage regulator overheating and 'damaging the board', but the regulator chip (88% efficient) does have a thermal shutdown feature at 150 deg C. The text appears to be a carry over from previous Arduino's which used a linear supply.  

The supply is rated for 1000 ma. The Leonardo uses 82 mA (confirmed by testing). Connected to the Aux display with most segments lit power consumed is about 140 mA. Pull ups will add some to this.

**Leonardo/Yún Pin Mapping to Enguino Prototype**

| Arduino | 32U4  | Use         |IRQ|Analog|Counter| Assign  |
|---------|-------|-------------|---|------|-------|---------|
| D0      | PD2   | Yún RX      | * |      |       | Tach    |
| D1      | PD3   | Yún TX      | * |      |       | Fuel-F  |
| D2      | PD1   | SDA         | * |      |       | Aux-SDA |
| D3      | PD0   | SCL/PWM     | * |      |       | Aux-SCL |
| D4      | PD4   | TC Mux A0   |   |  A6  |       |         |
| D5      | PC6   | TC Mux A1   |   |      |       |         |
| D6      | PD7   | TC Mux A2   |   |  A7  |   *   |         |
| D7      | PE6   | TC Mux EN   | * |      |       |         |
| D8      | PB4   | Analog 8    |   |  A8  |       | MAP     |
| D9      | PB5   | TC CS       |   |  A9  |       |         |
| D10     | PB6   | Ether CS    |   | A10  |       |         |
| D11     | PB7   | PWM         |   |      |       | Aux-Sw  |
| D12     | PD6   | TC MISO     |   | A11  |   *   |         |
| D13     | PC7   | TC SCLK/LED |   |      |       |         ||

The SD card on the Leonardo can not be used with the thermocouple board attached as they both use pin D4. The thermocouple board also interferes with 4 of the analog inputs. The Leonardo itself also interferes with one of the analog inputs.

**Arduino Pin Mapping to Enguino Production**

| Arduino | 32U4  | Use           |IRQ|Analog|Counter| Assign Leo | Alternate    |
|---------|-------|---------------|---|------|-------|------------|--------------|
| D0      | PD2   | Serial RX/IRQ | * |      |       | Tach-1     |              |
| D1      | PD3   | Serial TX/IRQ | * |      |       | Tach-2     |              |
| D2      | PD1   | SDA/IRQ       | * |      |       | SDA        | Tach-1       |
| D3      | PD0   | SCL/IRQ       | * |      |       | SCL        | Tach-2       |
| D4      | PD4   | Analog 6      |   |  A6  |       | Volt divide|              |             
| D5      | PC6   |               |   |      |       |            | SDA          |
| D6      | PD7   |Analog 7/Count |   |  A7  |   *   | MAP        |Fuel-F/generic|
| D7      | PE6   | IRQ           | * |      |       | Fuel-F     |              |
| D8      | PB4   | Analog 8      |   |  A8  |       | Generic    |              |
| D9      | PB5   | Analog 9      |   |  A9  |       | Ammeter    | generic      |
| D10     | PB6   | Ethernet(Leo) |   | A10  |       | Ethernet   | MAP          |
| D11     | PB7   |               |   |      |       |            | SCL          |
| D12     | PD6   |Analog 11/Count|   | A11  |   *   | Generic    | Fuel-Flow2   |
| D13     | PC7   | LED           |   |      |       |            |              ||

* Leonardo ETH needs D10 for communications
* Tach needs an IRQ
* Fuel flow can either be IRQ or counters
* Fuel flow is either a dedicated pin or an analog pin w. 1k pull up & capacitor shorted
* Alternate generic is instead of MAP and Ammeter available on kit form

With the tach, assume 2 pulses per revolution at 2700 RPM the tach will max out at 90/cps so as long as IRQ disable time is <11ms no error should be expected. For the fuel flow with a k-factor of 68,000 and 13GPH it will max out at 250/cps so IRQ disable time must be <4ms.

### Software

The Arduino serves up several web pages which are used to create the display.
* Main page (http://) contains the static elements of the engine display.
* Dynamic page (http://d) contains the dynamic web content that displays the gauges.
* Setup page (http://s) contains a simple web form for setting the fuel totalizer.

The main webpage has a timer running in Javascript on the tablet that invokes a reload of the dynamic portion of the page using an AJAX request. The dynamic portion is drawn using SVG (scaled vectored graphics) that is created on the fly by the Arduino. The configuration tables in config.h control how the SVG is drawn, for example the layout of the gauges on the screen.

### Sensors

The sensor system is fairly generic but currently only Van's Aircraft engine sensors have predefined configurations. The resistive sensors will have a 240 ohm(1%) pull up to +5V. This will require up to 18 ma per sensor, or for the typical 5 sensors (fuel x 2, oil-p, oil-t, fuel-p) 90 ma total. This provides a good compromise between power usage, heat and loss of resolution. This provides 9 bits of resolution. To limit resistor heating to reasonable levels, .5 watt resistors should be used. Resistor temp. rise should be no more than 100 deg. C in free air. A resistance significantly out of range will mark the sensor inoperative. To convert from ADC units to ohms use this formula `ohms = 240 * (adc / (1024-adc))`, this will require long divided by long division unfortunately or a lookup/interpolation table.

Many of the sensors are 240-33.5 ohm sensors similar to Stewart Warner. These usually scale linearly by resistance. For those that don't, a custom interpolation table may be required. The oil temperature sensor is also resistive (its a *thermistor*) but it has a larger range. Using a Steinhar-Hart calculator the conversion formula becomes `degrees Kelvin = 1 / (0.0016207535760566691 + 0.0002609330007304247 * log(R) + -1.0278556187396396e-7 * log(R)^3)`. This is far too complicated to solve on the Arduino so an interpolation table is used for the conversion.

For resistive sensors still attached to the gauge, the gauge itself provides the pull up resistance and voltage. For Vans Aircraft engine instruments the pull up is 5 volts and the resistor is about 227 ohms(measured externally, internally the resistor appears to be 240 ohms, 5%). The pin can be directly connected if the voltage can't exceed Vcc by more than .5v. Otherwise a 15K series resistor could be attached to help isolate the pin.

For thermocouple sensors the board will detect open circuits. It directly reads out in degrees C (.25 resolution), but assumes K style thermocouples (which is what Van's uses). This can optionally be converted to F. To support J style thermocouples, the C output is adjusted by taking the thermocouple reading (before the CJT adjustment) `newC = oldC * 25599 / 32768`

For the voltage sensor a 4:1 voltage divider consisting of a 1k and 3.01k (1%) resistor is used. This limits the draw to .1 watt at 20 volts or .05 at 14 volts.

The tachometer is supplied with 12 volts, its a hall effect sensor that returns about 10 volt pulses, 4 pulses per revolution.

Vans manifold pressure sensor is 0-90mV which is too low to accurately measure with the Arduino. It's easier to use a new manifold pressure sensor that has a 0-5 volt output.

For a sensor that might contain a voltage higher than Vcc (such as Van's tachometer) using a voltage divider will cause a significant load which could create issues if still attached to a backup gauge. A 15K ohm resistor between the sensor and the pin will safely clip voltages between 20.5 and -15.5 volts. The goal here is to limit the internal clamping diodes in the Arduino's CPU to no more than 1ma after the .7 volt diode drop.

The tachometer measures RPM by recording the length of tim in uS time between rising edges on a pin. RPM = 60,000,000 / (time - last_time). Other interrupt functions (thermocouple and time keeping) will cause occasional jitter. Collecting 8 samples, throwing out the highest and lowest and averaging the middle 4 fixes that.

One or two sensors should support 4-20mA. For those have the option of grounding the 240 ohm resitor instead of pulling it up.

Fuel flow needs a 5K pull up resistor.

### Auxiliary Display

The auxiliary display consists of two lines of a 4 digit [7 segment LED displays]. Both are strung together on the same i2c communication bus, but the bottom display has the A0 jumper soldered so they can be individually addressed. The displays have power (from the shield board), ground, a data line and a clock line going to them.

Covering the LED displays with a dark red bezel makes them much easier to read in sunlight. A medium red gel filter behind clear plastic works well.

A master caution/warning [bicolor LED] annunciator is reworked by soldering onto two unused display row of the top display.

The pushbutton is connected to ground on one side. The other side of the switch goes back to the shield.

### Parts list
* Arduino Leonardo ETH - Digikey 1050-1007-ND
* Thermocouple Multiplexer Shield ($50 electronics123)
* Manifold pressure sensor - Digikey MPX4115AP-ND ($15)
* Bicolor LED - Digikey 754-1232-ND
* T 1 3/4 LED holder for panel - Digikey 67-1332-ND
* 2 Adafruit 7 segment displays - Digikey 1528-1473-ND
* 10 240 ohm 1% resistors - Digikey A121513CT-ND
* 15K ohm 5% resistors - Digikey 15.0KXBK-ND
* 3K ohm 1% resistor - Digikey 3.01KXBK-ND
* 1k ohm 1% resistor - Digikey 1.00KXBK-ND
* 12 input screw terminal block - Digikey ED10568-ND
* .025 square breakaway headers - Digikey 929834-04-36-ND (tin) or 929647-04-36-ND (gold) - will probably work well on the thermocouple board w/o a header extension.
* jack for auxiliary display 6 pin - Digikey 455-2271-ND
* header for auxiliary display 6 pin - Digikey 455-2218-ND
* 10 contacts for header - Digikey 455-1135-1-ND
* pushbutton switch - Digikey EG2015-ND
* K style thermocouple wire, 24-26 gauge, EBay
* Enclosure - Electrical box - B108R - Home Depot
* Medium Red gel filter - Roscolux \#27 - Theater supply store

## Status
Test flying in RV-6A in parallel with its analog gauges for critical functions (OT, OP, FP, Tach and fuel gauges).

Currently I only have the prototype so I have to remove it to test changes.

Also looking for a volunteer with an EE-bent to help design and layout a better shield and auxiliary display board.

[open source]:https://en.wikipedia.org/wiki/Open-source_model
[Stratux]:http://stratux.me
[example]:http://htmlpreview.github.com/?https://github.com/tomcourt/enguino/blob/master/efis.html
[photo]:https://github.com/tomcourt/enguino/blob/master/Enguino.jpg
[photo of the prototype]:https://github.com/tomcourt/enguino/blob/master/AuxDisplay.jpg
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
