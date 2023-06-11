#ifndef INC_SOC_H_
#define INC_SOC_H_

#include "timer.h"


#define MILLISECONDS_IN_SECOND  1000
#define SECONDS_IN_MINUTE        60
#define MINUTES_IN_HOUR          60

#define MAX_ACCUMULATOR_MILLICOULOMBS CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS * MINUTES_IN_HOUR * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND
#define SOC_BY_OCV_GOOD_QUALIFICATION_TIME_MS 5 * MINUTES_IN_HOUR * MILLISECONDS_IN_SECOND

typedef struct
{
    uint32_t initialMilliCoulombCount;
    int32_t accumulatedMilliCoulombs;
} CoulombCounter_S;

typedef struct
{
    // Inputs
    float minBrickVoltage;              // The current min brick voltage of the pack
    float curAccumulatorCurrent;        // The value of the current flowing into/out of the accumulator
    uint32_t deltaTimeMs;               // The time since the last update

    // Outputs
    bool socByOcvGood;                  // Whether or not we can rely on SOC by OCV
    CoulombCounter_S coulombCounter;    // The coulomb counter
    Timer_S socByOcvGoodTimer;          // The qualification timer to determine whether SOC by OCV can be used
    float socByOcv;                     // The state of charge using open circuit voltage
    float socByCoulombCounting;         // The state of charge using coulomb counting
    float soeByOcv;                     // The state of energy using open circuit voltage
    float soeByCoulombCounting;         // The state of energy using coulomb counting
} Soc_S;


float getSocFromCellVoltage(float cellVoltage);

float getSoeFromSoc(float soc);

void updateSocAndSoe(Soc_S* soc);


#endif /* INC_SOC_H_ */
