在wiki上面，metaclass是这样定义的：In object-oriented programming, a metaclass is a class whose instances are classes.

Python中对象模型如下图：

![Python对象模型](http://img.blog.csdn.net/20150901170858374)

其中，实线表示 is-kind-of（派生）的关系，虚线表示 is-instance-of（实例化）的关系。
可以看出，object 是其他所有类的基类。

这里并不完整，因为元类并不只有type一个，比如enum.EnumMeta就是标准库定义的一个元类，而且也可以自己定义元类：

![自定义元类](http://img.blog.csdn.net/20150901170920195)
##Python中metaclass的原理
metaclass的原理其实是这样的：当定义好类之后，创建类的时候其实是调用了 type 的 \__new__  方法为这个类分配内存空间，然后再调用type的\__init__ 方法初始化。所以 metaclass 的所有 magic 其实就在这个 \__new__ 方法里面了。

一般情况下，如果你要用自定义 metaclass 的话，该类需要继承自 type，而且通常会重写 type 的 \__new__ 方法来控制创建过程，对自定义类调用type() 函数的结果是`<class 'type'>`。
isinstance() 函数用来判断派生关系，type() 函数可以用来查看一个类型或变量的类型。

当用户定义一个类的时候，Python 解释器首先在当前类的定义中查找 metaclass，如果没有找到，就继续在基类中查找 metaclass，找到了，就使用它创建类，也就是说，metaclass 可以隐式地继承到子类，但子类自己却感觉不到。
一个类的定义方式可以是这样的：

	class C(object, metaclass=MyMetaClass):
		......
##\__new__，\__init__，metaclass之间的关系
class的\__new__ 是用来构建object的，\__init__是用来在构建好object后进行初始化的，\__new__ 第一个参数是class本身，\__init__的第一个参数是self，self就是已经构建好的object啊，其他参数相同。
这™不就是把c++里构造函数的功能分开了嘛！然而我觉得普通类的\__new__ 并没什么用，又不需要和c++似的去操纵内存，不过是一层一层调用基类的\__new__ 。

关键是Python还有个metaclass啊，用来构建class，这些metaclass的基类是type。metaclass的\__init__反而不重要了，只要构造出class来就行，不需要初始化。
metaclass的\__new__(cls, name, bases, attrs)是长这个样子的，其中：
> cls：指的是metaclass本身
> name：class的名字，是一个字符串，也就是我们通常用 类名.\__name__ 获得的那个
> bases：class的基类组成的元组
> attrs：class的属性组成的dict

什么时候构建类呢？就在类定义的时候，比如下面class X的定义：
```
class Meta(type):
    def __new__(cls, name, bases, attrs):
        print('in Meta')
        return type.__new__(cls, name, bases, attrs)

class X(object, metaclass=Meta):
    pass
# 输出in Meta
```
这个定义其实是被解释器解释为**`X = Meta('X', (object,), {})`**的，Meta既然是metaclass，那也是class啊，所以先调用了\__new__来构建X，然后调用\__init__来初始化X。所以打印出了“in Meta”。

在创建类的过程，我们可以在\__new__里面修改 name，bases，attrs 的值来达到我们的功能。这里常用的配合方法是 getattr 和 setattr。

另外还有一个限制，the metaclass of a derived class must be a (non-strict) subclass of the metaclasses of all its bases。