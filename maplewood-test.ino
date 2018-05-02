/*

Module:  maplewood-test.ino

Function:
        Test app for Catena-4470 as used at Maplewood.

Copyright notice and License:
        See LICENSE file accompanying this project.

Author:
        Terry Moore, MCCI Corporation	April 2018

*/

// line number should be l# - 2, for ArduinoUnit compatibility
// #line 17 "maplewood-test.ino"

#include <Arduino.h>
#include <Catena4470.h>
#include <Catena_led.h>
#include <SPI.h>
#include <wiring_private.h>
#include <Catena_Flash_at25sf081.h>
#include <Catena_Guids.h>

#include <Adafruit_BME280.h>
#include <BH1750.h>

#include <ArduinoUnit.h>

using namespace McciCatena;
using Catena = Catena4470;

Catena gCatena;
cFlash_AT25SF081 gFlash;

// declare lorawan (so we can provision)
Catena::LoRaWAN gLoRaWAN;

// declare the LED object
StatusLed gLed (Catena::PIN_STATUS_LED);

BH1750 gBH1750;
Adafruit_BME280 gBME280;

// SCK is D12, which is SERCOM1.3
// MOSI is D11, which is SERCOM1.0
// MISO is D10, which is SERCOM1.2
// Knowing that, you might be able to make sens of the following:
SPIClass gSPI2(&sercom1, 10, 12, 11, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);

void setup()
	{
	gCatena.begin();

	setup_platform();

	//** set up for flash work **
	setup_flash();
	}

void loop()
	{
        gCatena.poll();
	Test::run();
	}

void setup_platform()
	{
	while (! Serial)
		/* wait for USB attach */
		yield();

        Serial.print(
                "\n" 
                "-------------------------------------------------------------------------------\n"
                "This is the Maplewood test program.\n"
                "Enter 'help' for a list of commands.\n"
                "(remember to select 'Line Ending: Newline' at the bottom of the monitor window.)\n"
                "--------------------------------------------------------------------------------\n"
                "\n"
                );

        gLed.begin();
        gCatena.registerObject(&gLed);
        gLed.Set(LedPattern::FastFlash);

	Catena::UniqueID_string_t CpuIDstring;

        gCatena.SafePrintf(
		"CPU Unique ID: %s\n",
                gCatena.GetUniqueIDstring(&CpuIDstring)
                );
	}

//-----------------------------------------------------
// platform tests
//-----------------------------------------------------

test(platform_05_check_platform_guid)
	{
        const CATENA_PLATFORM * const pPlatform = gCatena.GetPlatform();
	const MCCIADK_GUID_WIRE m101Guid = GUID_HW_CATENA_4470_M101(WIRE);

	assertTrue(pPlatform != nullptr, "gCatena.GetPlatform() failed -- this is normal on first boot");
	assertEqual(
		memcmp(&m101Guid, &pPlatform->Guid, sizeof(m101Guid)), 0,
		"platform guid mismatch"
		);
	}

test(platform_10_check_syseui)
	{
        const Catena::EUI64_buffer_t *pSysEUI = gCatena.GetSysEUI();
	static const Catena::EUI64_buffer_t ZeroEUI = { 0 };
	bool fNonZeroEUI;

	assertTrue(pSysEUI != nullptr);
	fNonZeroEUI = memcmp(pSysEUI, &ZeroEUI, sizeof(ZeroEUI)) != 0;
	assertTrue(fNonZeroEUI, "SysEUI is zero. This is normal on first boot.");

	for (unsigned i = 0; i < sizeof(pSysEUI->b); ++i)
		{
		gCatena.SafePrintf("%s%02x", i == 0 ? "  SysEUI: " : "-", pSysEUI->b[i]);
		}
	gCatena.SafePrintf("\n");
	}

test(platform_20_lorawan_begin)
	{
	bool fPassed = gLoRaWAN.begin(&gCatena);

	// always register
	gCatena.registerObject(&gLoRaWAN);

	assertTrue(fPassed, "gLoRaWAN.begin() failed");
	}

test(platform_30_init_lux) 
	{
	uint32_t flags = gCatena.GetPlatformFlags();

	assertNotEqual(flags & Catena::fHasLuxRohm, 0, "No lux sensor in platform flags?");

	gBH1750.begin();
	gBH1750.configure(BH1750_CONTINUOUS_HIGH_RES_MODE_2);
	}

test(platform_40_init_bme)
	{
	uint32_t flags = gCatena.GetPlatformFlags();

	assertNotEqual(flags & Catena::fHasBme280, 0, "No BME280 sensor in platform flags?");
	assertTrue(
		gBME280.begin(BME280_ADDRESS, Adafruit_BME280::OPERATING_MODE::Sleep),
		"BME280 sensor failed begin(): check wiring"
		);
	}

testing(platform_50_lux)
	{
	assertTestPass(platform_30_init_lux);

	const uint32_t interval = 2000;
	const uint32_t ntries = 10;
	static uint32_t lasttime, thistry;
	uint32_t now = millis();

	if ((int32_t)(now - lasttime) < interval)
		/* skip */;
	else
		{
		lasttime = now;

		uint16_t light;

		light = gBH1750.readLightLevel();
		assertLess(light, 0xFFFF / 1.2, "Oops: light value pegged: " << light);
		gCatena.SafePrintf("BH1750:  %u lux\n", light);

		if (++thistry >= ntries)
			pass();
 		}
	}

testing(platform_60_bme)
	{
	assertTestPass(platform_40_init_bme);

	const uint32_t interval = 2000;
	const uint32_t ntries = 10;
	static uint32_t lasttime, thistry;
	uint32_t now = millis();

	if ((int32_t)(now - lasttime) < interval)
		/* skip */;
	else
		{
		lasttime = now;

	       	Adafruit_BME280::Measurements m = gBME280.readTemperaturePressureHumidity();
		Serial.print("BME280:  T: "); Serial.print(m.Temperature);
		Serial.print("  P: "); Serial.print(m.Pressure);
		Serial.print("  RH: "); Serial.print(m.Humidity); Serial.println("%");

		assertMore(m.Temperature, 10, "Temperature in lab must be > 10 deg C: " << m.Temperature);
		assertLess(m.Temperature, 40, "Temperature in lab must be < 40 deg C: " << m.Temperature);
		assertMore(m.Pressure, 800 * 100);
		assertLess(m.Pressure, 1100 * 100);
		assertMore(m.Humidity, 5);
		assertLess(m.Humidity, 100);

		if (++thistry >= ntries)
			pass();
		}
	}
//-----------------------------------------------------
//      Flash tests
//-----------------------------------------------------
void setup_flash()
	{	
	gSPI2.begin();

	// these *must* be after gSPI2.begin(), because the SPI
	// library resets the pins to defaults as part of the begin()
	// method.
	pinPeripheral(10, PIO_SERCOM);
	pinPeripheral(12, PIO_SERCOM);
	pinPeripheral(11, PIO_SERCOM);
	}

void logMismatch(
	uint32_t addr, 
	uint8_t expect,
	uint8_t actual
	)
	{
	gCatena.SafePrintf(
		"mismatch address %#x: expect %#02x got %02x\n",
		addr, expect, actual
		);
	}

uint32_t vNext(uint32_t v)
	{
	return v * 31413 + 3;
	}

// choose a sector.
const uint32_t sectorAddress = gFlash.DEVICE_SIZE_BYTES - gFlash.SECTOR_SIZE;

union sectorBuffer_t {
	uint8_t b[gFlash.SECTOR_SIZE];
	uint32_t dw[gFlash.SECTOR_SIZE / sizeof(uint32_t)];
	} sectorBuffer;

test(flash_00init)
	{
	assertTrue(flash_init(), "flash_init()");
	pass();
	}

test(flash_01erase)
	{
	// erase the sector.
	assertTestPass(flash_00init);
	assertTrue(gFlash.eraseSector(sectorAddress));
	pass();
	}

uint32_t flashBlankCheck(
	uint32_t a = sectorAddress,
	sectorBuffer_t &buffer = sectorBuffer
	)
	{
	// make sure the sector is blank
	memset(buffer.b, 0, sizeof(buffer));
	gFlash.read(a, buffer.b, sizeof(buffer.b));
	unsigned errs = 0;
	for (auto i = 0; i < sizeof(buffer.b); ++i)
		{
		if (buffer.b[i] != 0xFF)
			{
			logMismatch(a + i, 0xFF, buffer.b[i]);
			++errs;
			}
		}
	return errs;
	}

test(flash_02blankcheck)
	{
	auto errs = flashBlankCheck();

	assertEqual(errs, 0, "mismatch errors: " << errs);
	pass();
	}

void initBuffer(
	uint32_t v,
	sectorBuffer_t &buffer = sectorBuffer
	)
	{
	for (auto i = 0; 
	     i < sizeof(buffer.dw) / sizeof(buffer.dw[0]); 
	     ++i, v = vNext(v))
	     	{
		buffer.dw[i] = v;
		}
	}

const uint32_t vStart = 0x55555555u;

test(flash_03writepattern)
	{
	// write a pattern
	initBuffer(vStart, sectorBuffer);

	assertTrue(gFlash.program(sectorAddress, sectorBuffer.b, sizeof(sectorBuffer.b)),
		"Failed to program sector " << sectorAddress
		);
	}

test(flash_04readpattern)
	{
	// read the buffer
	for (auto i = 0; i < sizeof(sectorBuffer.b); ++i)
		sectorBuffer.b[i] = ~sectorBuffer.b[i];

	gFlash.read(sectorAddress, sectorBuffer.b, sizeof(sectorBuffer.b));

	union 	{
		uint8_t b[sizeof(uint32_t)];
		uint32_t dw;
		} vTest, v;
	v.dw = vStart;

	auto errs = 0;
	for (auto i = 0; 
	     i < sizeof(sectorBuffer.dw) / sizeof(sectorBuffer.dw[0]); 
	     ++i, v.dw = vNext(v.dw))
		{
		vTest.dw = sectorBuffer.dw[i];

		for (auto j = 0; j < sizeof(v.b); ++j)
			{
			if (v.b[j] != vTest.b[j])
				{
				++errs;
				logMismatch(sectorAddress + sizeof(uint32_t) * i + j, v.b[j], vTest.b[j]);
				}
			}
		}
	assertEqual(errs, 0, "mismatch errors: " << errs);
	pass();
	}

test(flash_05posterase)
	{
	assertTrue(gFlash.eraseSector(sectorAddress));
	pass();
	}

// boilerplate setup code
bool flash_init(void)
	{
        bool fFlashFound;

	gCatena.SafePrintf("Init FLASH\n");

	if (gFlash.begin(&gSPI2, 5))
		{
		uint8_t ManufacturerId;
		uint16_t DeviceId;

		gFlash.readId(&ManufacturerId, &DeviceId);
		gCatena.SafePrintf(
			"FLASH found, ManufacturerId=%02x, DeviceId=%04x\n",
			ManufacturerId, DeviceId
			);
		gFlash.powerDown();
		fFlashFound = true;
		}
	else
		{
		gCatena.SafePrintf("No FLASH found\n");
		fFlashFound = false;
		}
	return fFlashFound;
	}

//-------------------------------------------------
//      Sensor tests
//-------------------------------------------------


