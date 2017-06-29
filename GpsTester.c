#include "Engine/UBXCtrlHandler.h"

#include "Device/GPSDeviceIF.h"

extern int UBXPacketRead(void);

int main(void)
{
	int bTemp = 0;

	GPSDeviceInit();

	GetGPSComDevice()->open();

	bTemp = SetGpsRate();

	bTemp = BookUbxNAVTIMEUTC(1);

	bTemp = BookUbxCFGNAVX5(1);

	bTemp = SetGpsVerion();

	bTemp = GpsNmeaSetting(1);

	while (1) {
		int ret = UBXPacketRead();
	}

	return 0;
}