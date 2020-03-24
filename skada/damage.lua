local function comp_by_damage(a, b)
	return a.damage > b.damage
end
local function comp_by_source(a, b)
	return a.source_tid > b.source_tid
end
local function comp_by_target(a, b)
	return a.target_tid > b.target_tid
end

------------------------------------------------------------
local function in_merge_skillset(dest, src, adopt)
	for skillid,item in pairs(src) do
		local t = dest[skillid]
		if t then
			t.damage = t.damage + item.damage
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
local function in_merge_skillset2(dest, src, adopt)
	for skillid,item in pairs(src) do
		local t = dest[skillid]
		if t then
			t.damage = t.damage + item.damage
			t.count = t.count + item.count
			t.baoji = t.baoji + item.baoji
			t.shanduo = t.shanduo + item.shanduo
			t.gedang = t.gedang + item.gedang
			t.mingzhong = t.mingzhong + item.mingzhong
			if t.maxdmg < item.maxdmg then t.maxdmg = item.maxdmg end
			if t.mindmg > item.mindmg then t.mindmg = item.mindmg end
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
			t.damage = t.damage + item.damage
		else
			if adopt then
				item.name = skada.getpawnname(item.isplayer, tid)
				dest[tid] = item
			else
				dest[tid] = skada.clone_table(item)
			end
		end
	end
end

------------------------------------------------------------
local function cal_curr(pre, merge, repair)
	if merge(pre()) then
		repair()
		return true
	else
		return false
	end
end
local function in_cal_old(battle, mode, repair)
	if battle.sort_ok[mode] then
		return false
	end
	repair(battle, true)
	battle.sort_ok[mode] = true
	return true
end
local function in_cal_sum(mode, merge, repair)
	local summary = mode..'_summary'
	if sumbattle[summary].OK then
		return false
	end
	for _,battle in ipairs(allbattle) do
		merge(battle[summary], sumbattle, false)
	end
	repair(sumbattle)
	sumbattle[summary].OK = true
	return true
end
local function cal_mode(battle, mode, pre, merge, repair)
	if battle == currbattle then
		return cal_curr(pre, merge, repair)
	end
	if battle == sumbattle then
		return in_cal_sum(mode, merge, repair)
	end
	return in_cal_old(battle, mode, repair)
end

------------------------------------------------------------
--将新来的数据做成与fsd_summary相同的格式
local function pre_fsd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.friend_send_damage
	if #allitems == 0 then return nil end
	battle.friend_send_damage = {}

	--先以tid排序，然后分组
	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	--为了防止id出现0的异常情况, 给它们都赋上初值
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

--将srcdata合并到fsd_summary
--adopt_data: 是否能直接将数据拿过来 为false表示需要深拷贝
local function merge_fsd(srcdata, battle, adopt_data)
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
				item.name, item.occu = skada.getroleinfo(roleid)
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

--对fsd_summary做一些统计计算
--part: 是否执行部分计算
local function repair_fsd(battle, part)
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
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, comp_by_damage)
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, comp_by_damage)
	end
	battle.fsd_sort1 = skada.trans_table(summary)
	--以伤害量排序
	table.sort(battle.fsd_sort1, comp_by_damage)
	battle.fsd_sort2 = skada.clone_array(battle.fsd_sort1)
	--以伤害速度排序
	table.sort(battle.fsd_sort2, function(a,b) return a.damage_rate>b.damage_rate end)
end

--将当前战斗的新的队友造成的伤害记录合并到currbattle中
--返回false表示未有变化
local function cal_fsd_curr()
	if merge_fsd(pre_fsd()) then
		repair_fsd()
		return true
	else
		return false
	end
end

--计算已有战斗中队友造成的伤害统计
--返回false表示未有变化
local function in_cal_fsd_old(battle)
	if battle.sort_ok.fsd then
		return false
	end
	repair_fsd(battle, true)
	battle.sort_ok.fsd = true
	return true
end

--计算sumbattle中队友造成的伤害统计
--返回false表示未有变化
local function in_cal_fsd_sum()
	if sumbattle.fsd_summary.OK then
		return false
	end
	for _,battle in ipairs(allbattle) do
		merge_fsd(battle.fsd_summary, sumbattle, false)
	end
	repair_fsd(sumbattle)
	sumbattle.fsd_summary.OK = true
	return true
end

local function cal_fsd(battle)
	if battle == currbattle then
		return cal_fsd_curr()
	end
	if battle == sumbattle then
		return in_cal_fsd_sum()
	end
	return in_cal_fsd_old(battle)
end

------------------------------------------------------------
local function pre_frd(battle)
	battle = battle or currbattle
	local allitems = battle.friend_recv_damage
	if #allitems == 0 then return nil end
	battle.friend_recv_damage = {}

	local semidata1 = {}
	table.sort(allitems, comp_by_target)
	table.insert(allitems, {target_tid=-1})
	local currid, currdamage, skillset = 0, 0, {}
	for _,item in ipairs(allitems) do
		if item.target_tid ~= currid then
			if currid ~= 0 then
				semidata1[currid] = {
					id = currid,
					damage = currdamage,
					skillset = skillset,
				}
				if item.target_tid == -1 then break end
			end
			currid, currdamage, skillset = item.target_tid, 0, {}
		end

		currdamage = currdamage + item.value

		do
			local temp = skillset[item.skillid]
			local value = item.value
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
	end

	local semidata2 = {}
	table.remove(allitems)
	table.sort(allitems, function(a,b) return a.skillid<b.skillid end)
	table.insert(allitems, {skillid=-999999999})
	currid, currdamage = 0, 0
	local targetset = {}
	for _,item in ipairs(allitems) do
		if item.skillid ~= currid then
			if currid ~= 0 then
				semidata2[currid] = {
					id = currid,
					damage = currdamage,
					targetset = targetset,
				}
				if item.skillid == -999999999 then break end
			end
			currid, currdamage, targetset = item.skillid, 0, {}
		end

		currdamage = currdamage + item.value

		do
			local temp = targetset[item.target_tid]
			local value = item.value
			if temp == nil then
				targetset[item.target_tid] = {
					id = item.target_tid,
					damage = value,
				}
			else
				temp.damage = temp.damage + value
			end
		end
	end

	return semidata1, semidata2
end

local function merge_frd(srcdata1, srcdata2, battle, adopt_data)
	if not srcdata1 then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle

	local summary = battle.frd_summary1
	local srcdata_not_empty = false

	for roleid,item in pairs(srcdata1) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				summary[roleid] = item
			else
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					damage = 0,
					skillset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			in_merge_skillset2(dest.skillset, item.skillset, adopt_data)
		end
	end

	summary = battle.frd_summary2
	for skillid,item in pairs(srcdata2) do
		srcdata_not_empty = true
		local dest = summary[skillid]
		if dest == nil then
			if adopt_data then
				item.name = skada.getskillname(skillid)
				for _,v in pairs(item.targetset) do
					v.name, v.occu = skada.getroleinfo(v.id)
				end
				summary[skillid] = item
			else
				dest = {
					id = skillid,
					name = item.name,
					damage = 0,
					targetset = {},
				}
				summary[skillid] = dest 
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			in_merge_targetset(dest.targetset, item.targetset, adopt_data)
		end
	end

	return srcdata_not_empty
end

local function repair_frd(battle, part)
	battle = battle or currbattle
	local summary = battle.frd_summary1
	for _,item in pairs(summary) do
		if not part then
			item.damage_ratio = item.damage / battle.total_werecv_damage
			item.damage_rate = item.damage / skada.get_friend_active_time(battle, _)
			for _,v in pairs(item.skillset) do
				v.avgdmg = v.damage / v.count
				v.ratio = v.damage / item.damage
			end
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, comp_by_damage)
	end
	battle.frd_sort1 = skada.trans_table(summary)
	table.sort(battle.frd_sort1, comp_by_damage)
	summary = battle.frd_summary2
	for _,item in pairs(summary) do
		if not part then
			item.damage_ratio = item.damage / battle.total_werecv_damage
		end
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, comp_by_damage)
	end
	battle.frd_sort2 = skada.trans_table(summary)
	table.sort(battle.frd_sort2, comp_by_damage)
end

local function in_cal_frd_sum()
	if sumbattle.frd_summary1.OK then
		return false
	end
	for _,battle in ipairs(allbattle) do
		merge_frd(battle.frd_summary1, battle.frd_summary2, sumbattle, false)
	end
	repair_frd(sumbattle)
	sumbattle.frd_summary1.OK = true
	return true
end

local function cal_frd(battle)
	if battle == currbattle then
		return cal_curr(pre_frd, merge_frd, repair_frd)
	end
	if battle == sumbattle then
		return in_cal_frd_sum()
	end
	return in_cal_old(battle, 'frd', repair_frd)
end

------------------------------------------------------------
local function pre_hsd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.hostile_send_damage
	if #allitems == 0 then return nil end
	battle.hostile_send_damage = {}

	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, last_source_xid, targetset = 0, 0, '', {}
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				semidata[currid] = {
					id = currid,
					damage = currdamage,
					isplayer = skada.isplayer(last_source_xid),
					targetset = targetset,
				}
				if item.source_tid == -1 then break end
			end
			currid, currdamage, targetset = item.source_tid, 0, {}
		end

		currdamage = currdamage + item.value
		last_source_xid = item.source_xid

		do
			local temp = targetset[item.target_tid]
			if temp == nil then
				targetset[item.target_tid] = {
					id = item.target_tid,
					damage = item.value,
					isplayer = skada.isplayer(item.target_xid),
				}
			else
				temp.damage = temp.damage + item.value
			end
		end
	end

	return semidata
end

local function merge_hsd(srcdata, battle, adopt_data)
	if not srcdata then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle
	local summary = battle.hsd_summary
	local srcdata_not_empty = false
	for tid,item in pairs(srcdata) do
		srcdata_not_empty = true
		local dest = summary[tid]
		if dest == nil then
			if adopt_data then
				item.name = skada.getpawnname(item.isplayer, tid)
				for _,v in pairs(item.targetset) do
					v.name = skada.getpawnname(v.isplayer, v.id)
				end
				summary[tid] = item
			else
				dest = {
					id = tid,
					name = item.name,
					damage = 0,
					targetset = {},
				}
				summary[tid] = dest
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			in_merge_targetset(dest.targetset, item.targetset, adopt_data)
		end
	end
	return srcdata_not_empty
end

local function repair_hsd(battle, part)
	battle = battle or currbattle
	local summary = battle.hsd_summary
	for _,item in pairs(summary) do
		if not part then
			for _,v in pairs(item.targetset) do
				v.ratio = v.damage / item.damage
			end
		end
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, comp_by_damage)
	end
	battle.hsd_sort = skada.trans_table(summary)
	table.sort(battle.hsd_sort, comp_by_damage)
end

------------------------------------------------------------
local function pre_hrd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.hostile_recv_damage
	if #allitems == 0 then return nil end
	battle.hostile_recv_damage = {}

	table.sort(allitems, comp_by_target)
	table.insert(allitems, {target_tid=-1})
	local currid, currdamage, last_target_xid, sourceset = 0, 0, '', {}
	for _,item in ipairs(allitems) do
		if item.target_tid ~= currid then
			if currid ~= 0 then
				semidata[currid] = {
					id = currid,
					damage = currdamage,
					isplayer = skada.isplayer(last_target_xid),
					sourceset = sourceset,
				}
				if item.target_tid == -1 then break end
			end
			currid, currdamage, sourceset = item.target_tid, 0, {}
		end

		currdamage = currdamage + item.value
		last_target_xid = item.target_xid

		do
			local temp = sourceset[item.source_tid]
			if temp == nil then
				sourceset[item.source_tid] = {
					id = item.source_tid,
					damage = item.value,
					isplayer = skada.isplayer(item.source_xid),
				}
			else
				temp.damage = temp.damage + item.value
			end
		end
	end

	return semidata
end

local function merge_hrd(srcdata, battle, adopt_data)
	if not srcdata then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle
	local summary = battle.hrd_summary
	local srcdata_not_empty = false
	for tid,item in pairs(srcdata) do
		srcdata_not_empty = true
		local dest = summary[tid]
		if dest == nil then
			if adopt_data then
				item.name = skada.getpawnname(item.isplayer, tid)
				for _,v in pairs(item.sourceset) do
					v.name = skada.getpawnname(v.isplayer, v.id)
				end
				summary[tid] = item
			else
				dest = {
					id = tid,
					name = item.name,
					damage = 0,
					sourceset = {},
				}
				summary[tid] = dest
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			in_merge_targetset(dest.sourceset, item.sourceset, adopt_data)
		end
	end
	return srcdata_not_empty
end

local function repair_hrd(battle, part)
	battle = battle or currbattle
	local summary = battle.hrd_summary
	for _,item in pairs(summary) do
		if not part then
			for _,v in pairs(item.sourceset) do
				v.ratio = v.damage / item.damage
			end
		end
		item.sourcesort_NS = skada.trans_table(item.sourceset)
		table.sort(item.sourcesort_NS, comp_by_damage)
	end
	battle.hrd_sort = skada.trans_table(summary)
	table.sort(battle.hrd_sort, comp_by_damage)
end

------------------------------------------------------------
local function pre_twd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.team_wrong_damage
	if #allitems == 0 then return nil end
	battle.team_wrong_damage = {}

	table.sort(allitems, comp_by_source)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, skillset, targetset = 0, 0, {}, {}
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				semidata[currid] = {
					id = currid,
					damage = currdamage,
					skillset = skillset,
					targetset = targetset,
				}
				if item.source_tid == -1 then break end
			end
			currid, currdamage = item.source_tid, 0
			skillset, targetset = {}, {}
		end

		currdamage = currdamage + item.value

		do
			local temp = skillset[item.skillid]
			local value = item.value
			if temp == nil then
				skillset[item.skillid] = {
					id = item.skillid,
					damage = value,
				}
			else
				temp.damage = temp.damage + value
			end
		end

		do
			local temp = targetset[item.target_tid]
			if temp == nil then
				targetset[item.target_tid] = {
					id = item.target_tid,
					damage = item.value,
				}
			else
				temp.damage = temp.damage + item.value
			end
		end
	end

	return semidata
end

local function merge_twd(srcdata, battle, adopt_data)
	if not srcdata then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle
	local summary = battle.twd_summary
	local srcdata_not_empty = false
	for roleid,item in pairs(srcdata) do
		srcdata_not_empty = true
		local dest = summary[roleid]
		if dest == nil then
			if adopt_data then
				item.name, item.occu = skada.getroleinfo(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				for _,v in pairs(item.targetset) do
					v.name, v.occu = skada.getroleinfo(v.id)
				end
				summary[roleid] = item
			else
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
			in_merge_skillset(dest.skillset, item.skillset, adopt_data)
			in_merge_targetset(dest.targetset, item.targetset, adopt_data)
		end
	end
	return srcdata_not_empty
end

local function repair_twd(battle, part)
	battle = battle or currbattle
	local summary = battle.twd_summary
	for _,item in pairs(summary) do
		if not part then
			item.damage_ratio = item.damage / battle.total_wrong_damage
			for _,v in pairs(item.skillset) do
				v.ratio = v.damage / item.damage
			end
			for _,v in pairs(item.targetset) do
				v.ratio = v.damage / item.damage
			end
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, comp_by_damage)
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, comp_by_damage)
	end
	battle.twd_sort = skada.trans_table(summary)
	table.sort(battle.twd_sort, comp_by_damage)
end

------------------------------------------------------------
skada.cal_fsd_curr = function() return cal_curr(pre_fsd, merge_fsd, repair_fsd) end
skada.cal_frd_curr = function() return cal_curr(pre_frd, merge_frd, repair_frd) end
skada.cal_hsd_curr = function() return cal_curr(pre_hsd, merge_hsd, repair_hsd) end
skada.cal_hrd_curr = function() return cal_curr(pre_hrd, merge_hrd, repair_hrd) end
skada.cal_twd_curr = function() return cal_curr(pre_twd, merge_twd, repair_twd) end
skada.cal_fsd = function(battle) return cal_mode(battle, 'fsd', pre_fsd, merge_fsd, repair_fsd) end
skada.cal_frd = cal_frd
skada.cal_hsd = function(battle) return cal_mode(battle, 'hsd', pre_hsd, merge_hsd, repair_hsd) end
skada.cal_hrd = function(battle) return cal_mode(battle, 'hrd', pre_hrd, merge_hrd, repair_hrd) end
skada.cal_twd = function(battle) return cal_mode(battle, 'twd', pre_twd, merge_twd, repair_twd) end
