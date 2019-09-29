
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mrsocket.h"
#include "mrtool.h"

const int server_session_id = 1;
const int client_session_id = 2;

static void handle_data(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("接收到服务端消息 session_id =%d, fd = %d, data = %s \n", session_id, fd, data);
    if (data[0] == 2)
    {
        char msg[128] = "\3那你要出来吃饭吗？";
        int ret = mr_socket_send(fd, msg, sizeof(msg));
        if (ret < 0)
        {
            printf("mr_socket_send faild ret = %d\n", ret);
        }
    }
}

static void handle_connect(uintptr_t uid, int fd, char* data, int size)
{
    printf("连接服务器成功 uid = %d, fd = %d, data =%s, size = %d \n", (int)uid, fd, data, size);
    char msg[128] = "\1靓女，你吃饭了吗？";
    int ret = mr_socket_send(fd, msg, sizeof(msg));
    if (ret < 0)
    {
        printf("mr_socket_send faild ret = %d\n", ret);
    }
}

static void handle_close(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("连接关闭 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
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
    int listen_session_id = uid;
    assert(listen_session_id == 0);
//
//    char* addr = malloc(size+1);
//    memset((void*)addr, 0, sizeof(addr));
//    memcpy(addr, data, size);
//    printf("监听到新的链接addr=%s, apt_fd=%d \n", addr, apt_fd);
//    free(addr);
//    
//    int session_id = _session_uid++;
//    mr_socket_start((uintptr_t)session_id, apt_fd);
//    printf("接受与客户端连接 session_id=%d\n", session_id);
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
    printf("mushroom启动成功\n");

    //启动服务器监听
    int server_fd = mr_socket_listen((uintptr_t)server_session_id, "0.0.0.0", 8080, 64);
    if (server_fd < 0) {
        printf("mr_socket_listen faild server_fd = %d\n", server_fd);
        assert(0);
    }
    mr_socket_start((uintptr_t)server_session_id, server_fd);
    printf("启动监听0.0.0.0:8080 server_fd = %d\n", server_fd);
  
    //启动连接服务器
    int client_fd = mr_socket_connect((uintptr_t)client_session_id, "127.0.0.1", 8080);
    if (client_fd < 0)
    {
        printf("连接失败 faild client_fd = %d\n", client_fd);
        assert(0);
    }
    printf("启动连接服务器 client_fd = %d\n", client_fd);

    //轮询
    while(1)
    {
        //业务线程循环检测是否有网络消息，有网络消息，就会通过回调函数返回
        //此线程会调用回调函数
        mr_socket_update();
        mr_sleep(1);
    }
    
    //销毁mushroo线程，并释放相应内存
    mr_socket_free();
    return 0;
}
