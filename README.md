<<<<<<< HEAD
# Spark Store
[![star](https://gitee.com/spark-store-project/spark-store/badge/star.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/stargazers)  [![fork](https://gitee.com/spark-store-project/spark-store/badge/fork.svg?theme=gvp)](https://gitee.com/spark-store-project/spark-store/members)
![star](https://gitcode.com/spark-store-project/spark-store/star/badge.svg)


## Introduction  

Welcome to the **Spark Store**! This is an application store designed for Linux platform users, aiming to address the issues of fragmented and difficult-to-obtain software within the Linux ecosystem. Regardless of your Linux distribution, you may find suitable software packages here.

The number of Linux applications is relatively limited, and obtaining Wine software can also be challenging. Excellent development kits and tool resources are scattered across various communities and forums, making it difficult for the entire ecosystem to achieve comprehensive improvement.

Ecosystem construction does not rely on isolated individual efforts but requires full community participation. Only when everyone's "spark" gathers in one place can it ignite a prairie fire.

To improve this situation, we have launched the Spark Store. The Spark Community extensively collects software packages for various user needs, aggregates high-quality small tools, and proactively adapts Wine applications. All resources are stored in our repository, allowing users to conveniently access these applications.

**Currently supported Linux distributions include (but are not limited to):**

- **amd64 Architecture:** deepin 20 / deepin 23 / Ubuntu 20.04 / Ubuntu 22.04 / UOS Home Edition 20 / Debian 11+
- **arm64 Architecture:** UOS Professional Edition 1060 / Ubuntu 22.04 / deepin 23
- **loong64 Architecture:** deepin 23

> Special Note: Spark Store also supports all Ubuntu OS versions newer than Ubuntu 22.04, such as Ubuntu 22.10, 23.04, 23.10, etc. Additionally, Spark Store may adapt to other Linux distributions beyond the listed platforms; you may conduct installation tests accordingly.

**Important Notice:** This software cannot guarantee continuous availability, uninterrupted operation, or meeting specific performance requirements. The Spark community makes no commitment to the functional completeness, stability, or error-free operation of this software. For example, if you plan to use it on UOS Professional Edition (or other similar specific platforms), please ensure you understand and enable the "Developer Mode" features. Basic troubleshooting capabilities are required. It is important to note that the Spark community cannot conduct extensive testing on some special platforms. Therefore, using the Spark Store client on these platforms may lead to various issues such as system update failures or data loss. Using this software indicates your understanding and acceptance that all risks must be borne by the user.

## About Team Collaboration

Organization Repository Links:

https://gitee.com/spark-store-project/spark-store  

https://gitcode.com/spark-store-project/spark-store   

https://github.com/spark-store-project/spark-store  

Refer to [this link](https://wiki.spark-app.store/#/Dev/Spark-Store-Git-Repo) for detailed documentation on branch management.  

We warmly welcome you to join our development team. Whether you wish to participate in development or plan to submit applications, you can find your place here to jointly promote the development of the Linux application ecosystem.

You can track our Issue processing status in real-time via the [Issue Board](https://gitee.com/spark-store-project/spark-store/board).  

If you have software packages to submit, please [click here to contribute](https://wiki.spark-app.store/#/Submit/Submit).  

## Table of Contents

- [Read the Copyright Notice](#read-the-copyright-notice)
- [Determine Your System Architecture](#determine-your-system-architecture)
- [System Support and Installation Instructions](#system-support-and-installation-instructions)
  - [For deepin Users](#for-deepin-users)
  - [For Ubuntu Users](#for-ubuntu-users)
  - [For Debian Users](#for-debian-users)
- [Frequently Asked Questions (FAQ)](#frequently-asked-questions-faq)
- [Contact and Feedback](#contact-and-feedback)

---

## Read the [Copyright Notice](LICENSE)


## Determine Your System Architecture

Before installing any software, you need to know which architecture your computer is running on (e.g., x86_64/amd64 or aarch64/arm64), which also applies to the Spark Store.

**How to Check:**

1. Open the Linux terminal.
2. Enter the command `uname -m` or `arch` and press `Enter`.

You will see the terminal output result, which helps determine your system architecture. Examples:

- If you see `x86_64`, your system is amd64 architecture.
- If you see `aarch64`, your system is arm64 architecture.

Based on this result, you need to download the corresponding version of the Spark Store installer to use it properly.

---

## System Support and Installation Instructions

Based on your Linux distribution and system architecture, here are detailed steps to install Spark Store.

### For deepin Users

#### For deepin Users

1. **Download and Install**

   You can directly search for `Spark Store` in the deepin App Store for installation, or copy this link to your browser address bar to jump to the deepin App Store installation page.

   > appstore://deepin-home-appstore-client?app_detail_info/spark-store

   Versions published in the deepin App Store may not be the latest. To use the latest version, visit the Spark Store [Gitee Release Page](https://gitee.com/spark-store-project/spark-store/releases) or [Gitcode Release Page](https://gitcode.com/spark-store-project/spark-store/releases) and download the latest version for deepin, then install it.

   Assuming you downloaded the installer to the Downloads folder under your home directory, we recommend using the `apt` tool for installation to avoid potential dependency issues:

   ```shell
   cd ~/Downloads
   sudo apt install ./spark-store*.deb
   ```

### For Ubuntu Users

#### For Ubuntu 20.04 Users

1. **Download Dependencies**
   * Visit the [Spark Store Dependency Package Download Page](https://gitee.com/spark-store-project/spark-store-dependencies/releases) to download the latest dependency package;
   * Extract the dependency package multiple times until you see numerous `.deb` installation packages;
   * Follow the instructions inside the dependency package to install all dependencies at once.

2. **Download and Install**

   Visit the Spark Store [Gitee Release Page](https://gitee.com/spark-store-project/spark-store/releases) or [Gitcode Release Page](https://gitcode.com/spark-store-project/spark-store/releases) to download and install the installer matching your computer's architecture.

   Assuming you downloaded the installer to the Downloads folder under your home directory, we recommend using the `apt` tool for installation to avoid potential dependency issues:

   ```shell
   cd ~/Downloads
   sudo apt install ./spark-store*.deb
   ```

#### For Ubuntu 22.04 and Newer Ubuntu Versions
1. **No Dependency Package Required**

2. **Download and Install**

   Visit the Spark Store [Gitee Release Page](https://gitee.com/spark-store-project/spark-store/releases) or [Gitcode Release Page](https://gitcode.com/spark-store-project/spark-store/releases) to download and install the installer matching your computer's architecture.

   Assuming you downloaded the installer to the Downloads folder under your home directory, we recommend using the `apt` tool for installation to avoid potential dependency issues:

   ```shell
   cd ~/Downloads
   sudo apt install ./spark-store*.deb
   ```

### For Debian Users

#### For Debian 11 Users

1. **Download Dependencies**
   * Visit the [Spark Store Dependency Package Download Page](https://gitee.com/spark-store-project/spark-store-dependencies/releases) to download the latest dependency package;
   * Extract the dependency package multiple times until you see numerous `.deb` installation packages;
   * Follow the instructions inside the dependency package to install all dependencies at once.

2. **Download and Install**

   Visit the Spark Store [Gitee Release Page](https://gitee.com/spark-store-project/spark-store/releases) or [Gitcode Release Page](https://gitcode.com/spark-store-project/spark-store/releases) to download and install the installer.

#### For Debian 12+ Users

1. **No Dependency Package Required**

2. **Download and Install**

   Visit the Spark Store [Gitee Release Page](https://gitee.com/spark-store-project/spark-store/releases) or [Gitcode Release Page](https://gitcode.com/spark-store-project/spark-store/releases) to download and install the installer matching your computer's architecture.

   Assuming you downloaded the installer to the Downloads folder under your home directory, we recommend using the `apt` tool for installation to avoid potential dependency issues:

   ```shell
   cd ~/Downloads
   sudo apt install ./spark-store*.deb
   ```

---

## Frequently Asked Questions (FAQ)

Please refer to the [Spark Store FAQ and Support Guide](https://gitee.com/spark-store-project/spark-store/blob/dev/FAQ.md);  
For advanced users requiring custom aptss config configuration, please refer to the [aptss Repository](https://gitee.com/GXDE-OS/aptss).

---

## Contact and Feedback

- If you have any questions or suggestions, please submit issues via email or the [Issue Page](https://gitee.com/spark-store-project/spark-store/issues);
- If you want to follow our development progress, visit the [Spark Store Board](https://gitee.com/spark-store-project/spark-store/board) for more information;
- Welcome to visit the [Spark Community Forum](https://bbs.spark-app.store/) to join discussions;
- Our QQ Discussion Group: 872690351 and 865927727;
- If you or your organization requires commercial support, please leave a message for consultation.
---
=======
### Spark Update Tool
#### Introduction

Welcome to Spark Software Updater. Use this tool to update applications on your Linux system.
This version is specifically designed for Linux distributions with Qt6 support.
Please run under root privileges (recommended: use `sudo`).
#### Currently Supported Linux Distributions
- [x] GXDE OS
- [x] Ubuntu
- [x] deepin
- [ ] Kylin


#### Contact & Feedback
momen@momen.world
>>>>>>> update-tool/dev
