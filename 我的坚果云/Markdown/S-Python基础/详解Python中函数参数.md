**1.位置参数** positional
```
def fun(a, b, c):
	print('a=%d, b=%d, c=%d'%(a,b,c))
fun(1,2,3)

a=1, b=2, c=3
```
**2.关键字参数** keyword
```
def fun(a, b, c=4):
	print('a=%d, b=%d, c=%d'%(a,b,c))
fun(1,c=2,b=3)
fun(1,b=3)

a=1, b=3, c=2
a=1, b=3, c=4
```
函数调用时，non-keyword参数必须在keyword参数之前。
**3.包裹位置传递**
以*起始的参数会接收所有其他参数
```
def fun(a, *b):
	print('a=%d'%a)
	# b是一个元组
	for i in b:
		print(i)
fun(1,2,3)

a=1
2
3
```
b也可以接受元组作为参数，只需要在参数前面加上*：

	par = (2,3,4)
	fun(1,*par)

**4.包裹关键字传递**
```
def fun(a, **b):
	print('a=%d'%a)
	print(b)
fun(2,key=89)

a=2
{'key': 89}
```
b会接收最后的命名参数，b也可以接受字典作为参数，只需要在参数前面加上**：
```
def fun(a, m=9, *b, **c):
	print('a=%d'%a)
	print(b)
	print(c)
par = {'d':'val0','e':99}
fun(2,5,6,**par)

a=2
(6,)
{'e': 'val', 'd': 99}
```
不过因为par的keys是作为关键字参数的，所以必须是str类型。
***
函数最多有一个包裹位置参数，最多有一个包裹关键字参数。

*arg，**kw这两个参数就可以接收所有的参数：
```Python
def fun2(*arg, **kw):
	print(arg, kw)
fun2(2,5,6,key=90)

(2, 5, 6) {'key': 90}
```
位置参数如果有默认参数，带默认参数的位置参数必须要在不带默认参数的位置参数之后。下面的函数就是错的：

	# 错误!
	def person(name, age, city='Beijing', job):
	    print(name, age, city, job)
	person('li', 25, job='Engineer')
不过可以添加一个空的包裹位置参数：

	def person(name, age, *, city='Beijing', job):
	    print(name, age, city, job)
	person('li', 25, job='Engineer')
\*是空的，所以它不接受任何参数，但\*之后的参数都被视为关键字参数，所以此时需要显示指定`job='Engineer'`。
***
Python里的参数传递都是引用传递，唯一的例外是，将一个字典作为包裹关键字参数传递时，传递的是字典的副本。