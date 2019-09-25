##table元素访问和更新三大法则
__index元方法用来对表访问，__newindex元方法用来对表更新 。

当通过键来访问table的时候，如果这个键没有值，那么Lua就会寻找该table的metatable（如果有metatable）中的__index键。如果__index指向一个表格，Lua会在表格中查找相应的键，如果__index指向一个函数，Lua会将以table和key作为参数调用该函数，并返回该函数的返回值。

**第一法则：访问表元素时的规则：**
1.在表中查找，如果找到，返回该元素，找不到则继续
2.判断该表是否有元表，如果没有元表，返回nil，有元表则继续
3.判断元表有没有__index键，如果__index为nil，则返回nil；如果__index指向一个表，则重复1、2、3；如果__index指向一个函数，调用该函数并返回该函数的返回值

**第二法则：更新表元素的规则：**
1.如果table有该元素，直接为该元素更新值；
2.如果table无该元素，且table的元表为nil或元表不为nil但元表的__newindex为nil，则直接为table添加元素；
3.如果table无该元素，且存在元表，元表的__newindex不为nil，则通过__newindex来操作，具体是：如果__newindex指向一个表，重复上述步骤；如果__newindex指向一个函数，以table，key，value作为参数调用该函数。

**第三法则：语法糖“:”**
```
-- 定义函数时，相当于把:换成.并将self作为第一个形参；
-- 调用函数时，相当于把:换成.并将调用函数的table作为第一个实参。
t={}
function t:f()
    print(self)
end
t:f()
```
##基于对象
一个简单实例：
```
Rectangle = {area = 0, length = 0, breadth = 0}

function Rectangle:new (o,length,breadth)
    local o = o or {}
    setmetatable(o, self)
    self.__index = self
    self.length = length or 0
    self.breadth = breadth or 0
    self.area = self.length*self.breadth
    return o
end

function Rectangle:printArea ()
    print("矩形面积为", self.area)
end

myshape = Rectangle:new(nil,2,10)
myshape:printArea()
```
在myshape.printArea(myshape)中：
因为myshape里边没有printArea，去找元表Rectangle的__index，__index指向Rectangle，Rectangle里有printArea，所以就去调用Rectangle的printArea。在Rectangle的printArea里打印myshape.area，myshape里没有area，又去Rectangle的__index里找，找到并打印了出来。

但是在这里，数据不是保存在“new”出来的实例中的，而是保存在Rectangle中，这就像c++里类的静态成员变量和静态成员函数。比如这样用：

	myshape1 = Rectangle:new(nil,2,5)
	myshape2 = Rectangle:new(nil,4,10)
	myshape1:printArea()
	myshape2:printArea()
打印出来的都是40。

我们换一种**更好的方式**：
```
Rectangle = {}
Rectangle.__index = Rectangle

function Rectangle.new (length,breadth)
    local o = {}
    setmetatable(o, Rectangle)
    o.length = length or 0
    o.breadth = breadth or 0
    o.area = o.length*o.breadth
    return o
end

function Rectangle:printArea ()
    print("矩形面积为", self.area)
end

myshape1 = Rectangle.new(2,10)
myshape2 = Rectangle.new(4,10)
myshape1:printArea()
myshape2:printArea()
```
##面向对象
通过__index实现继承的例子如下：
```
Rectangle = {}
Rectangle.__index = Rectangle

function Rectangle.new(length, breadth)
    local o = {}
    o.length = length or 0
    o.breadth = breadth or 0
    setmetatable(o, Rectangle)
    return o
end

function Rectangle:getArea()
    return self.length * self.breadth
end

Cube = {}
setmetatable(Cube, Rectangle)
Cube.__index = Cube

function Cube.new(length, breadth, height)
    local o = Rectangle.new(length, breadth)
    o.height = height or 0
    setmetatable(o, Cube)
    return o
end

function Cube:getVolume()
    return self.height * self:getArea()
end

r1 = Rectangle.new(2,3)
r2 = Rectangle.new(3,4)
print("r1的长是", r1.length, "宽是", r1.breadth)
print("r1的面积是", r1:getArea())
print("r2的面积是", r2:getArea())

c1 = Cube.new(4,5,6)
c2 = Cube.new(3,4,5)
print("c1的面积是", c1:getArea())
print("c1的体积是", c1:getVolume())
print("c2的面积是", c2:getArea())
print("c2的体积是", c2:getVolume())

--[[输出是:
r1的长是        2       宽是    3
r1的面积是      6
r2的面积是      12
c1的面积是      20
c1的体积是      120
c2的面积是      12
c2的体积是      60
]]--
```
关键在于`setmetatable(Cube, Rectangle)和setmetatable(o, Cube)`，在c1中查找getVolume时，会在c1的元表Cube的__index指向的Cube中查找，正好找得到；在c1中查找getArea时，却找不到，于是去Cube的元表Rectangle的__index指向的Rectangle中查找，就找到了。
##summary
虽然Lua也可以模拟实现封装、抽象、派生，但也有点太麻烦了，特意的OO可能反而得不偿失，要充分利用Lua本身的机制，灵活应用，再说OO应该交给宿主语言C++去做。