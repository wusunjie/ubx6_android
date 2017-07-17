#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

class MsgHandler {
public:
    virtual void proc(void) = 0;
    virtual ~MsgHandler(void) {}
};

extern void MsgQueueFlush(void);

extern void MsgQueueSend(MsgHandler *msg);

extern MsgHandler *MsgQueueRecv(void);

#endif