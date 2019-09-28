
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mrsocket.h"
#include "mrtool.h"

static int _session_uid = 1;

void server_handle_data(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("接收到客户端消息 session_id =%d, fd = %d, size = %d \n", session_id, fd, size);
    char msg[128] = {0};
    if (data[0] == 1)
    {
        sprintf(msg, "\2还没呢，你真贴心。^_^");
        int ret = mr_socket_send(fd, msg, sizeof(msg));
        if (ret < 0)
        {
            printf("[server]server_handle_data faild ret = %d\n", ret);
        }
    }
    else if (data[0] == 3)
    {
        sprintf(msg, "\4讨厌。才刚认识呢！我们先聊聊嘛");
        mr_socket_send(fd, msg, sizeof(msg));
        mr_socket_close((uintptr_t)session_id, fd);
    }
}

void server_handle_close(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    if (session_id == 0)
    {
        printf("服务端监听0.0.0.0:8080已关闭 session_id=%d\n", session_id);
    }
    else
    {
        printf("客户端已关闭连接 session_id=%d\n", session_id);
    }
}

void server_handle_accept(uintptr_t uid, int fd, char* data, int size, int apt_fd)
{
    int listen_session_id = uid;
    assert(listen_session_id == 0);
    
    char* addr = malloc(size+1);
    memset((void*)addr, 0, sizeof(addr));
    memcpy(addr, data, size);
    printf("监听到新的链接addr=%s, apt_fd=%d \n", addr, apt_fd);
    free(addr);
    
    int session_id = _session_uid++;
    mr_socket_start((uintptr_t)session_id, apt_fd);
    printf("接受与客户端连接 session_id=%d\n", session_id);
}

void server_handle_error(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生错误 session_id = %d, fd = %d, data = %s, size=%d \n", session_id, fd, data, size);
}

void server_handle_warning(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生警告 session_id = %d, fd = %d, data = %s, size=%d \n", session_id, fd, data, size);
}

static void server_handle_connect(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    if (session_id == 0){
        printf("监听0.0.0.0:8080成功\n");
    }else{
        printf("连接客户端成功 session_id = %d\n", session_id);
    }
//    assert(session_id == 0);
}

int main(int argc, char* argv[])
{
    //初始化mushroom（只能初始化一次）
    mr_socket_init();

    mr_set_handle_data(server_handle_data);
    mr_set_handle_close(server_handle_close);
    mr_set_handle_accept(server_handle_accept);
    mr_set_handle_error(server_handle_error);
    mr_set_handle_warning(server_handle_warning);
    mr_set_handle_connect(server_handle_connect);

    mr_socket_run();
    printf("mushroom启动成功\n");

    //启动监听端口
    //监听IP范围和端口号，ip若指定具体IP，只能接收该IP连接"
    int listen_session_id = 0;
    int server_fd = mr_socket_listen((uintptr_t)listen_session_id, "0.0.0.0", 8080, 64);
    if (server_fd < 0) {
       printf("mr_socket_listen faild server_fd = %d\n", server_fd);
       assert(0);
    }
    mr_socket_start((uintptr_t)listen_session_id, server_fd);
    printf("启动监听0.0.0.0:8080\n");

    while(1){
        //业务线程循环检测是否有网络消息，有网络消息，就会通过回调函数返回
        //与回调函数同一线程
       mr_socket_update();
       mr_sleep(1);
    }

    mr_socket_free();
    return 0;
}

