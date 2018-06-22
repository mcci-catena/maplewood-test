# Test for Maplewood Hardware

<!-- TOC depthFrom:2 updateOnSave:true -->

- [What needs to be tested?](#what-needs-to-be-tested)
- [What's our procedure?](#whats-our-procedure)
- [How do we test a transmitter assembly?](#how-do-we-test-a-transmitter-assembly)
- [Logistics for development](#logistics-for-development)
	- [What setup do I need in the Arduino Board Manager to build this sketch?](#what-setup-do-i-need-in-the-arduino-board-manager-to-build-this-sketch)
	- [What libraries do I need?](#what-libraries-do-i-need)
	- [What configuration should I use with VS Code?](#what-configuration-should-i-use-with-vs-code)

<!-- /TOC -->

## What needs to be tested?

- RS485 drivers
- Lux sensor
- BME280 sensor
- Flash
- LoRa Radio and antenna
- FRAM

## What's our procedure?

The test program erases the first sector of flash, writes "I am a duck", reads and checks the answer. If this succeeds, it proclaims that flash is working.

The test program initializes and periodically reads the following:

- BME280
- light
- Vbat
- Registers from meters at Modbus addresses 1,2 at baudrate 19200

It then formats and transmits a message using LoRaWAN, providing LoRaWAN is provisioned.

The loop is running the standard Catena polling system, so the USB command input is live.

## How do we test a transmitter assembly?

- Connect the U.FL to RP-SMA adapter to the Catena.

- Connect an antenna to the RP-SMA adapter.

- Connect test Modbus meters to Catena JP3 (N/C, Red, Black, Shield).

- Connect test battery to Catena

- Connect the Catena via USB to a test computer

- Start an Arduino environment and download the code

   - You'll note that on Windows, the serial port the first time may use the Adafuit VID/PID, and so the COM port might have to be changed at this step.

- Look at the USB window. You should see something like this at the end.

   ```
   Test summary: 8 passed, 11 failed, and 2 skipped, out of 21 test(s).
   ```

- Enter the following comands via the USB port. They won't echo, probably.

   - `system configure platformguid DD0A37A6-E469-43EC-B173-FED795129455`
   - <code>system configure syseui 00-02-cc-01-00-00-<em>xx-yy</em></code> -- look at the label on the board. **_If there is no label, stop and get one!_**

- Reboot the board. You should then see something like the following:

   ```
   ...
   Test 4_lora_00_provisioned failed.
   Test 4_lora_10_senduplink skipped.
   Test 4_lora_20_uplink_done skipped.
   Test summary: 17 passed, 2 failed, and 2 skipped, out of 21 test(s).
   ```

- Go to The Things Network Console, and register the device with the [maplewood-hvac-mfgtest](https://console.thethingsnetwork.org/applications/maplewood-hvac-mfgtest) app.  The convention is to name devices <code>device-<em>xx-yy</em></code>, where xx and yy are the last 2 bytes of the EUI.

- Enter the following commands:

   - <code>lorawan configure deveui 00-02-cc-01-00-00-<em>xx-yy</em></code>
   - <code>lorawan configure appeui <em><strong>value_from_console</strong></em></code>
   - <code>lorawan configure appkey <em><strong>value_from_console</strong></em></code>
   - `lorawan configure join 1`

- Switch the TTN console view of the device to `Data`, and reboot the device.  You should see somethign like this:

   ```
   Test 4_lora_00_provisioned passed.
   Test 4_lora_10_senduplink passed.
   EV_JOINING
   EV_TXSTART
   EV_JOINED
   NwkID:   00000013   DevAddr: 26022f07
   EV_TXSTART
   EV_TXSTART
   EV_TXCOMPLETE
   Test 4_lora_20_uplink_done passed.
   Test summary: 21 passed, 0 failed, and 0 skipped, out of 21 test(s).
   ```

- Check the SNR of the uplink messages in the Console. We want to make sure that the cable and antenna are working. If RSSI is less than -80, or SNR is less than 8, we may have a problem.

- Check the Lux, pressure, RH and Vbat values for sanity. The test program also checks, but only for really outragous errors.

- Declare that the device passes.

## Logistics for development

### What setup do I need in the Arduino Board Manager to build this sketch?

For best LoRaWAN support, MCCI implemented our board-support package, availalable from https://github.com/mcci-catena/arduino-boards.

- add https://github.com/mcci-catena/arduino-boards/raw/master/BoardManagerFiles/package_mcci_index.json to your "Additional Boards Manager URLs" in the Arduino IDE under `File>Preferences`. 

- Install support for the "MCCI Catena SAMD Boards" via `Tools>Board "whatever">Boards Manager...`

- Select board: "Catena 4470" via `Tools>Board` -- you'll have to scroll to the bottom of the list.

### What libraries do I need?

Take a look at `git-boot.dat` in this directory. It lists all the libraries that are needed.

However, for reference, here they are:

- https://github.com/mcci-catena/Catena-Arduino-Platform
- https://github.com/mcci-catena/catena-mcciadk
- https://github.com/mcci-catena/arduino-lorawan
- https://github.com/mcci-catena/arduino-lmic
- https://github.com/mcci-catena/Adafruit_FRAM_I2C
- https://github.com/mcci-catena/Adafruit_BME280_Library
- https://github.com/mcci-catena/Adafruit_Sensor
- https://github.com/mcci-catena/BH1750
- https://github.com/mcci-catena/Modbus-for-Arduino
- https://github.com/mcci-catena/MCCI-WattNode-Modbus
- https://github.com/mcci-catena/arduinounit

### What configuration should I use with VS Code?

Here's what I use for SAMD builds.

```json
{
    "configurations": [
        {
	    "name": "Arduino",
	    "defines": [
		"ARDUINO=183" 
	    ],
            "includePath": [
		"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\cores\\arduino",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/lib/gcc/arm-none-eabi/4.8.3/include",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/hardware/samd/1.0.1/variants/feather_m0_express",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include/c++/4.8.3",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include/c++/4.8.3/arm-none-eabi/armv7-ar/thumb/softfp",
		"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/CMSIS-Atmel/1.1.0/CMSIS/Device/ATMEL",
		"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\libraries\\SPI",
		"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\libraries\\Wire",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\Catena-Arduino-Platform\\src",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\catena-mcciadk\\src",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\arduino-lorawan\\src",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\arduino-lmic\\src",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_FRAM_I2C",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_BME280_Library",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_Sensor",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\BH1750",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\Modbus-for-Arduino",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\MCCI-WattNode-Modbus\\src",
		"c:\\users\\tmm\\documents\\arduino\\libraries\\arduinounit\\src"
            ],
            "browse": {
                "limitSymbolsToIncludedHeaders": false,
                "path": [
			"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\cores\\arduino",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/lib/gcc/arm-none-eabi/4.8.3/include",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/hardware/samd/1.0.1/variants/feather_m0_express",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include/c++/4.8.3",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/arm-none-eabi-gcc/4.8.3-2014q1/arm-none-eabi/include/c++/4.8.3/arm-none-eabi/armv7-ar/thumb/softfp",
			"c:/users/tmm/appdata/local/arduino15/packages/mcci/tools/CMSIS-Atmel/1.1.0/CMSIS/Device/ATMEL",
			"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\libraries\\SPI",
			"C:\\Users\\tmm\\AppData\\Local\\Arduino15\\packages\\mcci\\hardware\\samd\\1.0.1\\libraries\\Wire",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\Catena-Arduino-Platform\\src",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\catena-mcciadk\\src",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\arduino-lorawan\\src",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\arduino-lmic\\src",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_FRAM_I2C",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_BME280_Library",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\Adafruit_Sensor",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\BH1750",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\Modbus-for-Arduino",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\MCCI-WattNode-Modbus\\src",
			"c:\\users\\tmm\\documents\\arduino\\libraries\\arduinounit\\src"
			]
            },
            "intelliSenseMode": "clang-x64",
            "cStandard": "c11",
            "cppStandard": "c++17"
        }
    ],
    "version": 3
}
```