```
#include <stdlib.h>
#include <limits.h>
#include "lua.h"
#include "lauxlib.h"

#define BITS_PER_WORD (CHAR_BIT*sizeof(unsigned int))
#define I_WORD(n) ((n-1+BITS_PER_WORD)/BITS_PER_WORD)
#define I_MASK(n) (1<<((n-1)%BITS_PER_WORD))

typedef struct BoolArray {
	int size;
	unsigned int value[0];
} BoolArray;

static int newArray(lua_State *L)
{
	int n = luaL_checkint(L, 1);
	luaL_argcheck(L, n>0, 1, "invalid size");
	int nbytes = sizeof(int) + sizeof(unsigned int)*I_WORD(n);
	BoolArray *b = (BoolArray*)lua_newuserdata(L, nbytes);
	b->size = n;
	int i=0;
	for(; i<I_WORD(n); ++i)
		b->value[i] = 0;
	luaL_getmetatable(L, "ba.meta");
	lua_setmetatable(L, -2);//将创建的userdata的metatable设为栈顶的ba.meta
	return 1;
}

static int setArray(lua_State *L)
{
	BoolArray *b = (BoolArray*)luaL_checkudata(L, 1, "ba.meta");
	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, index>0 && index<=b->size, 1, "index out of range");
	if(lua_toboolean(L, 3))
		b->value[I_WORD(index)] |= I_MASK(index);
	else
		b->value[I_WORD(index)] &= ~I_MASK(index);
	return 0;
}

static int getArray(lua_State *L)
{
	BoolArray *b = (BoolArray*)luaL_checkudata(L, 1, "ba.meta");
	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, index>0 && index<=b->size, 1, "index out of range");
	lua_pushboolean(L, b->value[I_WORD(index)] & I_MASK(index));
	return 1;
}

static int getSize(lua_State *L)
{
	BoolArray *b = (BoolArray*)luaL_checkudata(L, 1, "ba.meta");
	lua_pushinteger(L, b->size);
	return 1;
}

static const luaL_Reg lib[] = {
	{"new", newArray},
	{NULL, NULL}
};
static const luaL_Reg lib_m[] = {
	{"size", getSize},
	{"set", setArray},
	{"get", getArray},
	{NULL, NULL}
};

LUA_API int luaopen_boolarray (lua_State *L) {
	luaL_newmetatable(L, "ba.meta");//创建一个metatable
	lua_pushvalue(L, -1);//复制它
	lua_setfield(L, -2, "__index");//将它的__index指向它自己
	luaL_register(L, NULL, lib_m);//为栈顶的table添加一些成员
	luaL_register(L, "boolarray", lib);//创建名为boolarray的table,并为其添加一个new成员
	return 1;
}
```
编译
gcc -fpic -shared boolarray.c -o boolarray.so

测试
```
require "boolarray"
m=boolarray.new(4)
print(m:size())
m:set(3,true)
m:set(2,false)
print(m:get(1))
print(m:get(2))
print(m:get(3))
print(m:get(4))
```