# Coly v2.0.0

This document was edited in **Chinese**.

---

## 更新的内容

- 更新了Python语言自动同步的支持

## 安装和使用

请你使用安装脚本，或者按照下面的结构手动安装。

你需要在`Linux`的`/lib/`中创建的文件夹按下面描述的所示
- **/lib/**
  - **Coly/**
    - **Settings/**
      - **LanguageMap.json（请手动拷贝LanguageMap_Linux.json）**
    - **VariableSyncService/**
      - **server（你可以自己改名，可以开机自启动，但是要求在Coly启动之前必须启动）**
    - **VariableSyncLib/（存储了你编写C++代码块时需要用的库，采用了标准的ColyVariableSyncService接口，使用时include "ColyCppSync.hpp"）**
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio/**

另外，你需要创建`/usr/local/share/Coly/TempCode`文件夹，并执行
```Bash
sudo chmod 777 /usr/local/share/Coly -R
sudo chown nobody:nogroup /usr/local/share/Coly -R
```

你需要在`Windows`的`C:\`中创建的文件夹按下面描述的所示
- **C:\\**
  - **Coly\\**
    - **Settings\\**
      - **LanguageMap.json（请手动拷贝LanguageMap_Windows.json）**
    - **TempCode\\**
    - **InteractiveColy.cly（如果你不需要可以不拷贝，但是如果你不拷贝这个文件在无命令行启动时会出现错误并且无法启动）**
    - **VariableSyncService\\**
      - **server.exe（你可以自己改名，可以开机自启动，但是要求在Coly启动之前必须启动）**
    - **VariableSyncLib\\（存储了你编写C++代码块时需要用的库，采用了标准的ColyVariableSyncService接口，使用时include "ColyCppSync.hpp"）**
      - **json.hpp**
      - **GXPass.hpp**
      - **NCInt.hpp**
      - **asio.hpp**
      - **asio\\**

请注意，**Coly**采用了**MSVC**编译工具链，所以`LanguageMap_Windows.json`中使用的是`cl.exe`，如果你需要使用`g++`需要自定更改命令。

请注意，`ColyVariableSyncService`不会加密你的数据。所以如果有数据保护的需求请更改代码删掉所有的输出，不影响**Client**的功能。

## 更新日志

- 1.9.3:
本次更新修复了`define var/privatevar`变量内容跟随的空格消失的问题，但命令的判定标准更加严格。不过你按照文档写是没有问题的。
本次更新新增了`[Language]:`系列变量，支持同步，具体用法后面写。
本次更新新增了`Size`和`ASCII`两个功能变量，不可同步，具体用法后面写。
本次更新修复了Coly在创建subprocess时同一毫秒内创建相同的`subprocessID`导致无法启动子进程的问题。

- 1.5.3:
我们在本次更新时修复了`Input`和`InputLine`在Linux下可能导致不可用的问题，原因是`'\r'`和`'\n'`没有被妥善处理。
我们在本次更新时新增了`privatevar`和`privatecode`，不会被同步到**Server**，能够在不需要的时候显著提升运行速度。
我们新增了`OnlyCompile`和`NoReg`变量，其中`NoReg`不会同步到`ColyVariableSyncService`。
**请注意，如果你指定了`NoReg = true`，那么`OnlyCompile`也不会生效，因为`OnlyCompile`标记通过VariableSyncService传递。**

- v1.3.2:
- 我们在本次更新时修复了`if`的运行bug，更改了`if`的判定，提升了整体体验。新增了`OnlyCompile`和`NoReg`两个功能变量（上一个版本有`OnlyCompile`，但是没有说明）

## Coly语言

**请注意，v0.3.0以及部分更新版本需要你手动启动Server之后才能运行Coly，否则会认为变量服务器不存在，不能运行Coly**

Coly是一个**简单易学**的编程语言，支持再一段代码中引用其他编程语言来便利初学者，这样你就可以充分发挥各个编程语言的长处来**节约代码量**。
Coly采用了语言逻辑而非数学逻辑，使代码可读性更高。但**缺点是代码的编写简易程度降低**。
**Coly会长期更新**，并且每次更新都会保证**一定的**兼容性。
Coly能够在不同的代码段之间**同步变量**来保证你的开发效率，因此Coly可以作为软件的Runtime Framework使用。
Coly只会同步你代码中特定类型且专门为Coly开发的变量，而**Coly代码会全部同步**。
支持其他语言编写的单独函数的功能还在开发中。

**我们强烈建议您成功通过编译代码块代码之后再放到Coly的代码块中**

### 特性

- Coly会同步你代码中使用了Coly规定的自动同步的变量类型。使用方式简单，你只需要为Coly的变量赋值即可。如果你需要使用Coly的变量，读取Coly变量即可。
- 我们会默认支持 **C++** 和 **Python** ，如果你有其他语言变量同步的需求，请你自行编写同步程序或者在网络上搜索可用的程序。**但请注意，如果你使用来自网络上的程序，我们无法保证你的电脑和代码的安全性。如果你不信任开发者，请你备份好你的代码。**
- ~~**InteractiveColy中的import因为Coly的性能优化不可用！**~~ **在v0.3.0以后，import成为了一条新的指令，你可以在InteractiveColy中使用，且原来的性能优化没有被删除，我们保留了两部分的代码。**
- ~~请注意，如果你频繁使用需要编译的语言，你的语言可能**会重新编译**，这个问题会在以后的更新中修复。~~**在v0.3.0中修复，增加了已编译判定，用于节约时间。请注意，你的代码可能在编译之后被替换，这会在之后的SafeColy中修复。**
- **SafeColy计划**： SafeColy是Coly的闭源工具，用于将你的代码使用GXPass技术加密存储并只在内存中存在源码，而不是SafeColy是闭源项目。**SafeColy会和Coly一起始终是开源项目。**
  - SafeColy的技术细节：使用SafeColy工具链输入你的`.cly`脚本，输出一个`.cpp`文件并自动编译。每次你使用SafeColy编译出的文件都会不同，就算是同一个文件，因为SafeColy会使用随机的字符串作为加密解密密码。
  - SafeColy的应用：作为NCSoft的第一个完善的编译型（解释型内核）的编程语言，SafeColy会被用在GXCC的升级版本中作为GXCC Runtime Framework。

#### 变量同步

- 在v0.3.0之后，Coly中的变量支持同步。在以后的更新中会支持第三方的语言并且允许扩展。如果你现在就需要使用变量同步，你可以在你的C++代码中手动引入Coly库并使用Coly的函数手动同步变量。后续更新不会影响兼容性，但是会导致你的变量发生重复提交。
- ~~你的SubProcess继承Process的所有变量，且**访问权限与Process相同**。请注意，如果你在Process中存在未定义的变量但SubProcess中使用了，**就算服务端中存在数据也不能使用**，这是由于Coly的本地缓存机制，且**不会与server保持实时同步**。这个机制既可以缓解访问压力也可以限制变量和代码的作用域，防止Process中的code被SubProcess中的同名code或者var覆盖。~~ 在Coly v1.3.2中，这个特性由于更频繁的变量同步而被覆盖。
- 不同的Process之间无法互相访问！如果你需要共享数据，你可以编写一个server用于创建父进程，无限暂停即可。
- 变量同步使用网络。如果你同意的话，你可以**为Server暴露**，这样你可以分享你的IP并与其他人共享数据。

---

### 示例 1

```cpp
#请注意，这里的语法与Aug 31, 2024时的文档中有所不同。
define code named 1 with C++
|#include <iostream>
|using namespace std;
|int main(int argc,char* argv[]){
|    cout<<"Hello World!"<<endl;
|    return 0;
|}
#请注意，这里的语法与Oct 3, 2025时的文档中有所不同。
use 1
#无限循环从define所在的行到jump所在的行，下面有讲解。
jump 1
```

---

### 基础语法

实在抱歉，下方的内容没有顺序，您可以通读，毕竟Coly是一个十分轻量的语言。

#### 注释

Coly注释的语法与Python大体一致，**但不支持Python的多行注释**，**且**注释必须注释一整行，且`#`**必须位于行首**且**之前不能存在空格**。
#### define

`define`能够让你定义一定的内容，可以是`code`或者`var`，并且存储在你指定名字的变量中。`define`允许你重复定义，但是**重复定义的内容会以新内容存储**。
在上面给出的`示例 1`中，定义了一个代码块名为1，使用C++编程语言，继承上文中使用的所有全局变量。
**你为变量起名时无需注意任何内容，非英文，emoji均可，无空格即可，因为Coly会依照空格区分内容。`$`会被识别成变量名字的一部分。**
在`define code`的内容中，你需要在每一行前加上`|`来区分内容。**请注意，你必须加在行首，这里Coly不会自动忽略你的空格。**

从**v1.5.3**开始，Coly支持`privatecode`和`privatevar`两个新类型，用法和`code`和`var`一样，但是不会同步到`ColyVariableSyncService`。

从**v1.9.3**开始，Coly支持`function`的新类型，支持创建自定义命令，也可以覆盖现有命令。请注意，一旦覆盖了现有命令，在此程序的后续代码中你将不能使用现有命令。作用域限制在自身进程，代码块进程不受影响，即使在重载之后。
例如：
```cly
define code named YourCodeName with Coly
|print $var
define function named YourFunction with YourCodeName var
YourFunction 123
#输出为123
```

#### use

`use`能够让你使用`code`，使用时如`示例 1`所示。
**`use`会保持当前Coly的读取进度继续运行。**

#### jump

`jump`能够让你使用`code`、`position`，使用时如`示例 1`所示。
**`jump`会使Coly跳跃到定义处继续运行。**

#### 调用变量

你可以使用`$varname`来调用你的变量，其中`varname`是你为变量取的名字。
**请注意Coly没有其他语言标准意义上的变量，如果需要变量处理建议引用其他语言。Coly中的变量均以string存储，不支持任何运算。**
**在代码块之间进行变量同步时，不会导致变量类型改变。**

#### 变量代码化

这个功能使你能够编写**与JIT有关的内容**，你可以把代码存储在变量中，然后使用`define code named codename with $LanguageType | $CodeInfo`来使变量代码化，之后你就可以通过`use code codename`来调用你的代码。**请注意，变量代码化对应的代码必须写在同一行内。**

#### 代码变量化

这并不是一个函数或者一个功能，这是一个特新。你定义的代码块可以像变量一样进行操作。例如`$codename`。
但是为了安全，我们不允许你对变量化的代码进行任何操作，防止你无意中破坏代码。如果需要对代码进行修改，你需要将代码输入给一个代码块进行处理，**并且此代码块不能是Coly**。

#### print

`print`是**新增内容**，在Aug 31, 2024的Coly文档中没有体现。
`print`能够让你输出一定的内容，具体用法如下
```Coly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
`print`会以空格分别内容，但不会使文章中的空格减少。在你调用完一个变量后，你需要使用空格来说明变量名已经结束。
如果你使用`print`，它会在输出玩内容后自动换行。
例如
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

`printwithoutanewline`是print的一种变体，用法与print完全相同，但是不会像print一样输出完后会自动换行。

#### do

`do`允许你直接在当前环境下将变量代码化并立即执行，可用于编写交互性的Coly程序。
`do`依赖于创建一个新的Coly处理线，并且单行执行，所执行的代码赋予的`fake_lineid`为`-1`，所以你无法定位处理的内容，也无法在InteractiveColy中写循环。

#### 功能变量

功能变量例如`Input`，能够让你在使用时进行一定的交互，使用之前需要提前声明。
例如`define var named Input`。

##### Input InputLine

`$Input`和`$InputLine`能够获取用户的输入，但`$Input`仅获取到下一个空格或换行，而`$InputLine`一直获取到换行。
请注意，`Input`只是可以参考C++中`cin`的用法，并不完全相同。在每次输入完之后，你需要回车而不是空格。
用法示例
```Coly
#初始定义两个变量
define var named Input
define var named InputLine
#分别对两个Input输入并展示结果
printwithoutanewline Input:
print $Input
#你必须在$Input之后使用下面的这一句，原因请参考C++的cin和getline之间的冲突问题
define var named NULL with $InputLine
printwithoutanewline InputLine:
print $InputLine
print
print Press Enter to continue...
#这里的NULL变量没有用到，无需处理。
define var named NULL with $InputLine
```

##### OnlyCompile

`OnlyCompile`有两种模式，未定义时默认是`false`。
如果`OnlyCompile`是`true`，并且启动的代码使用了`InitColySyncService()`，那么代码会在运行到这个宏时退出。（仅限C++）

##### NoReg

`NoReg`有两种模式，未定义时默认是`false`。
如果`NoReg`是`true`，启动代码块时不会为代码块注册，也就意味着代码块无法链接到ColyServer，也无法操作Coly变量，但是可以避免登陆凭证的无效注册，也可以提高代码块的运行速度。

##### Size

`Size`是一个用来获取var长度的一个功能变量，使用方法是先`define var named Size with ...`，然后调用时写`$Size`。
为了保证稳定，`Size`不会参与同步。请你在每次调用之前使用一次定义为`Size`赋值，但一次赋值的`Size`可以多次使用。
例如：
```cly
#这是一个简单的加法运算演示
define var named Input
define var named a with $Input
#请注意，如果你要创建空变量，请使用privatevar。为了流畅度考虑，空的var不会同步到server。
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

`ASCII`允许你为其赋值并获取该ASCII值对应的字符。请注意，你必须使用数字进行输入。
例如：
```cly
define var named ASCII with 65
print $ASCII
#应输出A
```

##### [Language]:

该系列的功能变量的作用是在内存中代替`LanguageMap.json`中的设置。因为是覆盖设置，所以你不能通过这个变量来读取设置。

###### [Language]:needcompile

该变量定义了你的语言需不需要进行编译，赋值应为`true`或`false`。
无论是解释型变量还是编译型变量，你都需要定义这个。

###### [Language]:extension

该变量定义了你的语言的扩展名形式。
如果你只是想用`[Language]:`系列变量调用系统命令，也请定义此变量。

###### [Language]:compilerun

该变量定义了你的语言编译并运行所需的命令。

###### [Language]:run

该变量定义了你的语言运行所需的命令。 **请注意，如果是解释型语言，请使用这个变量。如果你使用编译型变量，为了性能优化，你也需要定义这个变量。**

内容示例
```cly
#定义语言内容
define var named Call:needcompile with true
define var named Call:extension with .txt
define var named Call:run with start cmd
define var named Call:compilerun with start notepad
#你必须要定义一个代码块使用你定义的语言类型
define code named 1 with Call
#第一次调用notepad，使用compilerun
use 1
#调用cmd，因为Coly的性能优化，使用run
use 1
```

###### 代码文件参数

- `$`: 代表源代码文件的完整路径
- `^`: 代表源代码文件去掉扩展名之后的完整路径
- `*`: 代表`subprocessid`，支持Coly自动同步的变量依赖此进行subprocess的注册。

#### if ifn

`if`和`ifn`是**新增内容**。
`if`和`ifn`能够在Coly中判断两个变量是否相等。如果相等，`if`会执行后方的`code`，`ifn`则不会.若不相等则反之。
**请注意，`type`分别为`code`和`var`的变量不影响比较。其中比较的两个内容必须是变量，如果要和常量比较请先定义`privatevar`。**
用法
```Coly
if $var1 $var2 [Coly Code]
ifn $var1 $var2 [Coly Code]
```
其中`[Coly Code]`代表可以执行的Coly代码，仅限一行。
例如
```Coly
define var named 1 with 1
define var named 2 with 1
if $1 $2 print 1
```
结果就会输出1。

#### 库文件

库文件是**新增内容**，在Aug 31, 2024的文档中没有体现。
你可以通过引用库文件来使用别人已经提供的代码块。具体方法是`import lib ...`
在v0.3.0版本中引入了**ColyVariableSyncService**，并且你可以在交互状态下使用`import lib ...`
请注意，你无需在Windows下使用转义字符。

#### commitvaroperation

`commitvaroperation`允许你直接向**ColyVariableSyncService**发起请求，以便多元的开发流程。
**请注意如果遇到问题，你会收到错误信息，并且在Server的控制台/日志中会有体现。你需要自行解决错误信息带来的影响。在Coly中，我们已经加入了错误判断。**
用法
```Coly
#向ColyVariableSyncServer发起注册子进程的请求。
commitvaroperation reg subprocess 123
```

##### 所有的请求

下面是可用指令的树结构，使用方法是从树根节点开始向下延伸直到子节点**再加上备注的信息。**
如果你想要测试手动请求，你可以使用`commitvaroperation.cly`中测试。

- set
    - var *JSON，格式详见下方VarContainer说明*
    - process *JSON，格式详见下方ProcessContainer说明*
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
    - process *ProcessID，通常可用时间戳*
    - subprocess *SubprocessID*
- login
    - subprocess *SubprocessID*

下面是两个JSON的结构，你也可以通过Coly目录下的`client`来获取，具体方法是
```VariableSync
reg process test
set var {"Name":"1","Value":"1","Timestamp":1}
get var 1
get process
```

###### VarContainer的JSON结构

```JSON
{
    "Name": "VarName",
    "Value": "Your Var Value",
    "Timestamp": 123456
}
```

###### ProcessContainer的JSON结构

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

`VarID`是由Server生成的**不可逆向的**A-Z字符串。
- 注：Server使用`GXPass::num2ABC(GXPass::compile(data))`生成字符串，这种方法没有任何安全性，仅用于生成唯一识别码。在GXPass库中要求的指定版本否则会损失数据的提示并不会起作用，不用担心Coly的数据损失问题，因为Coly在下次更新时你需要关闭Coly，届时没有任何数据保存。


上面两个JSON结构中的`Timestamp`是当前操作的时间戳，需要上传的时候自定确定。时间戳会用于同步时解决冲突。但在Coly中你无法更改变量的一部分，**所以你只能给`Timestamp`指定固定值或者调用其他语言来修改该`Timestamp`。**

例如（其中123是变量的名字）
```VariableSync
get var 123
```

## C++语言

我们在v1.0.0版本中更新了针对于C++写出的运行库，你一共需要明白下面的两个宏、一个函数和一个类型。

### InitColySyncService();

这个宏需要被引入到main函数的第一行，会自动判定主进程的`ColyProcessID`，需要你引入argc和argv。如果启动环境不满足代码块的要求，会自动退出。
你也可以把`InitColySyncService()`代码粘贴到你的主函数进行自定义操作。
如果你在Coly或者其他地方中定义了`NoReg`并且其值为`true`，你的程序会在运行到这里时退出。
**请注意不要更改库文件以免引起兼容性问题**

### RegColyVar(varname);

这个函数可以让你定义一个`std::string`类型的变量，并且具有基本的`std::string`的操作。如果你发现有些操作没有被定义，你可以自行使用`varname.data.(std::string的操作)`，然后进行变量同步。其余情况下本类型可以和`std::string`混用。
**请注意，在注册之后，原先存在于Server上的数据会被覆盖！**

### ColySyncString

这是`ColyCppSyncLib`的自动同步的`std::string`类型的变量，包含了Coly主进程仅支持的`string`类型。
如果你要进行同步操作，请先给`ColySyncString`初始化和赋值，赋值直接使用赋值语句即可，适配`std::string`和`char*`。
不建议手动使用。

### sync_variable(*varname);

该函数可以自动同步你的变量到**ColySyncServer**。
传入的参数是`ColySyncString`类型的变量指针。
不建议手动使用。

### ReadColyVar(varname)

可以允许你从Server中以只读方式读取数据，之后你可以使用`varname`进行字符串操作。你可以通过隐式转换为`std::string`，之后可以赋值给`ColyVariable`，可以弥补`RegColyVar()`的缺陷。

例如：
```cpp
#include <iostream>
#include <string>
// 把这个头文件提前添加到环境变量或者使用绝对路径
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

我们在v1.0.0版本中更新了针对于Python写出的运行库，你一共需要明白下面的一个类型。

### ColySyncString

这是Python可以直接连接到Coly的变量，任何支持的操作都会自动同步数据。如果需要高性能操作，请使用string。

#### RegColyVar(varname)

这个函数能够使你注册一个名字为`varname`的`ColyVar`。
使用方法具体是`A = RegColyVar("A")`

#### ReadColyVar(varname)

这个函数能够使你读取已有的`ColyVar`。
请注意函数返回的类型不是`string`，是一个只读类型，修改不会影响`ColyServer`中的值。

### InitColySyncService(argv)

这个函数可以使你连接到`ColyServer`并进行变量操作。


例如：
```py
import sys
argv=sys.argv
if len(argv)<2 :
  exit()
if InitColySyncService(argv)==0 :
  exit()  # OnlyCompile的兼容
A = RegColyVar("A")
A = "Hello Coly Server!"
```