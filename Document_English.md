# Coly v2.0.1

This document was originally written in **Chinese**.
Translated by **DeepSeek V4 Pro**

---

## Update Highlights

- Fixed the issue where the Python module was inconvenient to load. You can now load it using the method described below.

## Installation & Deployment

Please use the installation script, or perform manual installation following the directory structure outlined below.

### Linux Directory Layout
Create the following folders under `/lib/` on Linux systems:
- **/lib/**
  - **Coly/**
    - **Settings/**
      - **LanguageMap.json** (manually copy `LanguageMap_Linux.json` to this path)
    - **VariableSyncService/**
      - **server** (you may rename this executable freely; it supports auto-start on boot and must launch before Coly initializes)
    - **VariableSyncLib/** (stores C++ development libraries implementing the standard ColyVariableSyncService interface; import via `#include "ColyCppSync.hpp"`)
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio/**

Additionally, create the directory `/usr/local/share/Coly/TempCode` and run the following bash commands:
```Bash
sudo chmod 777 /usr/local/share/Coly -R
sudo chown nobody:nogroup /usr/local/share/Coly -R
```

### Windows Directory Layout
Create the following folders under the root `C:\` drive on Windows systems:
- **C:\\**
  - **Coly\\**
    - **Settings\\**
      - **LanguageMap.json** (manually copy `LanguageMap_Windows.json` to this path)
    - **TempCode\\**
    - **InteractiveColy.cly** (optional file; omitting this will cause errors and prevent Coly from starting when launched without command-line arguments)
    - **VariableSyncService\\**
      - **server.exe** (renaming permitted; supports boot auto-start and must run prior to Coly startup)
    - **VariableSyncLib\\** (C++ development libraries for ColyVariableSyncService; import via `#include "ColyCppSync.hpp"`)
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio\\**

### Critical Notes
- Coly uses the **MSVC** compiler toolchain, so `LanguageMap_Windows.json` references `cl.exe`. Modify the compiler command manually if you intend to use `g++`.
- `ColyVariableSyncService` does **not** encrypt your data. If data protection is required, edit the source code to remove all output logic—this will not affect **client** functionality.

## Changelog

- 2.0.0
Added Python language auto-sync support

- 1.9.3
This update fixes an issue where trailing spaces assigned to variables declared via `define var/privatevar` would disappear. However, command validation rules have become stricter—code written in accordance with this documentation will function normally.
This update introduces the `[Language]:` series synchronized variables; usage details are covered later in this document.
This update introduces two new non-synchronized functional variables, `Size` and `ASCII`; their usage is explained below.
This update fixes a subprocess creation bug where identical `subprocessID` values generated within the same millisecond prevented child processes from launching.

- 1.5.3
This update fixes broken `Input` and `InputLine` behavior on Linux caused by improper handling of `'\r'` and `'\n'` characters.
This update introduces `privatevar` and `privatecode` declarations; these resources are never synced to the **Server** and deliver substantial performance gains when synchronization is unnecessary.
This update introduces the `OnlyCompile` and `NoReg` functional variables. `NoReg` is not synchronized to `ColyVariableSyncService`.
**Important: If you set `NoReg = true`, `OnlyCompile` will also be disabled, as the `OnlyCompile` flag is transmitted through VariableSyncService.**

- v1.3.2
This update fixes execution bugs in the `if` statement and refines conditional evaluation for improved overall usability. Introduces the `OnlyCompile` and `NoReg` functional variables (`OnlyCompile` existed in the previous release but was undocumented).

## The Coly Language

**Important: Starting from v0.3.0 and certain subsequent updates, you must manually start the Server before running Coly. Otherwise, Coly will treat the variable server as nonexistent and refuse to run.**

Coly is a programming language that is **simple and easy to learn**, supporting the embedding of other programming languages within a single script to help beginners, allowing you to leverage the strengths of each language to **reduce code volume**.
Coly adopts linguistic logic rather than mathematical logic, making the code more readable. However, the **downside is reduced ease of writing**.
**Coly will be updated long-term**, and each update guarantees **a certain level of** backward compatibility.
Coly can **synchronize variables** across different code blocks to improve development efficiency, allowing Coly to serve as a software Runtime Framework.
Coly only synchronizes variables of specific types developed specifically for Coly, while **all Coly code is fully synchronized**.
Support for standalone functions written in other languages is still under development.

**We strongly recommend that you successfully compile your code block code before placing it into a Coly code block.**

### Features

- Coly synchronizes variables of auto-sync types defined by Coly in your code. Usage is simple—just assign values to Coly variables. If you need to use a Coly variable, simply read it.
- We provide native support for **C++** and **Python**. If you need variable synchronization for other languages, please write your own sync program or search for available programs online. **However, please note that if you use programs from the internet, we cannot guarantee the security of your computer or code. If you do not trust the developer, please back up your code.**
- ~~**`import` in InteractiveColy is unavailable due to Coly performance optimizations!**~~ **Starting from v0.3.0, `import` has become a new command usable in InteractiveColy. The original performance optimizations have not been removed—we have retained both parts of the code.**
- ~~Note that if you frequently use languages that require compilation, your code may be **recompiled unnecessarily**. This issue will be fixed in a future update.~~ **Fixed in v0.3.0, which added compiled-code detection to save time. Note that your code may be replaced after compilation; this will be fixed in the upcoming SafeColy.**
- **SafeColy Initiative**: SafeColy is a closed-source tool for Coly that encrypts your code using GXPass technology and only retains source code in memory. Note that it is the SafeColy *tool* that is closed-source, not the SafeColy project.
  - Technical Details: Feed your `.cly` script into the SafeColy toolchain, which outputs a `.cpp` file and compiles it automatically. Each compiled file will be different every time you use SafeColy, even for the same source file, because SafeColy uses random strings as encryption/decryption passwords.
  - Application: As NCSoft's first fully-featured compiled (interpreted-core) programming language, SafeColy will be used in the upgraded version of GXCC as the GXCC Runtime Framework.

#### Variable Synchronization

- Starting from v0.3.0, variable synchronization is supported in Coly. Third-party language support and extensions will be added in future updates. If you need variable synchronization now, you can manually include the Coly library in your C++ code and use Coly's functions to manually sync variables. Future updates will not break compatibility, but may cause duplicate submissions of your variables.
- ~~Your SubProcess inherits all variables from the Process, with **the same access permissions as the Process**. Note that if a variable exists in the Process but is undefined in the SubProcess, **it cannot be used even if data exists on the server**. This is due to Coly's local caching mechanism, which **does not maintain real-time synchronization with the server**. This mechanism both eases access pressure and restricts variable/code scope, preventing code blocks in the Process from being overwritten by identically-named code or var blocks in the SubProcess.~~ As of Coly v1.3.2, this behavior has been overridden due to more frequent variable synchronization.
- Different Processes cannot access each other! If you need to share data, you can write a server to create a parent process and suspend it indefinitely.
- Variable synchronization uses the network. If you consent, you can **expose the Server** to share your IP and data with others.

---

### Example 1

```cpp
# Note: syntax here differs from the documentation as of Aug 31, 2024.
define code named 1 with C++
|#include <iostream>
|using namespace std;
|int main(int argc,char* argv[]){
|    cout<<"Hello World!"<<endl;
|    return 0;
|}
# Note: syntax here differs from the documentation as of Oct 3, 2025.
use 1
# Infinite loop from the define line to the jump line, explained below.
jump 1
```

---

### Basic Syntax

Apologies—the content below is not ordered. You may read through it in its entirety; after all, Coly is a very lightweight language.

#### Comments

Coly comment syntax is largely consistent with Python, **but does not support Python-style multi-line comments**, **and** comments must occupy an entire line, with `#` **required at the beginning of the line** with **no preceding whitespace**.

#### define

`define` allows you to define certain content—either `code` or `var`—and store it in a variable with a name you specify. `define` allows redefinition, but **redefined content will be stored as the new content**.
In `Example 1` above, a code block named `1` is defined, using the C++ programming language, inheriting all global variables defined above.
**When naming variables, there are no restrictions—non-English characters and emojis are all fine—as long as there are no spaces, because Coly uses spaces to separate content. `$` will be recognized as part of the variable name.**
In `define code` content, you need to prefix each line with `|` to distinguish the content. **Note that you must place it at the beginning of the line; Coly will not automatically ignore your whitespace here.**

Starting from **v1.5.3**, Coly supports two new types: `privatecode` and `privatevar`. Their usage is identical to `code` and `var`, but they are not synchronized to `ColyVariableSyncService`.

Starting from **v1.9.3**, Coly supports the new `function` type, allowing you to create custom commands and even override existing commands. Note that once an existing command is overridden, you will no longer be able to use it in subsequent code of that process. The scope is limited to the current process; child process code blocks are unaffected, even after overloading.
Example:
```cly
define code named YourCodeName with Coly
|print $var
define function named YourFunction with YourCodeName var
YourFunction 123
# Output: 123
```

#### use

`use` allows you to invoke a `code` block, as shown in `Example 1`.
**`use` will continue running from the current Coly reading position.**

#### jump

`jump` allows you to invoke `code` or `position`, as shown in `Example 1`.
**`jump` causes Coly to jump to the definition location and continue running from there.**

#### Referencing Variables

You can use `$varname` to reference your variable, where `varname` is the name you gave the variable.
**Note that Coly does not have variables in the standard sense of other languages; if variable processing is needed, it is recommended to use another language. All variables in Coly are stored as strings and do not support any arithmetic operations.**
**When synchronizing variables between code blocks, the variable type will remain unchanged.**

#### Variable-to-Code Conversion

This feature allows you to write **JIT-related** content. You can store code in a variable, then use `define code named codename with $LanguageType | $CodeInfo` to convert the variable into code. Afterwards, you can invoke your code using `use code codename`. **Note that the variable-to-code conversion statement must be written on a single line.**

#### Code-as-Variable Reference

This is not a function or feature—it is a characteristic. Code blocks you define can be manipulated like variables, e.g., `$codename`.
However, for security reasons, we do not allow any operations on variable-ized code, to prevent accidental code corruption. If you need to modify the code, you must pass it as input to a code block for processing, **and this code block cannot be Coly**.

#### print

`print` is a **new addition**, not present in the Aug 31, 2024 Coly documentation.
`print` allows you to output certain content. Usage is as follows:
```Coly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
`print` separates content by spaces, but does not reduce spaces within the text. After calling a variable, you need to use a space to indicate the end of the variable name.
When you use `print`, it will automatically add a newline after output.
Example:
```
>  ./Coly.exe
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

`printwithoutanewline` is a variant of `print`, with identical usage, but does not automatically add a newline after output like `print` does.

#### do

`do` allows you to convert variable content into code and execute it immediately in the current environment, useful for writing interactive Coly programs.
`do` relies on creating a new Coly processing line and executes a single line; the executed code is assigned a `fake_lineid` of `-1`, so you cannot locate the processed content, nor can you write loops in InteractiveColy.

#### Functional Variables

Functional variables, such as `Input`, allow you to perform certain interactions during use, and must be declared beforehand.
Example: `define var named Input`.

##### Input / InputLine

`$Input` and `$InputLine` can capture user input, but `$Input` only reads up to the next space or newline, while `$InputLine` reads all the way to the newline.
Note that `Input` can be thought of as similar to `cin` in C++, but is not identical. After each input, you need to press Enter rather than Space.
Usage example:
```Coly
# Initially define two variables
define var named Input
define var named InputLine
# Input and display results for both
printwithoutanewline Input:
print $Input
# You must use the following line after $Input; the reason is similar to the conflict between C++'s cin and getline
define var named NULL with $InputLine
printwithoutanewline InputLine:
print $InputLine
print
print Press Enter to continue...
# The NULL variable here is unused; no further processing is needed.
define var named NULL with $InputLine
```

##### OnlyCompile

`OnlyCompile` has two modes; the default is `false` when undefined.
If `OnlyCompile` is `true`, and the launched code uses `InitColySyncService()`, the code will exit when reaching this macro. (C++ only)

##### NoReg

`NoReg` has two modes; the default is `false` when undefined.
If `NoReg` is `true`, the code block will not be registered when launched, meaning the code block cannot connect to ColyServer and cannot manipulate Coly variables. However, this avoids invalid registration of login credentials and can improve code block execution speed.

##### Size

`Size` is a functional variable used to get the length of a `var`. Usage: first `define var named Size with ...`, then reference it as `$Size` when calling.
For stability, `Size` does not participate in synchronization. Please define `Size` each time before use to assign it a value, but one assignment of `Size` can be used multiple times.
Example:
```cly
# A simple addition operation demonstration
define var named Input
define var named a with $Input
# Note: if you need to create an empty variable, use privatevar. For performance, empty var entries are not synced to the server.
define privatevar named i
define position named loop
define var named Size with $i
ifn $Size $a define var named i with $i #
ifn $Size $a jump loop
define var named b with $Input
define privatevar named i2
define position named loop
define var named Size with $i2
ifn $Size $b define var named i2 with $i2 #
ifn $Size $b jump loop
define var named c with $i $i2
define var named Size with $c
print $Size
```

##### ASCII

`ASCII` allows you to assign a value to it and get the character corresponding to that ASCII value. Note that you must use numeric input.
Example:
```cly
define var named ASCII with 65
print $ASCII
# Should output: A
```

##### [Language]:

The `[Language]:` series functional variables override settings in `LanguageMap.json` at runtime. Since they override the settings, you cannot use these variables to read the settings.

###### [Language]:needcompile

This variable defines whether your language requires compilation; the value should be `true` or `false`.
This must be defined for both interpreted and compiled languages.

###### [Language]:extension

This variable defines the file extension format for your language.
Even if you only want to use the `[Language]:` series variables to invoke system commands, please define this variable.

###### [Language]:compilerun

This variable defines the command required to compile and run your language.

###### [Language]:run

This variable defines the command required to run your language. **Note: for interpreted languages, use this variable. For compiled languages, you also need to define this variable for performance optimization.**

Content example:
```cly
# Define language configuration
define var named Call:needcompile with true
define var named Call:extension with .txt
define var named Call:run with start cmd
define var named Call:compilerun with start notepad
# You must define a code block that uses your custom language type
define code named 1 with Call
# First call uses notepad, via compilerun
use 1
# Second call uses cmd, via run due to Coly's performance optimization
use 1
```

###### Code File Parameters

- `$`: Represents the full path of the source code file
- `^`: Represents the full path of the source code file without the extension
- `*`: Represents the `subprocessid`; Coly auto-sync variables depend on this for subprocess registration.

#### if / ifn

`if` and `ifn` are **new additions**.
`if` and `ifn` determine whether two variables are equal in Coly. If they are equal, `if` executes the following `code`, while `ifn` does not. If they are not equal, the opposite occurs.
**Note that the `type` (whether `code` or `var`) of the variables does not affect comparison. Both compared items must be variables; if you need to compare against a constant, define a `privatevar` first.**
Usage:
```Coly
if $var1 $var2 [Coly Code]
ifn $var1 $var2 [Coly Code]
```
Where `[Coly Code]` represents executable Coly code, limited to a single line.
Example:
```Coly
define var named 1 with 1
define var named 2 with 1
if $1 $2 print 1
```
The result will output `1`.

#### Library Imports

Library imports are a **new addition**, not present in the Aug 31, 2024 documentation.
You can reference library files to use code blocks provided by others. The syntax is `import lib ...`
The **ColyVariableSyncService** was introduced in v0.3.0, and you can use `import lib ...` in interactive mode.
Note that you do not need to use escape characters on Windows.

#### commitvaroperation

`commitvaroperation` allows you to send requests directly to **ColyVariableSyncService** for versatile development workflows.
**Note that if issues arise, you will receive error messages, which will also appear in the Server console/log. You are responsible for resolving any issues caused by error messages. Error detection is already built into Coly.**
Usage:
```Coly
# Send a subprocess registration request to ColyVariableSyncServer.
commitvaroperation reg subprocess 123
```

##### All Available Requests

Below is the tree structure of available commands. To use them, traverse from the root node down to the child node, **plus the additional information specified.**
If you want to test manual requests, you can use `commitvaroperation.cly`.

- set
    - var *JSON, see the VarContainer format below*
    - process *JSON, see the ProcessContainer format below*
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
    - process *ProcessID, usually a timestamp*
    - subprocess *SubprocessID*
- login
    - subprocess *SubprocessID*

Below are the two JSON structures. You can also obtain them via the `client` in the Coly directory:
```VariableSync
reg process test
set var {"Name":"1","Value":"1","Timestamp":1}
get var 1
get process
```

###### VarContainer JSON Structure

```JSON
{
    "Name": "VarName",
    "Value": "Your Var Value",
    "Timestamp": 123456
}
```

###### ProcessContainer JSON Structure

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

`VarID` is an **irreversible** A-Z string generated by the Server.
- Note: The Server generates the string using `GXPass::num2ABC(GXPass::compile(data))`. This method offers no security and is solely used to generate unique identifiers. The version-specific warning in the GXPass library about potential data loss does not apply—there is no need to worry about Coly data loss, because Coly requires you to shut it down during updates, at which point no data is retained.

The `Timestamp` in both JSON structures above is the timestamp of the current operation and should be determined when uploading. Timestamps are used to resolve conflicts during synchronization. However, in Coly you cannot modify only a portion of a variable, **so you can only assign a fixed value to `Timestamp` or use another language to modify it.**

Example (where `123` is the variable name):
```VariableSync
get var 123
```

## C++ Language

In v1.0.0 we released a runtime library written for C++. You need to understand the following two macros, one function, and one type.

### InitColySyncService();

This macro must be placed as the first line of the `main` function. It automatically determines the main process's `ColyProcessID` and requires you to pass `argc` and `argv`. If the launch environment does not meet the code block's requirements, it will exit automatically.
You can also paste the `InitColySyncService()` code into your main function for custom operation.
If you have defined `NoReg` in Coly or elsewhere and its value is `true`, your program will exit when it reaches this point.
**Please do not modify the library files to avoid compatibility issues.**

### RegColyVar(varname);

This function allows you to define a variable of type `std::string`, with basic `std::string` operations. If you find that some operations are not defined, you can manually use `varname.data.(std::string operations)` and then sync the variable. In other cases, this type can be used interchangeably with `std::string`.
**Note that after registration, data previously existing on the Server will be overwritten!**

### ColySyncString

This is the auto-syncing `std::string` type variable in `ColyCppSyncLib`, covering the `string` type supported by the Coly main process.
If you need to perform sync operations, first initialize and assign a value to `ColySyncString`. Assignment is done using the assignment operator, compatible with `std::string` and `char*`.
Manual use is not recommended.

### sync_variable(*varname);

This function automatically syncs your variable to **ColySyncServer**.
The parameter passed is a pointer to a `ColySyncString` variable.
Manual use is not recommended.

### ReadColyVar(varname)

Allows you to read data from the Server in read-only mode. Afterwards, you can perform string operations using `varname`. You can implicitly convert it to `std::string` and then assign it to `ColyVariable`, which can compensate for the shortcomings of `RegColyVar()`.

Example:
```cpp
#include <iostream>
#include <string>
// Add this header to your environment variables or use an absolute path
#include <ColyCppSync.hpp>
using namespace std;
int main(int argc,char* argv[]){
    InitColySyncService();
    RegColyVar(A);
    A="1234567890";
    cout<<string(A)<<endl;
    return 0;
}
```

## Python

In v1.0.0 we released a runtime library written for Python. You need to understand the following one type.
If you want to load the sync library, execute `pip install .` in the `./LanguageSyncLib/ColyPythonSync/` directory.

### ColySyncString

This is a Python variable that can directly connect to Coly. Any supported operations will automatically sync data. If you need high-performance operations, use `string`.

#### RegColyVar(varname)

This function allows you to register a `ColyVar` named `varname`.
Usage: `A = RegColyVar("A")`

#### ReadColyVar(varname)

This function allows you to read an existing `ColyVar`.
Note that the returned type is not `string`; it is a read-only type. Modifications will not affect the value in `ColyServer`.

### InitColySyncService(argv)

This function allows you to connect to `ColyServer` and perform variable operations.

Example:
```py
import sys
from ColyPythonSync import InitColySyncService,RegColyVar
argv=sys.argv
if len(argv)<2 :
  exit()
if InitColySyncService(argv)==0 :
  exit()  # OnlyCompile compatibility
A = RegColyVar("A")
A = "Hello Coly Server!"
```
