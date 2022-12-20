/*
 * batteryPack.h
 *
 *  Created on: Aug 14, 2022
 *      Author: sebas
 */

#ifndef INC_BMB_H_
#define INC_BMB_H_

#include <stdio.h>
#include <stdbool.h>

#define DEVCFG1		0x10
#define GPIO 		0x11
#define MEASUREEN 	0x12
#define SCANCTRL	0x13

#define NUM_BRICKS_PER_BMB		12
#define NUM_BOARD_TEMP_PER_BMB 	4
#define NUM_BMBS_PER_PACK		1


typedef struct
{
	uint32_t numBricks;
	float brickV[NUM_BRICKS_PER_BMB];
	float stackV;
	float blockV;

	float brickTemp[NUM_BRICKS_PER_BMB];
	float boardTemp[NUM_BOARD_TEMP_PER_BMB];

	float maxBrickV;
	float minBrickV;
	float avgBrickV;

	float maxBrickTemp;
	float minBrickTemp;
	float avgBrickTemp;

	float maxBoardTemp;
	float minBoardTemp;
	float avgBoardTemp;

	float aux1;
	float aux2;
	// add gpio mode

} Bmb_S;

typedef struct
{
	uint32_t numBmbs;
	Bmb_S bmb[NUM_BMBS_PER_PACK];
} Pack_S;


bool initASCI(uint32_t *numBmbs);

void initBmbs(uint32_t numBmbs);
void setGpio(uint32_t numBmbs, uint16_t gpioSetting);
void updateBmbData(uint32_t numBmbs);
void aggregateBrickVoltages(uint32_t numBmbs);


#endif /* INC_BMB_H_ */
