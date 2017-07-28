#include "Engine/UBXCtrlHandler.h"
#include "Engine/UBXParser.h"
#include "Device/GPSDeviceIF.h"

int GpsEngineInit(struct GpsDataCallbacks *cb)
{
    GPSDeviceInit();
    UBXParserInit(cb);
    return GetGPSComDevice()->open();
}

int GpsEngineSetup(void)
{
    const struct GPSDeviceIF *device = GetGPSComDevice();

    if (-1 == SetGpsRate(device)) {
        return -1;
    }

    if (-1 == BookUbxNAVTIMEUTC(device, 1)) {
        return -1;
    }

    if (-1 == BookUbxCFGNAVX5(device, 1)) {
        return -1;
    }

    if (-1 == SetGpsVerion(device)) {
        return -1;
    }

    if (-1 == GpsNmeaSetting(device, 1)) {
        return -1;
    }

    return 0;
}

int GpsEnginePollEvent(void)
{
    return UBXPacketRead(GetGPSComDevice());
}
