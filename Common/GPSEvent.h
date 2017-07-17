#ifndef _GPS_EVENT_H
#define _GPS_EVENT_H

#include <unistd.h>

#include "Common/CommonDefs.h"

extern MERBOK_GPS_LOCAL ssize_t GPSEventRead(int fd, void *buf, size_t count);

extern MERBOK_GPS_LOCAL ssize_t GPSEventWrite(int fd, const void *buf, size_t count);

extern MERBOK_GPS_LOCAL int GPSEventClose(int fd);

#endif
