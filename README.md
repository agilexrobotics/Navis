# agilex_demo

#### 介绍

用于测试agilex api接口的python /c++ demo程序,
代码中只提供部分接口的使用方式，具体接口需要具体分析实现。
** 注意**，本项目中的程序 主要在Ubuntu18.04系统下开发,**不支持windows系统**。

#### user_api.html
 api文件，使用浏览器打开即可，包含所有可用的http接口、websocket接口

#### 接口使用通用流程
1. 创建两个客户端实例,http—client, websocket-client，后续简称(ws)
2. ws-client 长连接，需要定时1秒发送心跳之类
3. ws-client 发送订阅话题的json数据，会一直收到改话题对应的数据，除非通信端口，或者底层ros节点存在问题。
4. http—client主要用于获取图片以及图片的信息，用于转换坐标点，转换成地图真实位置后才可以使用。
5. 录包/建图/导航，这个过程根据实际使用来调用，没有地图需要先建图。
6. 提供实时任务的数据，用于导航。
7. 设置列表任务，用于导航

#### 安装教程
 - python demo 依赖
   1.  python3.6 以上（如何安装，请自行查资料）
   2. sudo apt-get install python3-pip
   2.  websocket(pip3 install websockets)
   3.  request (pip3 install request)

 - 第三方c++ 库通过源码安装，请自行查资料
  - sudo apt-get install build-essential
  - websocketpp 安装
    - 解压third_libs/websocketpp-master.zip(unzip websocketpp-master.zip)
    - ```
      cd websocketpp/
      mkdir build/
      cmake ..
      sudo  make  install
      ``` 

 - 第三方c++ 库二进制安装
   1.  websocketpp（c++）(sudo apt get install  libwebsocketpp-dev)
   2.  curl (c++) ( sudo apt-get install  libcurl4-openssl-dev)
   3.  boost (sudo apt-get install libboost-dev sudo apt-get install libboost-system-dev )
   4.  include/json.hpp也是第三方的库，不过只需要引入头文件即可，具体的使用案例可以参考
   [json.hpp 使用教程](https://blog.csdn.net/alwaysrun/article/details/122789088?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-1-122789088-blog-128267701.235^v32^pc_relevant_default_base3&spm=1001.2101.3001.4242.2&utm_relevant_index=4)

#### 使用说明
 - python
    `python.exe .\md_to_html.py`
 - c++
    ```
        mkdir build
        cd build 
        cmake ..
        make 
        ./ws_and_http_demo
    ```


