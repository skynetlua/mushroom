
#include "mr_socket.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include "win/winport.h"
#include "win/spinlock.h"
#else
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "spinlock.h"
#endif

#include "socket_server.h"
#include "mr_slist.h"
#include "mr_config.h"

#define MR_SOCKET_TYPE_DATA 1
#define MR_SOCKET_TYPE_CONNECT 2
#define MR_SOCKET_TYPE_CLOSE 3
#define MR_SOCKET_TYPE_ERROR 4
#define MR_SOCKET_TYPE_WARNING 5
#define MR_SOCKET_TYPE_COUNT 6
#define MR_SOCKET_TYPE_ACCEPT 7
#define MR_SOCKET_TYPE_UDP 8

struct mr_socket {
	struct mr_slist msg_list;
	struct spinlock list_lock;
	mr_callback4 cbs[MR_SOCKET_TYPE_COUNT];
	mr_callback5 apt_bc;
	mr_udp_callback udpcb;
	pthread_t thread;
	uint8_t thread_run;
};

struct mr_message {
	struct mr_slist_node node;
	int type;
	int fd;
	uintptr_t uid;
	int ud;
	char* buffer;
	int size;
	char* option;
	// char* info;
};

static struct socket_server * SOCKET_SERVER = NULL;
static struct mr_socket * MR_SOCKET = NULL;

static void mr_handle_data(uintptr_t uid, int fd, char* data, int size){
    printf("[mr_socket]mr_handle_data uid=%d fd=%d size=%d data=%s \n", (int)uid, fd, size, data);
}
static void mr_handle_connect(uintptr_t uid, int fd, char* data, int size){
	printf("[mr_socket]mr_handle_connect uid=%d fd=%d size=%d data=%s \n", (int)uid, fd, size, data);
}
static void mr_handle_close(uintptr_t uid, int fd, char* data, int size){
    printf("[mr_socket]mr_handle_close uid=%d fd=%d size=%d data=%s \n", (int)uid, fd, size, data);
}
static void mr_handle_accept(uintptr_t uid, int fd, char* data, int size, int apt_fd){
    printf("[mr_socket]mr_handle_accept uid=%d fd =%d accept_fd=%d data=%s\n", (int)uid, fd, apt_fd, data);
    mr_socket_start(uid, apt_fd);
}
static void mr_handle_error(uintptr_t uid, int fd, char* data, int size){
    printf("[mr_socket]mr_handle_error uid=%d fd=%d size=%d data=%s \n", (int)uid, fd, size, data);
}
static void mr_handle_warning(uintptr_t uid, int fd, char* data, int size){
    printf("[mr_socket]mr_handle_warning uid=%d fd=%d size=%d data=%s \n", (int)uid, fd, size, data);
}
static void mr_handle_udp(uintptr_t uid, int fd, char* data, int size, char* address){
	char udp_addr[256];
    mr_socket_udp_address(address, udp_addr, sizeof(udp_addr));

    char* tmp = (char*)malloc(size+1);
	memset(tmp, 0, size + 1);
    memcpy(tmp, data, size);
    printf("[mr_socket]mr_handle_udp[%s] uid=%d fd=%d size=%d data=%s \n", udp_addr, (int)uid, fd, size, tmp);
}

void mr_socket_init(void) {
	assert(!SOCKET_SERVER);
	// SOCKET_SERVER = socket_server_create(skynet_now());
	SOCKET_SERVER = socket_server_create(0);

	MR_SOCKET = (struct mr_socket*)MALLOC(sizeof(struct mr_socket));
	memset(MR_SOCKET, 0, sizeof(struct mr_socket));
	mr_slist_clear(&MR_SOCKET->msg_list);
	spinlock_init(&MR_SOCKET->list_lock);

	mr_callback4* cbs = MR_SOCKET->cbs;
	cbs[MR_SOCKET_TYPE_DATA] = mr_handle_data;
	cbs[MR_SOCKET_TYPE_CONNECT] = mr_handle_connect;
	cbs[MR_SOCKET_TYPE_CLOSE] = mr_handle_close;
	cbs[MR_SOCKET_TYPE_ERROR] = mr_handle_error;
	cbs[MR_SOCKET_TYPE_WARNING] = mr_handle_warning;
	MR_SOCKET->apt_bc = mr_handle_accept;
	MR_SOCKET->udpcb = mr_handle_udp;
}

void mr_set_handle_data(mr_callback4 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->cbs[MR_SOCKET_TYPE_DATA] = cb;
}
void mr_set_handle_connect(mr_callback4 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->cbs[MR_SOCKET_TYPE_CONNECT] = cb;
}
void mr_set_handle_close(mr_callback4 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->cbs[MR_SOCKET_TYPE_CLOSE] = cb;
}
void mr_set_handle_accept(mr_callback5 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->apt_bc = cb;
}
void mr_set_handle_error(mr_callback4 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->cbs[MR_SOCKET_TYPE_ERROR] = cb;
}
void mr_set_handle_warning(mr_callback4 cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->cbs[MR_SOCKET_TYPE_WARNING] = cb;
}
void mr_set_handle_udp(mr_udp_callback cb){
	assert(SOCKET_SERVER && cb);
	MR_SOCKET->udpcb = cb;
}

void mr_socket_clear(void) {
	assert(MR_SOCKET);
	if (!mr_slist_is_empty(&MR_SOCKET->msg_list)){
		struct mr_slist_node* node;

	    spinlock_lock(&MR_SOCKET->list_lock);
	    node = mr_slist_clear(&MR_SOCKET->msg_list);
	    spinlock_unlock(&MR_SOCKET->list_lock);

	    struct mr_message* msg;
	     while(node){
			msg = (struct mr_message*)node;
			node = node->next;
	      	if (msg->buffer){
	      		FREE(msg->buffer);
	      	}
	       	FREE(msg);
		}
	}
}

void mr_socket_free(void) {
	assert(SOCKET_SERVER);

	MR_SOCKET->thread_run = 0;
	pthread_join(MR_SOCKET->thread, NULL); 

	socket_server_release(SOCKET_SERVER);
	SOCKET_SERVER = NULL;

	mr_socket_clear();
	spinlock_destroy(&MR_SOCKET->list_lock);
	FREE(MR_SOCKET);
	MR_SOCKET = NULL;
}

void mr_socket_update(void){
	assert(SOCKET_SERVER);
	// socket_server_updatetime(SOCKET_SERVER, skynet_now());
	if (mr_slist_is_empty(&MR_SOCKET->msg_list)){
		return;
	}
	struct mr_slist_node* node;
    spinlock_lock(&MR_SOCKET->list_lock);
    node = mr_slist_clear(&MR_SOCKET->msg_list);
    spinlock_unlock(&MR_SOCKET->list_lock);

    assert(node != NULL);
    struct mr_message* msg;
    char* buffer;
    while(node){
    	msg = (struct mr_message*)node;
    	node = node->next;
    	buffer = msg->buffer == NULL?"":msg->buffer;
      	if (msg->type == MR_SOCKET_TYPE_UDP) {
      		MR_SOCKET->udpcb(msg->uid, msg->fd, buffer, msg->size, msg->option);
      	}else if (msg->type == MR_SOCKET_TYPE_ACCEPT) {
			MR_SOCKET->apt_bc(msg->uid, msg->fd, msg->buffer, msg->size, msg->ud);
      	}else{
      		MR_SOCKET->cbs[msg->type](msg->uid, msg->fd, msg->buffer, msg->size);
      	}
      	if (msg->buffer){
      		FREE(msg->buffer);
      	}
       	FREE(msg);
    }
}

static void forward_message(int type, bool padding, struct socket_message * result) {
	struct mr_message* msg = (struct mr_message*)MALLOC(sizeof(struct mr_message));
	msg->type = type;
	msg->fd = result->id;
	msg->ud = result->ud;
	msg->uid = result->opaque;
	if (padding){
		if (result->data){
			size_t msg_sz = strlen(result->data);
			if (msg_sz > 128){
				msg_sz = 128;
			}
			msg->size = msg_sz+1;
			msg->buffer = (char*)MALLOC(msg->size);
			memset(msg->buffer, 0, msg->size);
			memcpy(msg->buffer, result->data, msg->size);
		}else{
			msg->buffer = NULL;
			msg->size = 0;
		}
	}else{
		msg->buffer = result->data;
		msg->size = result->ud;
		if (msg->type == MR_SOCKET_TYPE_UDP) {
			msg->option = msg->buffer + msg->ud;
		}
		msg->ud = 0;
	}

	spinlock_lock(&MR_SOCKET->list_lock);
	mr_slist_link(&MR_SOCKET->msg_list, (struct mr_slist_node*)msg);
	spinlock_unlock(&MR_SOCKET->list_lock);
}

int mr_socket_poll(void) {
	struct socket_server *ss = SOCKET_SERVER;
	assert(ss);
	struct socket_message result;
	int more = 1;
	int type = socket_server_poll(ss, &result, &more);
	switch (type) {
	case SOCKET_EXIT:
		return 0;
	case SOCKET_DATA:
		forward_message(MR_SOCKET_TYPE_DATA, false, &result);
		break;
	case SOCKET_CLOSE:
		forward_message(MR_SOCKET_TYPE_CLOSE, false, &result);
		break;
	case SOCKET_OPEN:
		forward_message(MR_SOCKET_TYPE_CONNECT, true, &result);
		break;
	case SOCKET_ERR:
		forward_message(MR_SOCKET_TYPE_ERROR, true, &result);
		break;
	case SOCKET_ACCEPT:
		forward_message(MR_SOCKET_TYPE_ACCEPT, true, &result);
		break;
	case SOCKET_UDP:
		forward_message(MR_SOCKET_TYPE_UDP, false, &result);
		break;
	case SOCKET_WARNING:
		forward_message(MR_SOCKET_TYPE_WARNING, false, &result);
		break;
	default:
		if (type != -1){
			fprintf(stderr, "Unknown socket message type %d.\n",type);
		}
		return -1;
	}
	if (more) {
		return -1;
	}
	return 1;
}

static void *thread_socket(void* p) {
	int r;
	while (MR_SOCKET->thread_run) {
		r = mr_socket_poll();
		if (r==0) break;
	}
	mr_socket_clear();
	return NULL;
}

void mr_socket_run(void){
	MR_SOCKET->thread_run = 1;
	int ret = pthread_create(&MR_SOCKET->thread, NULL, (void *)&thread_socket, NULL);
	if (ret != 0) {
		fprintf(stderr, "Create thread failed");
		exit(1);
	}
}

int mr_socket_send(int fd, void* buffer, int sz) {
	char* sbuffer = (char*)MALLOC(sz);
	memcpy(sbuffer, buffer, sz);
	return socket_server_send(SOCKET_SERVER, fd, sbuffer, sz);
}

int mr_socket_send_lowpriority(int fd, void* buffer, int sz) {
	char* sbuffer = (char*)MALLOC(sz);
	memcpy(sbuffer, buffer, sz);
	return socket_server_send_lowpriority(SOCKET_SERVER, fd, sbuffer, sz);
}

void mr_socket_nodelay(int fd) {
	socket_server_nodelay(SOCKET_SERVER, fd);
}

int mr_socket_listen(uintptr_t uid, const char* host, int port, int backlog) {
	return socket_server_listen(SOCKET_SERVER, uid, host, port, backlog);
}

int mr_socket_connect(uintptr_t uid, const char *host, int port) {
	return socket_server_connect(SOCKET_SERVER, uid, host, port);
}

int mr_socket_bind(uintptr_t uid, int fd) {
	return socket_server_bind(SOCKET_SERVER, uid, fd);
}

void mr_socket_close(uintptr_t uid, int fd) {
	socket_server_close(SOCKET_SERVER, uid, fd);
}

void mr_socket_shutdown(uintptr_t uid, int fd) {
	socket_server_shutdown(SOCKET_SERVER, uid, fd);
}

void mr_socket_start(uintptr_t uid, int fd) {
	socket_server_start(SOCKET_SERVER, uid, fd);
}

int mr_socket_udp(uintptr_t uid, const char* addr, int port) {
	return socket_server_udp(SOCKET_SERVER, uid, addr, port);
}

int mr_socket_udp_connect(int fd, const char* addr, int port) {
	return socket_server_udp_connect(SOCKET_SERVER, fd, addr, port);
}

int mr_socket_udp_send(int fd, const char* address, const void *buffer, int sz) {
	char* sbuffer = (char*)MALLOC(sz);
	memcpy(sbuffer, buffer, sz);
	return socket_server_udp_send(SOCKET_SERVER, fd, (const struct socket_udp_address*)address, sbuffer, sz);
}

int mr_socket_udp_address(const char* address, char* udp_addr, int len){
	if (!address){
		return -1;
	}
	int type = address[0];
	int family;
	switch(type) {
		case PROTOCOL_UDP:
			family = AF_INET;
			break;
		case PROTOCOL_UDPv6:
			family = AF_INET6;
			break;
		default:
			return -1;
	}
	uint16_t port = 0;
    memcpy(&port, address+1, sizeof(uint16_t));
    port = ntohs(port);
    const void * addrptr = address+3;
    char strptr[256] = {0};
    if (!inet_ntop(family, addrptr, strptr, sizeof(strptr))) {
    	return -1;
    }
    snprintf(udp_addr, len, "%s:%d", strptr, port);
    return 0;
}

struct socket_info *mr_socket_info(void) {
	return socket_server_info(SOCKET_SERVER);
}
