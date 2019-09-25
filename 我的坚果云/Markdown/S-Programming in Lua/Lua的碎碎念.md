##语言设定
解释型语言的特征不在于它们是否被编译，而是编译器是语言运行时的一部分。

table和userdata可以有各自独立的元表，而其他类型的值则共享其类型所属的单一元表。

Lua的字符串是不可变的值。

只能对两个数字或两个字符串作大小性比较。
对于table、userdata和函数，Lua是作引用比较的。比如：

	a={}
	b={}
	print(a==b)
	结果是false

对于操作符and来说，如果它的第一个操作数为假，就返回第一个操作数，不然返回第二个操作数。
对于操作符or来说，如果它的第一个操作数为真，就返回第一个操作数，不然返回第二个操作数。

table的语法糖：
```
a={}
a.x=10
a.y=11

a={}
a["x"]=10
a["y"]=11

a={x=10,y=11}

a={["x"]=10,["y"]=11}


b={"r","g","b"}

b={[1]="r",[2]="g",[3]="b"}

b={}
b[1]="r"
b[2]="g"
b[3]="b"
```

进行函数调用时，函数若只有一个参数，并且此参数是一个字面字符串或table构造式，那么圆括号便是可有可无的。

若一个函数写在另一个函数之内，那么这个位于内部的函数便可以访问外部函数中的局部变量，这项特征称为词法域。
Lua将每个程序块都作为一个函数来处理。

2.5.5 – The Length Operator
The length operator is denoted by the unary operator #. The length of a string is its number of bytes (that is, the usual meaning of string length when each character is one byte).
The length of a table t is defined to be any integer index n such that t[n] is not nil and t[n+1] is nil; moreover, if t[1] is nil, n can be zero. For a regular array, with non-nil values from 1 to a given n, its length is exactly that n, the index of its last value. If the array has “holes” (that is, nil values between other non-nil values), then #t can be any of the indices that directly precedes a nil value (that is, it may consider any such nil value as the end of the array).
总之，对于不连续的table，#并不一定会取得table的长度，可以用table.maxn返回table的最大正索引数。

##编译问题
centos默认安装的lua是动态编译的，运行需要动态库liblua.so。
官网提供的编译lua的方式是静态编译，make完后会产生lua，luac，liblua.a三个文件。
luac用来将lua源文件编译成中间文件。
lua是解释器。
liblua.a是c调用lua时的静态库。

编译lua的时候会提示
gcc -std=gnu99 -O2 -Wall -Wextra -DLUA_COMPAT_5_2 -DLUA_USE_LINUX    -c -o lua.o lua.c
lua.c:82:31: error: readline/readline.h: No such file or directory
lua.c:83:30: error: readline/history.h: No such file or directory
通过yum install readline-devel安装readline库之后，就可以在/usr/include/readline/下找到这些头文件了。

c调用lua的时候，如果用gcc编译，需要包含lua.h、lualib.h、lauxlib.h三个头文件（可选），如果用g++编译，需要包含lua.hpp，lua.hpp的内容如下：
```
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
```
这样编译：
gcc main.c -llua -lm -ldl 
g++  main.cpp -llua -ldl

liblua.a需要数学库和动态链接库，g++会自动装载数学库。-llua必须放在main.c后面，不然ld就不会对liblua.a进行链接！

> 原来ld对于链接一系列的库的顺序是很敏感的，不然会报undefined referenced 的函数符号错误，意思就是未找到函数定义。实际上库是能正确打开的。如果库libA.a依赖于库libB.a，那么连接器的参数应该ln -lA -lB，必须这样写。不然就会错误。