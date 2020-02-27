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
local function sieve_battle_fields(battle)
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
		table.insert(temp, sieve_battle_fields(v))
	end
	print(skada.dump(temp, 'allbattle='))
end

function export_battle(battle)
	local s = skada.dump(sieve_battle_fields(battle), 'battle=')
	savedata(1, s)
end

function import_data(data)
	local code = load(data)
	if not pcall(code()) then
	end
end
