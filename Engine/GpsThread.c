#include "UBXCtrlHandler.h"

#include <stdint.h>

void *GpsWorkingThread(void *args)
{
	int bTemp = 0;

	bTemp = SetGpsRate();
	if (!bTemp) {
		
	}
	else {
		
	}

	bTemp = BookUbxNAVTIMEUTC(1);
	if (!bTemp) {
		
	}
	else {
		
	}

	bTemp = BookUbxCFGNAVX5(1);
	if (!bTemp) {
		
	}
	else {
		
	}

	bTemp = SetGpsVerion();
	if (!bTemp) {
		
	}
	else {
		
	}

	int bBook = GpsNmeaSetting(1);
}