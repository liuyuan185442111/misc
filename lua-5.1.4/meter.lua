#! /bin/env lua
allbattle = {}
currbattle = {}

function add_damage_or_heal(sourceid,targetid,sourcecamp,targetcamp,damage,heal,overdamage,overheal,skillid,flag)
	local item = {sourceid=sourceid,targetid=targetid,sourcecamp=sourcecamp,targetcamp=targetcamp,
	damage=damage,heal=heal,overdamage=overdamage,overheal=overheal,skillid=skillid,flag=flag}
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

dofile('skada.lua')
begin_battle()
add_damage_or_heal(1,2,3,4,5,6,7,8,9,0)
add_damage_or_heal(1,2,3,4,5,6,7,8,9,0)
begin_battle()
add_damage_or_heal(1,2,3,4,5,6,7,8,9,0)
finish_battle()
pack(allbattle, 'allbattle = ')
