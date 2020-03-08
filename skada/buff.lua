require('common')
require('port')

role_set = {}
buff_set = {}

--有可能buff在战斗之前就已经加上了: 进入战斗时认为此时添加所有已有buff
--战斗结束buff仍未消失
--buff存在的时候下线或离开视野
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

addbuff('a', 1)
delbuff('a', 1)
print(skada.dump(role_set, 'role_set = '))
print(skada.dump(buff_set, 'buff_set = '))
