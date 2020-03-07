#!/bin/env lua
require('common')
require('port')
role = {}
buff = {}
function addbuff(xid, buffid, favorable)
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
