#!/bin/bash
# Coly v1.5.3 Uninstall Script (Linux)
# Fully removes all Coly files and directories
# Usage: chmod +x uninstall_coly.sh && sudo ./uninstall_coly.sh

# Define core paths (matches install script)
COLY_ROOT="/lib/Coly"
TEMP_DIR="/usr/local/share/Coly"
SYMLINK="/usr/local/bin/coly"
SERVICE_SCRIPT="/usr/bin/coly-service"

# Stop running services first
echo "Stopping Coly services..."
pkill -f "VariableSyncService/server" > /dev/null 2>&1
pkill -f "coly" > /dev/null 2>&1

# Remove symlinks
echo "Removing system symlinks..."
if [ -L "${SYMLINK}" ]; then
    rm -f "${SYMLINK}"
    echo "Removed symlink: ${SYMLINK}"
fi

if [ -f "${SERVICE_SCRIPT}" ]; then
    rm -f "${SERVICE_SCRIPT}"
    echo "Removed service script: ${SERVICE_SCRIPT}"
fi

# Remove main installation directories
echo "Removing Coly directories..."
for dir in "${COLY_ROOT}" "${TEMP_DIR}"; do
    if [ -d "${dir}" ]; then
        rm -rf "${dir}"
        echo "Removed directory: ${dir}"
    else
        echo "Directory not found: ${dir} (skipping)"
    fi
done

# Clean up residual processes (if any)
echo "Cleaning up residual processes..."
pkill -9 -f "coly" > /dev/null 2>&1
pkill -9 -f "VariableSyncService" > /dev/null 2>&1

# Final verification
echo "========================================"
echo "Coly v1.5.3 Uninstallation Complete"
echo "========================================"
echo "All Coly files and directories have been removed."
echo "Note: System dependencies (g++/python3) were not removed."
echo "========================================"