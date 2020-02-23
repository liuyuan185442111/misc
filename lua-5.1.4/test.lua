#! /bin/env lua

require('strict')
dofile('export.lua')
dofile('common.lua')
dofile('port.lua')
dofile('death.lua')
dofile('damage.lua')

math.randomseed(1001086)

begin_battle()
add_damage_or_heal(1,101,11,4,true,5,0,7,1,1)
add_damage_or_heal(2,102,12,8,true,10,0,8,0,0)
add_damage_or_heal(1,102,12,8,true,15,0,9,0)
add_damage_or_heal(2,101,11,4,true,20,0,6,0)
finish_battle()
export_battle(currbattle)

begin_battle()
add_damage_or_heal(1,101,11,8,true,5,0,7,1)
add_damage_or_heal(2,102,12,4,true,12,0,8,0)
add_damage_or_heal(1,102,12,4,true,13,0,9,0)
add_damage_or_heal(2,101,11,8,true,20,0,6,0)
finish_battle()
export_battle(currbattle)

export_data()

cal_fsd_sum()
export_battle(sumbattle)
