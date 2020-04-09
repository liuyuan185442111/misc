#! /bin/env lua

require('strict')
require('common')
require('port')
require('json')
require('export')
require('base')
require('damage')
require('death')
require('heal')

math.randomseed(1001086)

skada.onlogin()

skada.begin_battle()
skada.add_damage_or_heal(1,1,11,11,1,1, false,55,1,7)
skada.add_damage_or_heal(2,2,12,12,8,0, false,10,2,8)
skada.add_damage_or_heal(1,2,11,12,9,0, false,15,3,9)
skada.add_damage_or_heal(2,1,12,11,6,0, false,20,4,6)
skada.finish_battle()

--[[
skada.begin_battle()
skada.add_damage_or_heal(1,101,11,8,7,1, true,5,0)
skada.add_damage_or_heal(2,102,12,4,8,0, true,12,0)
skada.add_damage_or_heal(1,102,12,4,9,0, true,13,0)
skada.add_damage_or_heal(2,101,11,8,6,0, true,20,0)
skada.finish_battle()
]]
