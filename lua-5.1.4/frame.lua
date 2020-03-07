#!/bin/env lua
require('common')
require('port')
role = {}
buff = {}

--有可能buff在战斗开始就已经加上了
--战斗结束buff仍未消失
--buff存在的时候下线或离开视野
--提供4个函数 有利 不利 添加 删除
function addbuff(xid, buffid)
	local temp = role[xid]
	if temp == nil then
		temp = {}
		role[xid] = temp
	end
	local temp2 = temp[buffid]
	if temp2 == nil then
		temp2 = {id=buffid,time=0}
		temp[buffid] = temp2
	end
	temp2.begin = skada.nowtime()

	temp = buff[buffid]
	if temp == nil then
		temp = {}
		buff[buffid] = temp
	end
	temp[xid] = true 
end

function delbuff(xid, buffid)
	local temp = role[xid]
	if temp == nil then
		return
	end
	local temp2 = temp[buffid]
	if temp2 == nil then
		return
	end
	temp2.time = temp2.time + skada.nowtime() - temp2.begin
	temp2.begin = 0
end

addbuff('a', 1)
delbuff('a', 1)
print(skada.dump(role, 'role = '))
print(skada.dump(buff, 'buff = '))
