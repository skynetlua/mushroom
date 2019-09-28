
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mrsocket.h"
#include "mrtool.h"

//win只支持select，故不能同时并发超过60个链接。
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
//测试同时请求60个链接
#define TEST_SESSION_NUM 60
#else
//测试并发请求1000个链接
#define TEST_SESSION_NUM 1000
#endif

static void client_handle_data(uintptr_t uid, int fd, char* data, int size)
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

static void client_handle_connect(uintptr_t uid, int fd, char* data, int size)
{
    printf("连接服务器成功 uid = %d, fd = %d, data =%s, size = %d \n", (int)uid, fd, data, size);
    char msg[128] = "\1靓女，你吃饭了吗？";
    int ret = mr_socket_send(fd, msg, sizeof(msg));
    if (ret < 0)
    {
        printf("mr_socket_send faild ret = %d\n", ret);
    }
}

static void client_handle_close(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("连接关闭 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
}

static void client_handle_error(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生错误 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
}

static void client_handle_warning(uintptr_t uid, int fd, char* data, int size)
{
    int session_id = (int)uid;
    printf("发生警告 session_id = %d, fd = %d, data=%s, size = %d \n", session_id, fd, data, size);
}

int main(int argc, char* argv[])
{
    //初始化mushroom（只能初始化一次）
    mr_socket_init();

    //设置回调函数
    //配置socket数据包回调函数
    mr_set_handle_data(client_handle_data);
    //配置socket链接回调函数
    mr_set_handle_connect(client_handle_connect);
    //配置socket关闭回调函数
    mr_set_handle_close(client_handle_close);
    //配置socket发生错误回调函数
    mr_set_handle_error(client_handle_error);
    //配置socket发生警告回调函数
    mr_set_handle_warning(client_handle_warning);

    //启动mushroom，并启动线程监听和接收数据包
    mr_socket_run();
    printf("mushroom启动成功\n");

    int session_id = 0;
    //同一时刻建立TEST_SESSION_NUM个连接（Linux默认限制1024个io，更多连接需要开启）
    for (; session_id < TEST_SESSION_NUM; ++session_id)
    {
        int fd = mr_socket_connect((uintptr_t)session_id, "127.0.0.1", 8080);
        if (fd < 0)
        {
            printf("连接失败 faild fd = %d\n", fd);
            assert(0);
        }
        printf("执行连接服务器 session_id=%d, fd =%d \n", session_id, fd);
    }

    while(1)
    {
        //业务线程循环检测是否有网络消息，有网络消息，就会通过回调函数返回
        //与回调函数同一线程
        mr_socket_update();
        mr_sleep(1);
    }

    //销毁线程，并释放相应内存
    mr_socket_free();
    return 0;
}
