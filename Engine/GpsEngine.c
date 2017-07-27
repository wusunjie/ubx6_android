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
    if (-1 == SetGpsRate(GetGPSComDevice())) {
        return -1;
    }

    if (-1 == BookUbxNAVTIMEUTC(GetGPSComDevice(), 1)) {
        return -1;
    }

    if (-1 == BookUbxCFGNAVX5(GetGPSComDevice(), 1)) {
        return -1;
    }

    if (-1 == SetGpsVerion(GetGPSComDevice())) {
        return -1;
    }

    if (-1 == GpsNmeaSetting(GetGPSComDevice(), 1)) {
        return -1;
    }

    return 0;
}

int GpsEnginePollEvent(void)
{
    return UBXPacketRead(GetGPSComDevice());
}
