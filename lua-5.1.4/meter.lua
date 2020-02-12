#! /bin/env lua
allbattle = {}
currbattle = {}

function add_damage_or_heal(sourceid,targetid,sourcecamp,targetcamp,damage,heal,overdamage,overheal,skillid,flag)
	local item = {sourceid=sourceid,targetid=targetid,sourcecamp=sourcecamp,targetcamp=targetcamp,
	damage=damage,heal=heal,overdamage=overdamage,overheal=overheal,skillid=skillid,flag=flag}
	--职业 时间
	table.insert(currbattle,item)
end

function begin_battle()
	if #currbattle ~= 0 then
		finish_battle()
	end
end
function finish_battle()
	table.insert(allbattle,currbattle)
	currbattle = {}
end

local function clonetable(org)
	return {table.unpack(org)}
end
local function clonespecificitems(t, key, value)
	local dest = {}
	local damage
	for _,v in ipairs(t) do
		if t[key] == 1 then
			table.insert(dest, v)
		end
	end
	return dest
end


--友方造成伤害
function friend_damage(battle)
	--先挑出友方来，按sourceid分组，每组按伤害量排序
	battle = clonespecificitems(battle, 'sourcecamp', 1)
	table.sort(battle, function(a, b) return a.sourceid < b.sourceid end)
	local flext = {}
	local currid = 0
	local damage = 0
	local item = {}
	for _,v in ipairs(battle) do
		if v.sourceid ~= currid then
			currid = v.sourceid
		end
			flext[v.sourceid] = {damage=0,item=v}
	end
	battle = clonetable(battle)
	local totol_damage=0
	for _,v in ipairs(battle) do
		totol_damage = totol_damage + v.damage
	end
	table.sort(battle,function(a,b) return a.damage < b.damage end)
	pack(battle)
end
--友方造成伤害速率
--友方承受伤害
--友方造成有效治疗
--友方造成过量治疗
--友方造成总计治疗
--友方获得治疗


dofile('skada.lua')
begin_battle()
add_damage_or_heal(1,2,3,4,35,6,7,8,9,0)
add_damage_or_heal(1,2,3,4,25,6,7,8,9,0)
friend_damage(currbattle)
pack(currbattle)
begin_battle()
add_damage_or_heal(1,2,3,4,5,6,7,8,9,0)
finish_battle()
--pack(allbattle, 'allbattle = ')
