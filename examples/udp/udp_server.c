
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

void server_handle_udp(uintptr_t uid, int fd, char* data, int size, char* address)
{
    // struct User* user = (struct User*)uid;

    char udp_addr[256];
    mr_socket_udp_address(address, udp_addr, sizeof(udp_addr));

    char* tmp = (char*)malloc(size+1);
    memset(tmp, 0, size + 1);
    memcpy(tmp, data, size);
    printf("server_handle_udp[%s] uid=%d fd =%d buffer=%s sz=%d ud=%d\n", udp_addr, (int)uid, fd, tmp, (int)strlen(tmp), size);
    

    int ret = mr_socket_udp_send(fd, address, data, size);
    if (ret < 0){
        printf("server_handle_udp faild ret = %d\n", ret);
    }
}

int main(int argc, char* argv[])
{
    mr_socket_init();
    mr_socket_run();

    mr_set_handle_udp(server_handle_udp);

    struct User* user = (struct User*)malloc(sizeof(struct User));
    int server_fd = mr_socket_udp((uintptr_t)user, TEST_SERVER_IP, TEST_SERVER_PORT);
    if (server_fd < 0)
    {
        printf("mr_socket_udp faild server_fd = %d\n", server_fd);
    }
    while(1)
    {
       mr_socket_update();
       mr_sleep(1);
    }
    free(user);
    return 0;
}

