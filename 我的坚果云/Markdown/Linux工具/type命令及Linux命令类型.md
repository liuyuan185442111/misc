type后跟一个指令，会展示当该指令作为一个命令时是如何解读的。

参数-a，type会打印指令的所有位置。
参数-t，type会打印alias，keyword，function，builtin，file的之一。alias：别名。 keyword：shell保留字。 function： shell函数。 builtin： shell内建命令。 file：磁盘文件，外部命令。

当我们键入某个命令时, shell会按照`alias->keyword->function->builtin->$PATH`的顺序进行搜索， 本着“先到先得”的原则，就是说如果有如名为mycmd的命令同时存在于alias和function中的话，那么定会使用alias的mycmd命令。但是hash比它们的优先级都高。

内建命令：
shell内建命令是指bash（或其它版本）工具集中的命令。一般都会有一个与之同名的系统命令，比如bash中的echo命令与/bin/echo是两个不同的命令，尽管他们行为大体相仿。内建命令比系统论命令有比较高的执行效率，外部命令执行时往往需要fork一个子进程，而内建命令一般不用。

hash：
linux系统下会有一个hash表，当你刚开机时这个hash表为空，每当你执行过一条命令时，hash表会记录下这条命令的路径，就相当于缓存一样。第一次执行命令shell解释器默认的会从PATH路径下寻找该命令的路径，当你第二次使用该命令时，shell解释器首先会查看hash表，没有该命令才会去PATH路径下寻找。输入hash可以查看hash表的内容，`hash -p a b`添加一项a改名为b，执行b时实际会执行a命令。

bash所有的内建指令：
```
bash, :, ., [, alias, bg, bind, break, builtin, caller, cd, command,
compgen, complete, compopt, continue, declare, dirs, disown, echo,
enable, eval, exec, exit, export, false, fc, fg, getopts, hash, help,
history, jobs, kill, let, local, logout, mapfile, popd, printf, pushd,
pwd, read, readonly, return, set, shift, shopt, source, suspend, test,
times, trap, ture, type, typeset, ulimit, umask, unalias, unset, wait
```
![builtin](http://img.blog.csdn.net/20160529110814907)