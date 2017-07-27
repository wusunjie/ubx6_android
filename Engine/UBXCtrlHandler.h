#ifndef _UBX_CTRL_HANDLER_H
#define _UBX_CTRL_HANDLER_H

#include <stdint.h>

#include "Common/CommonDefs.h"
#include "Device/GPSDeviceIF.h"

extern MERBOK_GPS_LOCAL int SetGpsRate(struct GPSDeviceIF *dev);

extern MERBOK_GPS_LOCAL int SetGpsVerion(struct GPSDeviceIF *dev);

extern MERBOK_GPS_LOCAL int GpsNmeaSetting(struct GPSDeviceIF *dev, int gstFlag);

extern MERBOK_GPS_LOCAL int ResetGpsReceiver(struct GPSDeviceIF *dev, uint16_t type);

extern MERBOK_GPS_LOCAL int BookUbxNAVTIMEUTC(struct GPSDeviceIF *dev, int bBookFlag);

extern MERBOK_GPS_LOCAL int BookUbxCFGNAVX5(struct GPSDeviceIF *dev, int bBookFlag);

#endif