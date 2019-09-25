内建的6个常量是（[参考](https://docs.python.org/3/library/constants.html)）

	'True', 'False', 'None', 'Ellipsis', '__debug__', 'NotImplemented'
True和False是bool的实例
None是NoneType的唯一实例
None, False, True and \__debug__不能再被赋值(assignments to them, even as an attribute name, raise SyntaxError)。
##\__debug__
This constant is true if Python was not started with an -O option. O表示optimization的意思。
这个常量和assert语句有关，assert语句等价于（[参考](https://docs.python.org/3/reference/simple_stmts.html#assert)）：

	if __debug__:
		if not expression: raise AssertionError
或

	if __debug__:
		if not expression1: raise AssertionError(expression2)
##Ellipsis
Ellipsis是ellipsis的实例（不过我没找到ellipsis定义于哪），代码中的...会自动变成Ellipsis，这有个例子（[参考](http://www.keakon.net/2014/12/05/Python%E8%A3%85%E9%80%BC%E7%AF%87%E4%B9%8BEllipsis)）：
```
# 等差数列构造器
class ProgressionMaker(object):
    def __getitem__(self, key):
        if isinstance(key, tuple) and len(key) == 4 and key[2] is Ellipsis:
            return range(key[0], key[-1] + 1, key[1] - key[0])
maker = ProgressionMaker()
print(list(maker[1, 3, ..., 9]))
```
##NotImplemented
NotImplemented是个特殊值，它能被二元特殊方法返回（比如\__eq__() 、 \__lt__()  、 \__add__() 、 \__rsub__() 等），表明某个类型没有像其他类型那样实现这些操作。同样，它或许会被原地处理（in place）的二元特殊方法返回（比如\__imul__()、\__iand__()等）。还有，它的bool值为True。
一个例子可以说明NotImplemented的用法（[参考](http://www.jb51.net/article/63208.htm)）：
```
class A(object):
    def __init__(self, value):
        self.value = value
    def __eq__(self, other):
        if isinstance(other, A):
            print('Comparing an A with an A')
            return other.value == self.value
        if isinstance(other, B):
            print('Comparing an A with a B')
            return other.value == self.value
        print('Could not compare A with the other class')
        return NotImplemented

class B(object):
    def __init__(self, value):
        self.value = value
    def __eq__(self, other):
        if isinstance(other, B):
            print('Comparing a B with another B')
            return other.value == self.value
        print('Could not compare B with the other class')
        return NotImplemented
a=A(2)
b=B(2)
print(b==a)
```
输出是：
Could not compare B with the other class
Comparing an A with a B
True

在Python中，a == b会调用a.\__eq__(b)。
B的\__eq__()并没有实现B与A的比较，b.\__eq__(a)方法返回NotImplemented，这样会导致调用A中的\__eq__()方法。而且由于在A中的\__eq__()定义了A和B之间的比较，所以就得到了正确的结果（True）。
如果A中的\__eq__()也返回NotImplemented，那么运行时会退化到使用内置的比较行为，即比较对象的标识符（在CPython中，是对象在内存中的地址）。