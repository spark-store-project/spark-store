# 清除已有的 aptss 补全（如果有的话）
complete -c aptss -e

# 禁用默认的文件补全（避免显示当前目录文件）
complete -c aptss -f

########################################################################
# aptss Fish 补全脚本（中文说明版，软件包补全显示简介）
#
# 说明：
# 1. 子命令和选项的说明采用中文显示。
# 2. 软件包补全部分不再调用 apt-cache，而是解析 aptss 自有的软件源索引文件，
#    从 /var/lib/aptss/lists/*Packages（或 *Sources）中提取软件包名称及简介信息。
#
# 注意：如果你的 aptss 软件源索引文件位置或格式有变化，请相应修改下面的 awk 命令。
########################################################################

### 辅助函数

# 解析 /var/lib/aptss/lists/*Packages 文件，输出符合当前输入前缀的“软件包<TAB>简介”
function __fish_aptss_print_packages
    set cur (commandline -ct)
    # 将所有匹配的 Packages 文件拼接后，用 awk 分段解析（RS="" 表示以空行为分段）
    awk -v cur="$cur" '
      BEGIN { RS=""; FS="\n" }
      {
         pkg = ""; desc = "";
         for(i=1; i<=NF; i++){
            if($i ~ /^Package: /) { pkg = substr($i, 10) }  # “Package: ”共9个字符
            else if($i ~ /^Description: /) { desc = substr($i, 14) }  # “Description: ”共13个字符
         }
         if(pkg != "" && (cur == "" || pkg ~ ("^" cur))) {
             print pkg "\t" desc
         }
      }
    ' /var/lib/aptss/lists/*Packages 2>/dev/null
end

# 解析已安装软件包（这里仍使用 dpkg-query，如果需要使用 aptss 数据，可另外构造）
function __fish_aptss_print_installed_packages
    set cur (commandline -ct)
    dpkg-query -W -f='${Package}\t${Description}\n' 2>/dev/null | grep -i "^$cur"
end

# 解析 /var/lib/aptss/lists/*Sources 文件，输出源代码包信息（如果存在）
function __fish_aptss_print_source_packages
    set cur (commandline -ct)
    awk -v cur="$cur" '
      BEGIN { RS=""; FS="\n" }
      {
         pkg = ""; desc = "";
         for(i=1; i<=NF; i++){
            if($i ~ /^Package: /) { pkg = substr($i, 10) }
            else if($i ~ /^Description: /) { desc = substr($i, 14) }
         }
         if(pkg != "" && (cur == "" || pkg ~ ("^" cur))) {
             print pkg "\t" desc
         }
      }
    ' /var/lib/aptss/lists/*Sources 2>/dev/null
end

# 翻译子命令为中文说明（用于补全时显示在括号内）
function __fish_translate_aptss_cmd
    switch $argv[1]
        case ssupdate
            echo "更新软件源"
        case list
            echo "列出软件包"
        case search
            echo "搜索软件包"
        case show
            echo "显示软件包信息"
        case showsrc
            echo "显示源包信息"
        case install
            echo "安装软件包"
        case remove
            echo "移除软件包"
        case purge
            echo "彻底移除软件包"
        case autoremove
            echo "自动移除不必要的软件包"
        case update
            echo "更新软件包列表"
        case upgrade
            echo "升级软件包"
        case full-upgrade
            echo "完全升级（可能移除其他软件包）"
        case dist-upgrade
            echo "发行版升级"
        case edit-sources
            echo "编辑软件源列表"
        case help
            echo "显示帮助信息"
        case source
            echo "下载源代码包"
        case build-dep
            echo "安装构建依赖"
        case clean
            echo "清除软件包缓存"
        case autoclean
            echo "自动清理旧缓存"
        case download
            echo "下载软件包"
        case changelog
            echo "显示更新日志"
        case moo
            echo "彩蛋"
        case depends
            echo "显示软件包依赖"
        case rdepends
            echo "显示软件包逆向依赖"
        case policy
            echo "显示软件包策略"
        case '*'
            echo $argv[1]
    end
end

### 定义各类子命令组

# 所有子命令列表
set -g __aptss_commands ssupdate list search show showsrc install remove purge autoremove update upgrade full-upgrade dist-upgrade edit-sources help source build-dep clean autoclean download changelog moo depends rdepends policy

# 需要补全二进制软件包名称的子命令（例如 install、show、search、download、changelog、depends、rdepends）
set -l __aptss_pkg_subcmds install show search download changelog depends rdepends

# 需要补全已安装软件包的子命令（例如 remove、purge、autoremove）
set -l __aptss_installed_pkg_subcmds remove purge autoremove

# 需要补全源代码包的子命令（例如 source、build-dep、showsrc、policy）
set -l __aptss_src_pkg_subcmds source build-dep showsrc policy

### 子命令补全
# 未输入子命令时，显示所有候选子命令，并在括号中显示中文说明
for cmd in $__aptss_commands
    set desc (__fish_translate_aptss_cmd $cmd)
    complete -c aptss -a $cmd -d "$desc" -n "not __fish_seen_subcommand_from $__aptss_commands"
end

### 公共选项（适用于一组子命令）
set -l group1 "install remove purge upgrade dist-upgrade full-upgrade autoremove"

complete -c aptss -n "__fish_seen_subcommand_from $group1" -l show-progress -d '显示进度'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l fix-broken -d '修复损坏的依赖'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l purge -d '清除配置文件'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l verbose-versions -d '显示详细版本'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l auto-remove -d '自动移除依赖'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s s -l simulate -d '模拟/试运行'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l download -d '下载软件包'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l fix-missing -d '修复丢失文件'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l fix-policy -d '修复策略'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l ignore-hold -d '忽略锁定'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l force-yes -d '强制确认'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l trivial-only -d '仅处理简单情况'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l reinstall -d '重新安装'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l solver -d '使用求解器'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s t -l target-release -d '目标版本'

# 附加的 GENERIC 选项
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s d -l download-only -d '仅下载'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s y -l assume-yes -d '默认确认'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -l assume-no -d '默认否定'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s u -l show-upgraded -d '显示升级情况'
complete -c aptss -n "__fish_seen_subcommand_from $group1" -s m -l ignore-missing -d '忽略缺失'

### 针对各个子命令的专用选项

# update 命令
complete -c aptss -n "__fish_seen_subcommand_from update" -l list-cleanup -d '清理列表'
complete -c aptss -n "__fish_seen_subcommand_from update" -l print-uris -d '显示 URI'
complete -c aptss -n "__fish_seen_subcommand_from update" -l allow-insecure-repositories -d '允许不安全的仓库'

# list 命令
complete -c aptss -n "__fish_seen_subcommand_from list" -l installed -d '已安装的软件包'
complete -c aptss -n "__fish_seen_subcommand_from list" -l upgradable -d '可升级的软件包'
complete -c aptss -n "__fish_seen_subcommand_from list" -l manual-installed -d '手动安装的软件包'
complete -c aptss -n "__fish_seen_subcommand_from list" -s v -l verbose -d '详细模式'
complete -c aptss -n "__fish_seen_subcommand_from list" -s a -l all-versions -d '显示所有版本'
complete -c aptss -n "__fish_seen_subcommand_from list" -s t -l target-release -d '目标版本'

# show 命令
complete -c aptss -n "__fish_seen_subcommand_from show" -s a -l all-versions -d '显示所有版本'

# depends 和 rdepends 命令（逐项添加各选项）
for opt in i important installed pre-depends depends recommends suggests replaces breaks conflicts enhances recurse implicit
    complete -c aptss -n "__fish_seen_subcommand_from depends rdepends" -l $opt -d $opt
end
complete -c aptss -n "__fish_seen_subcommand_from depends rdepends" -s i -d '选项 -i'

# search 命令
complete -c aptss -n "__fish_seen_subcommand_from search" -s n -l names-only -d '仅匹配名称'
complete -c aptss -n "__fish_seen_subcommand_from search" -s f -l full -d '全文搜索'

# showsrc 命令
complete -c aptss -n "__fish_seen_subcommand_from showsrc" -l only-source -d '仅显示源代码'

# source 命令
complete -c aptss -n "__fish_seen_subcommand_from source" -s s -l simulate -d '模拟'
complete -c aptss -n "__fish_seen_subcommand_from source" -s b -l compile -d '编译/构建'
complete -c aptss -n "__fish_seen_subcommand_from source" -s P -l build-profiles -d '构建配置'
complete -c aptss -n "__fish_seen_subcommand_from source" -l diff-only -d '仅显示差异'
complete -c aptss -n "__fish_seen_subcommand_from source" -l debian-only -d '仅限 Debian'
complete -c aptss -n "__fish_seen_subcommand_from source" -l tar-only -d '仅打包 tar'
complete -c aptss -n "__fish_seen_subcommand_from source" -l dsc-only -d '仅下载 DSC'
complete -c aptss -n "__fish_seen_subcommand_from source" -s t -l target-release -d '目标版本'

# build-dep 命令
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -s a -l host-architecture -d '主机架构'
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -s s -l simulate -d '模拟'
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -s P -l build-profiles -d '构建配置'
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -s t -l target-release -d '目标版本'
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -l purge -d '清除'
complete -c aptss -n "__fish_seen_subcommand_from build-dep" -l solver -d '求解依赖'

# moo 命令
complete -c aptss -n "__fish_seen_subcommand_from moo" -l color -d '彩蛋模式'

# clean 和 autoclean 命令
complete -c aptss -n "__fish_seen_subcommand_from clean autoclean" -s s -l simulate -d '模拟'

### 针对 -t/--target-release 的特殊补全
complete -c aptss -n '
    begin
        set -l prev (commandline -poc | string trim)
        test "$prev" = "-t" -o "$prev" = "--target-release"
    end
' -a '(__fish_aptss_target_release)' -d '目标版本'

### 软件包补全
# 对于需要二进制软件包名称的子命令，调用 __fish_aptss_print_packages，
# 输出的每一行格式为 "包名<TAB>简介"，Fish 会将 TAB 后内容显示为注释。
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_pkg_subcmds" -a '(__fish_aptss_print_packages)'

# 对于 remove、purge、autoremove 命令，补全已安装的软件包（使用 dpkg-query 输出）
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_installed_pkg_subcmds" -a '(__fish_aptss_print_installed_packages)' -d '已安装软件包'

# 对于 source、build-dep、showsrc、policy 命令，补全源代码包，
# 如果存在对应的 Sources 索引文件，则调用 __fish_aptss_print_source_packages，
# 否则可考虑默认使用二进制包的索引。
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_src_pkg_subcmds" -a '(__fish_aptss_print_source_packages)' -d '源代码包'
