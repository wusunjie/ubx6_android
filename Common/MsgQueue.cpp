#include "Common/MsgQueue.h"

#include <new>

MsgQueue *MsgQueueCreate(void)
{
    return new (std::nothrow) MsgQueue();
}

void MsgQueueFlush(MsgQueue *q)
{
    if (q) {

        while (!q->empty()) {
            delete q->front();
            q->pop_front();
        }

    }
}

void MsgQueueSend(MsgQueue *q, MsgHandler *msg)
{
    if (q) {
        q->push_back(msg);
    }
}

MsgHandler *MsgQueueRecv(MsgQueue *q)
{
    MsgHandler *msg = NULL;
    if (q) {
        if (!q->empty()) {
            msg = q->front();
        }
        if (msg) {
            q->pop_front();
        }
    }

    return msg;
}

void MsgQueueDestory(MsgQueue *q)
{
    if (q) {

        while (!q->empty()) {
            delete q->front();
            q->pop_front();
        }

        delete q;
    }
}
