#ifndef INC_BMBUTILS_H_
#define INC_BMBUTILS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>


/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */
//Generic Lookup Table
typedef struct
{
    const uint32_t length;
    const float* x;			// Pointer to x array
    const float* y;			// Pointer to y array
} LookupTable;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */
extern LookupTable ntcTable;
extern LookupTable zenerTable;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */
// TODO - add description
float lookup(float x, const LookupTable* table);




#endif /* INC_BMBUTILS_H_ */