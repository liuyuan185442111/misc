一切都是对象
##控制台输入输出
在Python 2中：
raw_input([prompt])，从控制台读取字符串。
input([prompt])，等价于eval(raw_input(prompt))。
eval函数将字符串当成有效Python表达式来求值，并返回计算结果，与repr函数互逆。
print语句，将之后的表达式转换为字符串后输出到控制台，相当于对每个表达式调用str函数之后获得字符串，再将其输出至控制台。

Python 3里删除了input函数，将raw_input函数重命名为input，将print语句改为print函数。
##断言
assert是关键字，语法是：

	assert condition [, error string]
如果condition为假，抛出AssertionError异常。
##异常
```
try:
    print('in try{')
    r = 10 / int('a')
    print('result:', r)
except ValueError as e:
    print('ValueError:', e)
except ZeroDivisionError as e:
    raise
else: # 如果没有抛出异常, 会执行else块
    print('in else')
finally: # 无论是否抛出异常, 都会执行finally块
    print('in finally}')
```
所有的异常类型都继承自BaseException，基类异常也会接收派生类的异常，所以注意except的顺序。
异常的派生关系见于[exceptions](https://docs.python.org/3/library/exceptions.html#exception-hierarchy)。
raise语句用来抛出异常，抛出的异常必须继承自BaseException，raise不带参数会将异常原样抛给上层。
##lambda
用lambda关键字能创建小型匿名函数。
Lambda函数能接收任何数量的参数但只能返回一个表达式的值，同时不能包含命令或多个表达式。
匿名函数不能直接调用print，因为lambda需要一个表达式。
lambda函数拥有自己的名字空间，且不能访问自有参数列表之外或全局名字空间里的参数。

	sum = lambda arg1, arg2: arg1 + arg2
	print("Value of total:", sum(10, 20))
##模块
Python的源文件叫模块，可用import关键字导入：

	import module1[, module2[,... moduleN]
从模块中导入一个指定的部分到当前命名空间中：

	from modname import name1[, name2[, ... nameN]]
把一个模块的所有内容全都导入到当前的命名空间也是可行的，只需使用如下声明：

	from modname import *
import random和form random import *的区别：
前者调用random.random()，后者调用random()，来获得下一个随机数。

dir()函数接受模块名作为参数，返回一个排好序的字符串列表，内容是一个模块里定义过的名字。

当导入一个模块，Python解析器对模块位置的搜索顺序是：
•	当前目录
•	如果不在当前目录，Python则搜索在shell变量PYTHONPATH下的每个目录
•	如果都找不到，Python会察看默认路径。Linux下，默认路径一般为/usr/lib/python2.6/
模块搜索路径存储在sys模块的sys.path变量中，这是一个list。

添加单文件模块的方法
直接把py文件拷贝到上述三个目录任意一个下即可。当导入example模块时，Python解释器会查看有没有相应的example.pyc文件，如果没有就会生成。pyc是字节码文件，速度更快。如果更新了example.py文件，需要删除example.pyc文件。不过我实际试了下，发现pyc文件会自动更新。
##数据类型
主要数据类型有：
数字、字符串、列表、元组、字典。
列表相当于数组，元组相当于只读数组，字典相当于unorderd_map，三者分别用[]、()、{}来定义。

所有的变量都是对象，通过引用的方式来计数。
系统自动回收引用计数为0的对象，也可以用del显式删除一个对象，这么做之后其引用计数就减一。
赋值就是增加一个引用计数。

允许这样赋值：

	a, b, c = 1, 2, "john" 
以逗号隔开的对象默认成为元组，比如1,2实际是(1,2)。

del可用来删除序列中元素。

字典的key必须不可变。

//是取整除，**是幂。
与或非分别为and，or，not。
in，not in用来判断是否在指定的序列中（字符串，列表，元组）。

每个对象拥有唯一的id，用id()可得到这个id（是一个整数）。
is和not is用来判断两个对象的id是否相同。

字符串的格式化输出（与c语言printf完全相同）：

	"My name is %s and weight is %d kg!"%('Zara',21)
	"My name is Zara and weight is 21 kg!"

字符串转换：

	print会把字符串中的转义字符都变为实际的字符，比如\n变为换行
	在字符串前加上r或R，串中的转义字符不发生作用，例如print(r'\n')会输出\n，实际发生的动作是将字符串中的\转换为\\
	字符串前加上u，变为Unicode字符串
	str(x)将表达式x转换为字符串，发生的动作是将表达式x的结果转换为字符串，x如果本身是字符串，str(x)的结果就是x本身
	repr(x)将表达式x转换为字符串，发生的动作是先将表达式x的结果转换为字符串，然后在字符串前加上r或R，使其转义不发生作用（即使x本身就是字符串，其首尾的引号成为结果字符串的一部分，比如'a'，结果变成"'a'"）
##其他
ord()函数用来获取字符的编码，chr()函数把编码转换为对应的字符。
str.encode()返回bytes型别的字符串，参数可以是`'ascii'，'utf-8'`。
`b'abc'`的型别是bytes，bytes.decode()返回str型别的字符串，参数可以是`'ascii'，'utf-8'`。

len()，对于str计算字符数，对于bytes计算字节数。
在Python中，如果你调用len()函数试图获取一个对象的长度，实际上，在len()函数内部，它自动去调用该对象的\__len__()方法，所以，下面的代码是等价的：

	>>> len('ABC')
	3
	>>> 'ABC'.__len__()
	3
我们自己写的类，如果也想用len(myObj)的话，就自己写一个\__len__()方法。

list.pop()会删除list末尾的元素，list.pop(i)会删除索引为i的元素。

tuple一旦初始化就不能修改。
只有一个元素的tuple，t = (1,)，加上逗号以防被误解为小括号。

Python 2里xrange返回一个生成器，range返回一个list，Python 3里删除了range，将xrange重命名为range，返回range类。

要创建一个set，需要提供一个list作为输入集合。

str是不可变对象，list是可变对象，str的很多函数返回的是一个str副本，list则就地修改。

	# -*- coding: utf-8 -*-
	def f(l=[]):
		l.append(1)
		print(l)
	f()
	f()
	# 会打印出
	# [1]
	# [1,1]
Python函数在定义的时候，默认参数L的值就被计算出来了，即[]，因为默认参数L也是一个变量，它指向对象[]，每次调用该函数，如果改变了L的内容，则下次调用时，默认参数的内容就变了，不再是函数定义时的[]了。
这里倒可以实现static变量，定义一次，调用多次。但参数是可变的才行。那就把它封装为list就好了：
```
代码丢失
```

迭代dict：

	for value in d
	如果要迭代values，for value in d.values()
	如果要同时迭代key和value，可以用for k, v in d.items()

如何判断一个对象是可迭代对象呢？方法是通过collections模块的Iterable类型判断：

	>>> from collections import Iterable
	>>> isinstance('abc', Iterable) # str是否可迭代
	True
凡是可作用于for循环的对象都是Iterable类型；
凡是可作用于next()函数的对象都是Iterator类型，它们表示一个惰性计算的序列；
集合数据类型如list、dict、str等是Iterable但不是Iterator，不过可以通过iter()函数获得一个Iterator对象。
Python的for循环本质上就是通过不断调用next()函数实现的，例如：

	for x in [1, 2, 3, 4, 5]:
	    pass
实际上完全等价于：

	# 首先获得Iterator对象:
	it = iter([1, 2, 3, 4, 5])
	# 循环:
	while True:
	    try:
	        # 获得下一个值:
	        x = next(it)
	    except StopIteration:
	        # 遇到StopIteration就退出循环
	        break
##一个简单的web server
```
# -*- coding: utf-8 -*-
body='''
<html><body>hello world!</body></html>
'''
def application(environ, start_response):
    start_response('200 OK', [('Content-Type', 'text/html')])
    return [body.encode('utf-8')]

from wsgiref.simple_server import make_server
httpd = make_server('', 8090, application)
print('serving on 8090...')
httpd.serve_forever()
```
Python写后台有两种方式，一种是通过cgi，与apache等来交互；另一种是自己做server，此时的标准是wsgi，只要实现application(environ, start_response)函数即可，environ表示传入的请求，start_response用来写http头，函数返回html代码。所有的Python web框架都是对这个函数进行封装。
***
以下，有些粗糙
##Data model
https://docs.python.org/3/reference/datamodel.html
有些类似c++运算符重载的操作
##Built-in Functions
https://docs.python.org/3/library/functions.html
|函数|返回类型|说明|
|-|-|
|abs(number)|number|
|all(iterable)|bool|如果iterable参数中的所有元素均为真则返回True
|any(iterable)|bool|
|ascii(object)|string|转换为ascii字符
|bin(x)|string|整数的二进制表示。将一个整数转换为二进制字符串，比如8,0b100，x必须为整数或包含返回整数的\__index__()的方法
|callable(object)|bool|如果类有\__call__()方法，则可调用
|chr(i)|Unicode字符|ord的反函数，参数有效范围from 0 through 1,114,111(0x10FFFF in base 16)都包含.ValueError will be raised if i is outside that range.
##迭代器和生成器(待完善)
collections.Iterable
collections.Iterator
types.GeneratorType

Iterator是Iterable的派生类

凡是可作用于for循环的对象都是Iterable类型

generator是types.GeneratorType类型

next会调用\__next__方法
iter会调用\__iter__方法
Iterable变成Iterator

	iter(...)
    iter(iterable) -> iterator
    iter(callable, sentinel) -> iterator

    Get an iterator from an object.  In the first form, the argument must
    supply its own iterator, or be a sequence.
    In the second form, the callable is called until it returns the sentinel.

for循环
Iterator对象表示的是一个数据流，Iterator对象可以被next()函数调用并不断返回下一个数据，直到没有数据时抛出StopIteration错误。以把这个数据流看做是一个有序序列，但我们却不能提前知道序列的长度，只能不断通过next()函数实现按需计算下一个数据，所以Iterator的计算是惰性的，只有在需要返回下一个数据时它才会计算。

凡是可作用于next()函数的对象都是Iterator类型，它们表示一个惰性计算的序列；
集合数据类型如list、dict、str等是Iterable但不是Iterator，不过可以通过iter()函数获得一个Iterator对象。

迭代
可迭代对象
判断可迭代：
```
>>> from import 
>>> isinstance('abc', Iterable)
```
##map和reduce函数
我们先看map。map()函数接收两个参数，一个是函数，一个是Iterable，map将传入的函数依次作用到序列的每个元素，并把结果作为新的Iterator返回。返回的是map对象。

map是内建的
from functools import reduce
再看reduce的用法。reduce把一个函数作用在一个序列[x1, x2, x3, ...]上，这个函数必须接收两个参数，reduce把结果继续和序列的下一个元素做累积计算，其效果就是：
reduce(f, [x1, x2, x3, x4]) = f(f(f(x1, x2), x3), x4)

和map()类似，filter()也接收一个函数和一个序列。和map()不同的时，filter()把传入的函数依次作用于每个元素，然后根据返回值是True还是False决定保留还是丢弃该元素。

这是因为直接显示变量调用的不是\__str__()，而是\__repr__()，两者的区别是\__str__()返回用户看到的字符串，而\__repr__()返回程序开发者看到的字符串，也就是说，\__repr__()是为调试服务的。
##类的特殊方法
以双下划线开头的（比如__name）代表类的私有成员，不能直接访问__name是因为Python解释器对外把__name变量改成了_ClassName__name。
以双下划线开头和结尾的（\__foo__）代表python里特殊方法专用的标识，如\__init__（）代表类的构造函数。
	
\__init__	构造函数
\__del__	析构方法，因为调用析构的时间不可知，尽量避免使用
\__getitem__	下标操作，即重载中括号
\__setitem__	赋值
\__delitem__	用于删除某个元素
\__dict__	类的属性字典
\__doc__	类的文档字符串
\__name__	类名
\__module__	类的定义所在的模块
\__bases__	类的所有基类构成的元组
\__str__	str()
\__repr__	repr()
\__cmp__	对象比较
getattr(obj, name[, default])	访问对象的属性
setattr(obj,name,value)	设置一个属性。如果属性不存在，会创建一个新属性
delattr(obj, name)	删除属性。

\__iter__
如果一个类想被用于for ... in循环，类似list或tuple那样，就必须实现一个\__iter__()方法，该方法返回一个迭代对象，然后，Python的for循环就会不断调用该迭代对象的\__next__()方法拿到循环的下一个值，直到遇到StopIteration错误时退出循环。

只有在没有找到属性的情况下，才调用\__getattr__
##参考
[Python教程](http://www.liaoxuefeng.com/wiki/0014316089557264a6b348958f449949df42a6d3a2e542c000)
[runoob](http://www.runoob.com/python/python-tutorial.html)