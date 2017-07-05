#ifndef _UBX_CTRL_HANDLER_H
#define _UBX_CTRL_HANDLER_H

#include <stdint.h>

#include "Common/CommonDefs.h"

extern MERBOK_GPS_LOCAL int SetGpsRate(void);

extern MERBOK_GPS_LOCAL int SetGpsVerion(void);

extern MERBOK_GPS_LOCAL int GpsNmeaSetting(int gstFlag);

extern MERBOK_GPS_LOCAL int ResetGpsReceiver(uint16_t type);

extern MERBOK_GPS_LOCAL int BookUbxNAVTIMEUTC(int bBookFlag);

extern MERBOK_GPS_LOCAL int BookUbxCFGNAVX5(int bBookFlag);

#endif