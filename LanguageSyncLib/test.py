# test_coly.py

from ColyPythonSync import (
    session,
    InitColySyncService,
    RegColyVar,
    ReadColyVar,
    ColySyncString
)
import ColyPythonSync


def main():
    # =========================
    # 模拟 C++ argv
    # =========================
    argv = ["test_coly.py", "PROCESS_001"]

    print("🚀 Initializing Coly Sync Service...")
    InitColySyncService(argv)

    if not ColyPythonSync.is_connected:
        print("❌ Connection Failed ")
        return

    print("✅ Connected")

    A = RegColyVar("A")

    A.data = "Hello"



if __name__ == "__main__":
    main()