#include "message.h"
#include "gmatrix.h"

void MsgQueue::Send()
{
    MSGQUEUE::iterator it = _queue.begin();
    for(; it != _queue.end(); ++it)
        gmatrix::HandleMessage(*it);
    _queue.clear();
}
