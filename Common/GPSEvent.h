#ifndef _GPS_EVENT_H
#define _GPS_EVENT_H

#include <stddef.h>

enum GPSEventMode {
	GPS_EVENT_MODE_BLOCK    = 0,
	GPS_EVENT_MODE_NONBLOCK = 1,
};

struct GPSEvent {
	int fd;
	enum GPSEventMode mode;
};

extern void GPSEventInit(struct GPSEvent *event, int fd, enum GPSEventMode mode);

extern int GPSEventRead(struct GPSEvent *event, void *buffer, size_t len);

extern int GPSEventWrite(struct GPSEvent *event, void *buffer, size_t len);

extern int GPSEventClose(struct GPSEvent *event);

#endif