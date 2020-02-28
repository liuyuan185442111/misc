function pre_fsd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.friend_send_damage
	if #allitems == 0 then return nil end
	table.sort(allitems, function(a,b) return a.source_tid<b.source_tid end)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, firsttime, lasttime, skillset, targetset = 0
	for _,v in ipairs(allitems) do
		if v.source_tid ~= currid then
			if currid ~= 0 then
				semidata[currid] = {
					tid = currid,
					damage = currdamage,
					firsttime = firsttime,
					lasttime = lasttime,
					skillset = skillset,
					targetset = targetset,
				}
				if v.source_tid == -1 then break end
			end
			currid, currdamage = v.source_tid, 0
			firsttime, lasttime = LONG_TIME_LATER, 0
			skillset, targetset = {}, {}
		end
		currdamage = currdamage + v.value
		firsttime = math.min(firsttime, v.time)
		lasttime = math.max(lasttime, v.time)
		do
			local temp = skillset[v.skillid]
			local value = v.value
			if temp == nil then
				skillset[v.skillid] = {
					id = v.skillid,
					damage = value,
					maxdmg = value,
					mindmg = value,
					count = 1,
					baoji = skada.isbaoji(v.flag) and 1 or 0,
				}
			else
				temp.damage = temp.damage + value
				temp.maxdmg = math.max(temp.maxdmg, value)
				temp.mindmg = math.min(temp.mindmg, value)
				if skada.isbaoji(v.flag) then temp.baoji = temp.baoji + 1 end
				temp.count = temp.count + 1
			end
		end
		do
			local temp = targetset[v.target_tid]
			if temp == nil then
				--TODO 如果npc的tid和角色的roleid重复呢?
				targetset[v.target_tid] = {id=v.target_tid, damage=v.value, isplayer=skada.isplayer(v.target_xid)}
			else
				temp.damage = temp.damage + v.value
			end
		end
	end
	battle.friend_send_damage = {}
	return semidata
end

local function local_fsd_summarize(t, total_damage)
	t.active_time = t.lasttime - t.firsttime
	t.damage_ratio = t.damage / total_damage
	t.damage_rate = t.damage / t.active_time
end
local function local_fsd_merge_skill(dest, src, sumdmg, adopt_data)
	for k,v in pairs(src) do
		local t = dest[k]
		if t then
			t.count = t.count + v.count
			t.baoji = t.baoji + v.baoji
			t.damage = t.damage + v.damage
			if t.maxdmg < v.maxdmg then
				t.maxdmg = v.maxdmg
			end
			if t.mindmg > v.mindmg then
				t.mindmg = v.mindmg
			end
			t.averdmg = t.damage / t.count
			t.ratio = t.damage / sumdmg
		else
			if adopt_data then
				v.averdmg = v.damage / v.count
				v.ratio = v.damage / sumdmg
				dest[k] = v
			else
				local t = skada.clone_table(v)
				t.averdmg = t.damage / t.count
				t.ratio = t.damage / sumdmg
				dest[k] = t
			end
		end
	end
end
local function local_fsd_merge_target(dest, src, sumdmg, adopt_data)
	for k,v in pairs(src) do
		local t = dest[k]
		if t then
			t.damage = t.damage + v.damage
			t.ratio = t.damage / sumdmg
		else
			if adopt_data then
				v.ratio = v.damage / sumdmg
				dest[k] = v
			else
				local t = skada.clone_table(v)
				t.ratio = t.damage / sumdmg
				dest[k] = t
			end
		end
	end
end

function merge_fsd(srcdata, battle, adopt_data)
	if not srcdata then
		return false
	end
	if adopt_data == nil then
		adopt_data = true
	end
	battle = battle or currbattle
	local summary = battle.fsd_summary
	for k,v in pairs(srcdata) do
		local t = summary[k]
		if not t then
			if adopt_data then
				v.occu = skada.getroleoccu(k)
				v.name = skada.getrolename(k)
				for _,v in pairs(v.skillset) do
					v.name = skada.getskillname(v.id)
				end
				for _,v in pairs(v.targetset) do
					v.occu = v.isplayer and skada.getroleoccu(v.id) or 0
					v.name = v.isplayer and skada.getrolename(v.id) or skada.getnpcname(v.id)
				end
				v.skillsort_NS = skada.trans_table(v.skillset)
				table.sort(v.skillsort_NS, function(a,b) return a.damage>b.damage end)
				v.targetsort_NS = skada.trans_table(v.targetset)
				table.sort(v.targetsort_NS, function(a,b) return a.damage>b.damage end)
				summary[k] = v
			else
				t = {
					tid = k,
					occu = v.occu,
					name = v.name,
					damage = 0,
					firsttime = LONG_TIME_LATER,
					lasttime = 0,
					skillset = {},
					targetset = {},
				}
				summary[k] = t
			end
		end
		if t then
			t.lasttime = math.max(t.lasttime, v.lasttime)
			t.firsttime = math.min(t.firsttime, v.firsttime)
			t.damage = t.damage + v.damage
			local_fsd_merge_skill(t.skillset, v.skillset, t.damage, adopt_data)
			local_fsd_merge_target(t.targetset, v.targetset, t.damage, adopt_data)
		end
	end
	return true
end

function merge_fsd_repair(battle)
	battle = battle or currbattle
	local summary = battle.fsd_summary
	for _,t in pairs(summary) do
		local_fsd_summarize(t, battle.total_wesend_damage)
		for _,v in pairs(t.skillset) do
			v.averdmg = v.damage / v.count
			v.ratio = v.damage / t.damage
		end
		t.skillsort_NS = skada.trans_table(t.skillset)
		table.sort(t.skillsort_NS, function(a,b) return a.damage>b.damage end)
		for _,v in pairs(t.targetset) do
			v.ratio = v.damage / t.damage
		end
		t.targetsort_NS = skada.trans_table(t.targetset)
		table.sort(t.targetsort_NS, function(a,b) return a.damage>b.damage end)
	end
	battle.fsd_sort1 = skada.trans_table(summary)
	battle.fsd_sort2 = skada.clone_array(battle.fsd_sort1)
	table.sort(battle.fsd_sort1, function(a,b) return a.damage>b.damage end)
	table.sort(battle.fsd_sort2, function(a,b) return a.damage_rate>b.damage_rate end)
	return true
end

--返回false表示未有变化
function cal_fsd()
	return merge_fsd(pre_fsd()) and merge_fsd_repair() or false
end

--返回false表示未有变化
function cal_fsd_sum()
	if sumbattle.fsd_summary.OK then
		return false
	end
	for _,v in ipairs(allbattle) do
		merge_fsd(v.fsd_summary, sumbattle, false)
	end
	merge_fsd_repair(sumbattle)
	sumbattle.fsd_summary.OK = true
	return true
end
------------------------------------------------------------

function cal_currbattle()
	cal_fsd()
end
