# 清除已有的 aptss 补全（如果有的话）
complete -c aptss -e

# 禁用默认的文件补全
complete -c aptss -f

########################################################################
# aptss Fish 补全脚本（中文说明版）
#
# 该脚本参考了 Debian 的 aptss bash 补全和 apt 的 Fish 补全，
# 并将子命令的说明翻译为中文。
########################################################################

### 辅助函数

# 输出所有可用的包名（调用 apt-cache 并指定 aptss 的缓存目录）
function __fish_aptss_print_packages
    apt-cache --no-generate pkgnames -o Dir::Cache="/var/lib/aptss/" 2>/dev/null
end

# 输出所有可用的源包（从 apt-cache dumpavail 中提取 Source 字段）
function __fish_aptss_print_sources
    apt-cache dumpavail -o Dir::Cache="/var/lib/aptss/" 2>/dev/null | \
        grep "^Source:" | cut -d' ' -f2 | sort -u
end

# 输出 target-release 备选项（从 apt-cache policy 中提取）
function __fish_aptss_target_release
    apt-cache policy -o Dir::Cache="/var/lib/aptss/" | \
        grep -oE 'a=[^,]*|n=[^,]*' | cut -d= -f2 | sort -u
end

# 翻译子命令为中文说明
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

# 需要补全包名的子命令（例如安装、显示、搜索等）
set -l __aptss_pkg_subcmds install show search download changelog depends rdepends

# 需要补全“已安装”包的子命令（如 remove、purge、autoremove）
set -l __aptss_installed_pkg_subcmds remove purge autoremove

# 需要补全源包的子命令（结合 apt-cache dumpavail）
set -l __aptss_src_pkg_subcmds source build-dep showsrc policy

### 子命令补全
# 当未输入子命令时，显示所有候选子命令，并使用中文说明
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

### 包名补全
# 对于需要包名参数的子命令，调用 __fish_aptss_print_packages
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_pkg_subcmds" -a '(__fish_aptss_print_packages)' -d '软件包名称'

# 对于 remove、purge、autoremove 命令，补全“已安装”包
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_installed_pkg_subcmds" -a '(__fish_aptss_print_packages)' -d '已安装软件包'

# 对于 source、build-dep、showsrc、policy 命令，组合补全包名及源包名称
complete -c aptss -n "__fish_seen_subcommand_from $__aptss_src_pkg_subcmds" -a '(__fish_aptss_print_packages; __fish_aptss_print_sources)' -d '源代码包'

### 文件补全
# 对于 install 命令：若参数看似为文件路径，则仅补全 .deb 文件
complete -c aptss -n '__fish_seen_subcommand_from install; and test (string match -q "/*" (commandline -ct))' -a "(_filedir -X '*.deb')" -d 'Deb 安装包'

# 对于 edit-sources 命令：补全 /etc/apt/sources.list 与 /etc/apt/sources.list.d 下的 .list 文件
complete -c aptss -n "__fish_seen_subcommand_from edit-sources" -a "(__fish_complete_path /etc/apt/sources.list /etc/apt/sources.list.d/*.list)" -d '软件源文件'
