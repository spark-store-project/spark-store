# FAQ
## Spark Store FAQ and Support Guide

### Introduction
The Spark Store is an application store designed for Linux users, supporting various Linux distributions. Below are some frequently asked questions and their solutions. Please note that this document primarily targets ordinary users unfamiliar with Linux and the `apt` package management system.

---

### Communication and Feedback
**Q: How can I participate in discussions?**  
**A:** You can click [this link](https://bbs.spark-app.store/)  to enter the Spark Forum for communication.  
> 📣 You can also join our QQ Discussion Groups: 872690351 and 865927727.

---

### Difference Between arm64 and amd64

#### arm64

* Full Name: Advanced RISC Machine 64-bit  
* Primary Use: Mainly applied to mobile devices, embedded systems, and some new servers and desktop computers  
* Advantages: Low power consumption, high efficiency, suitable for battery-powered and heat-sensitive devices  
* Major Manufacturers: Apple, Qualcomm, HiSilicon, Phytium, etc.

#### amd64

* Full Name: AMD 64-bit, also known as x86_64  
* Primary Use: Mainly applied to desktop computers, laptops, and servers  
* Advantages: High performance, suitable for compute-intensive applications  
* Major Manufacturers: Intel and AMD

The main difference between the two lies in their instruction sets and application scenarios. amd64 is typically used for high-performance computers and servers, while arm64 is more commonly used in environments with strict power consumption requirements.

#### How to Check:

- Open the Linux terminal  
- Enter the command `uname -m` or `arch` and press `Enter`  
- You will see your computer's processor architecture (e.g., x86_64, aarch64)

---

### Domestic Architecture Support
**Q: My computer uses a domestic architecture. How can I obtain the Spark Store?**  
**A:** Currently, the Spark Store supports **ARM** architecture and **LoongArch New World** architecture. You can download deb packages (software installation packages) for arm64 and loong64 architectures.  
> ⚠️ Please note that this support is experimental. If you encounter issues, please report them to us via the Spark Forum or QQ Discussion Group.

---

### Dependency Installation Issues
**Q: What if dependency package installation fails?**  
**A:** If you are using UOS or deepin, do not install dependencies. Only some Linux distributions require dependency packages. For unsupported distributions, you may need to compile manually. See the [Spark Store Introduction](https://gitee.com/spark-store-project/spark-store/blob/dev/README.md).   
> 💡 When errors occur, run `sudo apt update` first and try again. If the issue persists, refer to the above and seek help via communication platforms.

---

### Software Submission and Updates
**Q: Where can I submit software?**  
**A:** You can find the "Submit Application" option in the top-right menu of the app store interface. We provide two submission methods: **Submission Tools** and **Online Platform**.

---

### Support for Non-deepin/UOS Users
**Q: Can I use Spark Store if I am not a deepin/UOS user?**  
**A:** It can be installed normally on Debian-based distributions. For example, if you use Ubuntu 22.04, install directly. For Ubuntu 20.04 / Debian 10 / Debian 11, install dependencies first. Additionally, experience may not be guaranteed if installed on non-Debian Linux distributions.

---

### Installation Methods
**Q: Can I install using `dpkg -i`?**  
**A:** We **strongly advise against this**. In the directory where the package is saved, use `sudo apt install ./spark-store*.deb` or a graphical installer like Gdebi.  
> ⚠️ Direct use of `dpkg` generally does not handle software dependencies, potentially causing installation failures or damaging the system's dependency environment.

---

### System Updates
**Q: Will Spark Store affect normal system updates?**  
**A:** No. The current version of Spark Store has decoupled its application sources from system sources and will not affect normal system updates.

---

### Reporting Issues and Application Removal
**Q: Some applications are outdated or invalid. I want them removed.**  
**A:** You can [report issues here](https://gitee.com/spark-store-project/software_-issue). 

---

### Installing Spark Store
If you need to install Spark Store, visit the [Release Page](https://gitee.com/spark-store-project/spark-store/releases),  find the latest version, and download the installer suitable for your system.  

> ⚠️ Special Note: If you are using Debian 10 / Debian 11 or Ubuntu 20.04, you need to download the dependency supplementary package additionally. After downloading, extract multiple times until dependency instructions are visible, then follow steps to install dependencies. For Ubuntu 22.04 and later, no dependency package is required. Follow the above instructions to install the main program directly.
---