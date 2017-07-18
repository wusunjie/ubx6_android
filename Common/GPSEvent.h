#ifndef _GPS_EVENT_H
#define _GPS_EVENT_H

#include "Common/CommonDefs.h"

struct GPSEvent;

MERBOK_EXTERN_C_BEGIN

extern MERBOK_GPS_LOCAL struct GPSEvent *GPSEventCreate(void);

extern MERBOK_GPS_LOCAL int GPSEventWait(struct GPSEvent *ev);

extern MERBOK_GPS_LOCAL int GPSEventSignal(struct GPSEvent *ev);

extern MERBOK_GPS_LOCAL void GPSEventDestory(struct GPSEvent *ev);

MERBOK_EXTERN_C_END

#endif