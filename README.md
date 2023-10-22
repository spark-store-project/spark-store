# Spark App Store
[![star](https://gitee.com/deepin-community-store/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/stargazers) [![fork](https://gitee.com/deepin-community-store/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/deepin-community-store/spark-store/members)

## Introduction

Welcome to the Spark App Store! This is an app store designed for Linux users, aiming to solve the problem of dispersed and difficult-to-obtain applications in the Linux ecosystem. No matter what type of Linux distribution you are using, you may find suitable software packages here.

The number of Linux applications is relatively limited, and the availability of Wine software is also quite challenging. Excellent development and tool resources are scattered across various communities and forums, making it difficult for the entire ecosystem to improve comprehensively.

The construction of the ecosystem is not dependent on the isolated efforts of individuals but requires the participation of the entire community. Only when everyone's "sparks" gather together can they ignite a "prairie fire."

To improve this situation, we have launched this app store. We have broadly included various user-demand software packages, gathered high-quality tools, and actively adapted Wine applications, all stored in our software library for easy user access.

**Currently supported Linux distributions include:**

- **amd64 architecture:** deepin 20 / deepin 23 / Ubuntu 20.04 / Ubuntu 22.04 / UOS Home Edition 20
- **arm64 architecture:** UOS Professional Edition 1060 / Ubuntu 22.04 / deepin 23

> Special Note: We also support all versions of Ubuntu operating systems higher than Ubuntu 22.04, such as Ubuntu 22.10, 23.04, 23.10, etc.

**Important Notice:** This software does not provide any form of warranty. If you plan to use it on UOS Professional Edition, please make sure to understand and enable Developer Mode. Make sure you have basic troubleshooting capabilities. It should be clear that we have not conducted extensive testing on the UOS operating system. Therefore, using the Spark client may lead to a series of issues like failed system updates, data loss, etc., all risks to be borne by the user.

## Team Collaboration: For detailed documentation related to branch management, please see [this link](https://deepin-community-store.gitee.io/spark-wiki/#/Dev/Spark-Store-Git-Repo).

We warmly welcome you to join our development team. Whether you want to participate in development or submit applications, you can find your place here to jointly promote the development of the Linux application ecosystem.

You can track our Issue handling status in real-time through the following link: [Gitee Issue Board](https://gitee.com/deepin-community-store/spark-store/board).

If you have a software package you'd like to submit, please [click here to submit](https://upload.deepinos.org/index).

## Contents

- [Determine your system architecture](#determine-your-system-architecture)
- [System support and installation guide](#system-support-and-installation-guide)
  - [For Deepin / UOS Users](#for-deepin-users)
  - [For Ubuntu Users](#for-ubuntu-users)
  - [For Debian Users](#for-debian-users)
- [Frequently Asked Questions (FAQ)](#frequently-asked-questions-faq)
- [Contact and Feedback](#contact-and-feedback)

---

## Determine your system architecture

Before installing any software, you need to know what architecture your computer runs on (such as x86_64/amd64 or aarch64/arm64).

**How to check:**

1. Open a Linux terminal.
2. Type `uname -m` or `arch` and press Enter.

You will see an output, and that's your system architecture.

- If you see `x86_64`, your system is AMD64 architecture.
- If you see `aarch64`, your system is ARM64 architecture.

---

## System support and installation guide

### For Deepin Users

#### For Deepin V20 Users

1. **Install Dependencies**
  
  Open terminal and execute:

  ```shell
  sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools dde-qt5wayland-plugin libqt5svg5*
  ```

2. **Download and Install**

  Please visit [Spark App Store's Release page](https://gitee.com/deepin-community-store/spark-store/releases) and download the latest version suitable for Deepin. Install it to use.

  Assuming you download it to the user directory under the Downloads folder, we recommend using APT tools for installation:

  ```shell
  cd ~/Downloads
  sudo apt install ./spark-store*.deb
  ```

### For Ubuntu Users

#### For Ubuntu 20.04 Users

1. **Download Dependencies**
* Visit [Spark App Store's dependency download page](https://spark-app.store/download_dependencies_latest), download the latest dependencies.
* Unzip the dependencies multiple times until you can see many installation packages ending with .deb.
* Install all dependencies at once, following the instructions in the dependency package.

2. **Download and Install**

  Visit [Spark App Store's Release page](https://gitee.com/deepin-community-store/spark-store/releases), download and install.

  Assuming you download it to the user directory under the Downloads folder, we recommend using APT tools for installation:

  ```shell
  cd ~/Downloads
  sudo apt install ./spark-store*.deb
  ```

#### For Ubuntu 22.04 Users

1. **Install Dependencies**

  Open terminal and execute:

  ```shell
  sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev qt

webengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools
  ```

2. **Download and Install**

  Visit [Spark App Store's Release page](https://gitee.com/deepin-community-store/spark-store/releases), download and install.

  Assuming you download it to the user directory under the Downloads folder, we recommend using APT tools for installation:

  ```shell
  cd ~/Downloads
  sudo apt install ./spark-store*.deb
  ```

Certainly, here's the English translation of the additional text:

---

### For Debian 11 Users

1. **Download Dependencies**
* Please visit the [Spark App Store's dependency download page](https://spark-app.store/download_dependencies_latest) to download the latest dependencies.
* Unzip the dependencies multiple times until you see many installation packages ending with .deb.
* Install all dependencies at once, following the instructions in the dependency package.

2. **Download and Install**

  Please visit the [Spark App Store's Release page](https://gitee.com/deepin-community-store/spark-store/releases) and download. Install it to use.

---

## Frequently Asked Questions (FAQ)

Please refer to the [Spark App Store FAQ and Support Guide](https://gitee.com/deepin-community-store/spark-store/blob/dev/FAQ.md).

You can also check the [Chinese version](https://gitee.com/deepin-community-store/spark-store/blob/dev/FAQ.zh.md) here.

---

## Contact and Feedback

- If you have any questions or suggestions, please submit them via email or on our [Gitee page](https://gitee.com/deepin-community-store/spark-store/issues).
- If you want to follow our development progress, you can go to the [Spark App Store Board](https://gitee.com/deepin-community-store/spark-store/board) for more information.
- Our [Forum](https://www.deepinos.org/)
- Our [QQ Group](https://blog.shenmo.tech/post/%E6%95%85%E9%9A%9C%E5%85%AC%E5%91%8A/)

---
