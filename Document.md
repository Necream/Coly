# Coly

This document was edited in **Chinese**.

---

## Coly语言

一个**简单易学**的编程语言，支持再一段代码中引用其他编程语言来便利初学者，这样你就可以充分发挥各个编程语言的长处来**节约代码量**。
Coly采用了语言逻辑而非数学逻辑，使代码可读性更高。但**缺点是代码的编写简易程度降低**。
**Coly会长期更新**，并且每次更新都会保证一定的兼容性。

### 特性

- Coly会自动同步你的代码中的全局变量，这依赖对代码进行分析。请注意，你的部分自定义类型的变量不会进行同步，因为我们无法确定你的类型怎样进行同步。**在Coly v0.1.0中不可用**
- 我们会默认支持C++和Python，如果你有其他语言变量同步的需求，请你自行编写同步程序或者在网络上搜索可用的程序。**但请注意，如果你使用来自网络上的程序，我们无法保证你的电脑和代码的安全性。如果你不信任开发者，请你备份好你的代码。**
- **InteractiveColy中的import因为Coly的性能优化不可用！**

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
#请注意，这里的语法与Aug 31, 2024时的文档中有所不同。
use code 1
```

---

### 基础语法

#### 注释

Coly注释的语法与Python一致，**但不支持Python的多行注释**，**且**注释必须注释一整行，且`#`**必须位于行首**且**之前不能存在空格**。
#### define

`define`能够让你定义一定的内容，可以是`code`或者`var`，并且存储在你指定名字的变量中。`define`允许你重复定义，但是**重复定义的内容会以新内容存储**。
在上面给出的`示例 1`中，定义了一个代码块名为1，使用C++编程语言，继承上文中使用的所有全局变量。
**你为变量起名时无需注意任何内容，非英文，emoji均可，无空格即可，因为Coly会依照空格区分内容。请注意，你的变量名不能以其他变量的内容明明，`$`会被识别成变量名字的一部分。**
在`define`的内容中，你需要在每一行前加上`|`来区分内容。**请注意，你必须加在行首，这里Coly不会自动忽略你的空格。**

#### use

`use`能够让你使用一定的`code`，使用时如`示例 1`所示。
**`use`会保持当前Coly的读取进度继续运行。**

#### jump

`jump`能够让你使用一定的`code`，使用时如`示例 1`所示。
**`jump`会使Coly跳跃到定义处继续运行。**

#### 调用变量

你可以使用`$varname`来调用你的变量，其中`varname`是你为变量取的名字。
**请注意Coly没有其他语言标准意义上的变量，如果需要变量处理建议引用其他语言。Coly中的变量均以string存储，不支持任何运算。**
**在代码块之间进行变量同步时，不会导致变量类型改变。**

#### 变量代码化 *在Coly v0.1.0中暂不支持*

这个功能使你能够编写**与JIT有关的内容**，你可以把代码存储在变量中，然后使用`define code named codename with $LanguageType | $CodeInfo`来使变量代码化，之后你就可以通过`use code codename`来调用你的代码。**请注意，变量代码化对应的代码必须写在同一行内。**

#### 代码变量化

这并不是一个函数或者一个功能，这是一个特新。你定义的代码块可以像变量一样进行操作。例如`$codename`。
但是为了安全，我们不允许你对变量化的代码进行任何操作，防止你无意中破坏代码。如果需要对代码进行修改，你需要将代码输入给一个代码块进行处理，**并且此代码块不能是Coly**。

#### print

`print`是**新增内容**，在Aug 31, 2024的Coly文档中没有体现。
`print`能够让你输出一定的内容，具体用法如下
```coly
define var named varname with Hello, Coly!
print $varname
print Hello, World!
```
`print`会以空格分别内容，但不会使文章中的空格减少。在你调用完一个变量后，你需要使用空格来说明变量名已经结束。
如果你使用`print`，它会在输出玩内容后自动换行。

#### printwithoutanewline

`printwithoutanewline`是print的一种变体，用法与print完全相同，但是不会像print一样输出完后会自动换行。

#### do

`do`允许你直接在当前环境下将变量代码化并立即执行，可用于编写交互性的Coly程序。

#### \$Input和\$InputLine

这是两个相似的变量，使用之前需要提前声明。
这两个变量具有特殊的功能，`$Input`和`$InputLine`能够获取用户的输入，但`$Input`仅获取到下一个空格或换行，而`$InputLine`一直获取到换行。
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

#### if

`if`是**新增内容**。
`if`能够在Coly中判断两个变量是否相等。**请注意，`type`分别为`code`和`var`的变量不影响比较。**
用法
```coly
if $var1 $var2 codename
```

#### 库文件

库文件是**新增内容**，在Aug 31, 2024的文档中没有体现。
你可以通过引用库文件来使用别人已经提供的代码块。具体方法是者`import lib ...`