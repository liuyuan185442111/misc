#! /bin/env lua
battle = {}
curr = {}
curr.source = 1
curr.target = 2
curr.sourcecamp = 1
curr.targetcamp = 2
curr.value = 10
curr.skill = 2
curr.heal = 34
curr.damage = 456
curr.overheal = 34
curr.overdamage = 456
curr.flag = 1

function pushback(sourceid,targetid,sourcecamp,targetcamp,damage,heal,overdamage,overheal,skillid,flag)
	local item = {}
	table.insert(curr,item)
end