coroutine包的主要函数：
coroutine.create(f)
接受一个函数，创建一个协程并返回它
coroutine.resume(co, ...)
启动或再次启动一个协程的执行。第一个参数是create的返回值。用于首次启动一个协程时，后面的参数作为co的参数；用于再次启动一个协程时，程序将回到协程co调用yield释放运行权的地方，并将resume后面的参数作为yield的返回值。如果程序没有任何运行错误，会返回ture和协程co下次调用yield时传给yield的参数，如果有错误，则返回false加上错误信息。如果协程co运行结束，resume将返回ture和co的返回值。
coroutine.yield(...)
yield将释放当前协程的运行权，运行权交给对该协程调用resume的协程，并将yield的参数返回给原协程，作为resume的第二个返回值。被resume唤醒时，resume传入的参数将作为yield的返回值。

类似coroutine.create，coroutine.wrap这个函数也将创建一个coroutine，但是它并不返回coroutine本身，而是返回一个函数取而代之。一旦你调用这个返回函数，就会切入coroutine运行。所有传入这个函数的参数等同于传入coroutine.resume的参数。Returns the same values returned by resume, except the first boolean。和coroutine.resume不同，coroutine.wrap不捕获任何错误；所有的错误都应该由调用者自己传递。

coroutine.running()
返回当前正在运行的协程，如果被主线程调用返回nil。正在运行的协程用它来获得自己的信息，所以不需要参数。
coroutine.status(co)
返回协程co的状态。当创建一个协程时，它处于suspended状态；被resume启动后，协程处于running状态；协程结束后，处于dead状态；当yield之后，协程处于normal状态。

摘取一段云风的代码来解释协程的工作机制：
```
function foo(a)
        print("infoo",a)
        return coroutine.yield(2*a)
end

co = coroutine.create(function(a,b)
        print("body1",a,b)
        local r=foo(a+1)
        print("body2",r)
        local r,s=coroutine.yield(a+b,a-b)
        print("body3",r,s)
        return b,"end"
end)

print("main1",coroutine.resume(co,6,2))
print("main2",coroutine.resume(co,"m"))
print("main3",coroutine.resume(co,"x","y"))
print("main4",coroutine.resume(co))

--[[
body1   6       2
infoo   7
main1   true    14
body2   m
main2   true    8       4
body3   x       y
main3   true    2       end
main4   false   cannot resume dead coroutine
--]]
```

用Lua的协程来模拟生产者-消费者问题：
```
function product()
     local i = 0
     while true do
          i = i + 1
          coroutine.yield(i)    --将生产的物品发送给消费者
     end
end

function request(productor)
     local status, value = coroutine.resume(productor)
     return value
end

function consume(productor)
     for m=1,10 do
          local i = request(productor)    --向生产者请求物品
          print("得到物品",i)
     end
end

productor = coroutine.create(product)
consume(productor)
```
不像两个线程，一个生产一个消费，这里就只有一个线程，生产者主动yield来释放所有权。

[这里](https://blog.csdn.net/liuyuan185442111/article/details/82626963)有coroutine.wrap的一些例子。

参考
理解Lua中最强大的特性-coroutine
https://my.oschina.net/wangxuanyihaha/blog/186401
Lua 协同程序(coroutine)
http://www.runoob.com/lua/lua-coroutine.html
Lua 5.1 Reference Manual
http://www.lua.org/manual/5.1/index.html