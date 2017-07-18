#ifndef _GPS_SAFEIO_H
#define _GPS_SAFEIO_H

#include <unistd.h>

#include "Common/CommonDefs.h"

MERBOK_EXTERN_C_BEGIN

extern MERBOK_GPS_LOCAL ssize_t GPSSafeRead(int fd, void *buf, size_t count);

extern MERBOK_GPS_LOCAL ssize_t GPSSafeWrite(int fd, const void *buf, size_t count);

extern MERBOK_GPS_LOCAL int GPSSafeClose(int fd);

MERBOK_EXTERN_C_END

#endif
