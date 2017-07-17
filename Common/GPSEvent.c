#include "Common/GPSEvent.h"

#include <unistd.h>
#include <sys/select.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static ssize_t safe_read(int fd, void *buf, size_t count);
static ssize_t safe_write(int fd, const void *buf, size_t count);

static ssize_t safe_read(int fd, void *buf, size_t count)
{
    ssize_t n;

    do {
        n = read(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

static ssize_t safe_write(int fd, const void *buf, size_t count)
{
    ssize_t n;

    do {
        n = write(fd, buf, count);
    } while(n < 0 && errno == EINTR);

    return n;
}

void GPSEventInit(struct GPSEvent *event, int fd, enum GPSEventMode mode)
{
    event->fd = fd;
    event->mode = mode;
}

int GPSEventRead(struct GPSEvent *event, void *buffer, size_t len)
{
    if (GPS_EVENT_MODE_BLOCK == event->mode) {
        return safe_read(event->fd, (unsigned char *)buffer, len);
    }
    else {
        size_t r = 0;
        fd_set rfds;
        fd_set efds;
        int ret;
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&efds);
        FD_ZERO(&rfds);
        FD_SET(event->fd, &rfds);
        FD_SET(event->fd, &efds);
        while (1) {
            ret = select(event->fd + 1, &rfds, 0, &efds, &tv);
            if ((ret < 0) && (EINTR == errno)) {
                /* do nothing */
            }
            else if (ret > 0) {
                int i;
                for (i = 0; i < ret; i++) {
                    if (FD_ISSET(event->fd, &efds)) {
                        return -1;
                    }
                    else if (FD_ISSET(event->fd, &rfds)) {
                        int t;
                        t = safe_read(event->fd, (unsigned char *)buffer + r, len);
                        if (t < 0) {
                            if ((EAGAIN == errno) || (EWOULDBLOCK == errno)) {
                                continue;
                            }
                            else {
                                return -1;
                            }
                        }
                        else {
                            len -= t;
                            r += t;
                        }
                    }
                }
            }
            else {
                return -1;
            }
        }
        return -1;
    }
}

int GPSEventWrite(struct GPSEvent *event, void *buffer, size_t len)
{
    if (GPS_EVENT_MODE_BLOCK == event->mode) {
        return safe_write(event->fd, (unsigned char *)buffer, len);
    }
    else {
        size_t r = 0;
        int ret;
        fd_set wfds;
        fd_set rfds;
        fd_set efds;
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&wfds);
        FD_ZERO(&rfds);
        FD_ZERO(&efds);
        FD_SET(event->fd, &wfds);
        FD_SET(event->fd, &efds);
        while (len) {
            ret = select(event->fd + 1, &rfds, &wfds, &efds, &tv);
            if ((ret < 0) && (EINTR == errno)) {
                /* do nothing */
            }
            else if (ret > 0) {
                int i;
                for (i = 0; i < ret; i++) {
                    if (FD_ISSET(event->fd, &efds)) {
                        return -1;
                    }
                    else if (FD_ISSET(event->fd, &wfds)) {
                        int t;
                        t = safe_write(event->fd, (unsigned char *)buffer + r, len);
                        if (t < 0) {
                            if ((EAGAIN == errno) || (EWOULDBLOCK == errno)) {
                                continue;
                            }
                            else {
                                return -1;
                            }
                        }
                        else {
                            len -= t;
                            r += t;
                        }
                    }
                }
            }
            else {
                return -1;
            }
        }
        return 0;
    }
}

int GPSEventClose(struct GPSEvent *event)
{
    int ret = close(event->fd);
    event->fd = -1;
    return ret;
}
