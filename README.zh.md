#  星火应用商店
[![star](https://gitee.com/deepin-community-store/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/stargazers)  [![fork](https://gitee.com/deepin-community-store/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/members)



国内的Linux应用比较少，wine应用难以获取，优质工具分散在民间各大论坛，无法形成合力，难以改善生态

生态构建需要的不是某一方的单打独斗，而是人人行动起来，汇聚星火，产生燎原之势

我们创建了这个应用商店，广泛收录大家需要的软件包，搜集优质小工具，主动适配wine应用，存放到储存库供大家获取

发行版支持：

* amd64: deepin 20 / deepin 23 / Ubuntu 22.04 / UOS家庭版20
* arm64: UOS专业版1060 / Ubuntu 22.04 / deepin 23

 **请注意：本程序不包含任何保证，若你要在UOS专业版上使用，请确保你打开了开发者模式且拥有排查错误的能力，风险自负！** 

## 关于协作：分支相关的文档见 [这里](https://deepin-community-store.gitee.io/spark-wiki/#/Dev/Spark-Store-Git-Repo)

*关于OpenKylin和deepin 23*

支持计划将会在对应系统发布正式版之后开始评估和执行

希望看到这里的人也可以加入我们的队伍，开发或者投递应用都很欢迎，共同构建Linux应用生态

在这里追踪我们的Issue处理情况 https://gitee.com/deepin-community-store/spark-store/board

如果有想要提交的软件包，请 [在这里投稿](https://upload.deepinos.org/index)


## 🙌 简单的开始

如果想安装 `星火应用商店` ，请打开右侧的 [Release] 页面，找到最新版本，并选择适用于当前系统的安装包下载。

如果你在使用 `Debian 11/Ubuntu 20.04`，你需要额外下载[依赖补充包](https://zunyun01.store.deepinos.org.cn/spark-store-dependencies-kylin.zip)

---
#### 编译安装

 **dev分支用于开发，Flamescion分支是amd64,Reason分支是aarch64**。然而，绝大多数情况下，Flamescion和Reason的内容是一致的

Deepin V20/UOS 21 系统下, 安装依赖

```shell
sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools dde-qt5wayland-plugin libqt5svg5*

```

Ubuntu 22.04 系统下, 安装依赖
```shell
sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev qtwayland5 qtwayland5-dev-tools libqt5svg5*

```

然后
```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
dpkg-buildpackage -j
```

或者： 编译并安装
```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
./build_and_install.sh
```

## EULA

这里是我们的[EULA](https://mzh.moegirl.org.cn/%E4%BC%98%E8%8F%88%C2%B7%E5%8A%B3%E4%BC%A6%E6%96%AF)


## 🚀 协作

非常感谢有兴趣的开发者或爱好者参与 `星火应用商店` 项目，分享你的见解与思路。

### 论坛

https://www.deepinos.org/

#### QQ群：

https://blog.shenmo.tech/post/%E6%95%85%E9%9A%9C%E5%85%AC%E5%91%8A/
