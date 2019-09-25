Windows下的IDE有一些有用的功能，比如sourceinsight里shift-F8高亮符号，ctrl-m查看书签，codeblocks里F11切换源文件和头文件，快捷的代码补全，源文件symbol窗口，这里安装一些插件来使vim实现这些功能，这些插件直接拷贝到.vim/plugin目录就行了。
符号高亮
-
mark.vim 
http://www.vim.org/scripts/script.php?script_id=1238

	<Leader>m  标记或取消光标所在符号的高亮标记
	<Leader>n  如果光标所在处的符号已高亮，清除当前词的高亮，否则清除所有符号的高亮
标记加强
-
ShowMarks
http://www.vim.org/scripts/script.php?script_id=152

在设定了一个标记后，它就会在你的vim窗口中显示出标记的名字并高亮该行。
配置：
```
""""""""""""""""""""""""""""""
" showmarks setting
""""""""""""""""""""""""""""""
" Enable ShowMarks
let showmarks_enable = 1
" Show which marks
let showmarks_include = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
" Hilight lower & upper marks
let showmarks_hlline_lower = 1
let showmarks_hlline_upper = 1
" Ignore help, quickfix, non-modifiable buffers
let showmarks_ignore_type = "hqm"
```
首先，使能showmarks插件，然后定义showmarks只显示全部的大写和小写标记，并高亮这两种标记，对文件类型为help、quickfix和不可修改的缓冲区，则不显示标记的位置。

ShowMarks已经定义了一些快捷键：
```
<Leader>mt   - 打开/关闭ShowMarks插件
<Leader>mo   - 强制打开ShowMarks插件
<Leader>mh   - 清除当前行的标记
<Leader>ma   - 清除当前缓冲区中所有的标记
<Leader>mm   - 在当前行打一个标记，使用下一个可用的标记名 
```

Marks Browser : A graphical marks browser
http://www.vim.org/scripts/script.php?script_id=1706

用`:MarksBrowser`命令打开浏览窗口，回车前往某个标记，按d键删除标记。
如果要保持标记浏览窗口持续打开，在vimrc中添加：
```
let marksCloseWhenSelected = 0 
""""""""""""""""""""""""""""""
" markbrowser setting
""""""""""""""""""""""""""""""
nmap <silent> <leader>mk :MarksBrowser<cr>
```
头文件跳转
-
a.vim
http://www.vim.org/scripts/script.php?script_id=31
在源文件和头文件间进行切换

```
:A  源文件头文件切换
:AS 上下分割窗口后切换
:AV 垂直分割窗口后切换
:AT 新建标签后切换

将光标所在处单词作为文件名打开
:IH  切换至光标所在文件
:IHS 分割窗口后切换至光标所在文件
:IHV 垂直分割窗口后切换
:IHT 新建标签后切换
```
symbol窗口
-
taglist.vim : Source code browser (supports C/C++, java, perl, python, tcl, sql, php, etc) 
http://www.vim.org/scripts/script.php?script_id=273

下载zip文件并解压，将taglist.vim放入plugin目录，taglist.txt放入doc目录，进入doc目录，在vim中执行`:helptags`命令添加帮助文件。
```
let Tlist_Show_One_File = 1         "不同时显示多个文件的tag，只显示当前文件的
let Tlist_Exit_OnlyWindow = 1       "如果taglist窗口是最后一个窗口，则退出vim
let Tlist_Use_Right_Window = 1      "在右侧窗口中显示taglist窗口，默认如此
```
在Vim命令行下运行`:TlistToggle`命令就可以打开Taglist窗口，再次运行`:TlistToggle`则关闭。

设置Tlist_Sort_Type为”name”可以使taglist以tag名字进行排序，缺省是按tag在文件中出现的顺序进行排序。
如果想在启动VIM后，自动打开taglist窗口，设置Tlist_Auto_Open为1。
如果希望在选择了tag后自动关闭taglist窗口，设置Tlist_Close_On_Select为1。
在使用`:TlistToggle`打开taglist窗口时，如果希望输入焦点在taglist窗口中，设置Tlist_GainFocus_On_ToggleOpen为1.
Tlist_WinHeight和Tlist_WinWidth可以设置taglist窗口的高度和宽度。Tlist_Use_Horiz_Window为１设置taglist窗口横向显示。

tag窗口中支持以下命令：
```
<CR>          跳到光标下tag所定义的位置
p             预览tag定义，但不跳转
o             在一个新打开的窗口中显示光标下tag
<Space>       显示光标下tag的原型定义
u             更新taglist窗口中的tag
s             更改排序方式，在按名字排序和按出现顺序排序间切换
x             taglist窗口放大和缩小，方便查看较长的tag
+             打开一个折叠，同zo
-             将tag折叠起来，同zc
*             打开所有的折叠，同zR
=             将所有tag折叠起来，同zM
q             关闭taglist窗口
<F1>          显示帮助
```
SuperTab
-
SuperTab : Do all your insert-mode completion with Tab. 
http://www.vim.org/scripts/script.php?script_id=1643

SuperTab使用很简单，只要在输入变量名或路径名等符号中途按Tab键，就能得到以前输入过的符号列表，并通过Tab键循环选择。

关于补全方式参见`:help ins-completion`
ctrl-n和ctrl-p 关键字补全
ctrl-x，ctrl-] 根据tag补全

在你的~/.vimrc文件中加上这两句:
```
let g:SuperTabDefaultCompletionType="<C-X><C-]>"
" 设置按下<Tab>后默认的补全方式, 默认是<C-P>

let g:SuperTabRetainCompletionType=2
" 0 - 不记录上次的补全方式
" 1 - 记住上次的补全方式,直到用其他的补全命令改变它
" 2 - 记住上次的补全方式,直到按ESC退出插入模式为止
```
我的设置
-
公司用的vim版本是6.3，上面的插件只有marks browser只支持vim 7。
关于这几个插件我的配置是：
```
" ShowMarks
let showmarks_enable = 0
let showmarks_include = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
nmap <Leader>md <Leader>mh

" alternate headfile
map <silent> <F11> :A<CR>

" Taglist
let Tlist_Show_One_File = 1
let Tlist_Exit_OnlyWindow = 1
nmap TT :TlistToggle<CR>

" markset
map <F8> <Plug>MarkSet
```
ShowMarks如果启用，启动vim时过一段时间才会高亮书签，所以默认给它关了，感觉用md删除一个书签更合理点；
用F11快捷键来切换头文件，TT来打开taglist窗口；
因为有了ShowMarks的快捷键，markset的`<Leader>m`会有一个延迟时间，所以直接用F8来高亮。
参考
-
http://blog.csdn.net/liuyuan185442111/article/details/51471093
http://blog.csdn.net/liuyuan185442111/article/details/51485803
http://easwy.com/blog/archives/advanced-vim-skills-advanced-move-method/
http://blog.csdn.net/vaqeteart/article/details/4146618
http://easwy.com/blog/archives/advanced-vim-skills-omin-complete/