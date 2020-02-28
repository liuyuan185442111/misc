#! /bin/env lua

require('common')
require('port')
require('strict')

require('export')
require('death')
require('process')
require('base')

math.randomseed(1001086)

begin_battle()
add_damage_or_heal(1,101,11,4,1,1, true,5,0,7)
add_damage_or_heal(2,102,12,8,0,0, true,10,0,8)
add_damage_or_heal(1,102,12,8,9,0, true,15,0)
add_damage_or_heal(2,101,11,4,6,0, true,20,0)
finish_battle()
export_battle(currbattle)

begin_battle()
add_damage_or_heal(1,101,11,8,7,1, true,5,0)
add_damage_or_heal(2,102,12,4,8,0, true,12,0)
add_damage_or_heal(1,102,12,4,9,0, true,13,0)
add_damage_or_heal(2,101,11,8,6,0, true,20,0)
finish_battle()
export_battle(currbattle)

export_allbattle()

cal_fsd_sum()
export_battle(sumbattle)
