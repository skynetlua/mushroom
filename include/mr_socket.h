#ifndef mr_socket_h
#define mr_socket_h

#include "socket_info.h"

typedef void (*mr_callback4)(uintptr_t, int, char*, int);
typedef void (*mr_callback5)(uintptr_t, int, char*, int, int);
typedef void (*mr_udp_callback)(uintptr_t, int, char*, int, char*);

//callback
void mr_set_handle_data(mr_callback4 cb);
void mr_set_handle_connect(mr_callback4 cb);
void mr_set_handle_close(mr_callback4 cb);
void mr_set_handle_accept(mr_callback5 cb);
void mr_set_handle_error(mr_callback4 cb);
void mr_set_handle_warning(mr_callback4 cb);
void mr_set_handle_udp(mr_udp_callback cb);

void mr_socket_run(void);
void mr_socket_update(void);

void mr_socket_init(void);
void mr_socket_exit(void);
void mr_socket_free(void);

int mr_socket_send(int fd, void* buffer, int sz);
int mr_socket_send_lowpriority(int fd, void* buffer, int sz);
void mr_socket_nodelay(int fd);

//tcp part
int mr_socket_listen(uintptr_t uid, const char* host, int port, int backlog);
int mr_socket_connect(uintptr_t uid, const char* host, int port);
int mr_socket_bind(uintptr_t uid, int fd);
void mr_socket_close(uintptr_t uid, int fd);
void mr_socket_shutdown(uintptr_t uid, int fd);
void mr_socket_start(uintptr_t uid, int fd);

//udp part
int mr_socket_udp(uintptr_t uid, const char* addr, int port);
int mr_socket_udp_connect(int fd, const char* addr, int port);
int mr_socket_udp_send(int fd, const char* address, const void* buffer, int sz);
int mr_socket_udp_address(const char* address, char* udp_addr, int len);

struct socket_info * mr_socket_info(void);

#endif
