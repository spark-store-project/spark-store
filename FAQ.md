# FAQ
## Spark App Store FAQ and Support Guide

### Introduction
The Spark App Store is an application store aimed at Linux users and supports multiple Linux distributions. Below are some commonly asked questions and solutions. Please note that this document is primarily intended for ordinary users who are not familiar with Linux and the APT package management system.

---

### Communication and Feedback
**Q: Where can I join the community group?**  
**A:** You can click [here](https://www.deepinos.org/) to enter the main communication platform of the Spark Store.  
> ⚠️ If the above link doesn't work, you can also join our QQ groups as an alternative. The group numbers are 872690351 and 865927727. We also offer forums for communication.

#### Differences Between arm64 and amd64

arm64

* Full Name: Advanced RISC Machine 64-bit
* Main Use: Primarily used in mobile devices, embedded systems, and some new servers and desktop computers.
* Advantages: Low power consumption, high efficiency, suitable for battery-driven and heat-sensitive devices.
* Main Manufacturers: Apple, Qualcomm, HiSilicon, Phytium, etc.

amd64

* Full Name: AMD 64-bit, also known as x86_64
* Main Use: Primarily used in desktop computers, laptops, and servers.
* Advantages: High performance, suitable for computation-intensive applications.
* Main Manufacturers: Intel and AMD.
* The main differences between the two are in the instruction sets and application scenarios. amd64 is usually used for high-performance computers and servers, while arm64 is more commonly used in power-sensitive settings.

#### How to Check:

Linux:

- Open the terminal.
- Type `uname -m` or `arch` and press Enter.
- You will see your processor architecture (possibly x86_64, aarch64, etc.).

---

### Domestic Architecture Support
**Q: I use a domestic chip architecture, how can I get applications?**  
**A:** Currently, Spark Store supports **arm** architecture for domestic chips. You can download deb packages suitable for arm64 architecture.  
> ⚠️ Please note that this is experimental support. If you encounter any issues, please provide feedback via Spark's communication platform, QQ group, or forums.

---

### Dependency Issues
**Q: I get errors when installing dependencies**  
**A:** If you are using UOS or deepin, please do not install the dependency packages. For Kali Linux/Kdeneon etc., temporary support is not available, please compile and install yourself.  
> 💡 If errors occur, try running `sudo apt update` before attempting the installation again. If the issue persists, refer to the first question and seek help on the community platform.

---

### Submission and App Updates
**Q: Where can I submit applications?**  
**A:** You can find the "Submit Application" option in the upper-right corner menu of the app store interface.

---

### Support for Non-deepin/UOS Users
**Q: I'm not a deepin/UOS user; can I use the Spark App Store?**  
**A:** Yes, you can directly install if you're using Ubuntu 22.04. For Ubuntu 20.04/Debian10/Debian11, please install the dependencies first.

---

### Installation Methods
**Q: Can I use dpkg -i for installation?**  
**A:** No! Use `sudo apt install ./xxxx.deb` to install, or use graphical installers like gdebi.  
> ⚠️ Using dpkg directly will not handle software dependencies, which is a common mistake.

---

### System Updates
**Q: Will the Spark Store affect normal system updates?**  
**A:** No, the Spark Store has separated its application sources from system sources and will not affect regular system updates.

---

### Reporting Issues and Removing Apps
**Q: Some apps are outdated or inactive, and I want them removed**  
**A:** You can report issues [here](https://gitee.com/deepin-community-store/software_-issue).

---

### Installing Spark App Store
If you wish to install the Spark App Store, please visit the [Release page](https://gitee.com/deepin-community-store/spark-store/releases), find the latest version, and choose the package suitable for your current system.

> ⚠️ Special Note: If you are using Debian10/Debian11 or Ubuntu 20.04, you may need to download additional dependency packages. After downloading, unzip multiple times until you see the software user guide. For operating systems later than Ubuntu 22.04, you do not need to install dependency packages. Just follow the steps above to install the main program directly.

---