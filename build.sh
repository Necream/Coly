#!/bin/bash

# ============================
# Coly Build Script
# ============================

# 检查 g++
if ! command -v g++ &> /dev/null; then
    echo -e "\033[31mError: g++ is not installed. Please install it first.\033[0m"
    exit 1
fi

# 更新 GXPass 的函数
update_gxpass() {
    echo "Updating GXPass.hpp..."
    mkdir -p temp
    cd temp || exit 1
    git clone https://github.com/Necream/GXPass.git
    cp GXPass/GXPass.hpp ../
    cd ..
    rm -rf temp
    echo -e "\033[32mUpdated to the latest version.\033[0m"
}

echo "You need g++ to build this project."
echo "If you have it installed, please run this script."

# 是否更新
read -p "Do you want to update GXPass.hpp and Coly to the latest version? (Y/N): " choice
case "$choice" in
    [Yy]* )
        update_gxpass
        git fetch --all
        git reset --hard origin/main
        git pull
        ;;
    [Nn]* ) echo "Skipping update..." ;;
    * ) echo "Invalid choice. Please enter Y or N."; exit 1 ;;
esac

# 编译
read -p "Do you want to build with g++? (Y/N): " choice
case "$choice" in
    [Yy]* )
        mkdir -p build
        g++ Coly.cpp -o build/Coly -I.
        if [ $? -ne 0 ]; then
            echo -e "\033[31mBuild failed.\033[0m"
            read -p "Do you want to update GXPass and retry? (Y/N): " retry_choice
            case "$retry_choice" in
                [Yy]* )
                    update_gxpass
                    g++ Coly.cpp -o build/Coly -I.
                    if [ $? -ne 0 ]; then
                        echo -e "\033[31mBuild failed again. Please check your g++ installation.\033[0m"
                        exit 1
                    fi
                    ;;
                [Nn]* ) echo "Skipping retry."; exit 1 ;;
                * ) echo "Invalid choice."; exit 1 ;;
            esac
        fi
        echo -e "\033[32mBuild complete. You can run ./build/Coly now.\033[0m"
        ;;
    [Nn]* )
        echo "Skipping build..."
        ;;
    * )
        echo "Invalid choice. Please enter Y or N."
        exit 1
        ;;
esac

echo "Build script finished."
echo "You can run 'sudo ./install.sh' to install Coly now."
echo "(Sorry for the sudo, but it's needed for system-wide install.)"

exit 0
