--将新来的数据做成与fsd_summary相同的格式
local function pre_fsd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.friend_send_damage
	if #allitems == 0 then return nil end
	battle.friend_send_damage = {}

	--先以tid排序，然后分组
	table.sort(allitems, function(a,b) return a.source_tid<b.source_tid end)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, firsttime, lasttime, skillset, targetset = 0
	for _,item in ipairs(allitems) do
		if item.source_tid ~= currid then
			if currid ~= 0 then
				--记录上一组
				semidata[currid] = {
					id = currid,
					damage = currdamage,
					firsttime = firsttime,
					lasttime = lasttime,
					skillset = skillset,
					targetset = targetset,
				}
				if item.source_tid == -1 then break end
			end
			--开始新一组
			currid, currdamage = item.source_tid, 0
			firsttime, lasttime = LONG_TIME_LATER, 0
			skillset, targetset = {}, {}
		end

		currdamage = currdamage + item.value
		firsttime = math.min(firsttime, item.time)
		lasttime = math.max(lasttime, item.time)

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
				}
			else
				temp.damage = temp.damage + value
				temp.maxdmg = math.max(temp.maxdmg, value)
				temp.mindmg = math.min(temp.mindmg, value)
				if skada.isbaoji(item.flags) then temp.baoji = temp.baoji + 1 end
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

local function in_fsd_merge_skill(dest, src, adopt_data)
	for skillid,v in pairs(src) do
		local t = dest[skillid]
		if t then
			t.count = t.count + v.count
			t.baoji = t.baoji + v.baoji
			t.damage = t.damage + v.damage
			if t.maxdmg < v.maxdmg then t.maxdmg = v.maxdmg end
			if t.mindmg > v.mindmg then t.mindmg = v.mindmg end
		else
			if adopt_data then
				dest[skillid] = v
			else
				dest[skillid] = skada.clone_table(v)
			end
		end
	end
end
local function in_fsd_merge_target(dest, src, adopt_data)
	for targettid,v in pairs(src) do
		local t = dest[targettid]
		if t then
			t.damage = t.damage + v.damage
		else
			if adopt_data then
				dest[targettid] = v
			else
				dest[targettid] = skada.clone_table(v)
			end
		end
	end
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
				item.occu = skada.getroleoccu(roleid)
				item.name = skada.getrolename(roleid)
				for _,v in pairs(item.skillset) do
					v.name = skada.getskillname(v.id)
				end
				for _,v in pairs(item.targetset) do
					v.name = v.isplayer and skada.getrolename(v.id) or skada.getnpcname(v.id)
				end
				summary[roleid] = item
			else
				--插入一个空的项目
				dest = {
					id = roleid,
					occu = item.occu,
					name = item.name,
					damage = 0,
					firsttime = LONG_TIME_LATER,
					lasttime = 0,
					skillset = {},
					targetset = {},
				}
				summary[roleid] = dest
			end
		end
		if dest then
			dest.damage = dest.damage + item.damage
			dest.lasttime = math.max(dest.lasttime, item.lasttime)
			dest.firsttime = math.min(dest.firsttime, item.firsttime)
			in_fsd_merge_skill(dest.skillset, item.skillset, adopt_data)
			in_fsd_merge_target(dest.targetset, item.targetset, adopt_data)
		end
	end
	return srcdata_not_empty
end

local function in_fsd_summarize_item(item, total_damage)
	item.active_time = item.lasttime - item.firsttime
	if item.active_time < 1 then
		item.active_time = 1
	end
	item.damage_ratio = item.damage / total_damage
	item.damage_rate = item.damage / item.active_time
end

--对fsd_summary做一些统计计算
--part: 是否执行部分计算
local function repair_fsd(battle, part)
	battle = battle or currbattle
	local summary = battle.fsd_summary
	for _,item in pairs(summary) do
		if not part then
			in_fsd_summarize_item(item, battle.total_wesend_damage)
			for _,v in pairs(item.skillset) do
				v.avgdmg = v.damage / v.count
				v.ratio = v.damage / item.damage
			end
			for _,v in pairs(item.targetset) do
				v.ratio = v.damage / item.damage
			end
		end
		item.skillsort_NS = skada.trans_table(item.skillset)
		table.sort(item.skillsort_NS, function(a,b) return a.damage>b.damage end)
		item.targetsort_NS = skada.trans_table(item.targetset)
		table.sort(item.targetsort_NS, function(a,b) return a.damage>b.damage end)
	end
	battle.fsd_sort1 = skada.trans_table(summary)
	--以伤害量排序
	table.sort(battle.fsd_sort1, function(a,b) return a.damage>b.damage end)
	battle.fsd_sort2 = skada.clone_array(battle.fsd_sort1)
	--以伤害速度排序
	table.sort(battle.fsd_sort2, function(a,b) return a.damage_rate>b.damage_rate end)
end

--将当前战斗的新的队友造成的伤害记录合并到currbattle中
--返回false表示未有变化
function cal_fsd_curr()
	if merge_fsd(pre_fsd()) then
		repair_fsd()
		return true
	else
		return false
	end
end

--计算已有战斗中队友造成的伤害统计
--返回false表示未有变化
function cal_fsd_old(battle)
	if battle.sort_ok[2] then
		return false
	end
	repair_fsd(battle, true)
	battle.sort_ok[2] = true
	return true
end

--计算sumbattle中队友造成的伤害统计
--返回false表示未有变化
function cal_fsd_sum()
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
------------------------------------------------------------

function skada.cal_currbattle()
	cal_fsd_curr()
	local sort_ok = currbattle.sort_ok
	for i=1,skada.MODE_SIZE do
		table.insert(sort_ok, true)
	end
end

skada.cal_fsd = function(battle)
	if battle == currbattle then
		return cal_fsd_curr()
	elseif battle == sumbattle then
		return cal_fsd_sum()
	end
	return cal_fsd_old(battle)
end
