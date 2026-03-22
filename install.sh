#!/bin/bash
# Coly v1.5.3 Full Install & Build Script (Linux)
# Auto-install dependencies, create directories, compile and set permissions
# Usage: chmod +x install_build.sh && sudo ./install_build.sh

# 1. Install required system dependencies
echo "Installing required dependencies..."
apt update > /dev/null 2>&1
apt install -y g++ libasio-dev python3 > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "Error: Failed to install system dependencies"
    exit 1
fi

# 2. Define core paths (matches documentation structure)
COLY_ROOT="/lib/Coly"
SRC_DIR="${COLY_ROOT}/src"
LIB_DIR="${COLY_ROOT}/VariableSyncLib"
SETTINGS_DIR="${COLY_ROOT}/Settings"
SERVICE_DIR="${COLY_ROOT}/VariableSyncService"
TEMP_DIR="/usr/local/share/Coly/TempCode"
OUTPUT_BIN="${COLY_ROOT}/coly"
SERVICE_BIN="${SERVICE_DIR}/server"

# 3. Create all required directories with full permissions
echo "Creating directory structure..."
for dir in "${COLY_ROOT}" "${SRC_DIR}" "${LIB_DIR}" "${SETTINGS_DIR}" "${SERVICE_DIR}" "${TEMP_DIR}"; do
    mkdir -p "${dir}"
    chmod 777 "${dir}"
    
    # Verify directory creation and permissions
    if [ ! -d "${dir}" ] || [ ! -w "${dir}" ]; then
        echo "Error: Failed to create/write to directory ${dir}"
        exit 1
    fi
done

# 4. Create default source files (if missing) - replace with your actual source code
echo "Creating default source files (replace with actual code in production)..."
# Main Coly executable source
cat > "${SRC_DIR}/main.cpp" << EOF
#include <iostream>
int main(int argc, char* argv[]) {
    std::cout << "Coly v1.5.3 - Linux Build" << std::endl;
    return 0;
}
EOF

# VariableSyncService server source
cat > "${SERVICE_DIR}/server.cpp" << EOF
#include <iostream>
int main(int argc, char* argv[]) {
    std::cout << "Coly VariableSyncService - Running" << std::endl;
    while(true) { sleep(1); } // Keep server running
    return 0;
}
EOF

# 5. Copy default library headers (adjust paths to your actual lib files)
echo "Copying default library headers..."
# Example: Copy asio headers (adjust if using local asio)
if [ -d "/usr/include/asio" ]; then
    cp -r /usr/include/asio "${LIB_DIR}/"
fi

# Create placeholder for other required headers
touch "${LIB_DIR}/json.hpp" "${LIB_DIR}/GXPass.hpp" "${LIB_DIR}/NCInt.hpp"

# 6. Compile with C++20 standard
echo "Compiling Coly core (C++20)..."
g++ -std=c++20 \
    -I. \
    -I"${LIB_DIR}" \
    "${SRC_DIR}/main.cpp" \
    -o "${OUTPUT_BIN}" \
    -lpthread

if [ ! -f "${OUTPUT_BIN}" ]; then
    echo "Error: Failed to compile Coly core executable"
    exit 1
fi

echo "Compiling VariableSyncService (C++20)..."
g++ -std=c++20 \
    -I. \
    -I"${LIB_DIR}" \
    "${SERVICE_DIR}/server.cpp" \
    -o "${SERVICE_BIN}" \
    -lpthread

if [ ! -f "${SERVICE_BIN}" ]; then
    echo "Error: Failed to compile VariableSyncService"
    exit 1
fi

# 7. Set executable permissions
chmod 777 "${OUTPUT_BIN}" "${SERVICE_BIN}"

# 8. Generate LanguageMap_Linux.json (C++20)
echo "Generating LanguageMap configuration..."
cat > "${SETTINGS_DIR}/LanguageMap_Linux.json" << EOF
{
    "C++": "g++ -std=c++20 -I${LIB_DIR} \$SRC -o \$OUT -lpthread",
    "Python": "python3 \$SRC"
}
EOF

# 9. Create startup script for VariableSyncService
echo "Creating startup script for VariableSyncService..."
cat > "/usr/bin/coly-service" << EOF
#!/bin/bash
${SERVICE_BIN} &
echo "Coly VariableSyncService started (PID: \$!)"
EOF
chmod 777 /usr/bin/coly-service

# 10. Final setup verification
echo "========================================"
echo "Coly v1.5.3 Install & Build Complete"
echo "========================================"
echo "Core executable: ${OUTPUT_BIN}"
echo "Service executable: ${SERVICE_BIN}"
echo "Temp directory: ${TEMP_DIR}"
echo "LanguageMap: ${SETTINGS_DIR}/LanguageMap_Linux.json"
echo "Start service: coly-service"
echo "Run Coly: ${OUTPUT_BIN}"