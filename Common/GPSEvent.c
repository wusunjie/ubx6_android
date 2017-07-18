#include <stdlib.h>

#include <sys/eventfd.h>

#include "Common/GPSSafeIO.h"

struct GPSEvent {
    int fd;
};

struct GPSEvent *GPSEventCreate(void)
{
    struct GPSEvent *ev = NULL;
    int fd = eventfd(0, 0);
    if (-1 != fd) {
        ev = (struct GPSEvent *)malloc(sizeof(*ev));
        ev->fd = fd;
    }

    return ev;
}

int GPSEventWait(struct GPSEvent *ev)
{
    if (ev) {
        uint64_t n;
        GPSSafeRead(ev->fd, &n, 8);
        return 0;
    }
    else {
        return -1;
    }
}

int GPSEventSignal(struct GPSEvent *ev)
{
    if (ev) {
        uint64_t n = 1;
        GPSSafeWrite(ev->fd, &n, 8);
        return 0;
    }
    else {
        return -1;
    }
}

void GPSEventDestory(struct GPSEvent *ev)
{
    if (ev) {
        GPSSafeClose(ev->fd);
        free(ev);
    }
}

