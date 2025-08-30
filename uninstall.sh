echo "Are you sure you want to uninstall Coly? (Y/N): "
read choice
case "$choice" in
    [Yy]* )
        echo "Uninstalling Coly..."
        rm -rf /lib/Coly
        echo "Uninstallation complete."
        ;;
    [Nn]* )
        echo "Uninstallation cancelled."
        exit 0
        ;;
    * )
        echo "Invalid choice. Please enter Y or N."
        exit 1
        ;;
esac