#ifndef _UBX_CTRL_HANDLER_H
#define _UBX_CTRL_HANDLER_H

extern int WriteUbxCfgRate(void);

extern int SetGpsVerion(void);

extern int GpsNmeaSetting(int gstFlag);

extern int ResetGpsReceiver(uint16_t type);

extern int BookUbxNAVTIMEUTC(int bBookFlag);

extern int BookUbxCFGNAVX5(int bBookFlag);

#endif