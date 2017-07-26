#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <hardware/gps.h>

#include "Common/CommonDefs.h"

#include <stdint.h>

MERBOK_EXTERN_C_BEGIN

extern MERBOK_GPS_LOCAL const GnssConfigurationInterface *GetConfigurationInterface(void);

extern MERBOK_GPS_LOCAL uint32_t GetConfigCapabilities(void);

extern MERBOK_GPS_LOCAL uint16_t GetConfigYearOfHW(void);

extern MERBOK_GPS_LOCAL uint16_t GetConfigGPSLocationFlags(void);

MERBOK_EXTERN_C_END

#endif