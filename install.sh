#!/bin/bash
set -e

# ===================== 核心配置（通用，无需改） =====================
CURRENT_DIR=$(pwd)
COLY_ROOT="/lib/Coly"
SERVICE_DIR="${COLY_ROOT}/VariableSyncService"
TEMP_DIR="/usr/local/share/Coly/TempCode"
# Server重命名为ColyServer
SERVER_NEW_NAME="ColyServer"
# 需要复制的配置文件（添加你的InteractiveColy.cly）
CONFIG_FILE="InteractiveColy.cly"

# 关键：获取真正的登录用户（即使sudo运行，也能拿到原用户）
REAL_USER=$(logname 2>/dev/null || echo $SUDO_USER)
# 兜底：如果以上都获取不到，用当前用户
if [ -z "${REAL_USER}" ]; then
    REAL_USER=$(whoami)
fi
# 登录用户的家目录
USER_HOME="/home/${REAL_USER}"
# 登录用户的bashrc路径
USER_BASHRC="${USER_HOME}/.bashrc"
# PATH配置项（同时添加Coly和ColyServer的目录）
COLY_PATH_ENTRY="export PATH=\"\$PATH:${COLY_ROOT}:${SERVICE_DIR}\""

# ===================== 1. 安装系统依赖 =====================
echo "Step 1/6: Installing system dependencies..."
apt update -y > /dev/null 2>&1
apt install -y g++ python3 > /dev/null 2>&1
echo "✅ System dependencies installed (g++/python3 only)."

# ===================== 2. 创建目标目录 =====================
echo "Step 2/6: Creating target directories..."
mkdir -p "${COLY_ROOT}" "${SERVICE_DIR}" "${TEMP_DIR}"
chmod 777 "${COLY_ROOT}" "${SERVICE_DIR}" "${TEMP_DIR}" -R
echo "✅ Target directories created: ${COLY_ROOT}, ${SERVICE_DIR}"

# ===================== 3. 本地编译 =====================
echo "Step 3/6: Compiling in current directory (${CURRENT_DIR})..."

# 3.1 编译Coly.cpp
if [ -f "${CURRENT_DIR}/Coly.cpp" ]; then
    g++ -std=c++20 -I. "${CURRENT_DIR}/Coly.cpp" -o "${CURRENT_DIR}/coly" -lpthread
    chmod 777 "${CURRENT_DIR}/coly"
    echo "✅ Compiled: Coly.cpp → ${CURRENT_DIR}/coly"
else
    echo "❌ Error: Coly.cpp not found in current directory!"
    exit 1
fi

# 3.2 编译server.cpp（本地仍为server，复制时重命名）
if [ -f "${CURRENT_DIR}/server.cpp" ]; then
    g++ -std=c++20 -I. "${CURRENT_DIR}/server.cpp" -o "${CURRENT_DIR}/server" -lpthread
    chmod 777 "${CURRENT_DIR}/server"
    echo "✅ Compiled: server.cpp → ${CURRENT_DIR}/server"
else
    echo "❌ Error: server.cpp not found in current directory!"
    exit 1
fi

# ===================== 4. 复制编译产物（重命名Server为ColyServer） =====================
echo "Step 4/6: Copying binaries to target directory..."
# 复制Coly
cp -f "${CURRENT_DIR}/coly" "${COLY_ROOT}/"
echo "✅ Copied: coly → ${COLY_ROOT}/coly"
# 复制并将server重命名为ColyServer
cp -f "${CURRENT_DIR}/server" "${SERVICE_DIR}/${SERVER_NEW_NAME}"
chmod 777 "${SERVICE_DIR}/${SERVER_NEW_NAME}"
echo "✅ Copied & renamed: server → ${SERVICE_DIR}/${SERVER_NEW_NAME}"

# ===================== 5. 复制配置文件（关键：添加InteractiveColy.cly） =====================
echo "Step 5/6: Copying configuration file (${CONFIG_FILE})..."
if [ -f "${CURRENT_DIR}/${CONFIG_FILE}" ]; then
    # 复制到coly的运行目录（/lib/Coly），和报错路径一致
    cp -f "${CURRENT_DIR}/${CONFIG_FILE}" "${COLY_ROOT}/"
    chmod 777 "${COLY_ROOT}/${CONFIG_FILE}"
    echo "✅ Copied: ${CONFIG_FILE} → ${COLY_ROOT}/${CONFIG_FILE}"
else
    echo "❌ Error: ${CONFIG_FILE} not found in current directory!"
    exit 1
fi

# ===================== 6. 配置登录用户的PATH（包含ColyServer目录） =====================
echo "Step 6/6: Configuring PATH for user '${REAL_USER}'..."

# 检查用户bashrc是否存在
if [ ! -f "${USER_BASHRC}" ]; then
    echo "ℹ️ Creating ${USER_BASHRC} for user '${REAL_USER}'..."
    touch "${USER_BASHRC}"
    chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
fi

# 检查PATH是否已配置，避免重复添加
if ! grep -qxF "${COLY_PATH_ENTRY}" "${USER_BASHRC}"; then
    # 添加到用户bashrc（同时包含Coly和ColyServer的目录）
    echo "${COLY_PATH_ENTRY}" >> "${USER_BASHRC}"
    # 修复文件所有者（避免sudo导致文件归root）
    chown "${REAL_USER}:${REAL_USER}" "${USER_BASHRC}"
    echo "✅ Added ${COLY_ROOT} and ${SERVICE_DIR} to PATH (${USER_BASHRC})"
    # 临时生效当前会话（普通用户）
    export PATH="$PATH:${COLY_ROOT}:${SERVICE_DIR}"
else
    echo "ℹ️ PATH already contains ${COLY_ROOT} and ${SERVICE_DIR} for user '${REAL_USER}' (skipping)"
fi

# ===================== 完成提示（通用） =====================
echo -e "\n========================================"
echo "🎉 Build & Install Success!"
echo "========================================"
echo "🔧 User Info:"
echo "   - Installed for user: ${REAL_USER}"
echo "   - User home directory: ${USER_HOME}"
echo "🔧 File Info:"
echo "   - Local binaries: ${CURRENT_DIR}/coly, ${CURRENT_DIR}/server"
echo "   - Installed to: ${COLY_ROOT}/coly (run: coly)"
echo "   - Installed to: ${SERVICE_DIR}/${SERVER_NEW_NAME} (run: ${SERVER_NEW_NAME})"
echo "   - Config file: ${COLY_ROOT}/${CONFIG_FILE}"
echo "💡 How to use (for user '${REAL_USER}'):"
echo "   1. Run Coly immediately: source ${USER_BASHRC} && coly"
echo "   2. Run ColyServer immediately: source ${USER_BASHRC} && ${SERVER_NEW_NAME}"
echo "   3. Run in new terminal: coly / ${SERVER_NEW_NAME}"
echo "========================================"