#include "Device/GPSDeviceIF.h"
#include "Engine/UBXCtrlHandler.h"
#include "Engine/UBXParser.h"

int GpsEngineInit(struct GpsDataCallbacks *cb)
{
    GPSDeviceInit();
    UBXParserInit(cb);
    return 0;
}

int GpsEngineSetup(void)
{
    GetGPSComDevice()->open();

    if (-1 == SetGpsRate()) {
        return -1;
    }

    if (-1 == BookUbxNAVTIMEUTC(1)) {
        return -1;
    }

    if (-1 == BookUbxCFGNAVX5(1)) {
        return -1;
    }

    if (-1 == SetGpsVerion()) {
        return -1;
    }

    if (-1 == GpsNmeaSetting(1)) {
        return -1;
    }

    return 0;
}

int GpsEnginePollEvent(void)
{
    return UBXPacketRead();
}
