#ifndef _UBX_CTRL_HANDLER_H
#define _UBX_CTRL_HANDLER_H

#include <stdint.h>

extern int SetGpsRate(void);

extern int SetGpsVerion(void);

extern int GpsNmeaSetting(int gstFlag);

extern int ResetGpsReceiver(uint16_t type);

extern int BookUbxNAVTIMEUTC(int bBookFlag);

extern int BookUbxCFGNAVX5(int bBookFlag);

#endif