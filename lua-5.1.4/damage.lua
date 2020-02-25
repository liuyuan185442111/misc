local LONG_TIME_LATER = 7952313600000

--认为友方只能是玩家
local function newbattle()
	return {
		count=0,
		begintime=nowtime(),

		rival_tid=0,
		rival_level=0,
		rival_title='xxx',

		total_wrong_damage=0,
		team_wrong_damage={}, --队友误伤
		twd_summary={}, --以tid分组
		twd_sort={}, --队友误伤排序

		total_wesend_damage=0,
		friend_send_damage={}, --友方造成伤害
		fsd_summary={}, --以tid分组
		fsd_sort1={}, --造成伤害排序
		fsd_sort2={}, --每秒伤害排序

		total_werecv_damage=0,
		friend_recv_damage={}, --友方受到伤害
		frd_summary1={}, --以tid分组
		frd_sort1={}, --受到伤害排序
		frd_summary2={}, --以skillid分组
		frd_sort2={}, --承受法术伤害排序

		total_wereal_heal=0,
		total_weover_heal=0,
		friend_heal={}, --友方治疗
		fh_summary1={}, --以治疗者tid分组
		fh_sort1={}, --有效治疗排序
		fh_sort2={}, --过量治疗排序
		fh_sort3={}, --总计治疗排序
		fh_summary2={}, --以被治疗者tid分组
		fh_sort4={}, --获得治疗排序

		total_hesend_damage=0,
		hostile_send_damage={}, --敌对造成伤害
		hsd_summary={}, --以tid分组
		hsd_sort={}, --敌对造成伤害排序

		total_herecv_damage=0,
		hostile_recv_damage={}, --敌对受到伤害
		hrd_summary={}, --以tid分组
		hrd_sort={}, --敌对受到伤害排序

		total_hereal_heal=0,
		hostile_heal={}, --敌对治疗
		hh_summary1={}, --以治疗者tid分组
		hh_sort1={}, --敌方治疗排序
		hh_summary2={}, --以被治疗者tid分组
		hh_sort2={}, --敌方获得治疗排序

		death_record = {}, --参见death.lua
	}
end

local function newsumbattle()
	local begintime,endtime,count,total_wesend_damage,total_werecv_damage,total_wereal_heal = LONG_TIME_LATER,0,0,0,0,0
	for _,v in ipairs(allbattle) do
		begintime = math.min(begintime, v.begintime)
		endtime = math.max(endtime, v.endtime)
		count = count + v.count
		total_wesend_damage = total_wesend_damage + v.total_wesend_damage
		total_werecv_damage = total_werecv_damage + v.total_werecv_damage
		total_wereal_heal = total_wereal_heal + v.total_wereal_heal
	end
	local temp = newbattle()
	temp.begintime = begintime
	temp.endtime = endtime
	temp.count = count
	temp.total_wesend_damage = total_wesend_damage
	temp.total_werecv_damage = total_werecv_damage
	temp.total_wereal_heal = total_wereal_heal
	return temp
end

allbattle = {}
currbattle = newbattle()
sumbattle = newsumbattle()

function begin_battle()
	if currbattle.count == 0 then
		return
	end
	if not currbattle.endtime then
		finish_battle()
	end
	currbattle = newbattle()
end
function finish_battle()
	if currbattle.count == 0 then
		return
	end
	if currbattle.endtime then
		return
	end
	currbattle.endtime = nowtime()
	cal_currbattle()
	death_record_finish_battle(currbattle.death_record)
	table.insert(allbattle, 1, currbattle)
	if #allbattle > 10 then
		table.remove(allbattle)
	end
	sumbattle = newsumbattle()
end

function add_damage_or_heal(source_xid,target_xid,source_tid,target_tid,isdamage,value,overvalue,skillid,flag,targethp)
	local item = {
		source_xid=source_xid,target_xid=target_xid,source_tid=source_tid,target_tid=target_tid,
		isdamage=isdamage,value=value,overvalue=overvalue,skillid=skillid,flag=flag,time=nowtime()
	}
	if isdamage and isteammate(source_xid, source_tid) and isteammate(target_xid, target_tid) then
		table.insert(currbattle.team_wrong_damage, item)
		currbattle.total_wrong_damage = currbattle.total_wrong_damage + value
		death_record_add_activity(currbattle.death_record, target_tid, true, source_tid, skillid, -value, targethp)
		currbattle.count = currbattle.count + 1
		return
	end
	local discard_record = true
	if isdamage then
		local source_camp = skada.getcampinfo(source_xid)
		local target_camp = skada.getcampinfo(target_xid)
		local source_friend = source_camp>1
		local source_hostile = source_camp==1
		local target_friend = target_camp>1
		local target_hostile = target_camp==1
		if source_friend and target_hostile then
			if skada.isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_wesend_damage = currbattle.total_wesend_damage + value
			end
			table.insert(currbattle.hostile_recv_damage, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			local a,b,c = getrivalinfo(currbattle.rival_tid, currbattle.rival_level)
			if a then
				currbattle.rival_tid,currbattle.rival_level,currbattle.rival_title = a,b,c
			end
			discard_record = false
		end
		if source_hostile and target_friend then
			table.insert(currbattle.hostile_send_damage, item)
			currbattle.total_hesend_damage = currbattle.total_hesend_damage + value
			if skada.isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_werecv_damage = currbattle.total_werecv_damage + value
			end
			local a,b,c = getrivalinfo(currbattle.rival_tid, currbattle.rival_level)
			if a then
				currbattle.rival_tid,currbattle.rival_level,currbattle.rival_title = a,b,c
			end
			death_record_add_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, -value, targethp)
			discard_record = false
		end
		--[[
		if source_friend and target_friend then
			if skada.isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_wesend_damage = currbattle.total_wesend_damage + value
			end
			if skada.isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_werecv_damage = currbattle.total_werecv_damage + value
			end
		end
		if source_hostile and target_hostile then
			table.insert(currbattle.hostile_send_damage, item)
			table.insert(currbattle.hostile_recv_damage, item)
		end
		]]
	else
		local camp = skada.getcampinfo(target_xid)
		if camp>1 then --friend
			if skada.isplayer(source_xid) and skada.isplayer(target_xid) then
				table.insert(currbattle.friend_heal, item)
				currbattle.total_wereal_heal = currbattle.total_wereal_heal + value
				currbattle.total_weover_heal = currbattle.total_weover_heal + overvalue
				discard_record = false
			end
			death_record_add_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, value, targethp)
		elseif camp==1 then --hostile
			table.insert(currbattle.hostile_heal, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			discard_record = false
		end
	end
	if not discard_record then
		currbattle.count = currbattle.count + 1
	end
end

------------------------------------------------------------
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
					baoji = isbaoji(v.flag) and 1 or 0,
				}
			else
				temp.damage = temp.damage + value
				temp.maxdmg = math.max(temp.maxdmg, value)
				temp.mindmg = math.min(temp.mindmg, value)
				if isbaoji(v.flag) then temp.baoji = temp.baoji + 1 end
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
				local t = skada.clonetable(v)
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
				local t = skada.clonetable(v)
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
				v.occu = getroleoccu(k)
				v.name = getrolename(k)
				local sumdmg = v.damage
				for _,v in pairs(v.skillset) do
					v.name = getskillname(v.skillid)
				end
				for _,v in pairs(v.targetset) do
					v.occu = v.isplayer and getroleoccu(v.id) or 0
					v.name = v.isplayer and getrolename(v.id) or getnpcname(v.id)
				end
				v.skillsort_NS = skada.transtable(v.skillset)
				table.sort(v.skillsort_NS, function(a,b) return a.damage>b.damage end)
				v.targetsort_NS = skada.transtable(v.targetset)
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
		t.skillsort_NS = skada.transtable(t.skillset)
		table.sort(t.skillsort_NS, function(a,b) return a.damage>b.damage end)
		for _,v in pairs(t.targetset) do
			v.ratio = v.damage / t.damage
		end
		t.targetsort_NS = skada.transtable(t.targetset)
		table.sort(t.targetsort_NS, function(a,b) return a.damage>b.damage end)
	end
	battle.fsd_sort1 = skada.transtable(summary)
	battle.fsd_sort2 = skada.clonevector(battle.fsd_sort1)
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
