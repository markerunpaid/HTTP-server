#include "Connection.hpp"

FlushResult Connection::flush()
{
    while (woff < wlen)
    {
        IoResult r = sock.send(wbuf + woff, wlen - woff);
        if (r.status == IoStatus::Ok)
        {
            woff += r.bytes;
            continue;
        }
        if (r.status == IoStatus::Blocked)
            return FlushResult::Blocked;
        return FlushResult::Error;
    }
    return FlushResult::Ok;
}