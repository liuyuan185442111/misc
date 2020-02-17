#! /bin/env lua

dofile('common.lua')

math.randomseed(1001086)

--3 friendly  2 neutral  1 hostile  0 none
local function campinfo(xid)
	if xid < 100 then return 3 end
	if xid > 100 then return 1 end
	return math.random(4)
end
local function isplayer(xid)
	return true
end
local function isteammate(xid)
	if not isplayer(xid) then
		return false
	end
	return false
end

local function table_item_add_item(t, i, v)
	if t[i] ~= nil then
		table.insert(t[i], v)
	else
		t[i] = {[1]=v}
	end
end

local function nowtime()
	return os.time()
end

--认为友方只能是玩家
--team_wrong_damage:队友误伤
--friend_send_damage:友方造成伤害 友方造成伤害速率
--friend_recv_damage:友方承受伤害 友方承受伤害技能
--friend_heal:友方造成有效治疗总量及速率 友方造成过量治疗 友方造成总计治疗 友方获得有效治疗
--hostile_send_damage:敌方造成伤害
--hostile_recv_damage:敌方承受伤害
--hostile_heal:敌方造成有效治疗 敌方承受有效治疗

local function newbattle()
	return {
		count=0,begintime=nowtime(),
		team_wrong_damage={},
		total_send_damage=0,total_recv_damage=0,total_heal=0,
		friend_send_damage={},friend_recv_damage={},friend_heal={},
		fsd_summary={},fsd_skill={},fsd_target={},
		hostile_send_damage={},hostile_recv_damage={},hostile_heal={}
	}
end

--职业和名字放到一个表里
--current raw   减少一些计算
--current
--finished
--summary = finished + current
allbattle = {}
sumbattle = newbattle()

--集中全力搞定current add preproccess merge 
currbattle = newbattle()

function begin_battle()
	if currbattle.count ~= 0 then
		if not currbattle.endtime then
			finish_battle()
		end
		currbattle = newbattle()
	end
end
function finish_battle()
	currbattle.endtime = nowtime()
	table.insert(allbattle, currbattle)
end

function add_damage_or_heal(source_xid,target_xid,source_tid,target_tid,isdamage,value,overvalue,skillid,flag)
	local item = {source_xid=source_xid,target_xid=target_xid,source_tid=source_tid,target_tid=target_tid,
	isdamage=isdamage,value=value,overvalue=overvalue,skillid=skillid,flag=flag,time=nowtime()}
	if isdamage and isteammate(source_xid) and isteammate(target_xid) then
		table.insert(currbattle.team_wrong_damage, item)
		return
	end
	if isdamage then
		local source_friend = campinfo(source_xid) > 1
		local source_hostile = campinfo(source_xid) == 1
		local target_friend = campinfo(target_xid) > 1
		local target_hostile = campinfo(target_xid) == 1
		if source_friend and target_hostile then
			if isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_send_damage = currbattle.total_send_damage + value
			end
			table.insert(currbattle.hostile_recv_damage, item)
		end
		if source_hostile and target_friend then
			table.insert(currbattle.hostile_send_damage, item)
			if isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_recv_damage = currbattle.total_recv_damage + value
			end
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
		local camp = campinfo(source_xid)
		if camp>1 then
			if isplayer(source_xid) and isplayer(target_xid) then
				table.insert(currbattle.friend_heal, item)
				currbattle.total_heal = currbattle.total_heal + value
			end
		elseif camp==1 then
			table.insert(currbattle.hostile_heal, item)
		end
	end
end

--friend_send_damage:友方造成伤害 友方造成伤害速率
function cal_fsd(battle)
	battle = battle or currbattle
	local semidata, allitems = {}, battle.friend_send_damage
	table.sort(allitems, function(a,b) return a.source_tid<b.source_tid end)
	table.insert(allitems, {source_tid=-1})
	local currid, currdamage, firsttime, lasttime = 0
	local skillset, targetset = {},{}
	for _,v in ipairs(allitems) do
		if v.source_tid ~= currid then
			if currid ~= 0 then
				local tmpskill, tmptarget = {}, {}
				for k,v in pairs(skillset) do
					table.insert(tmpskill, v)
				end
				for k,v in pairs(targetset) do
					table.insert(tmptarget, v)
				end
				table.sort(tmpskill, function(a,b) return a.damage>b.damage end)
				table.sort(tmptarget, function(a,b) return a.damage>b.damage end)
				table.insert(semidata, {
					tid = currid,
					name = 'what',
					occu = 1,
					damage = currdamage,
					firsttime = firsttime,
					lasttime = lasttime,
					skillset = tmpskill,
					targetset = tmptarget
				})
				if v.source_tid == -1 then break end
			end
			currid = v.source_tid
			currdamage = 0
			firsttime = 7952313600000
			lasttime = 0
			skillset, targetset = {}, {}
		end
		currdamage = currdamage + v.value
		if firsttime>v.time then firsttime = v.time end
		if lasttime<v.time then lasttime = v.time end
		do
			local temp = skillset[v.skillid]
			local value = v.value
			if temp == nil then
				skillset[v.skillid] = {id=v.skillid,damage=value, maxdmg=value, mindmg=value, baoji=0, mingzhong=0}
			else
				temp.damage = temp.damage + value
				if value > temp.maxdmg then temp.maxdmg = value end
				if value < temp.mindmg then temp.mindmg = value end
				if 1 then temp.baoji = temp.baoji + 1 end
				if 1 then temp.mingzhong = temp.mingzhong + 1 end
			end
		end
		do
			local temp = targetset[v.target_tid]
			if temp == nil then
				targetset[v.target_tid] = {id=target_tid,damage=v.value}
			else
				temp.damage = temp.damage + v.value
			end
		end
	end
	battle.friend_send_damage = {}
	table.sort(semidata, function(a,b) return a.damage>b.damage end)
	meter.dump(semidata, 'semidata=')
	return semidata
end

function merge_fsd(semidata, battle)
end
