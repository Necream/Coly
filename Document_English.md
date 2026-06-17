# Coly v1.9.3
This document was originally written in **English**.
Translated by **Doubao**
---

## Update Highlights
- This update fixes an issue where trailing spaces assigned to variables declared via `define var/privatevar` would disappear. However, command validation rules have become stricter—code written in accordance with this documentation will function normally.
- New `[Language]:` series synchronized variables added; usage details are covered later in this document.
- Two new non-synchronized functional variables, `Size` and `ASCII`, have been introduced; their usage is explained below.
- Resolved a subprocess creation bug where identical `subprocessID` values generated within the same millisecond prevented child processes from launching.

## Installation & Deployment
You may use the official installation script, or perform manual installation following the directory structure outlined below.

### Linux Directory Layout
Create the following folders under `/lib/` on Linux systems:
- **/lib/**
  - **Coly/**
    - **Settings/**
      - **LanguageMap.json** (manually copy `LanguageMap_Linux.json` to this path)
    - **VariableSyncService/**
      - **server** (you may rename this executable freely; it supports auto-start on boot and must launch before Coly initializes)
    - **VariableSyncLib/** (stores C++ development libraries implementing the standard ColyVariableSyncService interface; import via `#include "ColyCppSync.hpp"`)
      - json.hpp
      - GXPass.hpp
      - NCInt.hpp
      - asio.hpp
      - asio/

Additionally, create the directory `/usr/local/share/Coly/TempCode` and run the following bash commands:
```bash
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
    - **InteractiveColy.cly** (optional file; omitting this will trigger launch errors when starting Coly without a command-line interface)
    - **VariableSyncService\\**
      - **server.exe** (renaming permitted; supports boot auto-start and must run prior to Coly startup)
    - **VariableSyncLib\\** (C++ development libraries for ColyVariableSyncService; import via `#include "ColyCppSync.hpp"`)
      - json.hpp
      - GXPass.hpp
      - NCInt.hpp
      - asio.hpp
      - asio\\

### Critical Notes
- Coly is compiled with the MSVC toolchain, so `LanguageMap_Windows.json` uses `cl.exe` as the default compiler. Modify the compiler command manually if you intend to use `g++`.
- ColyVariableSyncService transmits raw unencrypted data. If data confidentiality is required, edit the source code to remove all output logic—this modification will not break client-side functionality.

## Changelog
### v1.5.3
- Fixed broken `Input` and `InputLine` behavior on Linux caused by improper handling of carriage return (`\r`) and line feed (`\n`) characters.
- Introduced `privatevar` and `privatecode` declarations; these resources are never synced to the Server and deliver substantial performance gains when synchronization is unnecessary.
- Added the `OnlyCompile` and `NoReg` functional variables. Variables marked `NoReg = true` are excluded from synchronization with ColyVariableSyncService.
  > Important Note: Setting `NoReg = true` disables the `OnlyCompile` flag entirely, as the `OnlyCompile` marker is transmitted through VariableSyncService.

### v1.3.2
- Fixed logical execution bugs in the `if` statement and refined conditional evaluation for improved overall usability.
- Added the `OnlyCompile` and `NoReg` functional variables (`OnlyCompile` existed unannounced in the prior release).

## Coly Programming Language Overview
> Critical Reminder: Starting from v0.3.0 and all subsequent minor updates, you must manually start the Variable Sync Server before launching Coly. Coly will fail to initialize if it cannot detect the variable server instance.

Coly is an accessible scripting language designed to embed external programming languages within single scripts, allowing developers to leverage the unique strengths of each language to minimize code volume.
Coly prioritizes human-readable logical syntax over mathematical notation, which improves readability at the cost of slightly increased verbosity during development.
- Long-term active maintenance is guaranteed, with backward compatibility preserved across all updates.
- Cross-code-block variable synchronization streamlines development workflows, enabling Coly to operate as a general-purpose software runtime framework.
- Only Coly-native designated variable types are synchronized across instances; full Coly script source code is synchronized by default.
- Native support for standalone custom functions written in external languages is still in active development.

> Strong Recommendation: Fully compile and validate all embedded external language code snippets before integrating them into Coly script blocks.

### Core Features
1. Automatic synchronization for Coly-standard variable types: simply assign values to Coly variables to sync them, or read synced values directly from variable references.
2. Native built-in support for C++ and Python. To add synchronization support for other languages, implement custom sync logic or source third-party sync utilities.
   > Security Warning: No official safety guarantees are provided for third-party synchronization tools. Back up all source code if you cannot fully trust the tool’s developer.
3. ~~`import` statements within InteractiveColy were disabled for performance optimizations.~~ As of v0.3.0, `import` has been reimplemented as a dedicated native command usable in InteractiveColy; original performance optimization logic remains intact alongside the new import feature.
4. ~~Frequent execution of compiled-language code blocks triggers redundant recompilation.~~ Fixed in v0.3.0 via compiled artifact caching to eliminate redundant builds. Note that compiled binaries may be overwritten at runtime; this edge case will be resolved in the upcoming SafeColy toolkit.
5. SafeColy Initiative: SafeColy is a closed-source auxiliary toolkit that encrypts Coly scripts via GXPass and retains source code only in memory at runtime.
   > Clarification: While the SafeColy toolchain itself is proprietary, the core Coly interpreter will remain open-source indefinitely.
   - Technical Overview: Pass `.cly` Coly scripts into the SafeColy toolchain to generate auto-compiled `.cpp` binaries. Identical input scripts produce distinct compiled artifacts on each build, as randomized encryption/decryption keys are generated dynamically per compilation cycle.
   - Use Case: As the first fully featured compile-time language with an interpreted core developed under the NCSoft ecosystem, SafeColy will serve as the runtime framework for the next iteration of GXCC.

#### Variable Synchronization Rules
- Full cross-process variable synchronization is supported starting in v0.3.0. Third-party language extension compatibility will be added in future updates. For immediate custom C++ variable sync workflows, manually import the Coly sync library and invoke native sync functions directly. Future updates will maintain backward compatibility, though custom manual sync logic may result in duplicate variable submissions to the server.
- ~~Child subprocesses inherit all parent Process variables with identical access permissions. Undefined variables referenced in a subprocess cannot retrieve matching server-side values even if such entries exist—this limitation stems from Coly’s local cache design, which isolates variable/code scopes to prevent naming collisions between parent and child blocks.~~ This behavior was overridden in v1.3.2 following major improvements to synchronization frequency.
- Isolation rule: Independent top-level Process instances cannot access each other’s variable data. To share state across separate Processes, deploy a persistent parent Process server that suspends execution indefinitely to act as a shared data broker.
- Synchronization relies on network communication. You may expose the Server instance to external networks to share synchronized data with remote peers by distributing your host IP address.

### Example Script 1
```cly
# Syntax revised from documentation published Aug 31, 2024
define code named 1 with C++
|#include <iostream>
|using namespace std;
|int main(int argc,char* argv[]){
|    cout<<"Hello World!"<<endl;
|    return 0;
|}
# Syntax revised from documentation published Oct 3, 2025
use 1
# Infinite loop jumping back to the define declaration line; detailed explanation below
jump 1
```

### Core Syntax Reference
Content below is unordered; full reading is recommended given Coly’s lightweight design.

#### Comments
Coly comment syntax loosely mirrors Python but with strict restrictions:
- Multi-line block comments are unsupported.
- Comments must occupy an entire line, with the `#` symbol placed at the absolute start of the line (no leading whitespace permitted before `#`).

#### define
The `define` keyword declares reusable resources of type `code`, `var`, `privatecode`, `privatevar`, or `function`, bound to a custom identifier. Redefining an existing identifier overwrites the stored resource with new content.
As demonstrated in Example Script 1, `define code named 1 with C++` creates a C++ code block labeled `1` that inherits all global variables defined prior to its declaration.
- Naming rules: Variable/resource identifiers accept any Unicode characters (including emojis) with no character restrictions, so long as the name contains no whitespace. The `$` symbol is treated as a valid component of identifier names.
- Multi-line code block syntax: Every line of embedded code must be prefixed with a leading `|` at the absolute start of the line; leading whitespace before the pipe character is not automatically stripped by Coly.

New `define` variants added in major releases:
1. v1.5.3: `privatecode` and `privatevar` – identical usage to standard `code`/`var`, but excluded entirely from synchronization with ColyVariableSyncService.
2. v1.9.3: `function` – declares custom user commands capable of overriding built-in native commands.
   > Scope Note: Overridden built-in commands are only replaced within the parent Process; child subprocess code blocks retain original native command functionality unaffected.
Sample custom function implementation:
```cly
define code named YourCodeName with Coly
|print $var
define function named YourFunction with YourCodeName var
YourFunction 123
# Output: 123
```

#### use
The `use` keyword executes a pre-defined `code` block. Syntax reference in Example Script 1.
Execution behavior: The main Coly script resumes reading from the line immediately following the `use` statement after the target code block finishes running.

#### jump
The `jump` keyword redirects script execution to a pre-defined `code` block or labeled `position` marker, as shown in Example Script 1.
Execution behavior: Coly restarts execution starting at the original `define` or `position` declaration target.

#### Variable Reference Syntax
Reference stored variables using the format `$varname`, where `varname` denotes the declared resource identifier.
- Key Limitation: Coly native variables are stored exclusively as raw string values; no native arithmetic or logical operations are supported on Coly variables. Offload complex data processing to embedded external language blocks.
- Synchronization Guarantee: Variable data types remain consistent across cross-block synchronization cycles.

#### Variable-to-Code Conversion
This feature enables just-in-time (JIT) scripting workflows: store raw code text inside a standard variable, then dynamically construct executable code blocks at runtime via `define code named codename with $LanguageType | $CodeInfo`. Execute the generated block afterward with `use code codename`.
> Restriction: The full variable-to-code declaration statement must occupy a single line.

#### Code-as-Variable Reference
Defined code blocks may be referenced like standard variables using the `$codename` syntax.
- Security Restriction: Direct modification of code blocks via variable references is prohibited to prevent accidental corruption. To alter code block logic, pass the code reference as input to a non-Coly external language processing block.

#### print
A native output command added post-August 31, 2024, absent from early documentation releases.
`print` renders text or variable values to the standard output stream, automatically appending a newline character after execution.
Basic usage example:
```cly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
Whitespace parsing rules: Arguments passed to `print` are split by single spaces. Variable references must be separated by a trailing space to mark the end of the identifier name.
Interactive shell demonstration:
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
Variant of the `print` command with identical argument parsing logic; omits the automatic trailing newline after output completes.

#### do
The `do` keyword dynamically converts inline variable text into executable code and runs it immediately within the current runtime environment, designed for interactive Coly script development.
- Execution Model: Spawns a lightweight isolated Coly processing thread for single-line execution with a hardcoded fake line ID of `-1`.
- Limitation: No positional jump targeting is available for `do` runtime logic; loops cannot be implemented using `do` within InteractiveColy.

#### Functional Variables
Special interactive utility variables such as `Input` require explicit declaration via `define var` before use.

##### Input / InputLine
`$Input` and `$InputLine` capture user terminal input with distinct parsing behavior:
- `$Input`: Reads input up to the first whitespace or newline delimiter.
- `$InputLine`: Reads all input text until the newline character, capturing full lines of user input.
Behavior mirrors C++ standard input streams with caveats regarding carriage return buffer conflicts between short and full-line input methods.
Full usage example:
```cly
# Initialize core input variables
define var named Input
define var named InputLine
# Capture and print short input
printwithoutanewline Input:
print $Input
# Consume residual newline buffer to resolve C++ cin/getline input conflict
define var named NULL with $InputLine
# Capture and print full line input
printwithoutanewline InputLine:
print $InputLine
print
print Press Enter to continue...
# Flush residual input buffer again
define var named NULL with $InputLine
```

##### OnlyCompile
Dual-state functional variable with a default value of `false` if undeclared.
When set to `true` and embedded C++ code invokes the `InitColySyncService()` macro, the compiled binary terminates immediately upon reaching that macro invocation (C++ only).

##### NoReg
Dual-state functional variable with a default value of `false` if undeclared.
When set to `true`, the executing code block skips registration with ColyVariableSyncService. Disabling registration eliminates sync credential overhead and boosts execution speed at the cost of full variable synchronization functionality.

##### Size
Non-synchronized utility variable that retrieves the raw character length of a target string variable.
- Usage Rules: Re-declare and reassign `Size` to refresh its value before each read operation; a single assigned `Size` value may be referenced multiple times until reassigned.
- Empty Variable Rule: Empty string values must be stored in `privatevar` declarations; empty standard `var` entries are omitted from server synchronization for performance optimization.
Demonstration snippet:
```cly
# Simple length counter logic
define var named Input
define var named a with $Input
# Empty storage uses privatevar
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
Non-synchronized functional variable that maps integer ASCII codes to their corresponding text characters. Only numeric integer values are valid assignments.
Example:
```cly
define var named ASCII with 65
print $ASCII
# Output: A
```

##### [Language]: Series Synchronized Variables
This set of synchronized variables overrides compiler/interpreter configuration entries stored within `LanguageMap.json` at runtime. These write-only configuration variables cannot be read back to retrieve active settings.

###### [Language]:needcompile
Boolean flag (`true`/`false`) declaring whether the target language requires pre-compilation before execution. Mandatory declaration for both compiled and interpreted languages.

###### [Language]:extension
String value defining the standard file extension associated with the target language. Required even if only invoking system shell commands without source files.

###### [Language]:compilerun
Full shell command string that handles compilation followed by immediate execution of compiled binaries for compiled languages.

###### [Language]:run
Shell command string for direct runtime execution of interpreted language source files. Compiled languages must also declare this entry for optimized post-compilation execution workflows.
Full custom language configuration example:
```cly
# Register custom language runtime configuration
define var named Call:needcompile with true
define var named Call:extension with .txt
define var named Call:run with start cmd
define var named Call:compilerun with start notepad
# Declare executable code block targeting the custom language
define code named 1 with Call
# First invocation triggers compilerun command (notepad launch)
use 1
# Subsequent invocations trigger optimized run command (cmd launch)
use 1
```

#### if / ifn Conditional Statements
Native conditional commands added post-August 31, 2024, absent from early documentation releases.
- `if`: Executes the trailing inline Coly code snippet if the two input variables contain identical string values.
- `ifn`: Executes the trailing inline Coly code snippet if the two input variables hold different string values.
Key Rules:
  - Comparison logic ignores resource type distinctions between `code` and `var` entries; only raw string values are compared.
  - Direct literal value comparison is unsupported—wrap static constant values inside a temporary `privatevar` before comparison operations.
Syntax Template:
```cly
if $var1 $var2 [Single-Line Coly Code Snippet]
ifn $var1 $var2 [Single-Line Coly Code Snippet]
```
Functional example:
```cly
define var named 1 with 1
define var named 2 with 1
if $1 $2 print 1
# Terminal output: 1
```

#### Library Imports
Library import functionality added post-August 31, 2024, absent from early documentation releases.
Import pre-built shared code libraries with the syntax `import lib [LibraryName]`.
The ColyVariableSyncService stack introduced in v0.3.0 enables library imports within InteractiveColy interactive shell sessions.
Windows Path Note: Backslash escape sequences are not required for Windows file paths in import statements.

#### commitvaroperation
Low-level native command that manually sends raw synchronization requests directly to ColyVariableSyncService for advanced custom development workflows.
- Error Handling: Failed requests return descriptive error messages printed to the server console and persistent log files; developers must implement custom logic to resolve sync failures. Coly’s core interpreter includes built-in error detection for all sync operations.
Basic registration example:
```cly
# Send request to Variable Sync Server to register a new subprocess ID
commitvaroperation reg subprocess 123
```

##### Full List of Supported Sync Request Actions
Invoke requests by chaining hierarchical root and child command nodes with required trailing payload data as specified:
- set
  - var [JSON VarContainer payload]
  - process [JSON ProcessContainer payload]
- get
  - var [Target Variable Name String]
  - process
- sync
  - var [JSON VarContainer payload]
  - process
- del
  - var [Target Variable Name String]
  - process
- reg
  - process [Unique Process ID (typically Unix timestamp)]
  - subprocess [Unique Subprocess ID]
- login
  - subprocess [Target Subprocess ID]

Reference JSON schema definitions are also available via the Coly client utility with the following interactive workflow:
```VariableSync
reg process test
set var {"Name":"1","Value":"1","Timestamp":1}
get var 1
get process
```

###### VarContainer JSON Schema
```json
{
    "Name": "VarName",
    "Value": "Your Var Value",
    "Timestamp": 123456
}
```

###### ProcessContainer JSON Schema
```json
{
    "var":[
        "VarID":{
            ...
        }
    ],
    "Timestamp": 123456
}
```
- VarID Generation: Server generates irreversible alphabetical unique identifiers (A-Z strings) via the internal function `GXPass::num2ABC(GXPass::compile(data))`. This algorithm provides no cryptographic security and exists solely to produce unique resource labels.
- Data Persistence Note: Version mismatches in the GXPass library do not cause permanent data corruption; all Coly instances fully shut down during major version updates, clearing all temporary sync state automatically.
- Timestamp Field: Numeric Unix timestamp used to resolve cross-client synchronization conflicts. Coly does not support partial variable field edits, so developers must hardcode static timestamps or offload timestamp generation logic to embedded external language blocks.
Simple variable read request example (target variable ID = `123`):
```VariableSync
get var 123
```

## C++ Native Sync Library Reference
A dedicated C++ synchronization runtime library was introduced in v1.0.0. All core macros, functions, and custom type definitions are documented below.

### InitColySyncService();
Mandatory macro to be placed as the first executable line inside the program `main()` function. Automatically resolves the host Process ID and requires the standard `argc`/`argv` main function parameters.
- Exit Behavior: Terminates the compiled binary immediately if runtime environment prerequisites are unmet, or if `NoReg = true` is declared within the parent Coly script.
- Customization Note: The full source logic of `InitColySyncService()` may be copied into your main function for custom startup workflows. Avoid modifying distributed library headers to prevent cross-version compatibility failures.

### RegColyVar(varname);
Registration function that creates a synchronized `std::string` wrapper variable bound to the Variable Sync Server. The wrapped type supports all native `std::string` member operations; invoke raw string logic via `varname.data->[std::string method]` followed by manual sync calls for custom edits. The wrapper type is fully interchangeable with standard `std::string` and C-style `char*` pointers.
> Overwrite Warning: Calling `RegColyVar()` will fully overwrite any pre-existing matching variable entry stored on the sync server.

### ColySyncString
Custom synchronized string wrapper type defined within ColyCppSyncLib, exclusively designed for native Coly cross-process synchronization. Compatible with standard `std::string` and `char*` assignment operators. Direct manual instantiation and modification is discouraged; use official library helper functions instead.

### sync_variable(*varname);
Internal helper function that pushes local `ColySyncString` variable state to the remote sync server. Accepts a pointer to a `ColySyncString` instance as its only argument. Direct manual invocation is not recommended for standard development workflows.

### ReadColyVar(varname)
Read-only lookup function that retrieves remote variable state directly from the sync server and returns a value implicitly convertible to `std::string`. Resolves limitations of the overwrite behavior inherent to `RegColyVar()` by enabling read-before-write workflows for synchronized variables. Retrieved values may be assigned to local `ColySyncString` wrapped variables for bidirectional synchronization.