# mushroom

#### 介绍
mushroom是一个高性能的跨平台的移动网络通信解决方案。
5G、大数据浪潮来袭。对设备与设备的通信提出了更高要求。为此开发了这个高性能的网络框架。
mushroom让客户端非常容易创建并发网络链接，可以让手机快速创建上万条链接，甚至可以让手机作为服务器，被上万客户端连接。
mushroom支持TCP、UDP通信。在UDP的基础上，加上kcp协议，让拥堵的网络环境消息交互变得更及时。同时，它也支持p2p连接，让成千上万设备互相通信。
它之所以如此强大，是因为mushroom的网络模块来自服务端框架skynet，纯C语言开发，使用epoll，kqueue, select等高性能消息框架。
mushroom专门为跨平台设计，所有平台只有一个api，cmake构建工程，非常容易在vs和xcode下调试，支持linux,macos,win,android,ios等。

#### 软件架构
TCP、UDP通信来自skynet，然后在udp通信的基础上实现kcp协议，让UDP通信变得像tcp通信一样安全可靠。在kcp协议的基础上，非常容易实现p2p通信。
网络事件机制，Linux系统采用用epoll，macox和iOS采用kqueue，win采用select。


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

#### 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request


