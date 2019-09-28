
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

#define TEST_SERVER_IP "0.0.0.0"
#define TEST_SERVER_PORT 8765

struct User* serverUser = NULL;
struct User* clientUsers[0xffff] = {0};

struct User* create_user(){
    struct User* user = (struct User*)malloc(sizeof(struct User));
    user->buffer = mr_buffer_create(4);
    return user;
}

void destroy_user(struct User* user){
    mr_buffer_free(user->buffer);
    free(user);
}

void server_handle_data(uintptr_t uid, int fd, char* data, int size)
{
    printf("[main]server_handle_data uid =%d \n", (int)uid);

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

        uint32_t cur_time = mr_clock();
        printf("[server]id = %d, costtime = %d \n", id, cur_time-send_time);

        assert(id%2 == 0);
        char* enptr = buffer->read_data;
        enptr = mr_encode32u(enptr, ++id);

        mr_buffer_write_push(buffer, buffer->read_data, buffer->read_len);
        mr_buffer_write_pack(buffer);
        int ret = mr_socket_send(fd, buffer->write_data, buffer->write_len);
        if (ret < 0){
            printf("[server]server_handle_data faild ret = %d\n", ret);
        }
    }
}

void server_handle_close(uintptr_t uid, int fd, char* data, int size)
{
    printf("server_handle_close uid=%d\n", (int)uid);
    struct User* user = (struct User*)uid;
    if (user == serverUser){
        
    }else{
        int i = 0;
        for (; i < 0xffff; ++i)
        {
            if (clientUsers[i] == user)
            {
                clientUsers[i] = NULL;
                destroy_user(user);
                return;
            }
        }
    }
}

void server_handle_accept(uintptr_t uid, int fd, char* data, int size, int apt_fd)
{
    printf("server_handle_accept uid=%d, fd=%d, data=%s, size=%d, apt_fd=%d \n", (int)uid, fd, data, size, apt_fd);
    int i = 0;
    for (; i < 0xffff; ++i)
    {
        if (!clientUsers[i])
        {
            struct User* user = create_user();
            clientUsers[i] = user;
            mr_socket_start((uintptr_t)user, apt_fd);
            return;
        }
    }
    //too many client. refuse connect;
    mr_socket_close(uid, apt_fd);
}

void server_handle_error(uintptr_t uid, int fd, char* data, int size)
{
    printf("server_handle_error uid = %d, fd = %d, data = %s, size=%d \n", (int)uid, fd, data, size);
}

void server_handle_warning(uintptr_t uid, int fd, char* data, int size)
{
    printf("server_handle_warning uid = %d, fd = %d, data = %s, size=%d \n", (int)uid, fd, data, size);
}

int main(int argc, char* argv[])
{
    mr_socket_init();
    mr_socket_run();

    mr_set_handle_data(server_handle_data);
    mr_set_handle_close(server_handle_close);
    mr_set_handle_accept(server_handle_accept);
    mr_set_handle_error(server_handle_error);
    mr_set_handle_warning(server_handle_warning);

    struct User* user = create_user();
    serverUser = user;
    
    int server_fd = mr_socket_listen((uintptr_t)user, TEST_SERVER_IP, TEST_SERVER_PORT, 64);
    if (server_fd < 0) {
       printf("mr_socket_listen faild server_fd = %d\n", server_fd);
       assert(0);
    }
    mr_socket_start((uintptr_t)user, server_fd);
    printf("[main]start server\n");
    while(1){
       mr_socket_update();
       mr_sleep(1);
    }

    int i = 0;
    for (; i < 0xffff; ++i){
        if (clientUsers[i]){
            destroy_user(clientUsers[i]);
            clientUsers[i] = NULL;
        }
    }
    destroy_user(user);
    serverUser = NULL;
    mr_socket_free();
    return 0;
}

