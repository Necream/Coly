# Coly v1.3.2

This document was edited in **English**.
Translated by Doubao.

---

## Updated Content

In this update, we fixed the runtime bug of `if`, modified the judgment logic of `if`, and improved the overall experience. Two new functional variables, `OnlyCompile` and `NoReg`, have been added (`OnlyCompile` was available in the previous version but not documented).

No new installation script has been created. You need to create folders in `C:\` on `Windows` as described below:
- **C:\\**
  - **Coly\\**
    - **Settings\\**
      - **LanguageMap.json (please copy manually)**
    - **TempCode\\**
    - **InteractiveColy.cly (you can skip copying this file if not needed, but failing to do so will result in an error and prevent startup when launching without command line)**
    - **VariableSyncService\\**
      - **server.exe (you can rename it and set it to start automatically on boot, but it must be launched before Coly)**
    - **VariableSyncLib\\ (stores libraries required for writing C++ code blocks, adopting the standard ColyVariableSyncService interface; include "ColyCppSync.hpp" when using)**
      - **GXPass.hpp**
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio\\**

Please note that Coly uses the **MSVC** compilation toolchain in this update, so `cl.exe` is used in `LanguageMap.json`. You need to modify the command manually if you want to use `G++`.

Please note that **VariableSyncServer does not encrypt your data**. If data protection is required, modify the code to remove all output; this will not affect the functionality of the **Client**.

## Coly Language

**Please note that v0.3.0 and some updated versions require you to manually start the Server before running Coly; otherwise, it will be considered that the variable server does not exist and Coly cannot run.**

Coly is a **simple and easy-to-learn** programming language that supports referencing other programming languages in a single piece of code to facilitate beginners. This allows you to fully leverage the strengths of various programming languages to **reduce code volume**.

Coly adopts linguistic logic rather than mathematical logic, making code more readable. However, the **disadvantage is that the ease of code writing is reduced**.
**Coly will be updated long-term**, and each update will ensure **a certain level of** compatibility.

Coly can **synchronize variables** between different code blocks to ensure your development efficiency, so Coly can be used as a Runtime Framework for software.
Coly only synchronizes variables of specific types in your code that are specially developed for Coly, while **all Coly code will be fully synchronized**.
The feature of supporting standalone functions written in other languages is still under development.

**We strongly recommend that you successfully compile the code block before placing it into Coly's code block.**

### Features

- Coly synchronizes variable types that use Coly's specified automatic synchronization. The usage is simple: you only need to assign values to Coly's variables. If you need to use Coly's variables, just read them.
- We will support **C++** and **Python** by default. If you need variable synchronization for other languages, please write a synchronization program yourself or search for available programs on the Internet. **Please note that if you use programs from the Internet, we cannot guarantee the security of your computer and code. If you do not trust the developer, please back up your code.**
- ~~**import in InteractiveColy is unavailable due to Coly's performance optimization!**~~ **After v0.3.0, import has become a new instruction that you can use in InteractiveColy, and the original performance optimization has not been removed; we have retained both parts of the code.**
- ~~Please note that if you frequently use languages that require compilation, your code may **be recompiled**. This issue will be fixed in future updates.~~ **Fixed in v0.3.0 by adding a compiled judgment to save time. Please note that your code may be replaced after compilation, which will be fixed in SafeColy later.**
- **SafeColy Plan**: SafeColy is a closed-source tool for Coly (the kernel is interpreted) that encrypts and stores your code using GXPass technology and only keeps the source code in memory (SafeColy itself is an open-source project, not closed-source). **SafeColy will always be an open-source project along with Coly.**
  - Technical details of SafeColy: Input your `.cly` script using the SafeColy toolchain, output a `.cpp` file and compile it automatically. Each time you use a file compiled by SafeColy, it will be different—even for the same file—because SafeColy uses random strings as encryption and decryption passwords.
  - Application of SafeColy: As NCSoft's first complete compiled (interpreted kernel) programming language, SafeColy will be used as the GXCC Runtime Framework in the upgraded version of GXCC.

#### Variable Synchronization

- After v0.3.0, variables in Coly support synchronization. Future updates will support third-party languages and allow extensions. If you need to use variable synchronization now, you can manually introduce the Coly library in your C++ code and use Coly's functions to synchronize variables manually. Subsequent updates will not affect compatibility but may cause duplicate submissions of your variables.
- ~~Your SubProcess inherits all variables of the Process and has **the same access permissions as the Process**. Please note that if there are undefined variables in the Process but used in the SubProcess, **even if data exists in the server, it cannot be used** due to Coly's local caching mechanism, which **does not maintain real-time synchronization with the server**. This mechanism can both alleviate access pressure and limit the scope of variables and code to prevent code in the Process from being overwritten by code or variables with the same name in the SubProcess.~~ In Coly v1.3.2, this feature is overridden due to more frequent variable synchronization.
- Variables cannot be accessed between different Processes! If you need to share data, you can write a server to create a parent process and pause it indefinitely.
- Variable synchronization uses the network. If you agree, you can **expose the Server**, allowing you to share your IP and data with others.

---

### Example 1

```cpp
#Please note that the syntax here is different from that in the document dated Aug 31, 2024.
define code named 1 with C++
|#include <iostream>
|using namespace std;
|int main(int argc,char* argv[]){
|    cout<<"Hello World!"<<endl;
|    return 0;
|}
#Please note that the syntax here is different from that in the document dated Oct 3, 2025.
use 1
#Infinite loop from the line where define is located to the line where jump is located (explained below).
jump 1
```

---

### Basic Syntax

We apologize that the content below is unordered; you can read it through as Coly is an extremely lightweight language.

#### Comments

The syntax of Coly comments is largely consistent with Python, **but multi-line comments in Python are not supported**. Additionally, comments must cover the entire line, and `#` **must be at the beginning of the line** with **no spaces before it**.

#### define

`define` allows you to define certain content (either `code` or `var`) and store it in a variable with the name you specify. `define` permits redefinition, but **the newly defined content will overwrite the old one**.

In `Example 1` above, a code block named 1 is defined using the C++ programming language, inheriting all global variables used above.
**When naming variables, you do not need to pay attention to any restrictions—non-English characters, emojis are all allowed (as long as there are no spaces), because Coly distinguishes content by spaces. `$` will be recognized as part of the variable name.**

In the content of `define code`, you need to add `|` at the beginning of each line to distinguish the content. **Please note that you must add it at the start of the line; Coly will not automatically ignore your spaces here.**

#### use

`use` allows you to execute `code`, as shown in `Example 1`.
**`use` will continue running while maintaining the current Coly reading progress.**

#### jump

`jump` allows you to jump to `code` or `position`, as shown in `Example 1`.
**`jump` will make Coly jump to the definition position and continue running.**

#### Calling Variables

You can use `$varname` to call your variables, where `varname` is the name you gave to the variable.
**Please note that Coly does not have variables in the standard sense of other languages; if variable processing is required, it is recommended to reference other languages. All variables in Coly are stored as strings and do not support any operations.**
**When synchronizing variables between code blocks, the variable type will not change.**

#### Variable Code Conversion

This feature enables you to write **JIT-related content**. You can store code in a variable, then use `define code named codename with $LanguageType | $CodeInfo` to convert the variable to code. After that, you can call your code via `use code codename`. **Please note that the code corresponding to variable code conversion must be written in the same line.**

#### Code Variable Conversion

This is not a function or feature but a characteristic. The code blocks you define can be manipulated like variables (e.g., `$codename`).
However, for security, we do not allow you to perform any operations on code-converted variables to prevent accidental damage to the code. If you need to modify the code, you need to pass the code to a code block for processing, **and this code block cannot be Coly**.

#### print

`print` is a **newly added feature** not reflected in the Coly documentation dated Aug 31, 2024.
`print` allows you to output specific content, with the following usage:
```Coly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
`print` distinguishes content by spaces but does not reduce spaces in the output. After calling a variable, you need to use a space to indicate the end of the variable name.
If you use `print`, it will automatically wrap the line after outputting the content.

For example:
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

`printwithoutanewline` is a variant of `print` with identical usage but does not automatically wrap the line after output, unlike `print`.

#### do

`do` allows you to directly convert variables to code and execute them immediately in the current environment, which can be used to write interactive Coly programs.
`do` relies on creating a new Coly processing thread and executes single-line code with a assigned `fake_lineid` of `-1`. Therefore, you cannot locate the processed content, nor write loops in InteractiveColy.

#### Functional Variables

Functional variables such as `Input` enable certain interactions when used and need to be declared in advance.

##### Input InputLine

`$Input` and `$InputLine` can obtain user input: `$Input` only reads until the next space or newline, while `$InputLine` reads until the newline character.
Usage example:
```Coly
#Initially define two variables
define var named Input
define var named InputLine
#Input and display results for the two Inputs respectively
printwithoutanewline Input:
print $Input
#You must use the following line after $Input (refer to the conflict between cin and getline in C++ for the reason)
define var named NULL with $InputLine
printwithoutanewline InputLine:
print $InputLine
print
print Press Enter to continue...
#The NULL variable here is not used and does not need to be processed.
define var named NULL with $InputLine
```

##### OnlyCompile

`OnlyCompile` has two modes and defaults to `false` when undefined.
If `OnlyCompile` is `true` and the launched code uses `InitColySyncService()`, the code will exit when running to this macro (C++ only).

##### NoReg

`NoReg` has two modes and defaults to `false` when undefined.
If `NoReg` is `true`, the code block will not be registered when launched, meaning the code block cannot connect to the ColyServer or operate Coly variables. However, this avoids invalid registration of login credentials and improves the running speed of the code block.

#### if ifn

`if` and `ifn` are **newly added features**.
`if` and `ifn` can judge whether two variables are equal in Coly. If they are equal, `if` will execute the subsequent code, while `ifn` will not; the opposite applies if they are not equal.
**Please note that variables with `type` as `code` and `var` respectively do not affect the comparison.**

Usage:
```Coly
if $var1 $var2 [Coly Code]
ifn $var1 $var2 [Coly Code]
```

Example:
```Coly
define var named 1 with 1
define var named 2 with 1
if $1 $2 print 1
```
The result will output 1.

#### Library Files

Library files are a **newly added feature** not reflected in the Coly documentation dated Aug 31, 2024.
You can use code blocks provided by others by referencing library files with `import lib ...`.
The **ColyVariableSyncService** was introduced in version v0.3.0, and you can use `import lib ...` in interactive mode.
Please note that you do not need to use escape characters on Windows.

#### commitvaroperation

`commitvaroperation` allows you to directly send requests to the **VariableSyncServer** to enable diverse development processes.
**Please note that if problems are encountered, you will receive error messages, which will also be reflected in the Server's console/logs. You need to resolve the impact of error messages yourself. In Coly, we have added error judgment.**

Usage:
```Coly
#Send a request to register a subprocess to ColyVariableSyncServer.
commitvaroperation reg subprocess 123
```

##### All Available Requests

The tree structure of available commands is shown below; use them by extending from the root node to the child node **and adding the noted information**.

- set
    - var *JSON (see VarContainer description below for format)*
    - process *JSON (see ProcessContainer description below for format)*
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
    - process *ProcessID (usually a timestamp)*
    - subprocess *SubprocessID*
- login
    - subprocess *SubprocessID*

The structures of the two JSONs are shown below; you can also obtain them via the `client` in the Coly directory with the following method:
```VariableSync
reg process test
set var {"Name":"1","Value":"1",Timestamp:1}
get var 1
get process
```

###### JSON Structure of VarContainer

```JSON
{
    "Name": "VarName",
    "Value": "Your Var Value",
    "Timestamp": 123456
}
```

###### JSON Structure of ProcessContainer

```JSON
{
    "var":[
        {
            "VarID":{
                ...
            }
        }
    ],
    "Timestamp": 123456
}
```

`VarID` is an irreversible A-Z string generated by the Server.
- Note: The Server generates the string using `GXPass::num2ABC(GXPass::compile(data))`. This method has no security and is only used to generate a unique identifier. The prompt in the GXPass library requiring a specified version to avoid data loss will not take effect; there is no need to worry about data loss in Coly, as you need to close Coly during the next update, and no data will be saved at that time.

The `Timestamp` in the above two JSON structures is the timestamp of the current operation and needs to be determined independently when uploading. The timestamp is used to resolve conflicts during synchronization. However, you cannot modify part of a variable in Coly, **so you can only assign a fixed value to `Timestamp` or call other languages to modify the `Timestamp`**.

Example (where 123 is the variable name):
```VariableSync
get var 123
```

## C++ Language

We updated the runtime library written for C++ in version v1.0.0. You need to understand the following two macros, one function, and one type.

### InitColySyncService();

This macro needs to be added to the first line of the main function and will automatically determine the `ColyProcessID` of the main process (requiring argc and argv to be imported). If the startup environment does not meet the requirements of the code block, it will exit automatically.
You can also paste the `InitColySyncService()` code into your main function for custom operations.
If `NoReg` is defined (and its value is `true`) in Coly or elsewhere, your program will exit when running to this point.
**Please do not modify the library files to avoid compatibility issues.**

### RegColyVar(varname);

This function allows you to define a variable of type `std::string` with basic `std::string` operations. If you find some operations are undefined, you can manually use `varname.data.(std::string operation)` and then synchronize the variable. In other cases, this type can be mixed with `std::string`.
**Please note that after registration, existing data on the Server will be overwritten!**

### ColySyncString

This is a variable of type `std::string` with automatic synchronization in the `ColyCppSyncLib`, supporting only the `string` type of the Coly main process.
If you want to perform synchronization operations, initialize and assign a value to `ColySyncString` first (assignment can be done directly using an assignment statement, compatible with `std::string` and `char*`).
Manual use is not recommended.

### sync_variable(*varname);

This function can automatically synchronize your variables to the **ColySyncServer**.
The parameter passed is a pointer to a variable of type `ColySyncString`.
Manual use is not recommended.

### ReadColyVar(varname)

Allows you to read data from the Server in read-only mode, after which you can perform string operations using `varname`. You can implicitly convert it to `std::string` and then assign it to `ColyVariable`, which can make up for the defects of `RegColyVar()`.