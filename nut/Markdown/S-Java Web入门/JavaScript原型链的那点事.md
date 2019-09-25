函数也是一种对象，以下三种定义函数的方式本质是一样的：

	function f1(m){console.log(m)}
	var f2 = function(m){console.log(m)}
	var f3 = new Function('m','console.log(m)')
前两种可以算是第三种的语法糖。

```
function Person(m){this.value=m;}
var p = new Person(8);
```
作用上等价于
```
var Person = function(m){this.value=m;}
var p = {}
p.__proto__ = Person.prototype;
Person.call(p, 8);
```
call改变函数体内this的值，所以p.value为8。
`p.__proto__ = Person.prototype;`是为了实现继承，访问p的属性xxx时，如果p没有该属性，则会去它的`__proto__`指向的对象中去寻找，如果还没有就继续向上层去找。`__proto__`实际上是一个隐藏属性，浏览器可能并不会提供。

下面是JavaScript原型链的关系图：
![关系图](http://img.blog.csdn.net/20161226165409738?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1eXVhbjE4NTQ0MjExMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
其中：
var Animal = function()...
var anim = new Animal()...
![描述](http://img.blog.csdn.net/20161226165448099?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1eXVhbjE4NTQ0MjExMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

因为Animal、Function、Object都是函数，所以它们的`__proto__`都指向Function.prototype。
Function和Object是特殊的函数，Function是为了可以定义普通函数，Object是为了定义其他对象。

typeof Function.prototype虽然是function，但是别的函数都有对应的prototype，它却没有，这有点不统一了，所以我认为应该不把Function.prototype看做是function。

Animal是通过new Function得到的，所以`Animal.__proto__`指向Function.prototype。在定义Animal的时候，也定义了Animal.prototype。

在JavaScript中，每个函数对象都有名为“prototype”的属性（除了Function.prototype），用于引用原型对象。此原型对象又有名为“constructor”的属性，它反过来引用函数本身。
其他对象的prototype的`__proto__`都指向Object.prototype，Object.prototype的`__ptoto__`指向null。

稍稍修改后的关系图：
![关系图](http://img.blog.csdn.net/20161226165437027?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvbGl1eXVhbjE4NTQ0MjExMQ==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
函数原型对象的constructor属性又指向函数本身，所以可以通过anim.constructor.prototype得到Animal的原型对象，从而为其添加一些属性。
##参考
http://www.blogjava.net/heavensay/archive/2013/10/20/405440.html
http://rockyuse.iteye.com/blog/1426510
##by the way
有一个无符号右移运算符 >>>
对于负数，>>>先将其转换为无符号数，然后进行右移。
所以，-64>>>0就把-64转换为了无符号数。

全等号由三个等号表示（===），只有在无需类型转换运算数就相等的情况下，才返回 true。

---
For-in语句是严格的迭代语句，用于枚举对象的属性。
它的语法如下：
for (property in expression) statement
例子：
for (sProp in window) {
  document.writeln (sProp);
}
用于显示 window 对象的所有属性。

---
把对象的所有引用都设置为null，可以强制性地废除对象。例如：
var oObject = new Object;
// do something with the object here
oObject = null;
当变量oObject设置为null后，对第一个创建的对象的引用就不存在了。这意味着下次运行无用存储单元收集程序时，该对象将被销毁。