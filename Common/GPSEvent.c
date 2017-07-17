#include "Common/GPSEvent.h"

#include <errno.h>

ssize_t GPSEventRead(int fd, void *buf, size_t count)
{
    ssize_t n;

    do {
        n = read(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

ssize_t GPSEventWrite(int fd, const void *buf, size_t count)
{
    ssize_t n;

    do {
        n = write(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

int GPSEventClose(int fd)
{
    return close(fd);
}
