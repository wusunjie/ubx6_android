#include "Common/GPSSafeIO.h"

#include <errno.h>

ssize_t GPSSafeRead(int fd, void *buf, size_t count)
{
    ssize_t n;

    do {
        n = read(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

ssize_t GPSSafeWrite(int fd, const void *buf, size_t count)
{
    ssize_t n;

    do {
        n = write(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

int GPSSafeClose(int fd)
{
    return close(fd);
}
