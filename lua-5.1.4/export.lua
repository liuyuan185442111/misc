local not_save_fields_of_battle = {
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
	fh_sort4 = true,
	hostile_send_damage = true,
	hsd_sort = true,
	hostile_recv_damage = true,
	hrd_sort = true,
	hostile_heal = true,
	hh_sort1 = true,
	hh_sort2 = true,
	sort_ok = true,
}
local function sieve_battle_fields(battle)
	local r = {}
	for field,value in pairs(battle) do
		if not not_save_fields_of_battle[field] then
			r[field] = value
		end
	end
	return r
end

local function export(region, data)
	if not skada.savedata(region, data) then
		error('保存'..region..'失败')
	end
end
local function import(region)
	local data = skada.loaddata(region)
	if not data then
		error('读取'..region..'失败')
	else
		local code,info = load(data)
		if not code then
			print(info)
			error('加载'..region..'失败')
		else
			local ret,info = pcall(code)
			if not ret then
				print(info)
				error('执行'..region..'失败')
			end
		end
	end
end

--所有初始数据、排序后的数据、标识为NS的数据都不导出
local function export_allbattle()
	local temp = {}
	for _,battle in ipairs(allbattle) do
		table.insert(temp, sieve_battle_fields(battle))
	end
	export(0, skada.dump(temp, 'allbattle = '))
end

local function import_allbattle()
	import(0)
end

local function export_battle(battle)
	export(1, skada.dump(sieve_battle_fields(battle), 'battle = '))
end

skada.export_allbattle = export_allbattle
skada.import_allbattle = import_allbattle
skada.export_battle = export_battle
