
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mrsocket.h"
#include "mrtool.h"

int server_fd = 0;
int client_fd = 0;
int timer_count = 0;
int invite_count = 0;

//服务器监听session id
const int server_session_id = 1;
//服务器中与客户端连接的session id
const int accept_session_id = 2;
//客户端的session id
const int client_session_id = 3;

static void handle_data(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    if (client_session_id == session_id)
    {
        printf("[boy]接收到服务端消息 fd = %d, data = %s \n", fd, data);
        if (data[0] == 2)
        {
            char msg[128] = "\3那你要出来吃饭吗？";
            mr_socket_send(fd, msg, sizeof(msg));
        }
    }
    else if (accept_session_id == session_id)
    {
        printf("[girl]接收到客户端消息 fd = %d, data = %s \n", fd, data);
        if (data[0] == 1)
        {
            char msg[128] = "\2还没呢，你真贴心。^_^";
            mr_socket_send(fd, msg, sizeof(msg));
        }
        else if (data[0] == 3)
        {
            char msg[128] = "\4讨厌。才刚认识呢！我们先聊聊嘛";
            mr_socket_send(fd, msg, sizeof(msg));
            mr_socket_close((uintptr_t)session_id, fd);
            printf("[girl]拒绝邀约，关闭客户端\n");
        }
    }
    else
    {
        assert(0);
    }
}

static void handle_connect(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    if (server_session_id == session_id)
    {
        assert(server_fd == fd);
        printf("[server]服务器监听启动成功 data =%s, size = %d \n", data, size);
    }
    else if (client_session_id == session_id)
    {
        assert(client_fd == fd);
        printf("\n[boy]客户端连接服务器启动成功 data =%s, size = %d \n", data, size);
        char msg[128] = "\1靓女，你吃饭了吗？";
        mr_socket_send(fd, msg, sizeof(msg));
        printf("[boy]开始第%d次邀请妹纸\n", ++invite_count);
    }
    else if (accept_session_id == session_id)
    {
        printf("[girl]客户端连接服务器成功 data =%s, size = %d \n", data, size);
    }
    else
    {
        assert(0);
    }
}

static void handle_close(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    if (client_session_id == session_id)
    {
        printf("[boy]连接关闭 fd = %d, data=%s, size = %d \n", fd, data, size);
        //等3秒，再预约
        client_fd = 0;
        timer_count = 3000;
    }
    else if (accept_session_id == session_id)
    {
        printf("[girl]连接关闭 fd = %d, data=%s, size = %d \n", fd, data, size);
    }
}

static void handle_error(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生错误 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
}

static void handle_warning(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生警告 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
}

void handle_accept(uintptr_t uid, int fd, char* data, int size, int apt_fd)
{
    int session_id = (int)uid;
    assert(session_id == server_session_id);
    
    char* addr = malloc(size+1);
    memset((void*)addr, 0, sizeof(addr));
    memcpy(addr, data, size);
    printf("[server]监听到新的链接addr=%s, apt_fd=%d \n", addr, apt_fd);
    free(addr);
    
    mr_socket_start((uintptr_t)accept_session_id, apt_fd);
    printf("[girl]接受与客户端连接 apt_fd =%d\n", apt_fd);
}

int main(int argc, char* argv[])
{
    //初始化mushroom（只能初始化一次）
    mr_socket_init();
    //设置回调函数
    //配置socket数据包回调函数
    mr_set_handle_data(handle_data);
    //配置socket链接回调函数
    mr_set_handle_connect(handle_connect);
    //配置socket关闭回调函数
    mr_set_handle_close(handle_close);
    //配置socket发生错误回调函数
    mr_set_handle_error(handle_error);
    //配置socket发生警告回调函数
    mr_set_handle_warning(handle_warning);
    //配置socket监听到新连接回调函数
    mr_set_handle_accept(handle_accept);

    //启动mushroom，并启动线程监听和接收数据包
    mr_socket_run();
    printf("[mushroom]mushroom启动成功\n");
    //启动服务器监听
    server_fd = mr_socket_listen((uintptr_t)server_session_id, "0.0.0.0", 8080, 64);
    if (server_fd < 0) {
        printf("[server]mr_socket_listen faild server_fd = %d\n", server_fd);
        assert(0);
    }
    mr_socket_start((uintptr_t)server_session_id, server_fd);
    printf("[server]启动监听0.0.0.0:8080 server_fd = %d\n", server_fd);
    //启动连接服务器
    client_fd = mr_socket_connect((uintptr_t)client_session_id, "127.0.0.1", 8080);
    if (client_fd < 0)
    {
        printf("[boy]连接失败 faild client_fd = %d\n", client_fd);
        assert(0);
    }
    printf("[boy]启动连接服务器 client_fd = %d\n", client_fd);
    while(1)
    {
        //业务线程循环检测是否有网络消息，有网络消息，就会通过回调函数返回
        mr_socket_update();
        mr_sleep(1);
        
        if (timer_count == 0)
        {
            if (client_fd == 0)
                //断线重连服务器
                client_fd = mr_socket_connect((uintptr_t)client_session_id, "127.0.0.1", 8080);
        }
        else
        {
            timer_count--;
            if (timer_count<0)
                timer_count = 0;
        }
    }
    //销毁mushroo线程，并释放相应内存
    mr_socket_free();
    return 0;
}
