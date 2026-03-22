#!/bin/bash
# Coly v1.5.3 Uninstall Script (Linux)
# Core Function: Force terminate processes → Delete installation directory → Clean environment variables
set -e

# ===================== Core Configuration =====================
# Installation directory/temp directory/process names
COLY_ROOT="/lib/Coly"
SERVICE_DIR="${COLY_ROOT}/VariableSyncService"
TEMP_DIR="/usr/local/share/Coly/TempCode"
SERVER_NEW_NAME="ColyServer"

# Get the actual logged-in user (avoid operating root directory due to sudo)
REAL_USER=$(logname 2>/dev/null || echo $SUDO_USER)
if [ -z "${REAL_USER}" ]; then
    REAL_USER=$(whoami)
fi
USER_HOME="/home/${REAL_USER}"
USER_BASHRC="${USER_HOME}/.bashrc"
# PATH configuration line to clean (escape special characters for sed)
COLY_PATH_ENTRY="export PATH=\"\$PATH:${COLY_ROOT}:${SERVICE_DIR}\""
COLY_PATH_ENTRY_ESC=$(echo "${COLY_PATH_ENTRY}" | sed 's/[\/&]/\\&/g')

# ===================== 1. Force terminate Coly/ColyServer processes =====================
echo "🔌 Force terminating Coly & ColyServer processes..."
# -9: Force kill, -f: Match process name/command line
sudo pkill -9 -f "coly" > /dev/null 2>&1
sudo pkill -9 -f "${SERVER_NEW_NAME}" > /dev/null 2>&1
echo "✅ Process termination completed (no residues)"

# ===================== 2. Delete core installation directory =====================
echo -e "\n🗑️ Deleting Coly core installation directory..."
if [ -d "${COLY_ROOT}" ]; then
    sudo rm -rf "${COLY_ROOT}" > /dev/null 2>&1
    echo "✅ Deleted: ${COLY_ROOT}"
else
    echo "ℹ️ Directory ${COLY_ROOT} does not exist (skipped)"
fi

# ===================== 3. Delete temporary directory =====================
echo -e "\n🧹 Deleting temporary directory..."
if [ -d "${TEMP_DIR}" ]; then
    sudo rm -rf "${TEMP_DIR}" > /dev/null 2>&1
    echo "✅ Deleted: ${TEMP_DIR}"
else
    echo "ℹ️ Directory ${TEMP_DIR} does not exist (skipped)"
fi

# ===================== 4. Clean user environment variables =====================
echo -e "\n⚙️ Cleaning user PATH environment variables..."
if [ -f "${USER_BASHRC}" ]; then
    # Check if Coly PATH configuration exists
    if grep -qxF "${COLY_PATH_ENTRY}" "${USER_BASHRC}"; then
        # Safely delete matching line (use escaped string)
        sudo sed -i "/${COLY_PATH_ENTRY_ESC}/d" "${USER_BASHRC}"
        # Fix file owner (avoid root ownership due to sudo)
        sudo chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
        echo "✅ Removed Coly PATH configuration from ${USER_BASHRC}"
    else
        echo "ℹ️ No Coly PATH configuration found in ${USER_BASHRC} (skipped)"
    fi
else
    echo "ℹ️ File ${USER_BASHRC} does not exist (skipped)"
fi

# ===================== Uninstallation completion prompt =====================
echo -e "\n🎉 Coly v1.5.3 Uninstallation Complete!"
echo "========================================"
echo "✅ Cleaned content:"
echo "  - All running Coly/ColyServer processes"
echo "  - Core directory: ${COLY_ROOT}"
echo "  - Temporary directory: ${TEMP_DIR}"
echo "  - User environment variables (${USER_BASHRC})"
echo -e "\nℹ️ Your source code files (e.g., ${USER_HOME}/Coly/) are NOT modified!"
echo "========================================"