# =========================
# ColyPythonSync.py
# Fully compatible C++ port (fixed version)
# =========================

import json
import time
import socket

__version__ = "1.0.0"


# =========================
# Network Session (C++ NetworkSession*)
# =========================
class NetworkSession:
    def __init__(self):
        self.sock: socket.socket | None = None

    def connect(self, host: str, port: int):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

    def send_message(self, msg: str) -> str:
        if self.sock is None:
            return "[ERROR] Not connected to server."

        try:
            self.sock.sendall(msg.encode("utf-8"))
            data = self.sock.recv(65535)
            return data.decode("utf-8")
        except Exception as e:
            return f"[ERROR] {str(e)}"


# =========================
# Global state (C++ static replacement)
# =========================
session: NetworkSession = NetworkSession()   # ✅ FIX: never None
is_connected: bool = False

ColyProcessID: str = ""
RegEcho: str = ""


# =========================
# Core transport
# =========================
def send_message(sess: NetworkSession, msg: str) -> str:
    return sess.send_message(msg)


# =========================
# GET variable
# =========================
def get_variable(varname: str) -> str:
    global is_connected

    if not is_connected:
        return "[ERROR] Not connected to server."

    command = "get var " + varname
    return send_message(session, command)


# =========================
# SYNC variable
# =========================
def sync_variable(var) -> str:
    global is_connected

    if not is_connected:
        return "[ERROR] Not connected to server."

    payload = {
        "Name": var.varname,
        "Value": var.data,
        "Timestamp": int(time.time())
    }

    command = "sync var " + json.dumps(payload, ensure_ascii=False)
    return send_message(session, command)


# =========================
# Read-only variable
# =========================
class ColyGetString:
    def __init__(self, varname: str):
        self.varname = varname

        raw = get_variable(varname)

        try:
            j = json.loads(raw)
            self.data = j.get("Value", "")
        except Exception:
            self.data = raw

    def __str__(self):
        return self.data

    def __repr__(self):
        return self.data


# =========================
# Sync variable object
# =========================
class ColySyncString:
    def __init__(self, varname: str, value: str = ""):
        self.varname = varname
        self.data = value
        sync_variable(self)

    def __iadd__(self, other):
        self.data += str(other)
        sync_variable(self)
        return self

    def __add__(self, other):
        sync_variable(self)
        return self.data + str(other)

    def __eq__(self, other):
        sync_variable(self)
        return self.data == str(other)

    def __ne__(self, other):
        sync_variable(self)
        return self.data != str(other)

    def __lt__(self, other):
        sync_variable(self)
        return self.data < str(other)

    def __gt__(self, other):
        sync_variable(self)
        return self.data > str(other)

    def __le__(self, other):
        sync_variable(self)
        return self.data <= str(other)

    def __ge__(self, other):
        sync_variable(self)
        return self.data >= str(other)

    def __getitem__(self, index: int):
        sync_variable(self)
        return self.data[index]

    def __setitem__(self, index: int, value: str):
        lst = list(self.data)
        lst[index] = value
        self.data = "".join(lst)
        sync_variable(self)

    def __str__(self):
        sync_variable(self)
        return self.data

    def __len__(self):
        sync_variable(self)
        return len(self.data)

    def clear(self):
        self.data = ""
        sync_variable(self)

    def c_str(self):
        sync_variable(self)
        return self.data


# =========================
# Macros
# =========================
def RegColyVar(varname: str) -> ColySyncString:
    return ColySyncString(varname, "")


def ReadColyVar(varname: str) -> ColyGetString:
    return ColyGetString(varname)


# =========================
# Init service (C++ macro equivalent)
# =========================
def InitColySyncService(argv: list[str]) -> int:
    global is_connected, ColyProcessID, RegEcho

    if len(argv) < 2:
        return 0

    ColyProcessID = argv[1]

    # connect
    session.connect("127.0.0.1", 12345)
    is_connected = True

    # login subprocess
    RegEcho = send_message(
        session,
        "login subprocess " + ColyProcessID
    )

    # check compile flag
    only_compile = ReadColyVar("OnlyCompile")

    if str(only_compile) == "true":
        return 0

    return 1
