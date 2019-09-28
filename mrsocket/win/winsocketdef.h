#ifndef winsocketdef_h
#define winsocketdef_h

#include "win/winsocket.h"

#ifndef DONOT_USE_IO_EXTEND
#define write(fd, ptr, sz) write_extend_socket(fd, ptr, sz)
#define read(fd, ptr, sz)  read_extend_socket(fd, ptr, sz)
#define close(fd) close_extend_socket(fd)
#define pipe(fd) pipe_socket(fd)
#define connect(s, name, namelen) connect_extend_errno(s, name, namelen)
#define send(s, buffer, sz, flag) send_extend_errno(s, buffer, sz, flag)
#define recv(s, buffer, sz, flag) recv_extend_errno(s, buffer, sz, flag)
#define getsockopt(s, level, optname, optval, optlen) getsockopt_extend_voidptr(s, level, optname, optval, optlen)
#define setsockopt(s, level, optname, optval, optlen) setsockopt_extend_voidptr(s, level, optname, optval, optlen)
#define recvfrom(s, buf, len, flags, from, fromlen) recvfrom_extend_voidptr(s, buf, len, flags, from, fromlen)
#endif

#undef near


#endif