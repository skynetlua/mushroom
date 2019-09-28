#ifndef poll_socket_epoll_h
#define poll_socket_epoll_h


#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

// #include "win/inet.h"
#include <Winsock2.h>
#include <WS2tcpip.h>
#define O_NONBLOCK 1
#define F_SETFL 0
#define F_GETFL 1

static inline int fcntl(int fd, int cmd, long arg) {
  if (cmd == F_GETFL)
    return 0;

  if (cmd == F_SETFL && arg == O_NONBLOCK) {
    u_long ulOption = 1;
    ioctlsocket(fd, FIONBIO, &ulOption);
  }
  return 1;
}

#else

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#endif

struct fd_ctx
{
    int fd;
    void *ud;
    bool write;
};

struct sp_ctx
{
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;

	int used_size;
	struct fd_ctx fd_ctxs[FD_SETSIZE];
};

struct sp_ctx* ctx = NULL;

static bool 
sp_invalid(int efd) {
	return efd == -1;
}

static int
sp_create() {
	if (ctx != NULL){
		return -1;
	}
	ctx = (struct sp_ctx*)malloc(sizeof(struct sp_ctx));
    memset(ctx, 0, sizeof(struct sp_ctx));
    int i = 0;
	for (; i < FD_SETSIZE; ++i){
		ctx->fd_ctxs[i].fd = -1;
	}
	return 0;
}

static void
sp_release(int efd) {
    free(ctx);
    ctx = NULL;
}

static int 
sp_add(int efd, int sock, void *ud) {
	if (ctx->used_size >= FD_SETSIZE){
		fprintf(stderr, "[error]sp_add: select limit FD_SETSIZE:%d/%d \n", ctx->used_size, FD_SETSIZE);
		assert(0);
		return -1;
	}
	int i = 0;
	for (; i < FD_SETSIZE; ++i){
		if (ctx->fd_ctxs[i].fd < 0){
			ctx->fd_ctxs[i].ud = ud;
			ctx->fd_ctxs[i].fd = sock;
			ctx->fd_ctxs[i].write = false;
			ctx->used_size++;
			return 0;
		}
	}
	fprintf(stderr, "[error]sp_add: select limit FD_SETSIZE:%d \n", ctx->used_size);
	assert(0);
	return -1;
}

static void 
sp_del(int efd, int sock) {
	int i = 0;
	for (; i < FD_SETSIZE; ++i){
		if (ctx->fd_ctxs[i].fd == sock){
			ctx->fd_ctxs[i].ud = NULL;
			ctx->fd_ctxs[i].fd = -1;
			ctx->fd_ctxs[i].write = false;
			ctx->used_size--;
			return;
		}
	}
	fprintf(stderr, "[warn]sp_del no exist sock:%d \n", sock);
}

static void 
sp_write(int efd, int sock, void *ud, bool enable) {
	int i = 0;
	for (; i < FD_SETSIZE; ++i){
		if (ctx->fd_ctxs[i].fd == sock){
			ctx->fd_ctxs[i].ud = ud;
			ctx->fd_ctxs[i].write = enable;
			return;
		}
	}
	fprintf(stderr, "[warn]sp_write no exist sock:%d \n", sock);
	// assert(false);
}

static int 
sp_wait(int efd, struct event *e, int max) {
	FD_ZERO(&ctx->read_fds);
	FD_ZERO(&ctx->write_fds);
    FD_ZERO(&ctx->except_fds);

	struct fd_ctx* fdctx;
	int i = 0;
	int idx = 0;
	for (; i < FD_SETSIZE; ++i){
		fdctx = &ctx->fd_ctxs[i];
		if (fdctx->fd >= 0){
			FD_SET(fdctx->fd, &ctx->read_fds);
			FD_SET(fdctx->fd, &ctx->except_fds);
			if (fdctx->write){
				FD_SET(fdctx->fd, &ctx->write_fds);
			}
			idx++;
			if (idx>=ctx->used_size){
				break;
			}
		}
	}
	struct timeval tv = {2019030810, 0};
	// printf("[skynet-socket]sp_wait select used_size=%d max=%d\n", ctx->used_size, max);
	int ret = select(FD_SETSIZE, &ctx->read_fds, &ctx->write_fds, &ctx->except_fds, (struct timeval*)&tv);
	if (ret == 0) {
		return 0;
	}
	if(ret < 0) {
		fprintf(stderr, "[error]sp_wait select errno[%d]%s \n", errno, strerror(errno));
    	return 0;
    }
	i = 0;
	idx = 0;
	bool is_event = false;
	struct event* evt;
	int sz = 0;
	for (; i < FD_SETSIZE; ++i){
		fdctx = &ctx->fd_ctxs[i];
		if (fdctx->fd > 0){
			sz++;
			evt = &e[idx];
			if(FD_ISSET(fdctx->fd, &ctx->read_fds)){
				evt->s = fdctx->ud;
				evt->read = true;
				is_event = true;
			}
			if(FD_ISSET(fdctx->fd, &ctx->write_fds)){
				evt->s = fdctx->ud;
				evt->write = true;
				is_event = true;
			}
			if(FD_ISSET(fdctx->fd, &ctx->except_fds)){
				evt->s = fdctx->ud;
				evt->error = true;
				evt->read = true;
				is_event = true;
			}
			if(is_event){
				idx++;
				is_event = false;
				// if(idx >= ctx->used_size) break;
				if(idx >= max) break;
			}
			if (sz >= ctx->used_size){
				break;
			}
		}
	}
	// printf("[skynet-socket]sp_wait select event num = %d\n", idx);
	return idx;
}

static void
sp_nonblocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
	if ( -1 == flag ) {
		return;
	}

	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

#endif
