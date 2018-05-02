/*

Module:  maplewood-test.ino

Function:
        Test app for Catena-4470 as used at Maplewood.

Copyright notice and License:
        See LICENSE file accompanying this project.
 
Author:
        Terry Moore, MCCI Corporation	April 2018

*/

#include <Arduino.h>
#include <Catena4470.h>
#include <SPI.h>
#include <wiring_private.h>
#include <Catena_Flash_at25sf081.h>

using namespace McciCatena;
using Catena = Catena4470;

Catena gCatena;
cFlash_AT25SF081 gFlash;

// SCK is D12, which is SERCOM1.3
// MOSI is D11, which is SERCOM1.0
// MISO is D10, which is SERCOM1.2
//
SPIClass gSPI2(&sercom1, 10, 12, 11, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2); 

bool fFlashFound;
bool fFlashProgram;
bool fFlashReadOnly;
uint32_t flashAddress;
uint32_t nFlash;

void setup()
	{
	gCatena.begin();
	gSPI2.begin();
	pinPeripheral(10, PIO_SERCOM);
	pinPeripheral(12, PIO_SERCOM);
	pinPeripheral(11, PIO_SERCOM);

	flash_init();
        nFlash = 0;
	}

void loop()
	{
        gCatena.poll();
        if (nFlash < 64)
                {
                bool fEndSector;

                flash_test(fEndSector);
                if (fEndSector)
                        ++nFlash;
                }
	}

void flash_init(void)
	{
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
	fFlashProgram = true;
	fFlashReadOnly = false;
	}

void flash_test(bool &fEndSector)
	{
	uint32_t buffer32[4];
	uint8_t *buffer;

	buffer = (uint8_t *) buffer32;

	gFlash.powerUp();
	fEndSector = false;

	if (fFlashProgram && ! fFlashReadOnly)
		{
		fFlashProgram = false;
		buffer32[0] = flashAddress;
		buffer32[1] = 0;
		buffer32[2] = 0x55555555u;
		buffer32[3] = 0xaaaaaaaau;
		if (! gFlash.program(flashAddress, buffer, 16))
			gCatena.SafePrintf("** program at address %#06x failed **\n", flashAddress);
		}
	else
		{
		if (! fFlashReadOnly)
		fFlashProgram = true;
		gFlash.read(flashAddress, buffer, 16);

		gCatena.SafePrintf("FLASH 0x%06x: ", flashAddress);
		for (uint8_t a = 0; a < 16; ++a) 
			{
			gCatena.SafePrintf("%02x ", buffer[a]);
			}
		gCatena.SafePrintf("\n"); 

		flashAddress += gFlash.PAGE_SIZE;
		if ((flashAddress & (gFlash.SECTOR_SIZE - 1)) == 0)
			{
			gCatena.SafePrintf("Erase sector\n"); 
			gFlash.eraseSector(flashAddress - gFlash.SECTOR_SIZE);
			if (fFlashReadOnly)
				{
				fFlashReadOnly = false;
				fEndSector = true;
				}
			else
				{
				fFlashReadOnly = true;
				flashAddress -= gFlash.SECTOR_SIZE;
				}
			}
		}

	gFlash.powerDown();
	}
