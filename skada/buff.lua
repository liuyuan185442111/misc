require('common')
require('port')

role_set = {}
buff_set = {}

--忽略战斗期间始终存在的buff

--处理buff未消失又加上的情况
function addbuff(roleid, buffid)
	local temp = role_set[roleid]
	if temp == nil then
		temp = {}
		role_set[roleid] = temp
	end
	local temp2 = temp[buffid]
	if temp2 == nil then
		temp2 = {id=buffid, time=0}
		temp[buffid] = temp2
	end
	temp2.addtime = skada.nowtime()

	temp = buff_set[buffid]
	if temp == nil then
		temp = {}
		buff_set[buffid] = temp
	end
	temp[roleid] = true 
end

--处理未捕获加buff事件直接删buff的情况
function delbuff(roleid, buffid)
	local temp = role_set[roleid]
	if temp == nil then
		return
	end
	temp = temp[buffid]
	if temp == nil then
		return
	end
	temp.time = temp.time + skada.nowtime() - temp.addtime
	temp.addtime = 0
end

--统一处理结束战斗的情况
function clear()
end

addbuff('a', 1)
delbuff('a', 1)
print(skada.dump(role_set, 'role_set = '))
print(skada.dump(buff_set, 'buff_set = '))
