#  星火应用商店
[![star](https://gitee.com/spark-store-project/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/stargazers)  [![fork](https://gitee.com/spark-store-project/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/members)
![star](https://gitcode.com/spark-store-project/spark-store/star/badge.svg)


## 简介

欢迎来到**星火应用商店**！这是一个为 Linux 平台用户设计的应用商店，旨在解决 Linux 生态下应用分散、难以获取的问题。无论您使用什么类型的 Linux 发行版，在这里都有可能找到适合您的软件包。

Linux 应用的数量相对有限，Wine 软件的可获取性也颇为困难。优秀的开发套件和工具资源散布在各大社区和论坛之间，这种分散化让整个生态系统难以得到全面的提升。

生态系统的构建并非依赖个体的孤立努力，而需要全社区共同参与。只有当大家的“星火”聚集一处，方可引发“燎原之势”。

为了改善这一现状，我们推出了星火应用商店。星火社区广泛地收录了各种用户需求的软件包，汇集了高质量的小工具，并主动对 Wine 应用进行了适配，一切都储存在我们的软件库中，使得用户可以方便地获取这些应用。

**当前支持的 Linux 发行版包括（但不限于）：**

- **amd64 架构：** deepin 20 / deepin 23 / Ubuntu 20.04 / Ubuntu 22.04 / UOS 家庭版 20 / Debian 11＋
- **arm64 架构：** UOS 专业版 1060 / Ubuntu 22.04 / deepin 23
- **loong64 架构：** deepin 23

> 特别说明：星火应用商店还支持所有版本高于 Ubuntu 22.04 的 Ubuntu 操作系统，例如 Ubuntu 22.10、23.04、23.10 等。此外星火应用商店也可能适配除上述平台的其他 Linux 发行版，您可自行进行安装测试。

**重要须知：** 本软件无法保证持续可用、无中断运行或满足特定性能要求。星火社区对其功能完整性、稳定性及无错误运行不作任何承诺。例如，若您计划在 UOS 专业版（或其他类似特定平台）上使用，请务必了解并启用“开发者模式”相关功能。请确保您具备基础的故障排查能力。需要明确的是，星火社区无法在部分特殊平台上进行广泛测试。因此，在这些平台上使用星火应用商店客户端可能会导致一系列问题，如系统更新失败、数据丢失等；使用该软件，即代表您理解并同意所有风险需由用户自行承担。



## 关于团队协作

组织仓库链接：

https://gitee.com/spark-store-project/spark-store

https://gitcode.com/spark-store-project/spark-store 

https://github.com/spark-store-project/spark-store

参见[此链接](https://wiki.spark-app.store/#/Dev/Spark-Store-Git-Repo)以获取分支管理相关的详细文档。

我们热忱欢迎您加入我们的开发团队。无论您是希望参与开发，还是计划提交应用，都能在这里找到属于您的一席之地，共同推动 Linux 应用生态的发展。

您可以通过以下链接，实时跟踪我们的 Issue 处理状态：[Issue 看板](https://gitee.com/spark-store-project/spark-store/board)。

若您有软件包需要提交，敬请[点击此处进行投稿](https://wiki.spark-app.store/#/Submit/Submit)。




## 目录

- [星火应用商店](#星火应用商店)
  - [简介](#简介)
  - [关于团队协作](#关于团队协作)
  - [目录](#目录)
  - [阅读版权声明](#阅读版权声明)
  - [确定你的系统架构](#确定你的系统架构)
  - [系统支持与安装指引](#系统支持与安装指引)
    - [对于 deepin 用户](#对于-deepin-用户)
      - [对于 deepin 用户](#对于-deepin-用户-1)
    - [对于 Ubuntu 用户](#对于-ubuntu-用户)
      - [对于 Ubuntu 20.04 用户](#对于-ubuntu-2004-用户)
      - [对于 Ubuntu 22.04 及更高版本的 Ubuntu 用户](#对于-ubuntu-2204-及更高版本的-ubuntu-用户)
    - [对于 Debian 用户](#对于-debian-用户)
      - [对于 Debian 11 用户](#对于-debian-11-用户)
      - [对于 Debian 12+ 用户](#对于-debian-12-用户)
  - [常见问题（FAQ）](#常见问题faq)
  - [联系与反馈](#联系与反馈)

---

## 阅读[版权声明](LICENSE)


## 确定你的系统架构

在安装任何软件之前，您需要知道你的计算机运行在何种架构之上（如 x86_64/amd64 或 aarch64/arm64），对于星火应用商店亦是如此。

**如何检查：**

1. 打开 Linux 终端。
2. 输入命令 `uname -m` 或 `arch` 并按 `Enter` 键执行。

您会看到终端输出结果，借此您可判断计算机的系统架构。举例：

- 如果您看到 `x86_64`，说明您的系统为 amd64 架构。
- 如果您看到 `aarch64`，说明您的系统为 arm64 架构。

根据这一结果，您需要下载对应版本的星火应用商店安装包以正常使用。

---

## 系统支持与安装指引

根据您的 Linux 发行版和系统架构，以下是安装星火应用商店的详细步骤。

### 对于 deepin 用户

#### 对于 deepin 用户

1.  **下载并安装**

    您可直接在深度应用商店搜索 `星火应用商店` 安装，或复制此链接到浏览器地址栏，跳转至深度应用商店安装。 

    > appstore://deepin-home-appstore-client?app_detail_info/spark-store


    深度应用商店发布的版本可能不是最新的。若要使用最新版本，请访问星火应用商店 [Gitee 的 Release 页面](https://gitee.com/spark-store-project/spark-store/releases)或 [Gitcode 的 Release 页面](https://gitcode.com/spark-store-project/spark-store/releases)并下载适用于 deepin 的最新版本，安装后即可使用。

    假设您将安装包下载至用户目录下的 Downloads 文件夹，我们推荐您使用 `apt` 工具进行安装以避免潜在的依赖问题：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### 对于 Ubuntu 用户

#### 对于 Ubuntu 20.04 用户

1. **下载依赖包**
* 请访问[星火应用商店依赖包下载页面](https://gitee.com/spark-store-project/spark-store-dependencies/releases)，下载最新的依赖包；
* 请多次解压依赖包，直至您可以看到诸多以 deb 结尾的安装包；
* 依据依赖包内的说明，一次性安装所有的依赖包。


2. **下载并安装**

    请访问星火应用商店 [Gitee 的 Release 页面](https://gitee.com/spark-store-project/spark-store/releases)或 [Gitcode 的 Release 页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您将安装包下载至用户目录下的 Downloads 文件夹，我们推荐您使用 `apt` 工具进行安装以避免潜在的依赖问题：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

#### 对于 Ubuntu 22.04 及更高版本的 Ubuntu 用户
1. **无需安装依赖包**


2. **下载并安装**

    请访问星火应用商店 [Gitee 的 Release 页面](https://gitee.com/spark-store-project/spark-store/releases)或 [Gitcode 的 Release 页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您将安装包下载至用户目录下的 Downloads 文件夹，我们推荐您使用 `apt` 工具进行安装以避免潜在的依赖问题：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### 对于 Debian 用户

#### 对于 Debian 11 用户

1. **下载依赖包**
* 请访问[星火应用商店依赖包下载页面](https://gitee.com/spark-store-project/spark-store-dependencies/releases)， 下载最新的依赖包；
* 请多次解压依赖包，直到您可以看到诸多以 deb 结尾的安装包；
* 依据依赖包内的说明，一次性安装所有的依赖包。

2. **下载并安装**

    请访问星火应用商店 [Gitee 的 Release 页面](https://gitee.com/spark-store-project/spark-store/releases)或 [Gitcode 的 Release 页面](https://gitcode.com/spark-store-project/spark-store/releases)并下载，安装后即可使用。


#### 对于 Debian 12+ 用户

1. **无需安装依赖包**


2. **下载并安装**

    请访问星火应用商店 [Gitee 的 Release 页面](https://gitee.com/spark-store-project/spark-store/releases)或 [Gitcode 的 Release 页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您将安装包下载至用户目录下的 Downloads 文件夹，我们推荐您使用 `apt` 工具进行安装以避免潜在的依赖问题：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```
---

## 常见问题（FAQ）

请参见[星火应用商店 FAQ 与支持指南](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.zh.md)；

对于高级用户，如需自定义配置 aptss config，请参阅 [aptss 软件仓库](https://gitee.com/GXDE-OS/aptss)。

对于星火更新器，请参阅


---

## 联系与反馈

- 如果您有任何问题或建议，请通过邮件或在 [Issue 页面](https://gitee.com/spark-store-project/spark-store/issues)上提交问题；
- 如果你想关注我们的开发进度，可以跳转至[星火应用商店 Board](https://gitee.com/spark-store-project/spark-store/board) 获取更多信息；
- 欢迎访问[星火社区论坛](https://bbs.spark-app.store/)加入讨论；
- 我们的 QQ 交流群号：872690351 和 865927727；
- 若您和您的组织需要寻求商业支持，请留言咨询。

---
