local function has_valid_name(a)
	return a.name ~= skada.nullname
end
local function comp_by_source(a, b)
	return a.source_tid > b.source_tid
end
local function comp_by_target(a, b)
	return a.target_tid > b.target_tid
end

local function in_merge_skillset(dest, src, adopt)
	for skillid,item in pairs(src) do
		local t = dest[skillid]
		if t then
			t.heal = t.heal + item.heal
			t.overheal = t.overheal + item.overheal
			t.count = t.count + item.count
			t.baoji = t.baoji + item.baoji
			t.shanduo = t.shanduo + item.shanduo
			t.gedang = t.gedang + item.gedang
			t.mingzhong = t.mingzhong + item.mingzhong
			if t.maxheal < item.maxheal then t.maxheal = item.maxheal end
			if t.minheal > item.minheal then t.minheal = item.minheal end
		else
			if adopt then
				item.name = skada.getskillname(skillid)
				dest[skillid] = item
			else
				dest[skillid] = skada.clone_table(item)
			end
		end
	end
end
local function in_merge_targetset(dest, src, adopt)
	for tid,item in pairs(src) do
		local t = dest[tid]
		if t then
			t.heal = t.heal + item.heal
		else
			if adopt then
				item.name = skada.getpawnname(true, tid)
				dest[tid] = item
			else
				dest[tid] = skada.clone_table(item)
			end
		end
	end
end

local function pre_weheal(battle)
	battle = battle or currbattle
	local allitems = battle.friend_heal
	if #allitems == 0 then return nil end
	battle.friend_heal = {}

	local semidata1 = {}
	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	local currid, realheal, overheal, skillset, targetset = 0, 0, 0, {}, {}
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				semidata1[currid] = {
					id = currid,
					realheal = realheal,
					overheal = overheal,
					skillset = skillset,
					targetset = targetset,
				}
				if item.source_tid == -1 then break end
			end
			currid, realheal, overheal, skillset, targetset = item.source_tid, 0, 0, {}, {}
		end

		realheal = realheal + item.value
		overheal = overheal + item.overvalue

		do
			local temp = skillset[item.skillid]
			local value = item.value
			if temp == nil then
				skillset[item.skillid] = {
					id = item.skillid,
					heal = value,
					overheal = item.overvalue,
					maxheal = value,
					minheal = value,
					count = 1,
					baoji = skada.isbaoji(item.flags) and 1 or 0,
					shanduo = skada.isshanduo(item.flags) and 1 or 0,
					gedang = skada.isgedang(item.flags) and 1 or 0,
					mingzhong = skada.ismingzhong(item.flags) and 1 or 0,
				}
			else
				temp.heal = temp.heal + value
				temp.overheal = temp.overheal + item.overvalue
				temp.maxheal = math.max(temp.maxheal, value)
				temp.minheal = math.min(temp.minheal, value)
				if skada.isbaoji(item.flags) then temp.baoji = temp.baoji + 1 end
				if skada.isshanduo(item.flags) then temp.shanduo = temp.shanduo + 1 end
				if skada.isgedang(item.flags) then temp.gedang = temp.gedang + 1 end
				if skada.ismingzhong(item.flags) then temp.mingzhong = temp.mingzhong + 1 end
				temp.count = temp.count + 1
			end
		end

		do
			local temp = targetset[item.target_tid]
			if temp == nil then
				targetset[item.target_tid] = {
					id = item.target_tid,
					heal = item.value,
				}
			else
				temp.heal = temp.heal + item.value
			end
		end
	end

	local semidata2 = {}
	table.remove(allitems)
	table.sort(allitems, comp_by_target)
	table.insert(allitems, {target_tid=-1})
	currid, realheal = 0, 0
	for _,item in ipairs(allitems) do
		if item.target_tid ~= currid then
			if currid ~= 0 then
				semidata2[currid] = {
					id = currid,
					realheal = realheal,
				}
				if item.target_tid == -1 then break end
			end
			currid, realheal = item.target_tid, 0
		end
		realheal = realheal + item.value
	end

	return semidata1, semidata2
end

local function merge_weheal(srcdata1, srcdata2, battle, adopt_data)
	if not srcdata1 then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle

	local summary = battle.fh_summary1
	local srcdata_not_empty = false

	for roleid,item in pairs(srcdata1) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo2(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				for _,v in pairs(item.targetset) do
					v.name = skada.getpawnname(true, v.id)
				end
				summary[roleid] = item
			else
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					realheal = 0,
					overheal = 0,
					skillset = {},
					targetset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.realheal = dest.realheal + item.realheal
			dest.overheal = dest.overheal + item.overheal
			in_merge_skillset(dest.skillset, item.skillset, adopt_data)
			in_merge_targetset(dest.targetset, item.targetset, adopt_data)
		end
	end

	summary = battle.fh_summary2
	for roleid,item in pairs(srcdata2) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo2(roleid)
				summary[roleid] = item
			else
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					realheal = 0,
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.realheal = dest.realheal + item.realheal
		end
	end

	return srcdata_not_empty
end

local function repair_weheal(battle, part)
	battle = battle or currbattle
	local summary = battle.fh_summary1
	for _,item in pairs(summary) do
		if not part then
			item.heal_ratio = item.realheal / battle.total_wereal_heal
			item.heal_rate = item.realheal / skada.get_friend_active_time(battle, _)
			item.over_ratio = item.overheal / item.realheal
			for _,v in pairs(item.skillset) do
				v.avgheal = v.heal / v.count
			end
			for _,v in pairs(item.targetset) do
				v.ratio = v.heal / item.realheal
			end
		end
		if item.name == skada.nullname then
			item.name, item.occu = skada.getroleinfo2(item.id)
		end
		item.skillsort1_NS = skada.trans_table(item.skillset)
		item.skillsort2_NS = skada.clone_array(item.skillsort1_NS)
		item.skillsort3_NS = skada.clone_array(item.skillsort1_NS)
		table.sort(item.skillsort1_NS, function(a,b)return a.heal>b.heal end)
		table.sort(item.skillsort2_NS, function(a,b)return a.overheal>b.overheal end)
		table.sort(item.skillsort3_NS, function(a,b)return a.heal+a.overheal>b.heal+b.overheal end)
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, function(a,b)return a.heal>b.heal end)
	end
	battle.fh_sort1 = skada.trans_table(summary, has_valid_name)
	table.sort(battle.fh_sort1, function(a,b)return a.realheal>b.realheal end)
	battle.fh_sort2 = skada.clone_array(battle.fh_sort1)
	table.sort(battle.fh_sort2, function(a,b)return a.overheal>b.overheal end)

	summary = battle.fh_summary2
	for _,item in pairs(summary) do
		if not part then
			item.heal_ratio = item.realheal / battle.total_wereal_heal
			item.heal_rate = item.realheal / skada.get_friend_active_time(battle, _)
		end
		if item.name == skada.nullname then
			item.name, item.occu = skada.getroleinfo2(item.id)
		end
	end
	battle.fh_sort5 = skada.trans_table(summary, has_valid_name)
	table.sort(battle.fh_sort5, function(a,b)return a.realheal>b.realheal end)
end

local function cal_weheal_curr()
	if merge_weheal(pre_weheal()) then
		repair_weheal()
		return true
	else
		return false
	end
end

local function in_cal_weheal_old(battle)
	if battle.sort_ok.weheal then
		return false
	end
	repair_weheal(battle, true)
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

local function pre_heheal(battle)
	battle = battle or currbattle
	local allitems = battle.hostile_heal
	if #allitems == 0 then return nil end
	battle.hostile_heal = {}

	local semidata1 = {}
	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	local currid, realheal, overheal, skillset = 0, 0, 0, {}
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				semidata1[currid] = {
					id = currid,
					realheal = realheal,
					overheal = overheal,
					skillset = skillset,
				}
				if item.source_tid == -1 then break end
			end
			currid, realheal, overheal, skillset = item.source_tid, 0, 0, {}
		end

		realheal = realheal + item.value
		overheal = overheal + item.overvalue

		do
			local temp = skillset[item.skillid]
			local value = item.value
			if temp == nil then
				skillset[item.skillid] = {
					id = item.skillid,
					heal = value,
					overheal = item.overvalue,
					maxheal = value,
					minheal = value,
					count = 1,
					baoji = skada.isbaoji(item.flags) and 1 or 0,
					shanduo = skada.isshanduo(item.flags) and 1 or 0,
					gedang = skada.isgedang(item.flags) and 1 or 0,
					mingzhong = skada.ismingzhong(item.flags) and 1 or 0,
				}
			else
				temp.heal = temp.heal + value
				temp.overheal = temp.overheal + item.overvalue
				temp.maxheal = math.max(temp.maxheal, value)
				temp.minheal = math.min(temp.minheal, value)
				if skada.isbaoji(item.flags) then temp.baoji = temp.baoji + 1 end
				if skada.isshanduo(item.flags) then temp.shanduo = temp.shanduo + 1 end
				if skada.isgedang(item.flags) then temp.gedang = temp.gedang + 1 end
				if skada.ismingzhong(item.flags) then temp.mingzhong = temp.mingzhong + 1 end
				temp.count = temp.count + 1
			end
		end
	end

	local semidata2 = {}
	table.remove(allitems)
	table.sort(allitems, comp_by_target)
	table.insert(allitems, {target_tid=-1})
	currid, realheal, overheal, skillset = 0, 0, 0, {}
	for _,item in ipairs(allitems) do
		if item.target_tid ~= currid then
			if currid ~= 0 then
				semidata2[currid] = {
					id = currid,
					realheal = realheal,
					overheal = overheal,
					skillset = skillset,
				}
				if item.target_tid == -1 then break end
			end
			currid, realheal, overheal, skillset = item.target_tid, 0, 0, {}
		end

		realheal = realheal + item.value
		overheal = overheal + item.overvalue

		do
			local temp = skillset[item.skillid]
			local value = item.value
			if temp == nil then
				skillset[item.skillid] = {
					id = item.skillid,
					heal = value,
					overheal = item.overvalue,
					maxheal = value,
					minheal = value,
					count = 1,
					baoji = skada.isbaoji(item.flags) and 1 or 0,
					shanduo = skada.isshanduo(item.flags) and 1 or 0,
					gedang = skada.isgedang(item.flags) and 1 or 0,
					mingzhong = skada.ismingzhong(item.flags) and 1 or 0,
				}
			else
				temp.heal = temp.heal + value
				temp.overheal = temp.overheal + item.overvalue
				temp.maxheal = math.max(temp.maxheal, value)
				temp.minheal = math.min(temp.minheal, value)
				if skada.isbaoji(item.flags) then temp.baoji = temp.baoji + 1 end
				if skada.isshanduo(item.flags) then temp.shanduo = temp.shanduo + 1 end
				if skada.isgedang(item.flags) then temp.gedang = temp.gedang + 1 end
				if skada.ismingzhong(item.flags) then temp.mingzhong = temp.mingzhong + 1 end
				temp.count = temp.count + 1
			end
		end
	end

	return semidata1, semidata2
end

local function merge_heheal(srcdata1, srcdata2, battle, adopt_data)
	if not srcdata1 then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle

	local summary = battle.hh_summary1
	local srcdata_not_empty = false

	for roleid,item in pairs(srcdata1) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo2(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				summary[roleid] = item
			else
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					realheal = 0,
					overheal = 0,
					skillset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.realheal = dest.realheal + item.realheal
			dest.overheal = dest.overheal + item.overheal
			in_merge_skillset(dest.skillset, item.skillset, adopt_data)
		end
	end

	summary = battle.hh_summary2
	for roleid,item in pairs(srcdata2) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo2(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				summary[roleid] = item
			else
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					realheal = 0,
					overheal = 0,
					skillset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.realheal = dest.realheal + item.realheal
			dest.overheal = dest.overheal + item.overheal
			in_merge_skillset(dest.skillset, item.skillset, adopt_data)
		end
	end

	return srcdata_not_empty
end

local function repair_heheal(battle, part)
	battle = battle or currbattle
	local summary = battle.hh_summary1
	for _,item in pairs(summary) do
		if not part then
			item.heal_ratio = item.realheal / battle.total_hereal_heal
			for _,v in pairs(item.skillset) do
				v.avgheal = v.heal / v.count
			end
		end
		if item.name == skada.nullname then
			item.name, item.occu = skada.getroleinfo2(item.id)
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, function(a,b)return a.heal>b.heal end)
	end
	battle.hh_sort1 = skada.trans_table(summary, has_valid_name)
	table.sort(battle.hh_sort1, function(a,b)return a.realheal>b.realheal end)

	summary = battle.hh_summary2
	for _,item in pairs(summary) do
		if not part then
			item.heal_ratio = item.realheal / battle.total_hereal_heal
			for _,v in pairs(item.skillset) do
				v.avgheal = v.heal / v.count
			end
		end
		if item.name == skada.nullname then
			item.name, item.occu = skada.getroleinfo2(item.id)
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, function(a,b)return a.heal>b.heal end)
	end
	battle.hh_sort2 = skada.trans_table(summary, has_valid_name)
	table.sort(battle.hh_sort2, function(a,b)return a.realheal>b.realheal end)
end

local function cal_heheal_curr()
	if merge_heheal(pre_heheal()) then
		repair_heheal()
		return true
	else
		return false
	end
end

local function in_cal_heheal_old(battle)
	if battle.sort_ok.heheal then
		return false
	end
	repair_heheal(battle, true)
	battle.sort_ok.heheal = true
	return true
end

local function in_cal_heheal_sum()
	if sumbattle.hh_summary1.OK then
		return false
	end
	for _,battle in ipairs(allbattle) do
		merge_heheal(battle.hh_summary1, battle.hh_summary2, sumbattle, false)
	end
	repair_heheal(sumbattle)
	sumbattle.hh_summary1.OK = true
	return true
end

local function cal_heheal(battle)
	if battle == currbattle then
		return cal_heheal_curr()
	end
	if battle == sumbattle then
		return in_cal_heheal_sum()
	end
	return in_cal_heheal_old(battle)
end

skada.cal_weheal = cal_weheal
skada.cal_heheal = cal_heheal
skada.cal_curr_heal = function()
	local sort_ok = currbattle.sort_ok
	cal_weheal_curr()
	sort_ok.weheal = true
	cal_heheal_curr()
	sort_ok.heheal = true
end
