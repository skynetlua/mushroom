# mushroom

## 介绍
5G、大数据浪潮来袭。对设备与设备的通信提出了更高要求。为此开发了这个高性能高并发的网络框架。

mushroom是一个高性能的跨全平台的socket网络框架。

mushroom支持TCP、UDP通信，让客户端非常容易创建并发网络链接，可以让手机快速创建上万条链接，甚至可以让手机作为服务器，被上万客户端连接。
mushroom还有另外一个分支mushroom-kcp，mushroom-kcp在UDP的基础上，加上kcp协议，让拥堵的网络环境消息交互变得更及时。同时，它也支持p2p连接，让成千上万设备互相通信。

mushroom之所以如此强大，是因为它的网络模块来自服务端框架skynet，纯C语言开发，使用epoll，kqueue, select等事件复用技术。
mushroom专门为跨平台设计，所有平台只有一个api，cmake构建工程，只有几个源代码文件，非常容易在vs和xcode下调试，支持linux,macos,win,android,ios等操作系统。在国内有众多依托skynet框架的游戏产品在运行，而mushroom的网络模块来自skynet，因此，它的稳定性已经得到很好保障。

## 软件架构
mushroom的网络模块来自skynet，采用事件复用技术，linux和安卓采用epoll，macox和iOS采用kqueue，win采用select。使用C语言开发，基本支持绝大多数操作系统。

mushroom内部只有一条线程运行，该线程堵塞监听所有的io事件，并把各种io事件进行划分成各类消息。mushroom线程接收到io事件后，会立刻把该消息添加到消息队列，消息队列缓存此消息，等待业务线程派发。消息队列通过原子锁保证数据安全和效率。逻辑业务线程通过轮询方式频繁调用update函数，把各种消息通过回调函数派发给业务线程。

mushroom内部有如下消息："start"，"close"，"open"，"accept"，"error"，"exit"，"udp"，"warning"等。业务层无需处理这些事件，mushroom内部会把消息转换成对应的回调函数。

## mushroom流水线
mushroom内部只有一条线程，简称事件线程；调用mushroom的api的线程，就叫业务线程。业务线程请求socket业务通过管道通知事件线程。事件线程从管道接收该socket业务并进行立刻处理，处理完后把消息发送到消息队列进行缓存，等待业务线程获取。业务线程通过轮询的方式获取消息，并通过消息调用对于的回调函数，回到业务线程。

1. 创建socket。业务线程通过mushroom提供的api，直接创建socket，然后通过管道把该socket的fd发送到mushroom的线程。在mushroom线程监听到该管道io事件，通过管道接收该fd。最后发送open消息，告诉业务层（业务层监听回调函数）socket创建成功
2. 启动socket。socket创建后，因为没有加入到io事件中，不会工作，需要进行start操作，把该socket加入到io系统事件中。业务线程执行mushroom提供的start函数，业务线程通过管道发该请求发到mushroom线程，mushroom线程接收到消息后，把该socket的fd加入到系统的IO事件中，并向业务层发送start消息。
3. 发送数据。业务线程直接通过mushroom提供的接口，直接调用write函数发送数据，如果socket的发送缓冲已满，就会把剩余的数据通过管道发到mushroom线程，mushroom线程监听到发送缓冲有空位，就会把剩余的数据发送完毕。
4. 接收数据。mushroom线程监听到read事件，就会立刻进行读取数据操作，然后发送data消息，并附上网络数据，告诉业务层。

## 项目源代码结构
mushroom源代码如下：
```c
examples        //测试例子。tcp和upd的测试例子
mrsocket        //mushroom的全部源代码。
mrtool          //辅助工具，方便处理网络数据包和二进制数据读取
demo_client.c   //客户端测试例子
demo_server.c   //服务端测试例子
CMakeLists.txt  //cmake文件
```
在mrsocket/socket_poll.h头文件描述了怎样选择事件复用技术。
```C
#if defined(__linux__)
	#include "socket_epoll.h"
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
	#include "socket_kqueue.h"
#else
	#include "socket_select.h"
#endif
```
如果不是linux和unix系统，就缺省选择select作为事件复用技术。
整个项目的源代码文件只有几个，可以手动把源代码添加到自己的项目中。非win系统，除了win文件夹之外，其他源文件加入即可。因为此网络框架是针对linux系统设计的，对于win系统，mushroom进行了处理，为win环境提供一套linux的接口，源代码中的win文件夹就是提供这个功能。
mushroom可以通过cmake快速生成各个平台的静态库。通过cmake构建工程，编译运行后，就会在主目录生成include和lib两个文件夹。里面提供了静态库需要的头文件和静态库文件。

## 安装教程
1. 环境准备
cmake构建工具，linux系统需要gcc编译器，window需要vs2017，macos/ios需要xcode，安卓需要ndk。 
2. 可以通过git工具下载源代码。
```
git clone https://github.com/skynetlua/mushroom.git
```
3. 创建build目录
在shell终端
```
cd xxx存放的文件夹/mushroom
mkdir build;cd ./build
```
4. 用cmake构建工程
以linux为例子，在shell终端，在上述/build下，
```
cmake ../
```
在win和macos中，cmake有可视化工具，鼠标点击即可构建。
在win系统，vs2017(其他版本未测试过)上可编译运行，通过cmake也可以导出xcode工程。
若macos，cmake可能存在找不到编译器的情况，执行下列命令即可
```
#macosx cmake
sudo xcode-select --switch /Applications/Xcode.app/
```
5. 编译项目
以linux为例，使用make命令，就完成编译。
```
make
```
编译完成后。在mushroom目录下会生成三个文件夹。bin、include和lib。直接把include和lib导入到工程就可以使用mushroom框架。bin文件夹内含一些测试程序。

## 简单教程
一个简单的实例。客户端和服务端写到一起。客户端（boy）邀请服务器（girl）吃饭。
```
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
```


