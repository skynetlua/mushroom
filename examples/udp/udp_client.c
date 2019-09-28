
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mrsocket.h"
#include "mrtool.h"

struct User{
    int id;
    int type;
    int fd;
    int snd_id;
    int rcv_id;
    struct mr_buffer* buffer;
};


#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
//60 connections
#define TEST_CLIENT_NUM 60
#else
//1000 connections
#define TEST_CLIENT_NUM 1000
//Yes,1000 socket connect sever
#endif
#define TEST_SERVER_IP "127.0.0.1"
// #define TEST_SERVER_IP "192.168.188.224"
#define TEST_SERVER_PORT 8765
struct User* clientUsers[TEST_CLIENT_NUM] = {0};


struct User* create_user(){
    struct User* user = (struct User*)malloc(sizeof(struct User));
    user->buffer = mr_buffer_create(4);
    return user;
}

void destroy_user(struct User* user){
    mr_buffer_free(user->buffer);
    free(user);
}

void client_handle_udp(uintptr_t uid, int fd, char* data, int size, char* address)
{
    char udp_addr[256];
    mr_socket_udp_address(address, udp_addr, sizeof(udp_addr));

    char* tmp = (char*)malloc(size+1);
	memset(tmp, 0, size + 1);
    memcpy(tmp, data, size);
    printf("client_handle_udp[%s] uid=%d fd =%d buffer=%s sz=%d ud=%d\n", udp_addr, (int)uid, fd, tmp, (int)strlen(tmp), size);

    //mr_sleep(100);

    static int _sid = 1;
    char sbuffer[2048] = {0};
    snprintf(sbuffer, 2048, "send data hello world sid=%d", _sid++);
    int ret = mr_socket_udp_send(fd, address, sbuffer, (int)strlen(sbuffer));
    if (ret < 0){
        printf("mr_socket_udp_send faild ret = %d\n", ret);
    }
}


int main(int argc, char* argv[])
{
    mr_socket_init();
    mr_socket_run();

    mr_set_handle_udp(client_handle_udp);

   int i = 0;
   for (; i < TEST_CLIENT_NUM; ++i){
        struct User* user = create_user();
        user->id = i;
        int fd = mr_socket_udp((uintptr_t)user, NULL, 0);
        if (fd < 0){
            printf("mr_socket_udp faild fd = %d\n", fd);
        }
        mr_socket_udp_connect(fd, TEST_SERVER_IP, TEST_SERVER_PORT);

        char buffer[2048] = {0};
        snprintf(buffer, 2048, "udp test send data hello world");
        int ret = mr_socket_send(fd, buffer, (int)strlen(buffer));
        if (ret < 0)
        {
           printf("mr_socket_send faild ret = %d\n", ret);
        }
        user->fd = fd;
        clientUsers[i] = user;
    }
    while(1)
    {
       mr_socket_update();
       mr_sleep(1);
    }

    i = 0;
    for (; i < TEST_CLIENT_NUM; ++i){
        if (clientUsers[i]){
            destroy_user(clientUsers[i]);
            clientUsers[i] = NULL;
        }
    }
    mr_socket_free();

    return 0;
}

