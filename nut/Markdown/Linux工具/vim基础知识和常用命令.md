本篇主要内容是vim的基本知识，常用命令等，不涉及需要额外安装的插件。

Vim的相关路径
/usr/bin/vim
/usr/share/vim/vim/vim63/

配置文件
/etc/vimrc，此文件影响整个系统的Vim。
~/.vimrc，此文件只影响本用户的Vim。
~/.vimrc文件中的配置会覆盖/etc/vimrc中的配置。

Vim的插件（plugin）安装在Vim的runtimepath目录下，你可以在Vim命令行下运行`set rtp`命令查看。

vim（visual improved）有normal、insert、visual、ex（command）四种模式。

insert模式
-
从normal模式进入insert模式：

	i insert, 在当前字符的左边插入
	a append, 在当前字符的右边插入
	I 在当前行首字符(非空白字符)的左边插入
	A 在当前行尾字符的右边插入
	o 在下一行插入
	O 在上一行插入
	s substitute, 删除当前字符然后进入插入模式
	S 删除当前行然后进入插入模式
通过c命令进入insert模式在下节介绍。

常用指令
-
指令|说明
-|-
**文件打开和关闭**|
vi +3 file|打开file并移动到第三行
vi + file|打开file并移动到最后一行
vi + /pattern file|这将使光标移动到第一个满足pattern的位置
vi -R file|以只读模式打开
vim -d file1 file2|以对比模式(diff)打开文件
vim -o file1 file2|打开横向水平窗口
vim -O file1 file2|打开纵向垂直窗口
:q|quit
:w|write
:x|write and quit
:qa|退出所有的
:e!|取消所有的更改，回到最初的模样
**光标行内移动**|
fx|移动光标到当前行的下一个字符x处
Fx|移动光标到当前行的上一个字符x处
tx|移动光标到当前行的下一个字符x左边的位置
;|重复上面的移动
,|反方向重复上面的移动
0|移动光标到当前行行首
^|移动光标到当前行第一个非空白字符处
$|移动光标到当前行行尾
**光标跨行跳转**|
%|跳转到配对的括号
[[|移动光标到上一个行首是{的行
]]|移动光标到下一个行首是{的行
w|移动光标到下一个词的头部，以标点和空白分隔
W|同上，但以空白分割，下面的E, B类同
e|移动光标到当前词或下一个词的尾部
b|移动光标到当前词或上一个词的头部
''|两个单引号，移动光标到光标上次停靠的地方
gd|移动光标到当前光标所在处函数或变量定义的地方
gf|跳转到相应的头文件(可通过`:set path+=xxx`来添加头文件目录xxx)
K|在man里查找当前光标所在处的词
ctrl-b|向上移动一屏
ctrl-f|向下移动一屏
ctrl-u|向上移动半屏
ctrl-d|向下移动半屏
ctrl-e|屏幕向上移动一行
ctrl-y|屏幕向下移动一行
gg|移动光标到文件首行
G|移动光标到文件末行
nG或:n|移动光标到第n行
`n\|`|移动到第n列
H|移动光标到当前屏首行
M|移动光标到当前屏中间行
L|移动光标到当前屏末行
3H|使光标移动到当前屏幕的首行的下数第三行
*|移动光标到下一个光标当前所在位置的字符串出现的位置
\#|移动光标到上一个光标当前所在位置的字符串出现的位置|
shift-{|上一个空行
shift-}|下一个空行
50%| 跳转到文档50%处
**搜索和替换**|
/pattern|搜索pattern
?pattern|反向搜索pattern
n|在同一方向重复上一次搜索命令
N|在反方向上重复上一次搜索命令
:s/old/new/g|在当前行中将old替换为new，g表示global，替换当前行的所有匹配
:m,ns/old/new/g|在m到n行中进行替换
:%s/old/new/g|全文替换，最后一项如加上c表示confirm，需要确认替换操作
**编辑**|
dd|delete, 删除当前行
cc|change, 删除当前行并进入插入模式
dw|删除下一个词，当前位置到下一个词的开始位置，前闭后开
cw|删除下一个词并进入插入模式，当前位置到下一个词的开始位置，前闭后开
D|删除从当前光标位置到行尾
C|删除从当前光标位置到行尾并进入插入模式
x|删除当前字符
X|删除前一个字符
yy|yank，复制当前行
yj/y1j/2yy | 复制两行
y$ |复制到行尾
p|put，在当前位置后粘贴
P|在当前位置前粘贴
]p |粘贴且正确缩进
r|replace，修改光标所在字符
R|进入替换模式，相当于按两次insert键
`"a`| 两个单引号，接下来的yank或paste操作使用使用寄存器a
`"ayy`|当前行复制到寄存器a中
~|转换大小写
g~iw |当前word切换大小写
gUiw |当前word大写
guiw |当前word小写
gU回车| 当前行大写
**代码相关**|
ctrl-p| 上一个补全
ctrl-n |下一个补全
ctrl-e|停止补全并回到原来录入的文字
ctrl-y|停止补全，并接受当前所选的项目
\>>|右移本行
<<|左移本行
==|缩进本行
:!cmd|执行shell命令
:r!date|将当前时间插入
gg=G|格式化整个文件
zf|生成折叠
zo |打开光标下的折叠
zO |循环打开光标下的折叠，也就是说，如果存在多级折叠，每一级都会被打开
zc |关闭光标下的折叠
zC | 循环关闭光标下的折叠 
[z | 到当前打开折叠的开始
]z | 到当前打开折叠的结束
zj | 向下移动到下一个折叠的开始处
zk | 向上移动到上一个折叠的结束处 
**标记和宏**|
ma|在当前光标的位置标记一个标记，名字为a
'a|单引号，到书签a处
'.|到上次编辑文件的地方
:marks|查看标记列表
:delmarks|可以删除指定标记
qa|开始录制宏a，按下q结束录制
@a|执行宏a
**其他**|
ctrl-g|文章行数和当前位置占总行数的百分比
g ctrl-g|文档字数统计
:ab mail y@qq.com|以后输入mail再按空格就会替换为y@qq.com
J|join，将上下两行合并，删除两行之间的换行符
u|撤销
.|repeat
z 回车|使当前行成为屏幕首行
z.|使当前行成为屏幕中间行
z-|使当前行成为屏幕尾行
**多窗口**|
ctrl-w s |split window
ctrl-w v |split window vertically
ctrl-w w|switch window
ctrl-w q|quit a window
**多标签**|
:tabe filename|edit file in new tab
gt|next tab
gT|previous tab
:tabr|first tab
:tabl|last tab
:tabm n|move current tab after tab n

visual模式
-
从normal进入visual模式：

	v 按字符选择
	V 按行选择
	ctrl-v 按块（矩形）选择

可视模式下，可以使用光标移动指令来选择字符，然后可对所选区域执行删除、复制等命令，以及以下命令：

	U 大写所选区域字母
	u 小写所选区域字母
	~ 更改所选区域字母大小写
	o 移动到标记区域的另一边
	O 移动到标记区域的另一角
	< 左移所选区域
	> 右移所选区域
	= 格式化所选区域

i和a修饰符
-

	i : 内部
	a : 周围
	iw : 单词内（不包括单词周围的空格）
	aw : 单词周围（包括单词周围的空格）
	
	i< : 选择一对“<>”中的所有字符
	i{ : 选择一对“{}”中的所有字符
	i[ : 选择一对“[]”中的所有字符
	i( : 选择一对“()”中的所有字符
	it : 选择一对html标签内部的所有字符
	i" : 选择一对双引号中的所有字符
	i' : 选择一对单引号中的所有字符
例如：visual模式下输入iw会选中光标所在单词，normal模式下`daw`会删除该单词和单词周围的空格。

特别的，如果光标当前位置在双引号处于同一行并在双引号前面，只需要输入`ci”`就可以删除双引号内部的所有内容，并将光标移到双引号中间进入插入模式。对于单引号和HTML标签也有效。

ex模式
-
ex（还是/还有ed？）是Unix下的一个文本编辑器，vi就是ex的“visual mode”，以冒号开头的命令，都属于ex命令。
早期的显示器只有按行显示的功能，当你希望显示某行，输入命令，显示器上才会显示那行。而控制显示器显示文本的命令行工具，就是ex。
对于大段文本的操作，使用ex命令更方便。
ex的命令都是基于行的，获得行号有下面几种方法：

	3 直接输入行号
	3,6 行范围, 前闭后闭
	. 当前行
	/pattern/ 满足pattern的第一行
	$ 最后一行
	% 每一行
部分命令举例：
命令|说明
-|-
:3|显示第三行
:3,6d|删除3到6行
:3,6m14|将3到6行放到14行的位置
`:.,.+3d`|删除当前行及后面3行，共4行
:/pattern/d|删除包含pattern的第一行
`:/pattern1/,/pattern2/d`|删除从pattern1到pattern2的行
:3,6w another|将3-6行另存为文件another
:9,13w >> another|将9-13行追加到文件another后面
:r another|将another文件的内容追到到当前文件光标后
:3r another|将another文件的内容追到到当前文件第3行后
:1,3co4|将第1到3行复制到第4行后
:1,3t4|将第1到3行复制到第4行后
:1,3 y/ya | 复制1到3行到剪切板中
:[range] y/ya name| 复制[range]到剪切板中name中
:n pu | 把剪切板中的内容粘贴到第n行下面
:n pu name| 把剪切板中名字为name的拷贝粘贴到第n行下面

tips
-
`''`，`'.`，`'a`中的引号可以是单引号，也可以是反引号，不同之处在于反引号在跳转时会精确到列，而单引号不会回到跳转时光标所在的那一列，而是把光标放在第一个非空白字符上。

`''`只能回到上次跳转位置上，如果想回到更老的跳转位置，使用命令`ctrl-o`，与它相对应的，是`ctrl-i`，它跳转到更新的跳转位置。这两个命令前面可以加数字来表示倍数。使用命令`:jumps`可以查看跳转表。

如果文件没有修改:x不会修改文件的时间戳，因此就不需要重新编译。而:wq命令，即使文件未修改也会修改时间戳，也即会触发重新编译。

通过`/\<word\>`来匹配整个词，而不是词的一部分，*和#就是这样查找的。

在编辑模式下，可以通过Ctrl+T手动增加本行缩进，Ctrl+D手动减少本行缩进。

通过`:!sh`来打开一个新的解析器，这样就省去了通过ex一行一行输入命令的时间，可以在使用完sh后，通过Ctrl+D回到vim。

使用"q/"和"q?"命令，在vim窗口最下面打开一个新的窗口，这个窗口会列出你的查找历史记录，你可以使用任何vim编辑命令对此窗口的内容进行编辑，然后再按回车，就会对光标所在的行的内容进行查找。

在normal模式下按下Ctrl+z组合键，vim就会被丢到后台执行，fg切换后台任务。

ctags
-
vim默认安装了ctags。它遍历源代码文件生成tags文件，这些tags文件能被编辑器或其它工具用来快速查找定位源代码中的符号（tag/symbol），如变量名，函数名等。

进入项目代码根目录，在shell里执行以下命令：
ctags -R
会在源代码目录生成tags文件，Vim默认会自动读取当前目录下的tags文件。可以在vi中输入变量名来查找tag，如：
:tag zgame
将查找包含变量名为zgame的符号。
也可以在编辑模式下，在函数或变量上面按下ctrl+]，光标将自动跳转到他们的定义处，按ctrl-t，则回到上次跳转前的位置。

也可以在vim里设置tags文件的位置：
:set tags=~/vim63/tags

my vimrc
-
```
" Sets how many lines of history VIM has to remember
set history=60

" display line number
set number

" Enable syntax highlighting
syntax enable

colorscheme desert
set background=dark

let mapleader = ","
let g:mapleader = ","

" Disable highlight when <leader><cr> is pressed
map <silent> <leader><cr> :noh<cr>

" Smart way to move between windows
map <C-j> <C-W>j
map <C-k> <C-W>k
map <C-h> <C-W>h
map <C-l> <C-W>l

" Enable filetype plugins
filetype plugin on
filetype indent on

" Set 5 lines to the cursor - when moving vertically using j/k
set so=5

" Ignore case when searching
set ignorecase

" When searching try to be smart about cases
" 小写字符忽略大小写,有大写字符则精确匹配
set smartcase

" Highlight search results
set hlsearch

" Makes search act like search in modern browsers
" 显示实时搜索效果
set incsearch

" Use spaces instead of tabs
set expandtab

" Be smart when using tabs
set smarttab

" 1 tab == 4 spaces
set shiftwidth=4
set tabstop=4

set ai "autoindent
set si "Smart indent

" 文件路径 格式 文件类型 ascii码 行列 百分比
set statusline=%F\ [%{&ff}]\ [%Y]\ [\%b/0x\%B]\ (%l,%v)[%p%%]

" 这个命令会确保你的状态条总会显示在窗口的倒数第二行
set laststatus=2

" 高亮匹配括号对，这样当你输入右括号时，光标会暂时跳转到左括号处闪烁
set showmatch

" 简单的调试用
namp <F10> osystem("echo \"\" >> ~/my.log");<Esc>16hi
iamp <F10> system("echo \"\" >> ~/my.log");<Esc>16hi

" 备份文件的保存位置
set backupdir=/tmp

" 解决一些乱码问题
set fileencodings=utf-8,gbk

" 添加一些头文件的目录,使'g f'命令更好用
set path=.,/usr/include,一些自定义目录
```

参考
-
全面介绍
http://blog.csdn.net/liuyuan185442111/article/details/51471093
ex的介绍
http://csprojectedu.com/2016/01/27/VimInAction-4/
一个vimrc配置文件
http://amix.dk/vim/vimrc.html