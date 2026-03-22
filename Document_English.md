# Coly v1.5.3
This document was edited in **English**.
Translated by Doubao.

---

## Update Content
We fixed an issue where `Input` and `InputLine` might be unavailable on Linux. The root cause was improper handling of `\r` and `\n`.
We added two new types: `privatevar` and `privatecode`. They will **not be synchronized to the Server**, which can significantly improve running speed when unnecessary synchronization is not needed.
We added two functional variables: `OnlyCompile` and `NoReg`. Among them, `NoReg` will not be synchronized to the ColyVariableSyncService.
**Note: If you set `NoReg = true`, `OnlyCompile` will not take effect either, because the `OnlyCompile` flag is transmitted via VariableSyncService.**

## Installation and Usage
Please use the installation script, or manually install following the directory structure below.

### Linux Directory Structure
Create the following folders under `/lib/` on Linux:
- **/lib/**
  - **Coly/**
    - **Settings/**
      - **LanguageMap.json (Manually copy LanguageMap_Linux.json)**
    - **VariableSyncService/**
      - **server (Can be renamed; supports auto-start on boot. Must start before Coly launches)**
    - **VariableSyncLib/** (Stores libraries for C++ code blocks; follows standard ColyVariableSyncService interface; import via `#include "ColyCppSync.hpp"`)
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio/**

Additionally, create the folder `/usr/local/share/Coly/TempCode` and run:
```Bash
sudo chmod 777 /usr/local/share/Coly -R
sudo chown nobody:nogroup /usr/local/share/Coly -R
```

### Windows Directory Structure
Create the following folders under `C:\` on Windows:
- **C:\\**
  - **Coly\\**
    - **Settings\\**
      - **LanguageMap.json (Manually copy LanguageMap_Windows.json)**
    - **TempCode\\**
    - **InteractiveColy.cly (Optional; missing this file causes startup errors when launching without command line)**
    - **VariableSyncService\\**
      - **server.exe (Can be renamed; supports auto-start on boot. Must start before Coly launches)**
    - **VariableSyncLib\\** (Stores libraries for C++ code blocks; follows standard ColyVariableSyncService interface; import via `#include "ColyCppSync.hpp"`)
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio\\**

Note: Coly uses the MSVC compilation toolchain, so `cl.exe` is configured in `LanguageMap_Windows.json`. Modify the command manually if you need to use `g++`.

Note: ColyVariableSyncService does not encrypt your data. Modify the code to remove all outputs if data security is required; this will not affect Client functions.

## Changelog
- v1.3.2: Fixed runtime bugs in `if` statements and updated judgment logic for better experience. Added two functional variables: `OnlyCompile` and `NoReg` (OnlyCompile existed in the previous version but was undocumented).

## Coly Language
**Note: v0.3.0 and some later versions require manual Server startup before running Coly; otherwise, Coly will detect no variable server and fail to launch.**

Coly is an easy-to-learn programming language that supports calling other languages in a single script, helping beginners leverage strengths of multiple languages to reduce code volume.
Coly adopts linguistic logic instead of mathematical logic for higher readability, at the cost of slightly reduced coding simplicity.
Coly will receive long-term updates with guaranteed backward compatibility for most versions.
Coly supports variable synchronization across code blocks to improve development efficiency, so it can be used as a Runtime Framework for software.
Coly only synchronizes specific official variables, while all Coly native code will be fully synchronized.
Support for standalone functions written in other languages is under development.

**Strong Recommendation: Ensure your compiled code blocks run successfully before importing them into Coly scripts.**

### Features
- Coly automatically synchronizes official predefined variables. Usage is simple: assign values to Coly variables directly, and read them when needed.
- Default supported languages: C++ and Python. Implement custom synchronization programs or use third-party tools for other languages. **Warning: Third-party tools cannot guarantee system or code security. Back up your code if you distrust the developer.**
- ~~`import` in InteractiveColy was disabled due to performance optimization.~~ **Restored in v0.3.0: `import` becomes an independent instruction available in InteractiveColy; original performance optimization code is retained.**
- ~~Repeated recompilation for compiled languages (fixed later).~~ **Fixed in v0.3.0: Added compiled cache detection to save time. Note: Compiled code may be overwritten; this will be fixed in the upcoming SafeColy.**
- SafeColy Plan: A closed-source Coly tool that encrypts code via GXPass and keeps source code only in memory. **SafeColy will remain open-source alongside Coly.**
  - Technical Details: Input `.cly` scripts via SafeColy toolchain to generate and auto-compile `.cpp` files. Compiled outputs are unique even for identical scripts, using random encryption keys.
  - Application: As NCSoft’s completed interpreted-compiled hybrid language, SafeColy will be integrated into the upgraded GXCC as its Runtime Framework.

#### Variable Synchronization
- Since v0.3.0, Coly supports variable synchronization with extensibility for third-party languages later. Manually import Coly libraries to sync variables in C++ code now; backward compatibility is guaranteed, but duplicate submissions may occur.
- ~~SubProcess inherits all variables and permissions of Process; undefined variables in Process cannot be read by SubProcess even if stored on Server, due to local cache and non-real-time synchronization. This limits scope and prevents variable overwriting.~~ **Overwritten in v1.3.2 with frequent real-time synchronization.**
- Independent Processes cannot access each other’s data. Create a persistent parent Server process for cross-Process data sharing if needed.
- Variable synchronization uses network transmission. Expose the Server publicly to share data with others if allowed.

---

### Example 1
```cpp
# Syntax differs from the version released on Aug 31, 2024
define code named 1 with C++
|#include <iostream>
|using namespace std;
|int main(int argc,char* argv[]){
|    cout<<"Hello World!"<<endl;
|    return 0;
|}
# Syntax differs from the version released on Oct 3, 2025
use 1
# Infinite loop from define to jump (explained below)
jump 1
```

---

### Basic Syntax
Note: Content below is unordered; read freely as Coly is lightweight.

#### Comments
Coly comment syntax is similar to Python, but does **not support multi-line comments**. Comments must occupy an entire line, with `#` at the line start with no leading spaces.

#### define
Use `define` to declare `code` or `var` stored with a custom name. Redefinition overwrites old content.
In Example 1, a C++ code block named `1` inherits all global variables.
Variable names support non-English characters, emojis and no spaces; `$` is treated as part of the name (split by spaces).
Prefix each line of `define code` content with `|` at line start (leading spaces are not ignored).

Since v1.5.3, two new types are added: `privatecode` and `privatevar`, same usage as standard types but excluded from ColyVariableSyncService synchronization.

#### use
Use `use` to execute declared `code` following the script reading progress. Refer to Example 1.

#### jump
Use `jump` to jump to declared `code` or `position` and resume execution. Refer to Example 1.

#### Variable Reference
Reference variables via `$varname`. Coly variables are stored as strings with no arithmetic operations supported; call other languages for calculations. Variable types remain unchanged during synchronization.

#### Dynamic Code Loading
Store code in variables, then use `define code named codename with $LanguageType | $CodeInfo` for dynamic loading. Execute via `use code codename`. Dynamic code must be written in one line.

#### Variable-like Code Access
Declared code blocks can be referenced like variables via `$codename`. Modification of referenced code is forbidden for safety; process code via external non-Coly code blocks if edits are needed.

#### print
New instruction added after Aug 31, 2024. Used for content output:
```Coly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
Separates content by spaces without trimming redundant spaces; auto-newline after output.
Demo:
```
> ./Coly.exe
Welcome to Coly!

If you are seeing this message, it means you are currently using InteractiveColy.
InteractiveColy relies on line-by-line Coly interaction, therefore the define code feature is not available in this mode.

If you need to use that feature, please edit your code in a file and then import the library into InteractiveColy.

For more information about Coly - including syntax and available features - please refer to the Coly documentation.
-----------------------------------
>>>define var named 1 with 1
>>>define var named 2 with 2
>>>print $1 $2
12
>>>print $1  $2
1 2
>>>
```

#### printwithoutanewline
Variant of `print` with identical usage; no auto-newline after output.

#### do
Execute dynamic loaded code immediately in current environment for interactive scripts. Creates an independent processing thread with `fake_lineid = -1`; no loop support in InteractiveColy.

#### Functional Variables
Predefine functional variables before use (e.g., `define var named Input`).

##### Input InputLine
`$Input` reads input until spaces/newlines; `$InputLine` reads full lines. Usage demo:
```Coly
# Predefine variables
define var named Input
define var named InputLine
# Read and display input
printwithoutanewline Input:
print $Input
# Required buffer clear (same as C++ cin/getline conflict)
define var named NULL with $InputLine
printwithoutanewline InputLine:
print $InputLine
print
print Press Enter to continue...
# Redundant buffer clear
define var named NULL with $InputLine
```

##### OnlyCompile
Default value: `false`. If set to `true`, the program exits when executing `InitColySyncService()` (C++ only).

##### NoReg
Default value: `false`. If set to `true`, code blocks skip Server registration and variable access; avoids invalid login registration and improves speed.

#### if ifn
New judgment instructions: compare two variables for equality. `if` executes inline code on match; `ifn` executes on mismatch. Variable types (code/var) do not affect comparison.
Syntax:
```Coly
if $var1 $var2 [Single-line Coly Code]
ifn $var1 $var2 [Single-line Coly Code]
```
Demo:
```Coly
define var named 1 with 1
define var named 2 with 1
if $1 $2 print 1
```
Output: `1`.

#### Library Import
New instruction added after Aug 31, 2024. Import shared code blocks via `import lib ...`; supported in InteractiveColy since v0.3.0. No escape characters required on Windows.

#### commitvaroperation
Send custom requests to ColyVariableSyncService for multi-scenario development. Errors are logged on Server and returned to Client; built-in error handling is supported.
Demo:
```Coly
# Register SubProcess to Server
commitvaroperation reg subprocess 123
```

##### All Supported Requests
Tree structure (traverse fully for valid usage):
- set
    - var *JSON (refer to VarContainer below)*
    - process *JSON (refer to ProcessContainer below)*
- get
    - var *VarName*
    - process
- sync
    - var *JSON*
    - process
- del
    - var *VarName*
    - process
- reg
    - process *ProcessID (usually timestamp)*
    - subprocess *SubprocessID*
- login
    - subprocess *SubprocessID*

JSON examples via built-in client:
```VariableSync
reg process test
set var {"Name":"1","Value":"1","Timestamp":1}
get var 1
get process
```

###### VarContainer JSON Schema
```JSON
{
    "Name": "VarName",
    "Value": "Your Var Value",
    "Timestamp": 123456
}
```

###### ProcessContainer JSON Schema
```JSON
{
    "var":[
        "VarID":{
            ...
        }
    ],
    "Timestamp": 123456
}
```

`VarID` is a non-reversible A-Z string generated by Server via `GXPass::num2ABC(GXPass::compile(data))` (for unique identification only, no encryption). Redundant version warnings in GXPass do not affect Coly data safety (no persistent storage between updates).

Set custom `Timestamp` for synchronization conflict resolution (single partial edit not supported; use fixed values or external languages).
Demo Request:
```VariableSync
get var 123
```

## C++ Language Support
Updated runtime libraries since v1.0.0; master 2 macros, 1 function and 1 type below.

### InitColySyncService();
Add to the first line of `main`; auto-detects main Process ID via `argc/argv` and exits if environment invalid. Customize by embedding raw code. Program exits early if `NoReg = true`. Do not modify library files to avoid compatibility issues.

### RegColyVar(varname);
Define synchronized `std::string` variables with full native methods. Extend manually via `varname.data.xxx` for missing operations; overwrites existing Server data after registration.

### ColySyncString
Auto-synchronized string type exclusive to Coly main Process. Assign directly with `std::string`/`char*`; manual modification not recommended.

### sync_variable(*varname);
Auto-sync variables to ColySyncServer via variable pointer; manual calling not recommended.

### ReadColyVar(varname)
Read variables from Server in read-only mode; convert to `std::string` and assign to Coly variables to supplement `RegColyVar()` limitations.
