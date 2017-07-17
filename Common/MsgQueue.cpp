#include "Common/MsgQueue.h"

#include <list>

static std::list<MsgHandler *> msgQueue;

void MsgQueueFlush(void)
{
    msgQueue.clear();
}

void MsgQueueSend(MsgHandler *msg)
{
    msgQueue.push_back(msg);
}

MsgHandler *MsgQueueRecv(void)
{
    MsgHandler *msg = NULL;
    if (!msgQueue.empty()) {
        msg = msgQueue.front();
    }
    if (msg) {
        msgQueue.pop_front();
    }
    return msg;
}
