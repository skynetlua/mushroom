#ifndef winsocket_h
#define winsocket_h

#include <WinSock2.h>
#include <WS2tcpip.h>

// static int write_extend_socket(int fd, const void *buffer, size_t sz);
// static int read_extend_socket(int fd, void *buffer, size_t sz);
// static int close_extend_socket(int fd);
// static int pipe_socket(int fd[2]);
// static int connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen);
// static int send_extend_errno(SOCKET s, const char* buffer, int sz, int flag);
// static int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag);
// static int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag);
// static int getsockopt_extend_voidptr(SOCKET s, int level, int optname, void* optval, int* optlen);
// static int setsockopt_extend_voidptr(SOCKET s, int level, int optname, const void* optval, int optlen);
// static int recvfrom_extend_voidptr(SOCKET s, void* buf, int len, int flags, struct sockaddr* from, int* fromlen);

// #ifndef DONOT_USE_IO_EXTEND
// #define write(fd, ptr, sz) write_extend_socket(fd, ptr, sz)
// #define read(fd, ptr, sz)  read_extend_socket(fd, ptr, sz)
// #define close(fd) close_extend_socket(fd)
// #define pipe(fd) pipe_socket(fd)
// #define connect(s, name, namelen) connect_extend_errno(s, name, namelen)
// #define send(s, buffer, sz, flag) send_extend_errno(s, buffer, sz, flag)
// #define recv(s, buffer, sz, flag) recv_extend_errno(s, buffer, sz, flag)
// #define getsockopt(s, level, optname, optval, optlen) getsockopt_extend_voidptr(s, level, optname, optval, optlen)
// #define setsockopt(s, level, optname, optval, optlen) setsockopt_extend_voidptr(s, level, optname, optval, optlen)
// #define recvfrom(s, buf, len, flags, from, fromlen) recvfrom_extend_voidptr(s, buf, len, flags, from, fromlen)
// #endif

// #undef near


static int write_extend_socket(int fd, const void* buffer, size_t sz) 
{
	int ret = send_extend_errno(fd, (const char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return write(fd, buffer, sz);
	return ret;
}

static int read_extend_socket(int fd, void* buffer, size_t sz) 
{
	int ret = recv_extend_errno(fd, (char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return read(fd, buffer, sz);
	return ret;
}

static int close_extend_socket(int fd) 
{
	int ret = closesocket(fd);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return close(fd);
    return ret;
}

static int pipe_socket(int fds[2]) 
{
	if (Win32WSAStartup()){
       printf("Error initializing ws2_32.dll");
       assert(0);
       return 0;
   }
    struct sockaddr_in name;
    int namelen = sizeof(name);
    SOCKET server = INVALID_SOCKET;
    SOCKET client1 = INVALID_SOCKET;
    SOCKET client2 = INVALID_SOCKET;

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	name.sin_port = 0;

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
        goto failed;

    int yes=1;
    if (setsockopt(server,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes)) == SOCKET_ERROR)
       goto failed;          

    if (bind(server, (struct sockaddr*)&name, namelen) == SOCKET_ERROR) 
        goto failed;

    if (listen(server, 5) == SOCKET_ERROR)
        goto failed;

    if(getsockname(server, (struct sockaddr*)&name, &namelen) == SOCKET_ERROR)
        goto failed;

    client1 = socket(AF_INET, SOCK_STREAM, 0);
    if (client1 == INVALID_SOCKET)
        goto failed;

    if (connect(client1, (struct sockaddr*)&name, namelen) == SOCKET_ERROR)
        goto failed;

    client2 = accept(server, (struct sockaddr*)&name, &namelen);
    if (client2 == INVALID_SOCKET)
        goto failed;

    closesocket(server);
    fds[0] = (uintptr_t)client1;
    fds[1] = (uintptr_t)client2;
    return 0;

failed:
    if (server != INVALID_SOCKET)
        closesocket(server);

    if (client1 != INVALID_SOCKET)
        closesocket(client1);

    if (client2 != INVALID_SOCKET)
        closesocket(client2);
    return -1;
}

static int connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen)
{
	int ret = connect(s, name, namelen);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EINPROGRESS;
	}
	return ret;
}

static int send_extend_errno(SOCKET s, const char* buffer, int sz, int flag) 
{
	int ret = send(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}

static int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag) 
{
	int ret = recv(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}

static int getsockopt_extend_voidptr(SOCKET s, int level, int optname, void* optval, int* optlen) 
{
    return getsockopt(s, level, optname, (char*)optval, optlen);
}

static int setsockopt_extend_voidptr(SOCKET s, int level, int optname, const void* optval, int optlen)
{
    return setsockopt(s, level, optname, (char*)optval, optlen);
}


static int recvfrom_extend_voidptr(SOCKET s, void* buf, int len, int flags, struct sockaddr* from, int* fromlen)
{
    int ret = recvfrom(s, (char*)buf, len, flags, from, fromlen);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
		if (errno == WSAECONNRESET)
			errno = EAGAIN;
	}
	return ret;
}


#endif