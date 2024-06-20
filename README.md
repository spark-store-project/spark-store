# Spark App Store
[![star](https://gitee.com/spark-store-project/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/stargazers) [![fork](https://gitee.com/spark-store-project/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/members)

## Introduction

Welcome to the Spark App Store! This is an app store designed for Linux users, aiming to solve the problem of dispersed and difficult-to-obtain applications in the Linux ecosystem. No matter what type of Linux distribution you are using, you may find suitable software packages here.

The number of Linux applications is relatively limited, and the availability of Wine software is also quite challenging. Excellent development and tool resources are scattered across various communities and forums, making it difficult for the entire ecosystem to improve comprehensively.

The construction of the ecosystem is not dependent on the isolated efforts of individuals but requires the participation of the entire community. Only when everyone's "sparks" gather together can they ignite a "prairie fire."

To improve this situation, we have launched this app store. We have broadly included various user-demand software packages, gathered high-quality tools, and actively adapted Wine applications, all stored in our software library for easy user access.

**Currently supported Linux distributions include:**

- **amd64 architecture:** deepin 20 / deepin 23 / Ubuntu 20.04 / Ubuntu 22.04 / UOS Home Edition 20
- **arm64 architecture:** UOS Professional Edition 1060 / Ubuntu 22.04 / deepin 23
- **loong64 architecture:** deepin 23

> Special Note: We also support all versions of Ubuntu operating systems higher than Ubuntu 22.04, such as Ubuntu 22.10, 23.04, 23.10, etc.

**Important Notice:** This software does not provide any form of warranty. If you plan to use it on UOS Professional Edition, please make sure to understand and enable Developer Mode. Make sure you have basic troubleshooting capabilities. It should be clear that we have not conducted extensive testing on the UOS operating system. Therefore, using the Spark client may lead to a series of issues like failed system updates, data loss, etc., all risks to be borne by the user.

## Team Collaboration: For detailed documentation related to branch management, please see [this link](https://wiki.spark-app.store/#/Dev/Spark-Store-Git-Repo).

We warmly welcome you to join our development team. Whether you want to participate in development or submit applications, you can find your place here to jointly promote the development of the Linux application ecosystem.

You can track our Issue handling status in real-time through the following link: [Gitee Issue Board](https://gitee.com/spark-store-project/spark-store/board).

If you have a software package you'd like to submit, please [click here to submit](https://upload.deepinos.org.cn/index).

## Contents

- [Read the copyright](#read-the-copyright)
- [Determine your system architecture](#determine-your-system-architecture)
- [System support and installation guide](#system-support-and-installation-guide)
  - [For Deepin / UOS Users](#for-deepin-users)
  - [For Ubuntu Users](#for-ubuntu-users)
  - [For Debian Users](#for-debian-users)
- [Frequently Asked Questions (FAQ)](#frequently-asked-questions-faq)
- [Contact and Feedback](#contact-and-feedback)

---

## Read [the copyright](LICENSE)



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

#### For Deepin Users

1. **Download and Install**

   You can directly search for Spark App Store in the Deepin Store to install, or copy this link to open and install in your browser:

   > appstore://deepin-home-appstore-client?app_detail_info/spark-store

   To use the latest version, please visit the [Spark App Store Release page](https://gitee.com/spark-store-project/spark-store/releases) and download the latest version suitable for Deepin. Install to use.

   Assuming you've downloaded to the `Downloads` folder in your user directory, we recommend you use the APT tool to install:

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### For Ubuntu Users

#### For Ubuntu 20.04 Users

1. **Download Dependencies**
* Please visit the [Spark App Store Dependency Download page](https://gitee.com/spark-store-project/spark-store-dependencies/releases) and download the latest dependency package.
* Unzip the dependency package multiple times until you see multiple installation packages ending with `.deb`.
* Follow the instructions in the dependency package to install all the dependencies at once.

2. **Download and Install**

   Visit the [Spark App Store Release page](https://gitee.com/spark-store-project/spark-store/releases), download the installation package that matches your computer's architecture, and install it.

   Assuming you've downloaded to the `Downloads` folder in your user directory, we recommend you use the APT tool to install:

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

#### For Ubuntu 22.04 and Higher Version Users

1. **No Need to Install Dependencies**

2. **Download and Install**

   Visit the [Spark App Store Release page](https://gitee.com/spark-store-project/spark-store/releases), download the installation package that matches your computer's architecture, and install it.

   Assuming you've downloaded to the `Downloads` folder in your user directory, we recommend you use the APT tool to install:

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

### For Debian Users

#### For Debian 11 Users

1. **Download Dependencies**
* Please visit the [Spark App Store Dependency Download page](https://gitee.com/spark-store-project/spark-store-dependencies/releases) and download the latest dependency package.
* Unzip the dependency package multiple times until you see multiple installation packages ending with `.deb`.
* Follow the instructions in the dependency package to install all the dependencies at once.

2. **Download and Install**

   Please visit the [Spark App Store Release page](https://gitee.com/spark-store-project/spark-store/releases) and download. Install to use.

#### For Debian 12+ Users

1. **No Need to Install Dependencies**

2. **Download and Install**

   Please visit the [Spark App Store Release page](https://gitee.com/spark-store-project/spark-store/releases), download the installation package that matches your computer's architecture, and install it.

   Assuming you've downloaded to the `Downloads` folder in your user directory, we recommend you use the APT tool to install:

    ```shell
    cd ~/Downloads
    sudo apt install ./spark-store*.deb
    ```

---

## Frequently Asked Questions (FAQ)

Please refer to the [Spark App Store FAQ and Support Guide](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.md).

You can also check the [Chinese version](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.zh.md) here.

---

## Contact and Feedback

- If you have any questions or suggestions, please submit them via email or on our [Gitee page](https://gitee.com/spark-store-project/spark-store/issues).
- If you want to follow our development progress, you can go to the [Spark App Store Board](https://gitee.com/spark-store-project/spark-store/board) for more information.
- Our [Forum](https://bbs.spark-app.store/)
- Our [QQ Group](https://blog.shenmo.tech/post/%E6%95%85%E9%9A%9C%E5%85%AC%E5%91%8A/)
- For commercial support, please leave your issue~

---