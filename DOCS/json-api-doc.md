# 基本格式
0. 应用信息

例子：https://cdn.d.store.deepinos.org.cn/store/tools/spark-store/app.json

```json
{
    "Name": "星火应用商店",
    "Version": "4.2.7.1",
    "Filename": "spark-store_4.2.7.1_amd64.deb",
    "Torrent_address": "spark-store_4.2.7.1_amd64.deb.torrent",
    "Pkgname": "spark-store",
    "Author": "shenmo <shenmo@spark-app.store>",
    "Contributor": "shenmo <jifengshenmo@outlook.com>",
    "Website": "https://www.spark-app.store/",
    "Update": "2023-09-01 23:22:23",
    "Size": "590.5 KB",
    "More": "* 修复：aptss加锁失败现在会正常报错\n  * 新增：在aptss的特定操作时添加了提示\n  * 新增：在aptss提示加粗\n  * 调整：ssinstall验证支持使用cdn.d.获取",
    "Tags": "community;ubuntu;deepin;uos;dtk5",
    "img_urls": "[\"https://examine-spark.oss-cn-shanghai.aliyuncs.com/images/2023/09/01/411c32fd691544bb985ecba87d151ea0.png\",\"https://examine-spark.oss-cn-shanghai.aliyuncs.com/images/2023/09/01/f44b3c2242c045e28f1161980d805e0d.png\",\"https://examine-spark.oss-cn-shanghai.aliyuncs.com/images/2023/09/01/00263ba857894667bd99240558bec69c.png\",\"https://examine-spark.oss-cn-shanghai.aliyuncs.com/images/2023/09/01/6fd248ad9eea4ef18c9c4acc2a9d372d.png\"]",
    "icons": "https://examine-spark.oss-cn-shanghai.aliyuncs.com/icons/2023/09/01/a88dd18cc1734396a02e7e3c6be59718.png"
}
```

*注意：img_urls和icons的信息不保证有效！大部分是有效的，少部分app.json可能缺失此项目！*

**对于icon和截图的获取需求请参考第三点**

1. 对于某分类下的所有应用信息
`{SOURCE_URL}/{ARCH}/{CATOGARY}/applist.json`
说明：SOURCE_URL：线路链接，目前推荐 https://cdn.d.store.deepinos.org.cn，你也可以使用其他的星火线路
ARCH：架构文件夹 x86是store或amd64-store，arm是aarch64-store。特别的，如果你使用非https://cdn.d.store.deepinos.org.cn的线路，你可能会发现amd64-store会返回404，因为不是所有的服务器都支持软连接
CATOGARY：分类目录。参考 https://gitee.com/deepin-community-store/spark-store/blob/dev/DOCS/spk-doc.md 中 store 直达的对应关系

例子：https://cdn.d.store.deepinos.org.cn/aarch64-store/tools/applist.json

2. 对于单个应用的应用信息
`{SOURCE_URL}/{ARCH}/{CATOGARY}/{Package Name}/app.json`
Package Name是包名。可从上级的applist.json读取

例子：https://cdn.d.store.deepinos.org.cn/store/tools/spark-store/app.json

3. 对应用截图和icon的获取

`{SOURCE_URL}/{ARCH}/{CATOGARY}/{Package Name}/icon.png`
`{SOURCE_URL}/{ARCH}/{CATOGARY}/{Package Name}/screen_n.png`(n=1-5)(至少为1，不是所有的都有到5. 404是正常的）


