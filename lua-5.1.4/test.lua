#! /bin/env lua

require('strict')
dofile('main.lua')
dofile('common.lua')
dofile('port.lua')
dofile('damage.lua')

math.randomseed(1001086)

local function sleep(n)
	if arg[1] then
		os.execute("sleep " .. n)
	end
end

begin_battle()
add_damage_or_heal(1,101,11,4,true,5,0,7,1)
sleep(1)
add_damage_or_heal(2,102,12,8,true,10,0,8,0)
sleep(1)
add_damage_or_heal(1,102,12,8,true,15,0,9,0)
sleep(1)
add_damage_or_heal(2,101,11,4,true,20,0,6,0)
finish_battle()

begin_battle()
add_damage_or_heal(1,101,11,4,true,5,0,7,1)
sleep(1)
add_damage_or_heal(2,102,12,8,true,10,0,8,0)
sleep(1)
add_damage_or_heal(1,102,12,8,true,15,0,9,0)
sleep(1)
add_damage_or_heal(2,101,11,4,true,20,0,6,0)
finish_battle()

--export_data()
export_sumbattle()
