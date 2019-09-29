# mushroom

## 介绍
5G、大数据浪潮来袭。对设备与设备的通信提出了更高要求。为此开发了这个高性能高并发的网络框架。
mushroom是一个高性能的跨全平台的socket网络框架。
mushroom支持TCP、UDP通信，让客户端非常容易创建并发网络链接，可以让手机快速创建上万条链接，甚至可以让手机作为服务器，被上万客户端连接。
mushroom还有另外一个分支mushroom-kcp，mushroom-kcp在UDP的基础上，加上kcp协议，让拥堵的网络环境消息交互变得更及时。同时，它也支持p2p连接，让成千上万设备互相通信。

mushroom之所以如此强大，是因为它的网络模块来自服务端框架skynet，纯C语言开发，使用epoll，kqueue, select等事件复用技术。
mushroom专门为跨平台设计，所有平台只有一个api，cmake构建工程，只有几个源代码文件，非常容易在vs和xcode下调试，支持linux,macos,win,android,ios等操作系统。在国内有众多依托skynet框架的游戏产品在运行，而mushroom的网络模块来自skynet，因此，它的稳定性已经得到很好验证。

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

## 新手教程


