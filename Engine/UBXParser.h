#ifndef _UBX_PARSER_H
#define _UBX_PARSER_H

#include "Engine/GpsEngine.h"
#include "Device/GPSDeviceIF.h"

extern MERBOK_GPS_LOCAL void UBXParserInit(struct GpsDataCallbacks *cb);

extern MERBOK_GPS_LOCAL int UBXPacketRead(const struct GPSDeviceIF *dev);

#endif
