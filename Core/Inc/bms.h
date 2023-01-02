#ifndef INC_BMS_H_
#define INC_BMS_H_


/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <bmb.h>


/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// The number of BMBs per battery pack
#define NUM_BMBS_PER_PACK		1
// Max allowable voltage difference between bricks for balancing
#define BALANCE_THRESHOLD_V		0.002f


/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
	uint32_t numBmbs;
	Bmb_S bmb[NUM_BMBS_PER_PACK];

	float maxBrickV;
	float minBrickV;
	float avgBrickV;

	float maxBrickTemp;
	float minBrickTemp;
	float avgBrickTemp;
} Bms_S;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */
/*!
  @brief   Initialization function for the battery pack
  @param   numBmbs - The expected number of BMBs in the daisy chain
*/
void initBatteryPack(uint32_t numBmbs);

/*!
  @brief   Handles balancing the battery pack
  @param   numBmbs - The expected number of BMBs in the daisy chain
  @param   balanceRequested - True if we want to balance, false otherwise
*/
void balancePack(uint32_t numBmbs, bool balanceRequested);


/*!
  @brief   Update BMS data statistics. Min/Max/Avg
  @param   numBmbs - The expected number of BMBs in the daisy chain
*/
void aggregatePackData(uint32_t numBmbs);


#endif /* INC_BMS_H_ */
