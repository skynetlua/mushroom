
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


static void client_handle_data(uintptr_t uid, int fd, char* data, int size)
{
    printf("client_handle_data uid = %d, fd = %d, size = %d \n", (int)uid, fd, size);
    struct User* user = (struct User*)uid;
    struct mr_buffer* buffer = user->buffer;
    mr_buffer_read_push(buffer, data, size);
    int ret = mr_buffer_read_pack(buffer);
    if (ret > 0){
        const char* ptr = buffer->read_data;
        uint32_t id = 0;
        ptr = mr_decode32u(ptr, &id);
        uint32_t send_time = 0;
        ptr = mr_decode32u(ptr, &send_time);
        uint32_t rcv_id = 0;
        ptr = mr_decode32u(ptr, &rcv_id);
        assert(user->rcv_id == rcv_id);
        user->rcv_id++;

        uint32_t cur_time = mr_clock();
        printf("[client]id = %d, rcv_id=%d, costtime = %d \n", id, rcv_id, cur_time-send_time);
        assert(id%2 == 1);

        char* enptr = buffer->read_data;
        enptr = mr_encode32u(enptr, ++id);
        enptr = mr_encode32u(enptr, cur_time);
        enptr = mr_encode32u(enptr, (uint32_t)user->snd_id);
        user->snd_id++;

        mr_buffer_write_push(buffer, buffer->read_data, buffer->read_len);
        mr_buffer_write_pack(buffer);
        int ret = mr_socket_send(fd, buffer->write_data, buffer->write_len);
        if (ret < 0)
        {
           printf("[client]mr_socket_send faild ret = %d\n", ret);
        }
    }
}

static void client_handle_connect(uintptr_t uid, int fd, char* data, int size)
{
    printf("client_handle_connect uid = %d, fd = %d, data =%s, size = %d \n", (int)uid, fd, data, size);

    struct User* user = (struct User*)uid;
    user->snd_id = 0;
    user->rcv_id = 0;
    //100KB data
    char tmp[1024*100] = {0};
    // snprintf(tmp, 2048, "send data hello world");
    memset(tmp, 97, sizeof(tmp)-1);
    char* ptr = tmp;

    uint32_t id = 0;
    ptr = mr_encode32u(ptr, id);
    uint32_t time = mr_clock();
    ptr = mr_encode32u(ptr, time);
    ptr = mr_encode32u(ptr, (uint32_t)user->snd_id);
    user->snd_id++;

   struct mr_buffer* buffer = user->buffer;
   mr_buffer_write_push(buffer, tmp, sizeof(tmp));
   mr_buffer_write_pack(buffer);
   int ret = mr_socket_send(fd, buffer->write_data, buffer->write_len);
   if (ret < 0)
   {
       printf("mr_socket_send faild ret = %d\n", ret);
   }
}

static void client_handle_close(uintptr_t uid, int fd, char* data, int size)
{
    printf("client_handle_close uid = %d, fd = %d, data=%s, size = %d \n", (int)uid, fd, data, size);
}

static void client_handle_error(uintptr_t uid, int fd, char* data, int size)
{
    printf("client_handle_error uid = %d, fd = %d, data=%s, size = %d \n", (int)uid, fd, data, size);
}

static void client_handle_warning(uintptr_t uid, int fd, char* data, int size)
{
    printf("client_handle_warning uid = %d, fd = %d, data=%s, size = %d \n", (int)uid, fd, data, size);
}

int main(int argc, char* argv[])
{
    mr_socket_init();
    mr_socket_run();

    mr_set_handle_data(client_handle_data);
    mr_set_handle_connect(client_handle_connect);
    mr_set_handle_close(client_handle_close);
    mr_set_handle_error(client_handle_error);
    mr_set_handle_warning(client_handle_warning);

    int i = 0;
    for (i = 0; i < TEST_CLIENT_NUM; ++i)
    {
        struct User* user = create_user();
        user->id = i;
        uintptr_t uid = (uintptr_t)user;
        int fd = mr_socket_connect(uid, TEST_SERVER_IP, TEST_SERVER_PORT);
        if (fd < 0)
        {
            printf("mr_socket_connect faild fd = %d\n", fd);
            assert(0);
        }
        printf("mr_socket_connect id=%d, uid=%ld, fd =%d \n", user->id, uid, fd);
        user->fd = fd;
        clientUsers[i] = user;
    }
    printf("start success\n");
    while(1)
    {
       mr_socket_update();
       mr_sleep(1);
    }

    i = 0;
    for (; i < TEST_CLIENT_NUM; ++i)
    {
        if (clientUsers[i])
        {
            destroy_user(clientUsers[i]);
            clientUsers[i] = NULL;
        }
    }
    mr_socket_free();
    return 0;
}
