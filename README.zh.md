#  星火应用商店
[![star](https://gitee.com/spark-store-project/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/stargazers)  [![fork](https://gitee.com/spark-store-project/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/members)



## 简介

欢迎来到星火应用商店！这是一个为Linux用户设计的应用商店，旨在解决Linux生态下应用分散、难以获取的问题。无论你是使用什么类型的Linux发行版，这里都有可能找到适合你的软件包。


Linux 应用的数量相对有限，Wine 软件的可获得性也颇为困难。优秀的开发和工具资源散布在各大社区和论坛之间，这种分散化让整个生态系统难以得到全面的提升。

生态系统的构建并非依赖个体的孤立努力，而需要全社区共同参与。只有当大家的“星火”聚集一处，方可引发“燎原之势”。

为了改善这一现状，我们推出了这个应用商店。我们广泛地收录了各种用户需求的软件包，汇集了高质量的小工具，并主动对 Wine 应用进行了适配，一切都储存在我们的软件库中，使得用户可以方便地获取这些应用。

**当前支持的Linux发行版包括：**

- **amd64架构：** deepin 20 / deepin 23 / Ubuntu 20.04 / Ubuntu 22.04 / UOS家庭版20
- **arm64架构：** UOS专业版1060 / Ubuntu 22.04 / deepin 23
- **loong64架构：** deepin 23

> 特别说明：我们还支持所有版本高于Ubuntu 22.04的Ubuntu操作系统，例如Ubuntu 22.10、23.04、23.10等。

**重要须知：** 本软件不提供任何形式的保证。若您计划在UOS专业版上使用，请务必了解并启用开发者模式。请确保您具备基础的故障排查能力。需要明确的是，我们没有在UOS操作系统上进行广泛的测试。因此，使用星火客户端可能会导致一系列问题，如系统更新失败、数据丢失等，所有风险需由用户自行承担。



## 关于团队协作

组织仓库链接：
https://gitee.com/spark-store-project/spark-store

https://gitcode.com/spark-store-project/spark-store 

https://github.com/spark-store-project/spark-store

分支管理相关的详细文档可参见 [此链接](https://wiki.spark-app.store/#/Dev/Spark-Store-Git-Repo)。

我们热忱欢迎您加入我们的开发团队。无论您是想参与开发，还是想提交应用，都能在这里找到属于您的一席之地，共同推动Linux应用生态的发展。

您可以通过以下链接，实时跟踪我们的Issue处理状态：[Gitee Issue看板](https://gitee.com/spark-store-project/spark-store/board)。

若您有软件包想要提交，敬请 [点击此处进行投稿](https://wiki.spark-app.store/#/Submit/Submit)。




## 目录

- [阅读版权声明](#阅读版权声明)
- [确定你的系统架构](#确定你的系统架构)
- [系统支持与安装指引](#系统支持与安装指引)
  - [对于 Deepin / UOS 用户](#对于deepin用户)
  - [对于 Ubuntu 用户](#对于ubuntu用户)
  - [对于 Debian 用户](#对于debian用户)
- [常见问题（FAQ）](#常见问题faq)
- [联系与反馈](#联系与反馈)

---

## 阅读[版权声明](LICENSE)


## 确定你的系统架构

在安装任何软件之前，你需要知道你的计算机运行的是哪种架构（如x86_64/amd64或aarch64/arm64）。

**如何检查：**

1. 打开Linux终端。
2. 输入 `uname -m` 或 `arch` 并按回车。

你会看到一个输出，这就是你的系统架构。

- 如果看到 `x86_64`，你的系统是AMD64架构。
- 如果看到 `aarch64`，你的系统是ARM64架构。

---

## 系统支持与安装指引

根据你的Linux发行版和系统架构，以下是安装星火应用商店的详细步骤。

### 对于 Deepin 用户

#### 对于 Deepin 用户

1.  **下载并安装**

    您可直接在深度商店搜索 星火应用商店 安装，或复制此链接到浏览器打开安装 

    > appstore://deepin-home-appstore-client?app_detail_info/spark-store


    若要使用最新版本，请访问[星火应用商店的Release页面](https://gitee.com/spark-store-project/spark-store/releases)或[Gitcode的Release页面](https://gitcode.com/spark-store-project/spark-store/releases)并下载适用于Deepin的最新版本。安装后即可使用。

    假设您下载到用户目录下的 Downloads 文件夹， 我们推荐您使用 APT 工具安装：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### 对于 Ubuntu 用户

#### 对于 Ubuntu 20.04 用户

1. **下载依赖包**
* 请访问[星火应用商店依赖包下载页面](https://gitee.com/spark-store-project/spark-store-dependencies/releases)， 下载最新的依赖包。
* 请多次解压依赖包，直到你可以看到诸多以 deb 结尾的安装包。
* 依据依赖包内的说明，一次性安装所有的依赖包。


2. **下载并安装**

    请访问[星火应用商店的Release页面](https://gitee.com/spark-store-project/spark-store/releases)或[Gitcode的Release页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您下载到用户目录下的 Downloads 文件夹， 我们推荐您使用 APT 工具安装：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

#### 对于 Ubuntu 22.04 及更高版本的 Ubuntu 用户
1. **无需安装依赖包**


2. **下载并安装**

    请访问[星火应用商店的Release页面](https://gitee.com/spark-store-project/spark-store/releases)或[Gitcode的Release页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您下载到用户目录下的 Downloads 文件夹， 我们推荐您使用 APT 工具安装：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### 对于 Debian 用户

#### 对于 Debian 11 用户

1. **下载依赖包**
* 请访问[星火应用商店依赖包下载页面](https://gitee.com/spark-store-project/spark-store-dependencies/releases)， 下载最新的依赖包。
* 请多次解压依赖包，直到你可以看到诸多以 deb 结尾的安装包。
* 依据依赖包内的说明，一次性安装所有的依赖包。

2. **下载并安装**

    请访问[星火应用商店的Release页面](https://gitee.com/spark-store-project/spark-store/releases)或[Gitcode的Release页面](https://gitcode.com/spark-store-project/spark-store/releases)并下载，安装后即可使用。


#### 对于 Debian 12+ 用户

1. **无需安装依赖包**


2. **下载并安装**

    请访问[星火应用商店的Release页面](https://gitee.com/spark-store-project/spark-store/releases)或[Gitcode的Release页面](https://gitcode.com/spark-store-project/spark-store/releases)，下载和您电脑相同架构的安装包并安装。

    假设您下载到用户目录下的 Downloads 文件夹， 我们推荐您使用 APT 工具安装：

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```
---

## 常见问题（FAQ）

请参见[星火应用商店FAQ与支持指南](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.md)。

在这里可以查阅[中文版本](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.zh.md)。

---

## 联系与反馈

- 如果您有任何问题或建议，请通过邮件或在[Gitee页面](https://gitee.com/spark-store-project/spark-store/issues)上提交问题。
- 如果你想关注我们的开发进度，可以跳转[星火应用商店Board](https://gitee.com/spark-store-project/spark-store/board)获取更多信息。
- 我们的[论坛](https://bbs.spark-app.store/)
- 我们的QQ群 872690351
- 商业支持请留言咨询~

---
