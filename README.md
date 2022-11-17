#  Spark App Store
[![star](https://gitee.com/deepin-community-store/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/stargazers)  [![fork](https://gitee.com/deepin-community-store/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/members)

Spark Store aims to collect Linux apps for the convieniece of Linux new comers

The collecting process needs everyone's help

We set up this APP Store and collect APPs/tools that everyone need widely. Also we pack Windows apps with wine. 

All packages will be shared in our repository for users to get freely. 

Distrobution supported：Deepin 20 ; Ubuntu 22.04 LTS / Ubuntu 20.04 LTS(May stop support in the future） ; UniontechOS Home 21

*About OpenKylin and deepin 23*

The adaptation work is scheduled after their official release.

You can track our Issue resoving progress here  https://gitee.com/deepin-community-store/spark-store/board


We hope people who see here can also join our team，development help or submit applications are welcomed

If you want to submit an APP to share with others，Please [Click here](https://upload.deepinos.org/index)


## 🙌 A simple start

If you simply want to install the Spark Store,just enter the  [Release] page, find the version you want and install.

If you are using Debian11/Ubuntu 20.04, you will need extra dependency package. Available [here](https://code.gitlink.org.cn/shenmo7192/spark-store-dependencies/raw/branch/master/spark-store-dependencies-kylin.zip)

---
#### Compile and developement


For Deepin V20/UOS 21/ Debian 11

```shell
sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev

```

Ubuntu 22.04
```shell
sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev

```

Then

```shell
git clone https://gitee.com/deepin-community-store/spark-store.git
cd spark-store
dpkg-buildpackage
```



## 🚀 Coorperation

We use Gitee as our code hosting platform. Please click here to contact us. 

https://gitee.com/deepin-community-store/spark-store

### Rocket Chat

https://chat.shenmo.tech/

PWA Client：

spk://store/chat/store.spark-app.feedback

（Copy and paste to search bar or in browser address bar after installing Spark Store）