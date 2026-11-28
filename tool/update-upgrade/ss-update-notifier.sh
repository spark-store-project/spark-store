#!/bin/bash

source /opt/durapps/spark-store/bin/bashimport/transhell.amber
load_transhell_debug

#############################################################

function has-command() {
    command -v "$1" >/dev/null 2>&1
}

# 发送通知
function notify-send() {
    local user
    user=$(detect-notify-user)

    if [ -z "$user" ]; then
        return 1
    fi

    # Detect uid of the user
    local uid=$(id -u $user)

    sudo -u $user  DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${uid}/bus notify-send "$@"
}

function detect-notify-user() {
    local user

    user=$(who | awk '{print $1}' | head -n 1)
    if [ -n "$user" ]; then
        echo "$user"
        return 0
    fi

    if command -v loginctl >/dev/null 2>&1; then
        user=$(loginctl list-sessions --no-legend 2>/dev/null | awk 'NR == 1 {print $3}')
        if [ -n "$user" ]; then
            echo "$user"
            return 0
        fi
    fi

    return 1
}

function load-ignored-apps() {
    declare -gA ignored_apps=()
    local config_paths=()
    declare -A seen_config_paths=()
    local user
    local user_home
    local config_path

    user=$(detect-notify-user)
    if [ -n "$user" ]; then
        user_home=$(getent passwd "$user" | cut -d: -f6)
        if [ -n "$user_home" ] && [ -d "$user_home" ]; then
            config_path="$user_home/.config/spark-store/ignored_apps.conf"
            if [ -f "$config_path" ] && [ -z "${seen_config_paths["$config_path"]}" ]; then
                config_paths+=("$config_path")
                seen_config_paths["$config_path"]=1
            fi
        fi
    fi

    local home_dir
    for home_dir in /home/*; do
        if [ ! -d "$home_dir" ]; then
            continue
        fi

        config_path="$home_dir/.config/spark-store/ignored_apps.conf"
        if [ -f "$config_path" ] && [ -z "${seen_config_paths["$config_path"]}" ]; then
            config_paths+=("$config_path")
            seen_config_paths["$config_path"]=1
        fi
    done

    local pkg_name
    local pkg_version
    for config_path in "${config_paths[@]}"; do
        while IFS='|' read -r pkg_name pkg_version || [ -n "$pkg_name" ]; do
            pkg_name=$(printf '%s' "$pkg_name" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            pkg_version=$(printf '%s' "$pkg_version" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            if [ -n "$pkg_name" ] && [ -n "$pkg_version" ]; then
                ignored_apps["$pkg_name|$pkg_version"]=1
            fi
        done < "$config_path"
    done
}

function get-apm-upgradable-list() {
    local output
    output=$(env LANGUAGE=en_US apm list --upgradable 2>/dev/null | awk 'NR>1')

    local ifs_old="$IFS"
    IFS=$'\n'

    local line
    for line in $output; do
        local pkg_name
        local pkg_new_ver
        local pkg_cur_ver
        pkg_name=$(echo "$line" | awk -F '/' '{print $1}')
        pkg_new_ver=$(echo "$line" | awk '{print $2}')
        pkg_cur_ver=$(printf '%s\n' "$line" | sed -n 's/.*\[\(upgradable from\|from\):[[:space:]]*\([^]]*\)\].*/\2/p')

        if [ -n "$pkg_name" ] && [ -n "$pkg_new_ver" ] && [ -n "$pkg_cur_ver" ]; then
            echo "${pkg_name} ${pkg_new_ver} ${pkg_cur_ver}"
        fi
    done

    IFS="$ifs_old"
}

# 检测网络链接畅通
function network-check() {
    # 超时时间
    local timeout=15

    # 目标网站
    local target=www.baidu.com

    # 获取响应状态码
    local ret_code=$(curl -I -s --connect-timeout ${timeout} ${target} -w %{http_code} | tail -n1)

    if [ "$ret_code" = "200" ]; then
        # 网络畅通
        return 0
    else
        # 网络不畅通
        return 1
    fi
}

has_aptss=0
has_apm=0

if has-command aptss; then
    has_aptss=1
fi

if has-command apm; then
    has_apm=1
fi

if [ "$has_aptss" -eq 0 ] && [ "$has_apm" -eq 0 ]; then
    exit 0
fi

# 初始化等待时间和最大等待时间
initial_wait_time=15  # 初始等待时间 15 秒
max_wait_time=$((12 * 3600))  # 最大等待时间 12 小时

# 检测网络，若不通则进行重试，采用指数退避算法
wait_time=$initial_wait_time
while ! network-check; do
    echo "$TRANSHELL_CONTENT_NETWORK_FAIL"
    echo "Waiting for network to recover... Retrying in ${wait_time} seconds."

    sleep $wait_time
    wait_time=$((wait_time * 2))  # 等待时间翻倍
    if [ $wait_time -gt $max_wait_time ]; then
        wait_time=$max_wait_time  # 最大等待时间限制为12小时
    fi
done

load-ignored-apps
spark_update_count=0
if [ "$has_aptss" -eq 1 ]; then
    # 每日更新星火源文件
    aptss update

    updatetext=$(LANGUAGE=en_US aptss ssupdate 2>&1)

    # 在网络恢复后，继续更新操作
    retry_count=0
    max_retries=12  # 最大重试次数，防止死循环

    until ! echo "$updatetext" | grep -q "E:"; do
        if [ $retry_count -ge $max_retries ]; then
            echo "Reached maximum retry limit for aptss ssupdate."
            exit 1
        fi

        echo "${TRANSHELL_CONTENT_UPDATE_ERROR_AND_WAIT_15_SEC}"
        sleep 15
        updatetext=$(LANGUAGE=en_US aptss ssupdate 2>&1)
        retry_count=$((retry_count + 1))
    done

    spark_update_count=$(env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist="/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" 2>/dev/null | grep -c upgradable)

    if [ "$spark_update_count" -gt 0 ]; then
        # 获取用户选择的要更新的应用
        PKG_LIST="$(/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgradable-list)"
        IFS_OLD="$IFS"
        IFS=$'\n'

        for line in $PKG_LIST; do
            PKG_NAME=$(echo "$line" | awk -F ' ' '{print $1}')
            PKG_NEW_VER=$(echo "$line" | awk -F ' ' '{print $2}')
            PKG_CUR_VER=$(echo "$line" | awk -F ' ' '{print $3}')

            dpkg --compare-versions "$PKG_NEW_VER" le "$PKG_CUR_VER"
            if [ $? -eq 0 ]; then
                spark_update_count=$((spark_update_count - 1))
                continue
            fi

            PKG_STA=$(dpkg-query -W -f='${db:Status-Want}' "$PKG_NAME")
            if [ "$PKG_STA" = "hold" ]; then
                spark_update_count=$((spark_update_count - 1))
                continue
            fi

            if [ -n "${ignored_apps["$PKG_NAME|$PKG_NEW_VER"]}" ]; then
                spark_update_count=$((spark_update_count - 1))
                continue
            fi
        done

        IFS="$IFS_OLD"
    fi
fi

apm_update_count=0
if [ "$has_apm" -eq 1 ]; then
    updatetext=$(LANGUAGE=en_US apm update 2>&1)
    retry_count=0
    max_retries=12

    until ! echo "$updatetext" | grep -q "E:"; do
        if [ $retry_count -ge $max_retries ]; then
            echo "Reached maximum retry limit for apm update."
            exit 1
        fi

        echo "Update failed...Will retry in 15sec"
        sleep 15
        updatetext=$(LANGUAGE=en_US apm update 2>&1)
        retry_count=$((retry_count + 1))
    done

    apm clean
    PKG_LIST="$(get-apm-upgradable-list)"
    apm_update_count=$(printf '%s\n' "$PKG_LIST" | awk 'NF { count++ } END { print count + 0 }')

    if [ "$apm_update_count" -gt 0 ]; then
        IFS_OLD="$IFS"
        IFS=$'\n'

        # APM 2.0 起为原生包管理：amber-pm-debug 已移除，
        # `apm list --upgradable` 本身已过滤 hold 与同版本项，无需再用 dpkg 复核。
        for line in $PKG_LIST; do
            PKG_NAME=$(echo "$line" | awk -F ' ' '{print $1}')
            PKG_NEW_VER=$(echo "$line" | awk -F ' ' '{print $2}')

            if [ -n "${ignored_apps["$PKG_NAME|$PKG_NEW_VER"]}" ]; then
                apm_update_count=$((apm_update_count - 1))
                continue
            fi
        done

        IFS="$IFS_OLD"
    fi
fi

update_app_number=$((spark_update_count + apm_update_count))
if [ "$update_app_number" -le 0 ]; then
    exit 0
fi
update_transhell

# 如果都是hold或者版本一致的那就直接退出，否则把剩余的给提醒了
# TODO: 除了apt-mark hold之外额外有一个禁止检查列表
# 如果不想提示就不提示

user=$(detect-notify-user)
if [ -n "$user" ] && [ -e "/home/$user/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify" ]; then
    echo "他不想站在世界之巅，好吧"
    echo "Okay he don't want to be at the top of the world, okay"
    exit
else
    notify-send -a spark-store "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_NOTIFY}" "${TRANSHELL_CONTENT_THERE_ARE_APPS_TO_UPGRADE}" || true # Some machine don't have bus, or who command just print nothing.
fi
