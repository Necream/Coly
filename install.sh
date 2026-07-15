#!/bin/bash
set -e

# ===================== Core Configuration =====================
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
COLY_PREFIX="/usr/local"
BIN_DIR="${COLY_PREFIX}/bin"
LIBEXEC_DIR="${COLY_PREFIX}/libexec/coly"
INCLUDE_DIR="${COLY_PREFIX}/include/coly"
SHARE_DIR="${COLY_PREFIX}/share/Coly"
TEMP_DIR="${SHARE_DIR}/TempCode"
SERVICE_FILE="/etc/systemd/system/coly.service"
SERVER_NEW_NAME="ColyServer"
CONFIG_FILE="InteractiveColy.cly"
LANGSYNC_DIR="${SCRIPT_DIR}/LanguageSyncLib"

REAL_USER=$(logname 2>/dev/null || echo "$SUDO_USER")
if [ -z "${REAL_USER}" ]; then
    REAL_USER=$(whoami)
fi
USER_HOME="/home/${REAL_USER}"
USER_BASHRC="${USER_HOME}/.bashrc"
COLY_PATH_ENTRY="export PATH=\"\$PATH:${COLY_PREFIX}/bin\""

ensure_root() {
    if [ "$(id -u)" -ne 0 ]; then
        exec sudo -k -p "[sudo] Password for ${REAL_USER}: " -- "$0" "$@"
    fi
}

ensure_root "$@"

# ===================== 1. Check Dependencies =====================
echo "Step 1/8: Checking dependencies..."
if ! command -v g++ >/dev/null 2>&1; then
    echo "Error: g++ not found in PATH."
    exit 1
fi
if ! command -v python3 >/dev/null 2>&1; then
    echo "Error: python3 not found in PATH."
    exit 1
fi
if ! command -v pip3 >/dev/null 2>&1; then
    echo "Error: pip3 not found in PATH."
    exit 1
fi
echo "Dependencies found: g++, python3, pip3."

# ===================== 2. Create Target Directories =====================
echo "Step 2/8: Creating target directories..."
mkdir -p "${BIN_DIR}" "${LIBEXEC_DIR}" "${INCLUDE_DIR}" "${SHARE_DIR}" "${TEMP_DIR}"
echo "Target directories created."

# ===================== 3. Check Source Files =====================
echo "Step 3/8: Checking source files..."
for file in "${SCRIPT_DIR}/Coly.cpp" "${SCRIPT_DIR}/server.cpp" "${SCRIPT_DIR}/${CONFIG_FILE}" "${SCRIPT_DIR}/json.hpp" "${SCRIPT_DIR}/GXPass.hpp" "${SCRIPT_DIR}/VariableSyncService.hpp" "${SCRIPT_DIR}/NCint.hpp" "${SCRIPT_DIR}/asio.hpp"; do
    if [ ! -f "${file}" ]; then
        echo "Error: $(basename "${file}") not found!"
        exit 1
    fi
done
echo "All source files found."

# ===================== 4. Compile =====================
echo "Step 4/8: Compiling with g++ (C++20)..."
g++ -std=c++20 -I"${SCRIPT_DIR}" -I"${SCRIPT_DIR}/LanguageSyncLib" "${SCRIPT_DIR}/Coly.cpp" -o "${SCRIPT_DIR}/coly" -lpthread
g++ -std=c++20 -I"${SCRIPT_DIR}" -I"${SCRIPT_DIR}/LanguageSyncLib" "${SCRIPT_DIR}/server.cpp" -o "${SCRIPT_DIR}/ColyServer" -lpthread
echo "Compiled: coly, ColyServer"

# ===================== 5. Copy Binaries =====================
echo "Step 5/8: Copying binaries..."
cp -f "${SCRIPT_DIR}/coly" "${BIN_DIR}/coly"
cp -f "${SCRIPT_DIR}/ColyServer" "${LIBEXEC_DIR}/${SERVER_NEW_NAME}"
chmod 755 "${BIN_DIR}/coly" "${LIBEXEC_DIR}/${SERVER_NEW_NAME}"
echo "Copied: coly, ${SERVER_NEW_NAME}"

# ===================== 6. Copy Libraries =====================
echo "Step 6/8: Copying headers..."
cp -f "${SCRIPT_DIR}/json.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/GXPass.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/VariableSyncService.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/NCint.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/asio.hpp" "${INCLUDE_DIR}/"
cp -rf "${SCRIPT_DIR}/asio" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/Coly.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/libVarContainer.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/LanguageSyncLib/ColyCppSync.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/LanguageSyncLib/VariableSyncService.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/LanguageSyncLib/GXPass.hpp" "${INCLUDE_DIR}/"
cp -f "${SCRIPT_DIR}/LanguageSyncLib/json.hpp" "${INCLUDE_DIR}/"
chmod 644 "${INCLUDE_DIR}"/*
chmod -R 755 "${INCLUDE_DIR}/asio"
echo "Copied: headers"

# ===================== 7. Copy Config =====================
echo "Step 7/8: Copying configuration files..."
cp -f "${SCRIPT_DIR}/${CONFIG_FILE}" "${SHARE_DIR}/"
cp -f "${SCRIPT_DIR}/Settings/LanguageMap_Linux.json" "${SHARE_DIR}/LanguageMap.json"
chmod 644 "${SHARE_DIR}/LanguageMap.json" "${SHARE_DIR}/${CONFIG_FILE}"
echo "Copied: LanguageMap.json, ${CONFIG_FILE}"

# ===================== 8. Install Python Package and Configure Auto Start =====================
echo "Step 8/8: Installing ColyPythonSync and configuring ColyServer auto start..."
if [ -f "${LANGSYNC_DIR}/ColyPythonSync/pyproject.toml" ]; then
    pip3 install "${LANGSYNC_DIR}/ColyPythonSync"
    echo "ColyPythonSync installed."
else
    echo "Warning: ColyPythonSync package not found, skipping."
fi

if command -v systemctl >/dev/null 2>&1; then
    cat > "${SERVICE_FILE}" <<EOF
[Unit]
Description=Coly Server
After=network.target

[Service]
Type=simple
ExecStart=${LIBEXEC_DIR}/${SERVER_NEW_NAME}
WorkingDirectory=${SHARE_DIR}
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
EOF
    systemctl daemon-reload
    systemctl enable --now coly.service
    echo "ColyServer service enabled and started."
else
    echo "Warning: systemctl not found, skipping auto-start service registration."
    nohup "${LIBEXEC_DIR}/${SERVER_NEW_NAME}" >/dev/null 2>&1 &
    echo "ColyServer started for this session."
fi

if ! grep -qxF "${COLY_PATH_ENTRY}" "${USER_BASHRC}" 2>/dev/null; then
    echo "${COLY_PATH_ENTRY}" >> "${USER_BASHRC}"
    chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
    echo "Added Coly to PATH (${USER_BASHRC})"
fi
export PATH="$PATH:${COLY_PREFIX}/bin"

# ===================== Complete =====================
echo ""
echo "========================================"
echo "Coly v2.3.0 Install Success!"
echo "========================================"
echo "User: ${REAL_USER}"
echo "Installed to: ${COLY_PREFIX}"
echo "Run: coly"
echo "Server: ${SERVER_NEW_NAME}"
echo "========================================"
