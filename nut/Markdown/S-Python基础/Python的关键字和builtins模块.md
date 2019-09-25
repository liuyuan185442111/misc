##关键字
	from keyword import kwlist
	print(kwlist)
于是得到了长度为33的list：

	['False', 'None', 'True', 'and', 'as', 'assert', 'break', 'class', 'continue', 'def', 'del', 'elif', 'else', 'except', 'finally', 'for', 'from', 'global', 'if', 'import', 'in', 'is', 'lambda', 'nonlocal', 'not', 'or', 'pass', 'raise', 'return', 'try', 'while', 'with', 'yield']
搞明白了这些关键字，就算了解了Python的基础语法。
##关于help
在交互界面里输入help会出现“
Type help() for interactive help, or help(object) for help about object.“。
help是啥？
help定义于模块builtins中，是class _sitebuiltins._Helper的实例，通过这样的语句定义出来的：

	# in builtins
	from _sitebuiltins import _Helper
	help=_Helper()
Python解释器启动时会自动执行：

	from builtins import *
你可以自己定义一个帮助实例：

	from _sitebuiltins import _Helper
	H=_Helper()
此时H和help的效果完全一样。
_sitebuiltins模块位于C:\Python34\Lib\_sitebuiltins.py中，在这个文件里就可以找到_Helper的定义：
```
class _Helper(object):
    """Define the builtin 'help'.

    This is a wrapper around pydoc.help that provides a helpful message
    when 'help' is typed at the Python interactive prompt.

    Calling help() at the Python prompt starts an interactive help session.
    Calling help(thing) prints help for the python object 'thing'.
    """

    def __repr__(self):
        return "Type help() for interactive help, " \
               "or help(object) for help about object."
    def __call__(self, *args, **kwds):
        import pydoc
        return pydoc.help(*args, **kwds)
```
help其实是pydoc.help的封装。
基本上所有的疑问都能在help里找到答案。
`help('modules')`可获得已安装的模块。
##builtins模块
dir()函数接受模块名作为参数，返回一个排好序的字符串列表，内容是一个模块里定义过的名字。比如：

	import builtins
	dir(builtins)
便获得了内建模块里定义的名字：

![dir(builtins)](http://img.blog.csdn.net/20150903121841944)

既然内建，自然是最常用或与语言概念完整性有关。
这个list的长度是150，然而不知道为啥，居然还有一个名字是`'_'`，而在py源文件中打印出来list里却没有`'_'`，还剩149个。然后我对这149个名字分分类：

**7个模块相关**

	'__build_class__', '__doc__', '__import__', '__loader__', '__name__', '__package__', '__spec__'
**4个特殊名字**

欢迎界面里的copyright，credits，help，license：
![copyright，credits，help，license](http://img.blog.csdn.net/20150903122749373)
help前面说过了，剩下的3个则是_sitebuiltins._Printer的实例，用来打印一些信息。

**6个内建常数**

	'True', 'False', 'None', 'Ellipsis', '__debug__', 'NotImplemented'

**68个内建函数**

![内建函数](http://img.blog.csdn.net/20150903123517955)
https://docs.python.org/3/library/functions.html

**61+3个异常**

https://docs.python.org/3/library/exceptions.html#exception-hierarchy
![另3个异常](http://img.blog.csdn.net/20150903123723296)