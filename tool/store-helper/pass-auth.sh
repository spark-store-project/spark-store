#!/bin/bash
# We use sudo twice to avoid ACE bug here 
# https://gitee.com/amber-ce/amber-ce-bookworm/commit/43e1a1599ede474b37e41aa10c53fd8afc4d35a1

#!/bin/bash
# We use sudo twice to avoid ACE bug here 
# https://gitee.com/amber-ce/amber-ce-bookworm/commit/43e1a1599ede474b37e41aa10c53fd8afc4d35a1

function zenity_prompt() {
    if [[ -e /usr/bin/garma ]]; then
        garma "$@"
    else
        $(command -v zenity) "$@"
    fi
}

# 检查sudo是否需要密码
if sudo -n true 2>/dev/null; then
    echo "sudo 无需密码，继续执行"
else
    # 循环输入密码直到成功或用户取消
    while true; do
        # 使用zenity弹出密码输入框
        PASSWORD=$(zenity_prompt --password --title="需要sudo权限")
        
        # 检查用户是否取消输入
        if [ -z "$PASSWORD" ]; then
            zenity_prompt --error --text="操作已取消"
            exit 1
        fi
        
        # 尝试使用输入的密码执行sudo命令
        echo "$PASSWORD" | sudo -S -v 2>/dev/null
        
        # 检查sudo是否成功
        if [ $? -eq 0 ]; then
            echo "密码正确，继续执行"
            break
        else
            zenity_prompt --error --text="密码错误，请重新输入"
        fi
    done
fi

# 使用sudo命令执行目标程序
echo "$PASSWORD" | sudo sudo -S "$@"
