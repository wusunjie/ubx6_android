#ifndef _GPS_EVENT_H
#define _GPS_EVENT_H

#include <stddef.h>

#include "Common/CommonDefs.h"

enum GPSEventMode {
    GPS_EVENT_MODE_BLOCK    = 0,
    GPS_EVENT_MODE_NONBLOCK = 1,
};

struct GPSEvent {
    int fd;
    enum GPSEventMode mode;
};

extern MERBOK_GPS_LOCAL void GPSEventInit(struct GPSEvent *event, int fd, enum GPSEventMode mode);

extern MERBOK_GPS_LOCAL int GPSEventRead(struct GPSEvent *event, void *buffer, size_t len);

extern MERBOK_GPS_LOCAL int GPSEventWrite(struct GPSEvent *event, void *buffer, size_t len);

extern MERBOK_GPS_LOCAL int GPSEventClose(struct GPSEvent *event);

#endif
