#ifndef _H_MACROS_
#define _H_MACROS_

#include "settings.h"

// ACK and error codes
#define SEND_BUSY Serial.println("BUSY");
#define SEND_NOERROR Serial.println("ACK: 0")
#define SEND_ERROR(err) Serial.println("ACK: " + String(err))
enum ERROR_CODES{
  ERROR_CALIB_FIRST = 1,
  ERROR_INVALID_PARAM,
  ERROR_UNKNOWN_CODE,
  ERROR_CALIB_FAILED,
  ERROR_INVALID_STATE
};

// Macro to convert cord length into steps
#define LENGTH_TO_STEPS(l) ((uint32_t)round((double)l*LENGTH_TO_STEPS_CONST))
#define STEPS_TO_LENGTH(s) ((double)s*STEPS_TO_LENGTH_CONST)

// Clamps the value of v between min and max
#define CLAMP(V,MIN,MAX) ((V)<(MIN)?(MIN):((V)>(MAX)?(MAX):(V)))


#endif
