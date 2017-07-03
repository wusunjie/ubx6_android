#include "Device/GpsDeviceIF.h"
#include "Engine/UBXCtrlHandler.h"

struct GpsLocationInfo {
    /** set to sizeof(GpsLocation) */
    size_t          size;
    /** Contains GpsLocationFlags bits. */
    uint16_t        flags;
    /** Represents latitude in degrees. */
    double          latitude;
    /** Represents longitude in degrees. */
    double          longitude;
    /**
     * Represents altitude in meters above the WGS 84 reference ellipsoid.
     */
    double          altitude;
    /** Represents speed in meters per second. */
    float           speed;
    /** Represents heading in degrees. */
    float           bearing;
    /** Represents expected accuracy in meters. */
    float           accuracy;
    /** Timestamp for the location fix. */
    int64_t      timestamp;
};

int GpsEngineInit(void)
{
	GPSDeviceInit();
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