#! /bin/env lua

--认为友方只能是玩家
local function newbattle()
	return {
		count=0, begintime=nowtime(),

		team_wrong_damage={}, --队友误伤
		twd_summary={}, --以tid分组
		twd_sort={}, --队友误伤排序

		total_send_damage=0, friend_send_damage={}, --友方造成伤害
		fsd_summary={}, --以tid分组
		fsd_sort1={}, --造成伤害排序
		fsd_sort2={}, --每秒伤害排序

		total_recv_damage=0, friend_recv_damage={}, --友方受到伤害
		frd_summary1={}, --以tid分组
		frd_sort1={}, --受到伤害排序
		frd_summary2={}, --以skillid分组
		frd_sort2={}, --承受法术伤害排序

		total_heal=0, friend_heal={}, --友方治疗
		fh_summary1={}, --以治疗者tid分组
		fh_sort1={}, --有效治疗排序
		fh_sort2={}, --过量治疗排序
		fh_sort3={}, --总计治疗排序
		fh_summary2={}, --以被治疗者tid分组
		fh_sort4={}, --获得治疗排序

		hostile_send_damage={}, --敌对造成伤害
		hsd_summary={}, --以tid分组
		hsd_sort={}, --敌对造成伤害排序

		hostile_recv_damage={}, --敌对受到伤害
		hrd_summary={}, --以tid分组
		hrd_sort={}, --敌对受到伤害排序

		hostile_heal={}, --敌对治疗
		hh_summary1={}, --以治疗者tid分组
		hh_sort1={}, --敌方治疗排序
		hh_summary2={}, --以被治疗者tid分组
		hh_sort2={}, --敌方获得治疗排序
	}
end

allbattle = {}
currbattle = newbattle()
currbattle_update_time = 0

finishedbattle = newbattle()
local finishedbattle_broken = true
sumbattle = newbattle()
sumbattle_update_time = 0

function sssss()
	local begintime,count,total_send_damage,total_recv_damage,total_heal = 0,0,0,0,0
	for _,v in ipairs(allbattle) do
		begintime = math.min(begintime, v.begintime)
		count = count + v.count
		total_send_damage = total_send_damage + v.total_send_damage
		total_recv_damage = total_recv_damage + v.total_recv_damage
		total_heal = total_heal + v.total_heal
	end
	finishedbattle.begintime = begintime
	finishedbattle.count = count
	finishedbattle.total_send_damage = total_send_damage
	finishedbattle.total_recv_damage = total_recv_damage
	finishedbattle.total_heal = total_heal
end


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
	cal_all()
	table.insert(allbattle, currbattle)
	if #allbattle > 10 then
		table.remove(allbattle, 1)
		sumbattle_expire = true
	end
end

function add_damage_or_heal(source_xid,target_xid,source_tid,target_tid,isdamage,value,overvalue,skillid,flag)
	local item = {
		source_xid=source_xid,target_xid=target_xid,source_tid=source_tid,target_tid=target_tid,
		isdamage=isdamage,value=value,overvalue=overvalue,skillid=skillid,flag=flag,time=nowtime()
	}
	if isdamage and isteammate(source_xid) and isteammate(target_xid) then
		table.insert(currbattle.team_wrong_damage, item)
		currbattle.count = currbattle.count + 1
		return
	end
	local discard_record = true
	if isdamage then
		local source_friend = getcampinfo(source_xid) > 1
		local source_hostile = getcampinfo(source_xid) == 1
		local target_friend = getcampinfo(target_xid) > 1
		local target_hostile = getcampinfo(target_xid) == 1
		if source_friend and target_hostile then
			if isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_send_damage = currbattle.total_send_damage + value
			end
			table.insert(currbattle.hostile_recv_damage, item)
			discard_record = false
		end
		if source_hostile and target_friend then
			table.insert(currbattle.hostile_send_damage, item)
			if isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_recv_damage = currbattle.total_recv_damage + value
			end
			discard_record = false
		end
		--[[
		if source_friend and target_friend then
			if isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_send_damage = currbattle.total_send_damage + value
			end
			if isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_recv_damage = currbattle.total_recv_damage + value
			end
		end
		if source_hostile and target_hostile then
			table.insert(currbattle.hostile_send_damage, item)
			table.insert(currbattle.hostile_recv_damage, item)
		end
		]]
	else
		local camp = getcampinfo(source_xid)
		if camp>1 then --friend
			if isplayer(source_xid) and isplayer(target_xid) then
				table.insert(currbattle.friend_heal, item)
				currbattle.total_heal = currbattle.total_heal + value
				discard_record = false
			end
		elseif camp==1 then --hostile
			table.insert(currbattle.hostile_heal, item)
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
			firsttime, lasttime = 7952313600000, 0
			skillset, targetset = {}, {}
		end
		currdamage = currdamage + v.value
		firsttime = math.min(firsttime, v.time)
		lasttime = math.max(lasttime, v.time)
		do
			local temp = skillset[v.skillid]
			local value = v.value
			if temp == nil then
				skillset[v.skillid] = {id=v.skillid, damage=value, maxdmg=value, mindmg=value, count=1, baoji=isbaoji(v.flag)}
			else
				temp.damage = temp.damage + value
				if value > temp.maxdmg then temp.maxdmg = value end
				if value < temp.mindmg then temp.mindmg = value end
				if isbaoji(v.flag) then temp.baoji = temp.baoji + 1 end
				temp.count = temp.count + 1
			end
		end
		do
			local temp = targetset[v.target_tid]
			if temp == nil then
				--TODO 如果npc的tid和角色的roleid重复呢?
				targetset[v.target_tid] = {id=v.target_tid, damage=v.value, isplayer=isplayer(v.target_xid)}
			else
				temp.damage = temp.damage + v.value
			end
		end
	end
	battle.friend_send_damage = {}
	return semidata
end

local function local_fsd_summarize(t, endtime, total_damage)
	t.active_time = t.lasttime - t.firsttime
	t.period_time = endtime - t.firsttime
	t.active_ratio = t.active_time / t.period_time * 100
	t.damage_ratio = t.damage / total_damage * 100
	t.damage_rate = t.damage / t.active_time
end
local function local_fsd_merge_skill(dest, src, sumdmg)
	for k,v in pairs(src) do
		local t = src[k]
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
			t.ratio = t.damage / sumdmg * 100
		else
			v.averdmg = v.damage / v.count
			v.ratio = v.damage / sumdmg * 100
			dest[k] = v
		end
	end
end
local function local_fsd_merge_target(dest, src, sumdmg)
	for k,v in pairs(src) do
		local t = dest[k]
		if t then
			t.damage = t.damage + v.damage
			t.ratio = t.damage / sumdmg * 100
		else
			v.ratio = v.damage / sumdmg * 100
			dest[k] = v
		end
	end
end

function merge_fsd(semidata, endtime, battle, notcal)
	if not semidata then
		return false
	end
	battle = battle or currbattle
	local summary = battle.fsd_summary
	for k,v in pairs(semidata) do
		local t = summary[k]
		if t then
			t.lasttime = math.max(t.lasttime, v.lasttime)
			t.firsttime = math.min(t.firsttime, v.firsttime)
			t.damage = t.damage + v.damage
			if not notcal then
				local_fsd_summarize(t, endtime, battle.total_send_damage)
			end
			local_fsd_merge_skill(t.skillset, v.skillset, t.damage)
			local_fsd_merge_target(t.targetset, v.targetset, t.damage)
			if not notcal then
				t.skillsort = skada.clonetable(t.skillset)
				table.sort(t.skillsort, function(a,b) return a.damage>b.damage end)
				t.targetsort = skada.clonetable(t.targetset)
				table.sort(t.targetsort, function(a,b) return a.damage>b.damage end)
			end
		else
			v.occu = getroleoccu(k)
			v.name = getrolename(k)
			if not notcal then
				local_fsd_summarize(v, endtime, battle.total_send_damage)
			end
			for _,v in pairs(v.skillset) do
				v.occu = getskilloccu(v.id)
				v.name = getskillname(v.skillid)
			end
			for _,v in pairs(v.targetset) do
				v.occu = v.isplayer and getroleoccu(v.id) or 0
				v.name = v.isplayer and getrolename(v.id) or getnpcname(v.id)
			end
			if not notcal then
				v.skillsort = skada.clonetable(v.skillset)
				table.sort(v.skillsort, function(a,b) return a.damage>b.damage end)
				v.targetsort = skada.clonetable(v.targetset)
				table.sort(v.targetsort, function(a,b) return a.damage>b.damage end)
			end
			summary[k] = v
		end
	end
	if not notcal then
		battle.fsd_sort1 = skada.clonetable(summary)
		battle.fsd_sort2 = skada.clonevector(battle.fsd_sort1)
		table.sort(battle.fsd_sort1, function(a,b) return a.damage>b.damage end)
		table.sort(battle.fsd_sort2, function(a,b) return a.damage_rate>b.damage_rate end)
	end
	return true
end

function cal_fsd(endtime)
	endtime = endtime or nowtime()
	return merge_fsd(pre_fsd(), endtime)
end
------------------------------------------------------------

function cal_all(endtime)
	endtime = endtime or nowtime()
	cal_fsd(endtime)
end
