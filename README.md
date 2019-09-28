# mushroom

#### 介绍
5G、大数据浪潮来袭。对设备与设备的通信提出了更高要求。为此开发了这个高性能高并发的网络框架。
mushroom是一个高性能的跨全平台的socket网络框架。
mushroom支持TCP、UDP通信，让客户端非常容易创建并发网络链接，可以让手机快速创建上万条链接，甚至可以让手机作为服务器，被上万客户端连接。
mushroom还有另外一个分支mushroom-kcp，mushroom-kcp在UDP的基础上，加上kcp协议，让拥堵的网络环境消息交互变得更及时。同时，它也支持p2p连接，让成千上万设备互相通信。

mushroom之所以如此强大，是因为它的网络模块来自服务端框架skynet，纯C语言开发，使用epoll，kqueue, select等事件复用技术。
mushroom专门为跨平台设计，所有平台只有一个api，cmake构建工程，非常容易在vs和xcode下调试，支持linux,macos,win,android,ios等操作系统。

#### 软件架构
网络模块来自skynet，采用事件复用技术，Linux和安卓采用epoll，macox和iOS采用kqueue，win采用select。使用C语言开发，基本支持绝大多数操作系统。

mushroom内部只有一条线程运行，该线程堵塞监听所有的io事件，并把各种io事件进行划分成各类消息。它接收到io事件后，会立刻把事件消息添加到消息队列，消息队列通过原子锁保证数据安全。逻辑业务线程通过轮询方式频繁调用update函数，把各种消息通过回调函数派发给业务线程。
mushroom内部有如下消息："start","close","open","accept","error","exit","udp","warning"等。业务层无需处理这些事件，已转换到各个回调函数。

#### mushroom流水线
mushroom内部只有一条线程。业务线程通过管道单方向通知mushroom的内部线程。mushroom内部线程把消息发送到消息队列进行缓存。业务线程通过轮询的方式获取消息队列，并通过消息调用对于的回调函数，通知业务线程。
1. 创建socket。业务线程通过mushroom提供的api，直接创建socket，然后通过管道把该socket的fd发送到mushroom的线程。在mushroom线程监听到该管道io事件，通过管道获取该fd。最后发送open消息，告诉业务层（业务层监听回调函数）socket创建成功
2. 启动socket。socket创建后，因为没有加入到io事件中，不会工作，需要进行start操作，把该socket加入到io系统事件中。业务线程执行mushroom提供的start函数，业务线程通过管道发该请求发到mushroom线程，mushroom线程接收到消息后，把该socket的fd加入到系统的IO事件中，并向业务层发送start消息。
3. 发送数据。业务线程直接通过mushroom提供的接口，直接调用write函数发送数据，如果socket的发送缓冲已满，就会把剩余的数据通过管道发到mushroom线程，mushroom线程监听到发送缓冲有空位，就会把剩余的数据发送完毕。
4. 接收数据。mushroom线程监听到read事件，就会立刻进行读取数据操作，然后发送data消息，并附上网络数据，告诉业务层。

#### 项目源代码结构



#### 安装教程

1. 环境准备
cmake构建工具，linux系统需要gcc编译器，window需要vs2017，macos/ios需要xcode，安卓需要ndk。 
2. 可以通过git工具下载源代码。
```
git clone https://github.com/linyouhappy/mushroom.git
```
3. 创建build目录，在build目录下，再分别创建各个系统的build文件，linux,win或者macos

在shell终端
```
 mkdir build;cd build;mkdir linux;mkdir win;mkdir macos
```
4. 用cmake构建工程
以linux为例子，在shell终端，在上述/build/linux下，
```
cmake ../..
```
在win和macos中，cmake有可视化工具，鼠标点击即可构建。
若macos，cmake可能存在找不到编译器的情况，执行下列命令即可
```
#macosx cmake
sudo xcode-select --switch /Applications/Xcode.app/
```

5. 编译项目
以linux为例，使用make命令，就完成编译。执行tutorial这个demo程序就可以。
win在vs2017(其他版本未测试过)上编译运行，macos在xcode上编译运行。
使用方法，可参照examples的例子

#### 使用说明

1. 此项目正在开发中
2. 此项目纯粹是为了设计一个高性能高并发的无平台相关的网络通信，尽可能支持各种通信场合。
3. 此项目会被设计成一套无平台相关的解决方案。会加入各种机制和工具。



