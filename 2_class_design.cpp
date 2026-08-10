/*
socket(wrapper for server_fd) - create, bind, listen, accept (also uses RAII, the destructor closes socket when it goes out of scope)
recieve
http parse
response
route


socket
destructor closes the socket
copy constructor/assignment = deleted, since two different objects will be managing same sockets (like unique pointer)




HTTP request
enum class for methods (implicit conversion is not allowec)
*/