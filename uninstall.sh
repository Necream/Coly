#!/bin/bash
set -e

# ===================== Core Configuration =====================
COLY_PREFIX="/usr/local"
BIN_DIR="${COLY_PREFIX}/bin"
LIBEXEC_DIR="${COLY_PREFIX}/libexec/coly"
INCLUDE_DIR="${COLY_PREFIX}/include/coly"
SHARE_DIR="${COLY_PREFIX}/share/Coly"
TEMP_DIR="${SHARE_DIR}/TempCode"
SERVICE_FILE="/etc/systemd/system/coly.service"
SERVER_NEW_NAME="ColyServer"

REAL_USER=$(logname 2>/dev/null || echo "$SUDO_USER")
if [ -z "${REAL_USER}" ]; then
    REAL_USER=$(whoami)
fi
USER_HOME="/home/${REAL_USER}"
USER_BASHRC="${USER_HOME}/.bashrc"
COLY_PATH_ENTRY="export PATH=\"\$PATH:${COLY_PREFIX}/bin\""
COLY_PATH_ENTRY_ESC=$(printf '%s' "${COLY_PATH_ENTRY}" | sed 's/[\/&]/\\&/g')

ensure_root() {
    if [ "$(id -u)" -ne 0 ]; then
        exec sudo -k -p "[sudo] Password for ${REAL_USER}: " -- "$0" "$@"
    fi
}

ensure_root "$@"

# ===================== 1. Stop Services =====================
echo "Stopping ColyServer..."
if command -v systemctl >/dev/null 2>&1; then
    systemctl disable --now coly.service >/dev/null 2>&1 || true
fi
if [ -f "${SHARE_DIR}/ColyServer.pid" ]; then
    pid=$(cat "${SHARE_DIR}/ColyServer.pid" 2>/dev/null || true)
    if [ -n "${pid}" ] && kill -0 "${pid}" 2>/dev/null; then
        kill "${pid}" 2>/dev/null || true
    fi
    rm -f "${SHARE_DIR}/ColyServer.pid"
fi
pkill -f "${LIBEXEC_DIR}/${SERVER_NEW_NAME}" >/dev/null 2>&1 || true
pkill -f "${BIN_DIR}/coly" >/dev/null 2>&1 || true
rm -f "${SERVICE_FILE}"
if command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload >/dev/null 2>&1 || true
fi

# ===================== 2. Remove Files =====================
echo "Removing Coly files..."
rm -rf "${COLY_PREFIX}/bin/coly"
rm -rf "${LIBEXEC_DIR}"
rm -rf "${INCLUDE_DIR}"
rm -rf "${SHARE_DIR}"

# ===================== 3. Clean PATH =====================
if [ -f "${USER_BASHRC}" ]; then
    if grep -qxF "${COLY_PATH_ENTRY}" "${USER_BASHRC}"; then
        sed -i "/${COLY_PATH_ENTRY_ESC}/d" "${USER_BASHRC}"
        chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
    fi
fi

echo "Coly removed from ${COLY_PREFIX}."
