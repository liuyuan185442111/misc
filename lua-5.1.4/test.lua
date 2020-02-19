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
add_damage_or_heal(1,101,11,4,true,5,0,7,8)
sleep(1)
add_damage_or_heal(2,102,12,4,true,10,0,7,8)
sleep(1)
--cal_all()
add_damage_or_heal(1,101,11,4,true,5,0,7,8)
sleep(1)
add_damage_or_heal(2,102,12,4,true,16,0,7,8)
finish_battle()

merge_fsd(currbattle.fsd_summary, nowtime(), sumbattle, true)
table.remove(allbattle)
table.insert(allbattle, sumbattle)
export_data()
