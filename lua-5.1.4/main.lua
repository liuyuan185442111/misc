#! /bin/env lua

skada = {}

local not_save_field = {
	team_wrong_damage = true,
	twd_sort = true,
	friend_send_damage = true,
	fsd_sort1 = true,
	fsd_sort2 = true,
	friend_recv_damage = true,
	frd_sort1 = true,
	frd_sort2 = true,
	friend_heal = true,
	fh_sort1 = true,
	fh_sort2 = true,
	fh_sort3 = true,
	fh_sort2 = true,
	fh_sort4 = true,
	hostile_send_damage = true,
	hsd_sort = true,
	hostile_recv_damage = true,
	hrd_sort = true,
	hostile_heal = true,
	hh_sort1 = true,
	hh_sort2 = true,
}
local function copy_battle_for_output(battle)
	local r = {}
	for field,value in pairs(battle) do
		if not not_save_field[field] then
			r[field] = value
		end
	end
	return r
end

--所有初始数据和排序后的数据都不导出
function export_data()
	local temp = {}
	for _,v in ipairs(allbattle) do
		table.insert(temp, copy_battle_for_output(v))
	end
	print(skada.dump(temp, 'allbattle='))
end

--for debug
function export_battle(battle)
	print(skada.dump(copy_battle_for_output(battle), 'battle='))
end

--load(str)()
