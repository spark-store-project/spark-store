# 补丁操作说明

- 克隆与构建本项目

    > 用于 zinface-community-cmake-build-system.patch 补丁化构建方式

    ```shell
    git clone https://gitee.com/deepin-community-store/spark-store
    cd spark-store
    git am < patchs/zinface-community-cmake-build-system.patch
    make package
    sudo dpkg -i build/*.deb
    ```


- 在应用补丁时，并对补丁进行的更新操作

    ```shell
    # 切换到一个新的分支，即可开始进行补丁内的更新提交
    git checkout -b cmake-build-system
    
    # origin/dev 表示本仓库的 dev 开发分支
    # 在应用过补丁，并产生了新的提交，即可在当前所在补丁更新分支内
    # 相对基于 origin/dev 为参考，目前所包含的所有最新提交内容将生成为一个补丁文件(其中尾部为增量更新)
    git format-patch --stdout origin/dev > patchs/zinface-community-cmake-build-system.patch

    # 最后，回到你的原 dev 分支，将被改变的补丁文件进行提交
    # 在推送完成后，即可放弃你在 cmake-build-system 分支中所有产生的内容(因为都已经进入补丁)
    ```

- 一些注意事项

    ```shell
    # 在不了解补丁时，你需要认识一下补丁，但补丁与补丁之间有着不同的用法
    # 本 patchs/zinface-community-cmake-build-system.patch 补丁为 cmake 化构建
    
    # 关于补丁的一些方面
    # 1. 首先你需要了解 git 是什么，以及简单的使用
    # 2. 你需要了解补丁是什么，以及简单的使用(应用补丁)
    # 3. 你需要了解如何创建一个补丁，最基本的就是相对于旧目标分支，将本分区新增的提交进行导出为补丁
    # 4. 你可能只会将单个提交、或多个提交导出为补丁，但这还不够，你需要具有绝对的对于补丁的理解
    # 5. 对于不同的目的补丁，应该是多个 patch 文件存在的形式
    # 6. 最后，不管在什么时候，你在应用补丁前，你应该考虑是否应用到当前分支？为什么不切换到一个新的分支呢？
    ```
