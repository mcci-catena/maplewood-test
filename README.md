# Test for Maplewood Hardware

## What needs to be tested?

- RS485 drivers
- Lux sensor
- BME280 sensor
- Flash
- LoRa Radio and antenna
- FRAM

## What's our test setup?

- Connect the U.FL to RP-SMA adapter to the Catena.
- Connect an antenna to the RP-SMA adapter.
- Connect test Modbus meters to Catena JP3 (N/C, Red, Black, Shield).
- Connect test battery to Catena
- Connect the Catena via USB to a test computer

## What's our procedure?

The test program erases the first sector of flash, writes "I am a duck", reads and checks the answer. If this succeeds, it proclaims that flash is working.

The test program periodically reads the following:

- BME280
- light
- Vbat
- Registers from meters at addresses 1,2 at baudrate 19200

It then formats and transmits a message using LoRaWAN.

The polling
