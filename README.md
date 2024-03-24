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

## Team Collaboration: For detailed documentation related to branch management, please see [this link](https://spark-store-project.gitee.io/spark-wiki/#/Dev/Spark-Store-Git-Repo).

We warmly welcome you to join our development team. Whether you want to participate in development or submit applications, you can find your place here to jointly promote the development of the Linux application ecosystem.

You can track our Issue handling status in real-time through the following link: [Gitee Issue Board](https://gitee.com/spark-store-project/spark-store/board).

If you have a software package you'd like to submit, please [click here to submit](https://upload.deepinos.org/index).

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

## Read the copyright

Spark Opensource LICENSE

Copyright (C) 2023 The Spark Community

This software is free software; you can modify and redistribute it under the terms of the GNU General Public License version 3 (GPL v3). However, to protect the rights of the original authors and the interests of the community users, please adhere to the following terms:

1. Applicability to all files in this repository: This license applies to all files in this repository (or project). Any individuals or organizations that use, modify, or redistribute this software must comply with this license.
2. Copyright notice and license files: You must not remove, hide, or modify the copyright notice and license files of the original authors included in this software. Preserving the rights information of the original authors is essential for maintaining the open-source software ecosystem.
3. Version annotation: If you modify and redistribute this software, you must mark in a prominent position that this version is not officially provided by the Spark community. This avoids misleading users into thinking that the software is an official version provided by the Spark community. This software is licensed for personal, non-profit use only, and any use of it for commercial purposes or in for-profit organizations requires the prior written permission of the Spark Community.
4. Trademark usage: You are not allowed to use terms such as "Spark App Store," "Spark Store," or the logo of Spark App Store in redistributed versions, as they may mislead users into believing that the software is provided by the official Spark community.
5. Terms of Service: Your use of the software of Spark Store will be deemed as your consent to collect version, log and other information on the premise of not violating your privacy, so as to facilitate the Spark community to provide you with better services.
6. Warehouse copyright terms: In order to better provide continuous services, Spark is only free for individual users to open the service warehouse, if you or your organization needs to provide commercial services or your organization is a for-profit organization, please contact Spark community to obtain commercial authorization.
7. Prohibited malicious behavior and mass crawling: Users or organizations are strictly prohibited to engage in any form of malicious behavior when using the software, including but not limited to malicious attacks, abuse, destruction, and mass crawling of software warehouses. The definition of malicious behavior is judged by the Spark community, and violators will be held legally responsible.
8. Distribution and redistribution rights: Spark Community reserves the right to distribute the software packages it produces. Any person or organization is prohibited from using the Spark Community software package for commercial purposes or redistributing it without the express authorization of the Spark Community. This clause is intended to ensure the spirit of open source while safeguarding the intellectual property rights of the Spark community.
9. Other Terms: In addition to the above, if you use the Spark Store main program or part of its code, you are subject to all other terms and requirements of GPL v3.

You can find the full text of GPLV3 license at: https://www.gnu.org/licenses/gpl-3.0.html

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
* Please visit the [Spark App Store Dependency Download page](https://spark-app.store/download_dependencies_latest) and download the latest dependency package.
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
* Please visit the [Spark App Store Dependency Download page](https://spark-app.store/download_dependencies_latest) and download the latest dependency package.
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
- Our [Forum](https://www.deepinos.org/)
- Our [QQ Group](https://blog.shenmo.tech/post/%E6%95%85%E9%9A%9C%E5%85%AC%E5%91%8A/)
- For commercial support, please leave your issue~

---