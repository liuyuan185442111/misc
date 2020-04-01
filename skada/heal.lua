local function pre_weheal(battle)
	battle = battle or currbattle
	local allitems = battle.friend_heal
	if #allitems == 0 then return nil end
	battle.friend_heal = {}

	local semidata1 = {}
	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, skillset, targetset = 0, 0, {}, {}
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				--记录上一组
				semidata[currid] = {
					id = currid,
					damage = currdamage,
					skillset = skillset,
					targetset = targetset,
				}
				if item.source_tid == -1 then break end
			end
			--开始新一组
			currid, currdamage, skillset, targetset = item.source_tid, 0, {}, {}
		end

		currdamage = currdamage + item.value

		do
			local temp = skillset[item.skillid]
			local value = item.value --伤害量
			if temp == nil then
				skillset[item.skillid] = {
					id = item.skillid,
					damage = value,
					maxdmg = value,
					mindmg = value,
					count = 1,
					baoji = skada.isbaoji(item.flags) and 1 or 0,
					shanduo = skada.isshanduo(item.flags) and 1 or 0,
					gedang = skada.isgedang(item.flags) and 1 or 0,
					mingzhong = skada.ismingzhong(item.flags) and 1 or 0,
				}
			else
				temp.damage = temp.damage + value
				temp.maxdmg = math.max(temp.maxdmg, value)
				temp.mindmg = math.min(temp.mindmg, value)
				if skada.isbaoji(item.flags) then temp.baoji = temp.baoji + 1 end
				if skada.isshanduo(item.flags) then temp.shanduo = temp.shanduo + 1 end
				if skada.isgedang(item.flags) then temp.gedang = temp.gedang + 1 end
				if skada.ismingzhong(item.flags) then temp.mingzhong = temp.mingzhong + 1 end
				temp.count = temp.count + 1
			end
		end

		do
			--如果npc的tid和角色的roleid重复呢?
			--我们的游戏里应该不存在这种情况
			local temp = targetset[item.target_tid]
			if temp == nil then
				targetset[item.target_tid] = {
					id = item.target_tid,
					damage = item.value,
					isplayer = skada.isplayer(item.target_xid), --将来用以获取名字
				}
			else
				temp.damage = temp.damage + item.value
			end
		end
	end

	return semidata
end

local function merge_weheal(srcdata1, srcdata2, battle, adopt_data)
	if not srcdata then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle
	local summary = battle.fsd_summary
	local srcdata_not_empty = false
	for roleid,item in pairs(srcdata) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo2(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				for _,v in pairs(item.targetset) do
					v.name = skada.getpawnname(v.isplayer, v.id)
				end
				summary[roleid] = item
			else
				--插入一个空的项目
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					damage = 0,
					skillset = {},
					targetset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			in_merge_skillset2(dest.skillset, item.skillset, adopt_data)
			in_merge_targetset(dest.targetset, item.targetset, adopt_data)
		end
	end
	return srcdata_not_empty
end

local function repair_weheal(battle, part)
	battle = battle or currbattle
	local summary = battle.fsd_summary
	for _,item in pairs(summary) do
		if not part then
			item.damage_ratio = item.damage / battle.total_wesend_damage
			item.damage_rate = item.damage / skada.get_friend_active_time(battle, _)
			for _,v in pairs(item.skillset) do
				v.avgdmg = v.damage / v.count
				v.ratio = v.damage / item.damage
			end
			for _,v in pairs(item.targetset) do
				v.ratio = v.damage / item.damage
			end
		end
		if item.name == skada.nullname then
			item.name, item.occu = skada.getroleinfo2(item.id)
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, comp_by_damage)
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, comp_by_damage)
	end
	battle.fsd_sort1 = skada.trans_table(summary, has_valid_name)
	--以伤害量排序
	table.sort(battle.fsd_sort1, comp_by_damage)
	battle.fsd_sort2 = skada.clone_array(battle.fsd_sort1)
	--以伤害速度排序
	table.sort(battle.fsd_sort2, function(a,b) return a.damage_rate>b.damage_rate end)
end

local function cal_weheal_curr()
	if merge_weheal(pre_weheal()) then
		repair_fh()
		return true
	else
		return false
	end
end

local function in_cal_weheal_old(battle)
	if battle.sort_ok.weheal then
		return false
	end
	repair_fh(battle, true)
	battle.sort_ok.weheal = true
	return true
end

local function in_cal_weheal_sum()
	if sumbattle.fh_summary1.OK then
		return false
	end
	for _,battle in ipairs(allbattle) do
		merge_weheal(battle.fh_summary1, battle.fh_summary2, sumbattle, false)
	end
	repair_weheal(sumbattle)
	sumbattle.fh_summary1.OK = true
	return true
end

local function cal_weheal(battle)
	if battle == currbattle then
		return cal_weheal_curr()
	end
	if battle == sumbattle then
		return in_cal_weheal_sum()
	end
	return in_cal_weheal_old(battle)
end

local cal_heheal

skada.cal_weheal = cal_weheal
skada.cal_heheal = cal_heheal
skada.cal_curr_heal = function()
	local sort_ok = currbattle.sort_ok
	--cal_weheal_curr()
	sort_ok.weheal = true
	--cal_heheal_curr()
	sort_ok.heheal = true
end
