#  Spark App Store
[![star](https://gitee.com/deepin-community-store/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/stargazers)  [![fork](https://gitee.com/deepin-community-store/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/members)



Spark Store aims to collect Linux apps for the convieniece of Linux new comers

The collecting process needs everyone's help

We set up this APP Store and collect APPs/tools that everyone need widely. Also we pack Windows apps with wine. 

All packages will be shared in our repository for users to get freely. 

Distrobution supported：

* amd64: deepin 20 / deepin 23 / Ubuntu 22.04 LTS / UniontechOS Home 21
* arm64: UniontechOS Professional 1060 / Ubuntu 22.04 LTS / deepin 23


You can track our Issue resoving progress here  https://gitee.com/deepin-community-store/spark-store/board


We hope people who see here can also join our team，development help or submit applications are welcomed

If you want to submit an APP to share with others，Please [Click here](https://upload.deepinos.org/index)


## 🙌 A simple start

If you simply want to install the Spark Store,just enter the  [Release] page, find the version you want and install.

If you are using Debian11/Ubuntu 20.04, you will need extra dependency package. Available [here](https://zunyun01.store.deepinos.org.cn/spark-store-dependencies-kylin.zip)

---
#### Compile and developement

 **dev branch is for development, Flamescion branch is for amd64 and Reason branch is for arrch64.** Although, for the most of the time, Flamescion branch is same as Reason.  

For Deepin V20/UOS 21/ Debian 11

```shell
sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools dde-qt5wayland-plugin libqt5svg5*

```

Ubuntu 22.04
```shell
sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev  qtwayland5 qtwayland5-dev-tools libqt5svg5*

```

Then

```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
dpkg-buildpackage -j
```

Or:
```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
./build_and_install.sh
```
## EULA

This is our [EULA](https://genshin-impact.fandom.com/wiki/Eula)


## 🚀 Coorperation

We use Gitee as our code hosting platform. Please click here to contact us. 

https://gitee.com/deepin-community-store/spark-store

### Forum

https://www.deepinos.org/

#### QQ Group

https://blog.shenmo.tech/post/%E6%95%85%E9%9A%9C%E5%85%AC%E5%91%8A/


## FAQ
### Spark Application Store FAQ and Support Guide

#### Introduction
The Spark Application Store is an application store for Linux users, supporting various Linux distributions. Here are some common questions and solutions. Please note that this document is primarily aimed at ordinary users who are not familiar with Linux and the APT software package management system.

---

#### Communication and Feedback
**Q: Where can I join the communication group?**  
**A:** You can click [here](https://www.deepinos.org/) to access the main communication platform for the Spark Store.  
> ⚠️ If the above link is not accessible, you can also join our QQ groups as an alternative, with group numbers 872690351 and 865927727. We also provide a forum as a communication platform.

#### Differences Between arm64 and amd64
**arm64**
- Full name: Advanced RISC Machine 64-bit
- Main usage: Primarily used in mobile devices, embedded systems, and some new servers and desktop computers.
- Advantages: Low power consumption, high efficiency, suitable for battery-driven and thermally sensitive devices.
- Main manufacturers: Apple, Qualcomm, Hisilicon, Feiteng, and more.

**amd64**
- Full name: AMD 64-bit, also known as x86_64
- Main usage: Mainly used in desktop computers, laptops, and servers.
- Advantages: High performance, suitable for compute-intensive applications.
- Main manufacturers: Intel and AMD.

The most significant differences between them lie in instruction sets and application scenarios. amd64 is typically used in computers and servers with high-performance requirements, while arm64 is more suitable for scenarios with stricter power consumption requirements.

To check your Linux system's processor architecture, you can follow these steps:
1. Open a terminal.
2. Enter `uname -m` or `arch` and press Enter.
3. You will see your processor architecture (possibly x86_64, aarch64, etc.).

#### Domestic Architecture Support
**Q: I have a domestic architecture. How can I get applications?**  
**A:** Currently, the Spark Store supports domestic chips with the **arm** architecture. You can download deb packages (software installation packages) for the arm64 architecture.  
> ⚠️ Please note that this is experimental support. If you encounter issues, please provide feedback on the Spark communication platform, QQ groups, or the forum.

#### Dependency Installation Issues
**Q: I encountered an error while installing dependencies**  
**A:** If you are using UOS or Deepin, please do not install dependency packages. For Kali Linux/Kdeneon, etc., which are temporarily not supported, you need to compile and install them yourself.  
> 💡 When an error occurs, try running `sudo apt update` before attempting to install. If the problem persists, refer to the first question and seek help on the communication platform.

#### Submissions and Application Updates
**Q: Where can I submit applications?**  
**A:** You can find the "Submit Application" option in the top-right menu of the app store interface.

#### Support for Non-deepin/UOS Users
**Q: I am not a deepin/UOS user. Can I use the Spark Application Store?**  
**A:** Yes, you can. If you are using Ubuntu 22.04, you can install it directly. For Ubuntu 20.04/Debian10/Debian11, please install the dependency packages first.

#### Installation Methods
**Q: Can I install with dpkg -i?**  
**A:** No! Use `sudo apt install ./xxxx.deb` to install, or use graphical installers like gdebi.  
> ⚠️ Using dpkg directly will not handle software dependencies, and this is a common mistake.

#### System Updates
**Q: Will the Spark Store affect normal system updates?**  
**A:** No, the Spark Store separates application sources from system sources and will not affect regular system updates.

#### Reporting Issues and Application Removal
**Q: Some applications are outdated or no longer working, and I want them removed**  
**A:** You can report issues [here](https://gitee.com/deepin-community-store/software_-issue).

#### Installing the Spark Application Store
If you want to install the Spark Application Store, please open the [Release page](https://gitee.com/deepin-community-store/spark-store/releases), find the latest version, and choose the installation package suitable for your current system.

> ⚠️ Special Note: If you are using Debian 10/Debian 11 or Ubuntu 20.04, you may need to download additional dependency supplement packages. After downloading the dependency packages, unzip multiple times until you see the software usage instructions inside. For operating system versions after Ubuntu 22.04, you do not need to install dependency packages. Follow the above instructions to directly install the main program.