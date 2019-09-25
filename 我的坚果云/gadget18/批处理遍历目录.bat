@goto code
这里是注释

setlocal enabledelayedexpansion
参考http://blog.csdn.net/kolamemo/article/details/18036021

重命名目录
for /d %%i in (*) do (set val=%%i && ren "!val!" "!val:~1,-1!")

字符串截取
set ifo=abcdefghijklmnopqrstuvwxyz0123456789
echo 截取前5个字符：
echo %ifo:~0,5%
echo 截取最后5个字符：
echo %ifo:~-5%
echo 截取第一个到倒数第6个字符：
echo %ifo:~0,-5%
:code

@setlocal enabledelayedexpansion
@rem 遍历目录
@for /d %%i in (*) do (set val=%%i && echo !val!)
@pause