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


## FAQ
### 星火应用商店FAQ与支持指南

#### 简介
星火应用商店是一个面向 Linux 用户的应用商店，支持多种不同的 Linux 发行版。以下是一些常见问题及解决方案。请注意，该文档主要面向对 Linux 和 APT 软件包管理系统不熟悉的普通用户。

---

#### 交流与反馈
**Q: 在哪进交流群？**  
**A:** 你可以点击[这里](https://www.deepinos.org/)进入星火商店的主交流平台。  
> ⚠️ 如果上面的链接无法使用，你还可以加入我们的 QQ 群作为备选方案，群号是 872690351 和 865927727。我们还提供论坛作为交流平台。

arm64与amd64的区别

arm64

* 全称: Advanced RISC Machine 64-bit
* 主要用途: 主要应用于移动设备、嵌入式系统以及一些新型的服务器和桌面计算机。
* 优势: 低功耗、高效率，适合用在电池驱动和热敏感的设备。
* 主要生产商: 苹果、高通、海思、飞腾等。

amd64

* 全称: AMD 64-bit, 也被称为 x86_64
* 主要用途: 主要应用于桌面计算机、笔记本以及服务器。
* 优势: 高性能，适用于计算密集型应用。
* 主要生产商: Intel和AMD。
* 两者最主要的不同在于指令集和应用场景。amd64通常用于高性能需求的计算机和服务器，而arm64则更多应用于功耗要求更严格的场合。

查看方法：

Linux：

- 打开终端。
- 输入 uname -m 或 arch 并按回车。
- 你将看到你的处理器架构（可能是 x86_64, aarch64 等）。
---

#### 国产架构支持
**Q: 我是国产架构，怎么获取应用？**  
**A:** 目前，星火商店支持**arm**架构的国产芯片。你可以下载适用于 arm64 架构的deb包（软件安装包）。  
> ⚠️ 请注意，这是实验性的支持。如果遇到问题，请在星火交流平台、QQ群或论坛向我们反馈。

---

#### 安装依赖问题
**Q: 安装依赖包出现错误**  
**A:** 如果你使用UOS或deepin，请不要安装依赖包。对于Kali Linux/Kdeneon等，暂时不支持，请自行编译安装。  
> 💡 出现错误时，尝试运行 `sudo apt update` 后再尝试安装。如果问题仍然存在，请参考第一条，进入交流平台寻求帮助。

---

#### 投稿与应用更新
**Q: 在哪里投稿？**  
**A:** 你可以在应用商店界面的右上角菜单找到“投递应用”的选项。

---

#### 非deepin/UOS用户支持
**Q: 我不是deepin/UOS用户，可以使用星火应用商店吗？**  
**A:** 可以，如果你是使用 Ubuntu 22.04，请直接安装。对于 Ubuntu 20.04/Debian10/Debian11，请先安装依赖包。

---

#### 安装方法
**Q: 我可以用dpkg -i安装吗？**  
**A:** 不可以！使用 `sudo apt install ./xxxx.deb` 来安装，或者使用gdebi等图形化的安装器。  
> ⚠️ 直接使用dpkg是不会处理软件依赖的，这是一个常见的错误。

---

#### 系统更新
**Q: 星火商店会影响系统正常更新吗？**  
**A:** 不会，星火商店已经将应用源与系统源分开，不会影响系统的正常更新。

---

#### 报告问题和应用下架
**Q: 有些应用已经过时或者失效了，我想让他下架**  
**A:** 你可以前往 [这里](https://gitee.com/deepin-community-store/software_-issue) 报告问题。

---

#### 安装星火应用商店
如果你想安装星火应用商店，请打开[Release页面](https://gitee.com/deepin-community-store/spark-store/releases)，找到最新版本，并选择适用于当前系统的安装包下载。

> ⚠️ 特别提示: 如果你在使用 Debian10/Debian11 或 Ubuntu 20.04，你可能额外下载依赖补充包。您需要在下载完依赖包后，解压多次一直到看到内部的软件使用说明，对于 Ubuntu22.04 以后的操作系统版本，您无需安装依赖包，您需要按照上面的操作直接安装本体程序。

---


