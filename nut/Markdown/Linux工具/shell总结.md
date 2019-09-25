主要是语法层面，不过多关注详细命令。主要是给自己参考用。
## 变量
### 只读变量
使用 readonly 命令可以将变量定义为只读变量，只读变量的值不能被改变。
```bash
rword="hello"
echo ${rword}
readonly rword
rword="bye" #运行时报错
```
### 删除变量
使用 unset 命令可以删除变量。unset 命令不能删除只读变量。
```bash
dword="hello"
echo ${dword}
# Output: hello
unset dword
echo ${dword}
# Output: (空)
```
### 变量类型
- **局部变量** - 局部变量只有在变量所在的代码块或者函数中才可见，需要使用 local 声明；
- **全局变量** - 用户自定义的普通变量默认是全局变量，可以在本文件中的其它位置引用；
- **环境变量** - 所有的程序（包括shell启动的程序）都能访问环境变量。如果一个shell脚本设置了环境变量，需要用 export 命令来通知脚本的环境。

环境变量 `$RANDOM` 表示一个 0 到 32767 之间的随机整数
## 字符串
### 单引号和双引号
shell 字符串可以用单引号 `''`，也可以用双引号 `""`，也可以不用引号。
- `''` ：单引号里的任何字符都会原样输出，单引号中对变量引用是无效的，且单引号中不能出现单引号；
- `""`：双引号里可以引用变量，可以出现转义字符。
### 获取字符串长度
```bash
text="12345"
echo ${#text}
# Output:
# 5
```
### 截取子串
${string:position}	在string中, 从位置position开始提取子串
${string:position:length} 在string中, 从位置position开始提取长度为length的子串
### 查找子串
```bash
#!/usr/bin/env bash

text="hello"
echo `expr index "${text}" ll`

# Output: 3
```
expr 的其他几种用法：
```bash
> expr length "this is a test"
14
> expr substr "this is a test" 3 5
is is
 > expr 14 % 9
 5
 > expr 30 \* 3
 90
 ```
### 删除子串
表达式 |含义
:-|:-
${string#substring}|从string的**开头**，删除**最短**匹配substring的子串
${string##substring}|从string的**开头**，删除**最长**匹配substring的子串
${string%substring}|从string的**结尾**，删除**最短**匹配substring的子串
${string%%substring}|从string的**结尾**，删除**最长**匹配substring的子串
注：substring 可以是正则表达式。
### 替换子串
表达式|含义
:-|:-
${string/substring/replacement}|使用 replacement 来代替第一个匹配的 substring
${string//substring/replacement}|使用 replacement 代替所有匹配的 substring
${string/#substring/replacement}|如果 string 的前缀匹配 substring，那么就用 replacement 来代替匹配到的 substring
${string/%substring/replacement}|如果 string 的后缀匹配 substring，那么就用 replacement 来代替匹配到的 substring
## 数组
bash 只支持一维数组。数组下标从 0 开始，下标可以是整数或算术表达式，其值应大于或等于 0。
### 创建数组
```bash
# 创建数组的不同方式
nums=([2]=2 [0]=0 [1]=1)
colors=(red yellow "dark blue")
arr_name[0]=value1
arr_name[1]=value2
arr_name[23]=value3
```
### 获得数组长度
```bash
echo ${#nums[*]}
# Output: 3

echo ${#nums[@]}
# Output: 3
```
### 访问数组元素
- **访问数组的单个元素：**
```bash
echo ${nums[1]}
# Output: 1
```
- **访问数组的所有元素：**
```bash
echo ${colors[*]}
# Output: red yellow dark blue

echo ${colors[@]}
# Output: red yellow dark blue
```
上面两行有很重要（也很微妙）的区别，为了将数组中每个元素单独一行输出，我们用 printf 命令：
```bash
printf "+ %s\n" ${colors[*]}
# Output:
# + red
# + yellow
# + dark
# + blue
```
为什么 dark 和 blue 各占了一行？尝试用引号包起来：
```bash
printf "+ %s\n" "${colors[*]}"
# Output:
# + red yellow dark blue
```
现在所有的元素都在一行输出 —— 这不是我们想要的！让我们试试 `${colors[@]}`：
```bash
printf "+ %s\n" "${colors[@]}"
# Output:
# + red
# + yellow
# + dark blue
```
在引号内，`${colors[@]}` 将数组中的每个元素扩展为一个单独的参数；数组元素中的空格得以保留。
- **访问数组的部分元素：**
```bash
echo ${nums[@]:0:2}
# Output: 0 1
```
### 向数组中添加元素
```bash
colors=(white "${colors[@]}" green black)
echo ${colors[@]}
# Output:
# white red yellow dark blue green black
```
### 从数组中删除元素
用 unset 命令来从数组中删除一个元素：
```bash
unset nums[0]
echo ${nums[@]}
# Output: 1 2
```
## 控制语句
### 循环控制
for 循环
```bash
for arg in elem1 elem2 ... elemN
do
  # 语句
done
```
在每次循环的过程中，`arg` 依次被赋值为从 `elem1` 到 `elemN` 。这些值还可以是通配符或者大括号扩展。
当然，我们还可以把`for`循环写在一行，但这要求`do`之前要有一个分号，就像下面这样：
```bash
for i in {1..5}; do echo $i; done
```
也可以像 c 语言那样使用 `for`，比如：
```bash
for (( i = 0; i < 10; i++ )); do
  echo $i
done
```
while 循环
```bash
while [[ condition ]]
do
  # 语句
done
```
until 循环
```bash
x=0
until [[ ${x} -ge 5 ]]; do
  echo ${x}
  x=`expr ${x} + 1`
done
```
### select
select 帮助我们组织一个用户菜单：
```bash
select answer in elem1 elem2 ... elemN
do
  # 语句
done
```
select 会打印 elem1..elemN 以及它们的序列号到屏幕上，之后会提示用户输入。提示符是 `$PS3`。用户的选择结果会被保存到answer 中。
### break 和 continue
如果想提前结束一个循环或跳过某次循环执行，可以使用 shell 的 `break` 和 `continue` 语句来实现。它们可以在任何循环和 select 中使用。
## 函数
bash 函数定义语法如下：
```bash
[function] funname [()] {
    action;
    [return int;]
}
```
> 说明：
> 1.函数定义时，`function` 关键字可有可无。
> 2.函数返回值类型只能为整数（0-255）。如果不加 return 语句，shell 默认将以最后一条命令的运行结果，作为函数返回值。
> 3.函数返回值在调用该函数后通过 `$?` 来获得。
> 4.所有函数在使用前必须定义。

位置参数是在调用一个函数并传给它参数时创建的变量。
变量| 描述
-|-
`$1 … $9`|第 1 个到第 9 个参数 
`${10} … ${N}`|第 10 个到 N 个参数
`$*` or `$@`|所有位置参数
`$#`|位置参数的个数
`$FUNCNAME`|函数名称（仅在函数内部有值）

:keyboard: 示例：
```bash
calc(){
  PS3="choose the oper: " # 选择菜单的提示符
  select oper in + - \* / # 生成操作符选择菜单
  do
    echo -n "enter first num: " && read x # 读取输入参数
    echo -n "enter second num: " && read y # 读取输入参数
    case ${oper} in
      "+")
        return $((${x} + ${y}))
        ;;
      "-")
        return `expr ${x} - ${y}`
        ;;
      "*")
        return `expr ${x} \* ${y}`
        ;;
      "/")
        return $((${x} / ${y}))
        ;;
      *)
        echo "${oper} is not support!"
        return 0
        ;;
    esac
    break
  done
}
calc
echo "the result is: $?" # $? 获取 calc 函数返回值
```
## Shell 扩展
### 大括号扩展
大括号扩展让生成任意的字符串成为可能。它跟文件名扩展很类似，举个例子：
```bash
echo beg{i,a,u}n # begin began begun
```
大括号扩展还可以用来创建一个可被循环迭代的区间：
```bash
echo {0..5} # 0 1 2 3 4 5
echo {00..8..2} # 00 02 04 06 08 00到8, 2为步长
```
### 命令置换
命令置换允许我们对一个命令求值，并将其值置换到另一个命令或者变量赋值表达式中。当一个命令被``或`$()`包围时，命令置换将会执行。举个例子：
```bash
now=`date +%T`
now=$(date +%T)
echo $now
```
### 算数扩展
在 bash 中，执行算数运算是非常方便的。算数表达式必须包在`$(( ))`中。算数扩展的格式为：
```bash
result=$(( ((10 + 5*3) - 7) / 2 ))
echo $result
```
在算数表达式中，使用变量无需带上`$`前缀：
```bash
x=4
y=7
echo $((x + y))     # 11
echo $(( ++x + y++ )) # 12
echo $((x + y))     # 13
```
### 单引号和双引号
单引号和双引号之间有很重要的区别。在双引号中，变量引用或者命令置换是会被展开的。在单引号中是不会的。举个例子：
```bash
echo "Your home: $HOME" # Your home: /Users/<username>
echo 'Your home: $HOME' # Your home: $HOME
```
当局部变量和环境变量包含空格时，它们在引号中的扩展要格外注意。举个例子：
```bash
INPUT="A string  with   strange    whitespace."
echo $INPUT   # A string with strange whitespace.
echo "$INPUT" # A string  with   strange    whitespace.
```
调用第一个 `echo` 时给了它 5 个单独的参数 —— `$INPUT` 被分成了单独的词，`echo` 在每个词之间打印了一个空格。第二种情况，调用 `echo` 时只给了它一个参数（整个$INPUT 的值，包括其中的空格）。
## 括号
小括号中的命令将会新开一个子 shell 顺序执行，括号中多个命令之间用分号隔开，最后一个命令可以没有分号，各命令和括号之间不必有空格。

[ 是 bash 的内部命令，和 test 等同。这个命令把它的参数作为比较表达式或者作为文件测试，并且根据比较的结果来返回一个退出状态码

[[ 是 bash 程序语言的关键字。并不是一个命令，[[ ]] 结构比 [ ] 结构更加通用。使用 [[ ... ]] 条件判断结构，而不是 [ ... ]，能够防止脚本中的许多逻辑错误。比如，&&、||、< 和 > 操作符能够正常存在于 [[ ]] 条件判断结构中，但是不能出现在 [ ] 结构中。
## 重定向
operator|description
-|-
`>`|重定向输出
`2>`|重定向错误输出
`&>`|重定向输出和错误输出
`&>>`|以附加的形式重定向输出和错误输出
`<`|重定向输入
`<<`|[Here 文档](http://tldp.org/LDP/abs/html/here-docs.html)，从多行输入
`<<<`|[Here 字符串](http://www.tldp.org/LDP/abs/html/x17837.html) ，从字符串输入

冒号是bash的一个内置命令，它什么效果都没有，<<是输入重定向，两个EOF（可用其它特殊成对字符替代）之间的内容通过<<输入给冒号（:），就相当于注释了：
```bash
:<<EOF
echo '这是多行注释'
echo '这是多行注释'
echo '这是多行注释'
EOF
```
## Debug
如果想采用 debug 模式运行某脚本，可以在其 shebang 中使用一个特殊的选项：

	#!/bin/bash options
options 是一些可以改变 shell 行为的选项。比如 `-v` 表示 verbose ，在执行每条命令前，向 `stderr` 输出该命令 ，`-x` 表示 xtrace，在执行每条命令前，向 `stderr` 输出该命令以及该命令的扩展参数。
有时我们值需要 debug 脚本的一部分。这种情况下，使用 `set` 命令会很方便。这个命令可以启用或禁用选项。使用 `-` 启用选项，`+` 禁用选项：
```bash
# 开启 debug
set -x
for (( i = 0; i < 3; i++ )); do
  echo ${i}
done
# 关闭 debug
set +x
for i in {1..5}; do printf ${i}; done
printf "\n"
```
## 参考
[一篇文章让你彻底掌握 shell 语言](https://github.com/dunwu/linux-tutorial/blob/master/docs/lang/shell.md)
[Bash Shell编程入门](https://www.jianshu.com/p/e1c8e5bfa45e)
[shell中的括号（小括号，中括号，大括号）](https://blog.csdn.net/tttyd/article/details/11742241)
