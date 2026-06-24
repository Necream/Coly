#!/bin/bash
set -e

# ===================== Core Configuration =====================
CURRENT_DIR=$(pwd)
COLY_ROOT="/lib/Coly"
SETTINGS_DIR="${COLY_ROOT}/Settings"
SERVICE_DIR="${COLY_ROOT}/VariableSyncService"
LIB_DIR="${COLY_ROOT}/VariableSyncLib"
TEMP_DIR="/usr/local/share/Coly/TempCode"
SERVER_NEW_NAME="ColyServer"
CONFIG_FILE="InteractiveColy.cly"
LANGSYNC_DIR="${CURRENT_DIR}/LanguageSyncLib"

REAL_USER=$(logname 2>/dev/null || echo $SUDO_USER)
if [ -z "${REAL_USER}" ]; then
    REAL_USER=$(whoami)
fi
USER_HOME="/home/${REAL_USER}"
USER_BASHRC="${USER_HOME}/.bashrc"
COLY_PATH_ENTRY="export PATH=\"\$PATH:${COLY_ROOT}:${SERVICE_DIR}\""

# ===================== 1. Install Dependencies =====================
echo "Step 1/8: Installing system dependencies..."
apt update -y > /dev/null 2>&1
apt install -y g++ python3 python3-pip > /dev/null 2>&1
echo "System dependencies installed (g++, python3)."

# ===================== 2. Create Target Directories =====================
echo "Step 2/8: Creating target directories..."
mkdir -p "${COLY_ROOT}" "${SETTINGS_DIR}" "${SERVICE_DIR}" "${LIB_DIR}" "${TEMP_DIR}"
echo "Target directories created."

# ===================== 3. Check Source Files =====================
echo "Step 3/8: Checking source files..."
if [ ! -f "${CURRENT_DIR}/Coly.cpp" ]; then
    echo "Error: Coly.cpp not found!"
    exit 1
fi
if [ ! -f "${CURRENT_DIR}/server.cpp" ]; then
    echo "Error: server.cpp not found!"
    exit 1
fi
if [ ! -f "${CURRENT_DIR}/${CONFIG_FILE}" ]; then
    echo "Error: ${CONFIG_FILE} not found!"
    exit 1
fi
echo "All source files found."

# ===================== 4. Compile =====================
echo "Step 4/8: Compiling with g++ (C++20)..."
g++ -std=c++20 -I. "${CURRENT_DIR}/Coly.cpp" -o "${CURRENT_DIR}/coly" -lpthread
chmod 777 "${CURRENT_DIR}/coly"
echo "Compiled: coly"

g++ -std=c++20 -I. "${CURRENT_DIR}/server.cpp" -o "${CURRENT_DIR}/server" -lpthread
chmod 777 "${CURRENT_DIR}/server"
echo "Compiled: server"

# ===================== 5. Copy Binaries =====================
echo "Step 5/8: Copying binaries..."
cp -f "${CURRENT_DIR}/coly" "${COLY_ROOT}/"
cp -f "${CURRENT_DIR}/server" "${SERVICE_DIR}/${SERVER_NEW_NAME}"
chmod 777 "${COLY_ROOT}/coly"
chmod 777 "${SERVICE_DIR}/${SERVER_NEW_NAME}"
echo "Copied: coly, ${SERVER_NEW_NAME}"

# ===================== 6. Copy Libraries =====================
echo "Step 6/8: Copying VariableSyncLib headers..."

if [ -f "${LANGSYNC_DIR}/json.hpp" ]; then cp -f "${LANGSYNC_DIR}/json.hpp" "${LIB_DIR}/"; fi
if [ -f "${LANGSYNC_DIR}/GXPass.hpp" ]; then cp -f "${LANGSYNC_DIR}/GXPass.hpp" "${LIB_DIR}/"; fi
if [ -f "${LANGSYNC_DIR}/ColyCppSync.hpp" ]; then cp -f "${LANGSYNC_DIR}/ColyCppSync.hpp" "${LIB_DIR}/"; fi
if [ -f "${LANGSYNC_DIR}/VariableSyncService.hpp" ]; then cp -f "${LANGSYNC_DIR}/VariableSyncService.hpp" "${LIB_DIR}/"; fi
if [ -f "${CURRENT_DIR}/NCInt.hpp" ]; then cp -f "${CURRENT_DIR}/NCInt.hpp" "${LIB_DIR}/"; fi
if [ -f "${CURRENT_DIR}/asio.hpp" ]; then cp -f "${CURRENT_DIR}/asio.hpp" "${LIB_DIR}/"; fi
if [ -d "${CURRENT_DIR}/asio" ]; then
    mkdir -p "${LIB_DIR}/asio"
    cp -rf "${CURRENT_DIR}/asio/"* "${LIB_DIR}/asio/"
fi
chmod 777 -R "${LIB_DIR}"
echo "Copied: VariableSyncLib headers"

# ===================== 7. Copy Config =====================
echo "Step 7/8: Copying configuration files..."

if [ -f "${CURRENT_DIR}/Settings/LanguageMap_Linux.json" ]; then
    cp -f "${CURRENT_DIR}/Settings/LanguageMap_Linux.json" "${SETTINGS_DIR}/LanguageMap.json"
    echo "Copied: LanguageMap.json (from LanguageMap_Linux.json)"
fi

cp -f "${CURRENT_DIR}/${CONFIG_FILE}" "${COLY_ROOT}/"
chmod 777 "${COLY_ROOT}/${CONFIG_FILE}"
echo "Copied: ${CONFIG_FILE}"

# ===================== 8. Set Permissions and Install Python Package =====================
echo "Step 8/8: Setting permissions and installing ColyPythonSync..."

chmod 777 -R "${TEMP_DIR}"
chown nobody:nogroup -R "/usr/local/share/Coly"

if [ -f "${LANGSYNC_DIR}/ColyPythonSync/pyproject.toml" ]; then
    pip3 install "${LANGSYNC_DIR}/ColyPythonSync" 2>/dev/null || pip install "${LANGSYNC_DIR}/ColyPythonSync"
    echo "ColyPythonSync installed."
else
    echo "Warning: ColyPythonSync package not found, skipping."
fi

# Configure PATH
if ! grep -qxF "${COLY_PATH_ENTRY}" "${USER_BASHRC}" 2>/dev/null; then
    echo "${COLY_PATH_ENTRY}" >> "${USER_BASHRC}"
    chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
    echo "Added Coly directories to PATH (${USER_BASHRC})"
    export PATH="$PATH:${COLY_ROOT}:${SERVICE_DIR}"
else
    echo "PATH already configured (skipping)"
fi

# ===================== Complete =====================
echo ""
echo "========================================"
echo "Coly v2.0.3 Install Success!"
echo "========================================"
echo "User: ${REAL_USER}"
echo "Installed to: ${COLY_ROOT}"
echo "Run: coly"
echo "Server: ${SERVER_NEW_NAME}"
echo "To use immediately: source ${USER_BASHRC}"
echo "========================================"
