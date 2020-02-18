#! /bin/env lua

math.randomseed(1001086)
dofile('meter.lua')

local function sleep(n)
	if arg[1] then
		os.execute("sleep " .. n)
	end
end

begin_battle()
add_damage_or_heal(1,101,11,4,true,5,0,7,8)
sleep(1)
add_damage_or_heal(2,102,12,4,true,10,0,7,8)
sleep(1)
cal_all()
add_damage_or_heal(1,101,11,4,true,5,0,7,8)
sleep(1)
add_damage_or_heal(2,102,12,4,true,16,0,7,8)
finish_battle()

--sleep(1)
--add_damage_or_heal(101,2,13,4,true,15,0,17,18)
--sleep(1)
--add_damage_or_heal(121,22,23,4,true,25,0,27,28)

local str = meter.dump(allbattle, 'halo=')
--load(str)()
--meter.dump(halo)
