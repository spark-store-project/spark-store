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

