#!/bin/bash

echo "You need g++ to build this project."
echo "If you have it installed, please run this script."

read -p "Do you want to build the latest version? (Y/N): " choice
case "$choice" in
    [Yy]* )
        git pull
        mkdir temp
        cd temp
        git clone https://github.com/Necream/GXPass.git
        cd GXPass
        cp ./GXPass.hpp ../../
        cd ../..
        rm -rf temp
        echo "Updated to the latest version."
        ;;
    [Nn]* )
        echo "Skipping git pull..."
        ;;
    * )
        echo "Invalid choice. Please enter Y or N."
        exit 1
        ;;
esac

read -p "Do you want to build with g++ (G)? " choice
case "$choice" in
    [Gg]* )
        g++ Coly.cpp -o Coly -I.
        If [ $? -ne 0 ]; then
            echo "Build failed."
            echo "Do you want to update to the latest version and try again? (Y/N): "
            read retry_choice
            case "$retry_choice" in
                [Yy]* )
                    git pull
                    mkdir temp
                    cd temp
                    git clone https://github.com/Necream/GXPass.git
                    cd GXPass
                    cp ./GXPass.hpp ../../
                    cd ../..
                    rm -rf temp
                    echo "Updated to the latest version."
                    g++ Coly.cpp -o Coly -I.
                    if [ $? -ne 0 ]; then
                        echo "Build failed again. Please check your g++ installation."
            exit 1
        fi
        echo "Build complete. You can run ./Coly now."
        ;;
    * )
        echo "Invalid choice. Only g++ is supported on Linux."
        exit 1
        ;;
esac

echo "Build complete. You can run 'sudo ./install.sh' to install Coly now."
echo "(We're so sorry that we need to do this, but it's for your better experience.)"

exit 0
