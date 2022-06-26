#  星火应用商店
[![star](https://gitee.com/deepin-community-store/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/stargazers)  [![fork](https://gitee.com/deepin-community-store/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/members)

众所周知，国内的Linux应用比较少，wine应用难以获取，优质工具分散在民间各大论坛，无法形成合力，难以改善生态

生态构建需要的不是某一方的单打独斗，而是人人行动起来，汇聚星火，产生燎原之势

我们创建了这个应用商店，广泛收录大家需要的软件包，搜集优质小工具，主动适配wine应用，存放到储存库供大家获取
我们支持：Deepin 20 ; Ubuntu 22.04 LTS / Ubuntu 20.04 LTS(将会逐渐停止支持） ; UOS Home 21

希望看到这里的人也可以加入我们的队伍，开发或者投递应用都很欢迎，共同构建Linux应用生态

如果有想要提交的软件包，请 [在这里投稿](https://upload.deepinos.org/index)


## 🙌 简单的开始

如果想安装 `星火应用商店` ，请打开右侧的 [Release] 页面，找到最新版本，并选择适用于当前系统的安装包下载。

如果你在使用 `Debian 11/Ubuntu 20.04`，你需要额外下载[依赖补充包](https://d.store.deepinos.org.cn/spark-store-dependencies-kylin.zip)

---
#### 编译安装


Deepin V20/UOS 21 系统下, 安装依赖

```shell
sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev

```

Ubuntu 22.04 系统下, 安装依赖
```shell
sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev

```

然后
```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
dpkg-buildpackage
```



## 🚀 协作

非常感谢有兴趣的开发者或爱好者参与 `星火应用商店` 项目，分享你的见解与思路。