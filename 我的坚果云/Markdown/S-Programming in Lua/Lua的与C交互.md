##c调Lua
c通过一个Lua提供的虚拟栈与Lua进行交互。第一个压入栈中的元素索引为1，第二个压入的元素索引为2，依此类推。-1表示栈顶元素，即随后压入的元素，-2表示栈顶下面的元素，依此类推。

c主要通过两个函数调用Lua的函数，lua_call和lua_pcall，《Lua 5.1 Reference Manual》对它们的介绍如下：
```
void lua_call (lua_State *L, int nargs, int nresults);

Calls a function.

To call a function you must use the following protocol: first, the function to be called
is pushed onto the stack; then, the arguments to the function are pushed in direct
order; that is, the first argument is pushed first. Finally you call lua_call; nargs is
the number of arguments that you pushed onto the stack. All arguments and the function
value are popped from the stack when the function is called. The function results are
pushed onto the stack when the function returns. The number of results is adjusted to
nresults, unless nresults is LUA_MULTRET. In this case, all results from the function
are pushed. Lua takes care that the returned values fit into the stack space. The
function results are pushed onto the stack in direct order (the first result is pushed
first), so that after the call the last result is on the top of the stack.

Any error inside the called function is propagated upwards (with a longjmp).

The following example shows how the host program can do the equivalent to this Lua code:
     a = f("how", t.x, 14)
Here it is in C:
     lua_getfield(L, LUA_GLOBALSINDEX, "f"); /* function to be called */
     lua_pushstring(L, "how");                        /* 1st argument */
     lua_getfield(L, LUA_GLOBALSINDEX, "t");   /* table to be indexed */
     lua_getfield(L, -1, "x");        /* push result of t.x (2nd arg) */
     lua_remove(L, -2);                  /* remove 't' from the stack */
     lua_pushinteger(L, 14);                          /* 3rd argument */
     lua_call(L, 3, 1);     /* call 'f' with 3 arguments and 1 result */
     lua_setfield(L, LUA_GLOBALSINDEX, "a");        /* set global 'a' */

Note that the code above is "balanced": at its end, the stack is back to its original
configuration. This is considered good programming practice.


int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);

Calls a function in protected mode.

Both nargs and nresults have the same meaning as in lua_call. If there are no errors
during the call, lua_pcall behaves exactly like lua_call. However, if there is any
error, lua_pcall catches it, pushes a single value on the stack (the error message), and
returns an error code. Like lua_call, lua_pcall always removes the function and its
arguments from the stack.

If errfunc is 0, then the error message returned on the stack is exactly the original
error message. Otherwise, errfunc is the stack index of an error handler function. (In
the current implementation, this index cannot be a pseudo-index.) In case of runtime
errors, this function will be called with the error message and its return value will be
the message returned on the stack by lua_pcall.

Typically, the error handler function is used to add more debug information to the error
message, such as a stack traceback. Such information cannot be gathered after the return
of lua_pcall, since by then the stack has unwound.

The lua_pcall function returns 0 in case of success or one of the following error codes
(defined in lua.h):
LUA_ERRRUN: a runtime error.
LUA_ERRMEM: memory allocation error. For such errors, Lua does not call the error handler function.
LUA_ERRERR: error while running the error handler function.
```
看一个简单的例子：
```
#include "lua.hpp"
#include <iostream>
using namespace std;

int main()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "npc.lua");

    for(int i=0;i<10;++i)
    {
        lua_getglobal(L, "random_fetch");
        lua_pushinteger(L, 50);
        lua_call(L,1,1);
        cout << (int)lua_tonumber(L,-1) << endl;
        lua_pop(L,1);
    }
    lua_close(L);
    return 0;
}

//npc.lua
#! /usr/bin/lua
npc_pool1 = {1,2,3}
npc_pool2 = {11,12,13,14}
npc_pool3 = {21,22,23,24,25}

function random_fetch(exp)
    if exp >= 200 then
        return npc_pool3[math.random(1,#npc_pool3)]
    elseif exp >= 100 then
        return npc_pool2[math.random(1,#npc_pool2)]
    elseif exp >= 0 then
        return npc_pool1[math.random(1,#npc_pool1)]
    end
    return 0
end

math.randomseed(tostring(os.time()):reverse():sub(1,7))
for i=1,10 do
    print(random_fetch(tonumber(arg[1])))
end
```
再看一个简单的Lua解释器程序：
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int main(void)
{
    char buff[256];
    int error;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    while(fgets(buff, sizeof(buff), stdin) != NULL)
    {
        error = luaL_loadbuffer(L, buff, strlen(buff), "test") || lua_pcall(L,0,0,0);
        if(error)
        {
            fprintf(stderr, "%s", lua_tostring(L,-1));
            lua_pop(L, 1);
        }
    }
    lua_close(L);
    return 0;
}
```
int luaL_loadbuffer (lua_State *L, const char *buff, size_t sz, const char *name);
Loads a buffer as a Lua chunk. This function uses lua_load to load the chunk in the buffer pointed to by buff with size sz. This function returns the same results as lua_load. name is the chunk name, used for debug information and error messages.
而lua_load的说明里这样描述Loads a Lua chunk. If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack.
##Lua调c
当Lua调用c函数时，也使用了一个栈，c函数从栈中获取函数参数，并将结果压入栈中。每个函数都有自己局部的栈，当Lua调用一个c函数时，第一个参数总是这个局部栈的索引1。将参数放在栈底部，c函数在返回时就无须清空栈中的参数。

所有注册到Lua中的函数都具有相同的原型，该原型就是定义在lua.h中的lua_CFunction：

	typedef int (*lua_CFunction)(lua_State *L);
它返回一个整数，表示其压入栈中返回值数量。这个函数无须在压入结果前清空栈。在它返回后，Lua会自动删除栈中结果之下的内容。

在Lua中注册一个全局函数的函数是lua_register(L, name, func);，然后就可以在Lua程序中使用函数name了。lua_register其实是个宏：

	// in lua.h
	#define lua_register(L,n,f) (lua_pushcfuction(L, (f)), lua_setglobal(L, (n)))
	#define lua_pushcfunction(L,f) lua_pushcclosure(L, (f), 0)
	#defind lua_setglobal(L,s) lua_setfield(L, LUA_GLOBALSINDEX, (s))
Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack.
一个例子：
```
//host.c
#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <math.h>

//反转2个参数
static int reverse(lua_State *L){
    int a = luaL_checknumber(L, 1);
    //栈索引2的位置不必取出再压入,直接可作为第一个返回值
    lua_pushnumber(L, a);
    return 2;
}

int main()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "swap", reverse);

    luaL_dofile(L, "try.lua");
    lua_getglobal(L, "fun");
    lua_call(L,0,0);
    lua_close(L);
    return 0;
}

//try.lua
function fun()
        print(swap(1,2))
end

//编译和输出
gcc host.c -llua -lm -ldl
2       1
```
以上两部分内容体现了lua的一种典型应用场景是：c程序作为宿主环境，提供给Lua一些方法，然后调用Lua获得一些配置或做一些计算。
下面则描述另一种应用场景：用c为Lua编写库。
##c模块
可以将多个c函数组成一个Lua模块，作为Lua的库，相关代码：

	// in lauxlib.h
	typedef struct luaL_Reg {
		const char *name;
		lua_CFunction func;
	} luaL_reg;
	void (luaL_register) (lua_State *L, const char *libname, const luaL_reg *l);

以从lmathlib.c提取一个只有sin的数学库作为例子：
```
//mymath.c
#include <math.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

static int math_sin (lua_State *L) {
	lua_pushnumber(L, sin(luaL_checknumber(L, 1)));
	return 1;
}
static const luaL_Reg mathlib[] = {
	{"sin", math_sin},
	{NULL, NULL}
};
extern int luaopen_mymath (lua_State *L) {
	luaL_register(L, "mymath", mathlib);
	return 1;
}
```
luaL_register根据给定的名称（如mymath）创建（或复用）一个table，并用数组mathlib中的信息填充这个table。在luaL_register返回时，会将这个table留在栈中。

在上面简单的Lua解释器实现中，在`luaL_openlibs(L);`后面加上`luaopen_mymath(L);`，并和mymath.c一起编译，就可以在解释器里运行`print(mymath.sin(9));`了。

通过c代码调用mymath库的代码如下：
```
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

extern int luaopen_mymath(lua_State *L);
int main()
{
    lua_State *L = luaL_newstate();
    luaopen_mymath(L);
    lua_getfield(L, LUA_GLOBALSINDEX, "mymath");
    lua_getfield(L, -1, "sin");
    lua_pushnumber(L, 9);
    if(lua_pcall(L, 1, 1, 0) != 0)
        error(L, "error running function sin: %s", lua_tostring(L,-1));
    if(!lua_isnumber(L, -1))
        error(L, "function sin must return number");
    printf("sin(9)=%f,stack size=%d\n", lua_tonumber(L, -1),lua_gettop(L));
    lua_pop(L, 3);
    lua_close(L);
    return 0;
}
```
将之与mymath.c一起编译，运行结果是`sin(9)=0.412118,stack size=3`

以上说明了在lua中通过静态编译使用lua库的方法，然而《Lua程序设计》第26章结尾指出使用自己写的c模块最简单的方法是使用动态链接机制。用
`gcc -shared -fpic mymath.c /usr/lib64/liblua-5.1.so -o mymath.so`
(注意不要有-c选项，否则会提示only ET_DYN and ET_EXEC can be loaded)
将其编译成动态库，并将这个库放入C路径(LUA_CPATH)，默认脚本的当前路径也是被包括在LUA_CPATH中的。然后在lua脚本里，require "mymath"之后即可使用mymath库。require这句调用会将动态库mymath链接到Lua，并会寻找luaopen_mymath函数，将其注册为一个Lua函数，然后调用它以打开模块，所以luaopen_mymath必须是lua_CFunction类型的。

---
以上基于Lua 5.1.4