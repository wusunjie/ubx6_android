#include "Engine/UBXCtrlHandler.h"

extern int UBXPacketRead(void);

int main(void)
{
	int bTemp = 0;

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