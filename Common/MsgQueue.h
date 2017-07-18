#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include <list>

#include "Common/CommonDefs.h"

class MsgHandler {
public:
    virtual void proc(void) = 0;
    virtual ~MsgHandler(void) {}
};

typedef std::list<MsgHandler *> MsgQueue;

extern MERBOK_GPS_LOCAL MsgQueue *MsgQueueCreate(void);

extern MERBOK_GPS_LOCAL void MsgQueueFlush(MsgQueue *q);

extern MERBOK_GPS_LOCAL void MsgQueueSend(MsgQueue *q, MsgHandler *msg);

extern MERBOK_GPS_LOCAL MsgHandler *MsgQueueRecv(MsgQueue *q);

extern MERBOK_GPS_LOCAL void MsgQueueDestory(MsgQueue *q);

#endif