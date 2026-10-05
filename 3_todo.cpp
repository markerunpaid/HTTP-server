/*
resusable sockets, to avoid time-wait failure

use string_view in place of strings, take care of the lifetime

(implement this in http parser)
Walk through the buffer character by character using pointers or std::string_view.
Extremely fast, Much harder
Libraries like nginx, Boost.Beast, and llhttp use approaches similar to Method 2.


PHASE-3
make HTTPresponse zero allocation
*/