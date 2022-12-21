/*
 * batteryPack.c
 *
 *  Created on: Aug 14, 2022
 *      Author: sebas
 */
#include "main.h"
#include "cmsis_os.h"
#include "bmb.h"
#include "spiUtils.h"

extern osSemaphoreId binSemHandle;
extern uint8_t spiRecvBuffer[64];
extern uint8_t spiSendBuffer[64];

Pack_S gPack;


bool initASCI(uint32_t *numBmbs)
{
	disableASCI();
	HAL_Delay(10);
	enableASCI();
	ssOff();
	bool successfulConfig = true;
	// dummy transaction since this chip sucks
	readRegister(R_CONFIG_3);

	// Set Keep_Alive to 0x05 = 160us
	successfulConfig &= writeAndVerifyRegister(R_CONFIG_3, 0x05);

	// Enable RX_Error, RX_Overflow and RX_Busy interrupts
	successfulConfig &= writeAndVerifyRegister(R_RX_INTERRUPT_ENABLE, 0xA8);

	clearRxBuffer();

	// Enable TX_Preambles mode
	successfulConfig &= writeAndVerifyRegister(R_CONFIG_2, 0x30);

	if (!(xSemaphoreTake(binSemHandle, 10) == pdTRUE))
	{
		printf("Interrupt failed to occur during initialization!\r\n");
		return false;
	}

	// Verify interrupt was caused by RX_Busy
	if (readRxBusyFlag())
	{
		clearRxBusyFlag();
	}
	else
	{
		successfulConfig = false;
	}

	// Verify RX_Busy_Status and RX_Empty_Status true
	successfulConfig &= (readRegister(R_RX_STATUS) == 0x21);

	// Enable RX_Stop INT
	successfulConfig &= writeAndVerifyRegister(R_RX_INTERRUPT_ENABLE, 0x8A);

	// Enable TX_Queue mode
	successfulConfig &= writeAndVerifyRegister(R_CONFIG_2, 0x10);

	// Verify RX_Empty
	successfulConfig &= !!(readRegister(R_RX_STATUS) & 0x01);

	clearTxBuffer();
	clearRxBuffer();

	// Send Hello_All command
	spiSendBuffer[0] = CMD_WR_LD_Q_L0;
	spiSendBuffer[1] = 0x03;			// Data length
	spiSendBuffer[2] = CMD_HELLO_ALL;	// HELLOALL command byte
	spiSendBuffer[3] = 0x00;			// Register address
	spiSendBuffer[4] = 0x00;			// Initialization address for HELLOALL

	if (!loadAndVerifyTxQueue((uint8_t *)&spiSendBuffer, 5))
	{
		printf("Failed to load all!\r\n");
		return false;
	}

	// Enable RX_Error, RX_Overflow and RX_Stop interrupts
	successfulConfig &= writeAndVerifyRegister(R_RX_INTERRUPT_ENABLE, 0x8A);

	sendSPI(CMD_WR_NXT_LD_Q_L0);


	if (!(xSemaphoreTake(binSemHandle, 10) == pdTRUE))
	{
		printf("Interrupt failed to occur during initialization!\r\n");
		return false;
	}

	if (readRxStopFlag())
	{
		clearRxStopFlag();
	}
	else
	{
		successfulConfig = false;
	}

	// Verify stop bit
	successfulConfig &= !!(readRegister(R_RX_STATUS) & 0x02);


	successfulConfig &= readNextSpiMessage((uint8_t *)&spiRecvBuffer, 3);

	if (rxErrorsExist() || !successfulConfig)
	{
		printf("Detected errors during initialization...\r\n");
		return false;
	}

	(*numBmbs) = spiRecvBuffer[3];

	return true;
}

void initBmbs(uint32_t numBmbs)
{
	// Enable alive counter byte
	// numBmbs set to 0 since alive counter not yet enabled
	writeAll(DEVCFG1, 0x1042, spiRecvBuffer, 0);

	// Enable measurement channels
	writeAll(MEASUREEN, 0xFFFF, spiRecvBuffer, numBmbs);

	// Enable GPIOs as outputs
	writeAll(GPIO, 0xF000, spiRecvBuffer, numBmbs);

	// Set brickOV voltage alert threshold
	// Set brickUV voltage alert threshold

}

void setGpio(uint32_t numBmbs, uint16_t gpioSetting)
{
	// Assuming all GPIOs configured as OUTPUT
	uint16_t data = 0xF000;
	data |= (gpioSetting & 0x000F);	// Extract last 4 bits from gpioSetting
	writeAll(GPIO, data, spiRecvBuffer, numBmbs);
}

void readBoardTemps(uint32_t numBmbs)
{
	Pack_S* pPack = &gPack;

	// Internal board temps are on MUX7 and MUX8
	for (int muxGpio = MUX7; muxGpio <= MUX8; muxGpio++)
	{
		setGpio(numBmbs, muxGpio);		// Set GPIO for desired MUX
		// Start acquisition
		writeAll(SCANCTRL, 0x0001, spiRecvBuffer, numBmbs);
		// TODO implement error checking in the case of bad data or broken comms
		// Read AUX registers
		for (int auxChannel = AIN1; auxChannel <= AIN2; auxChannel++)
		{
			// Read temperature from BMBs
			readAll(auxChannel, numBmbs, spiRecvBuffer);
			// TODO add catch if readall fails
			// Parse received data
			for (uint8_t i = 0; i < numBmbs; i++)
			{
				// Read AUX voltage in [15:4]
				uint32_t auxRaw = ((spiRecvBuffer[4 + 2*i] << 8) | spiRecvBuffer[3 + 2*i]) >> 4;
				float auxV = auxRaw * CONVERT_12BIT_TO_3V3;
				// Determine boardTempVoltage index for current reading
				int tempIdx = muxGpio - MUX7 + ((auxChannel == AIN2) ? 2 : 0);
				pPack->bmb[i].boardTempVoltage[tempIdx] = auxV;
			}
		}
	}
}

void updateBmbData(uint32_t numBmbs)
{
	Pack_S* pPack = &gPack;

	// Start acquisition
	writeAll(SCANCTRL, 0x0001, spiRecvBuffer, numBmbs);
	// Update cell data
	// TODO implement error checking in the case of bad data or broken comms
	for (uint8_t i = 0; i < 12; i++)
	{
		uint8_t cellReg = i + CELLn;
		if (!readAll(cellReg, numBmbs, spiRecvBuffer))
		{
			printf("Error during readAll!\r\n");
			break;
		}
		for (uint8_t j = 0; j < numBmbs; j++)
		{
			// Read brick voltage in [15:2]
			uint32_t brickVRaw = ((spiRecvBuffer[4 + 2*j] << 8) | spiRecvBuffer[3 + 2*j]) >> 2;
			float brickV = brickVRaw * CONVERT_14BIT_TO_5V;
			pPack->bmb[j].brickV[i] = brickV;
		}
	}
	// Read VBLOCK register
	if (!readAll(VBLOCK, numBmbs, spiRecvBuffer))
	{
		for (uint8_t j = 0; j < numBmbs; j++)
		{
			// Read block voltage in [15:2]
			uint32_t blockVRaw = ((spiRecvBuffer[4 + 2*j] << 8) | spiRecvBuffer[3 + 2*j]) >> 2;
			float blockV = blockVRaw * CONVERT_14BIT_TO_60V;
			pPack->bmb[j].blockV = blockV;
		}
	}
}

void aggregateBrickVoltages(uint32_t numBmbs)
{
	Pack_S* pPack = &gPack;

	for (int i = 0; i < numBmbs; i++)
	{
		Bmb_S* pBmb = &pPack->bmb[i];
		float maxBrickV = 0.0f;
		float minBrickV = 5.0f;
		float stackV	= 0.0f;
		for (int j = 0; j < NUM_BRICKS_PER_BMB; j++)
		{
			float brickV = pBmb->brickV[j];

			if (brickV > maxBrickV)
			{
				maxBrickV = brickV;
			}

			if (brickV < minBrickV)
			{
				minBrickV = brickV;
			}

			stackV += brickV;
		}
		pBmb->maxBrickV = maxBrickV;
		pBmb->minBrickV = minBrickV;
		pBmb->stackV	= stackV;
		pBmb->avgBrickV = stackV / NUM_BRICKS_PER_BMB;
	}
}
