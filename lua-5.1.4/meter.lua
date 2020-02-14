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
	return {count=0,begintime=nowtime(),team_wrong_damage={},
	friend_send_damage={},friend_recv_damage={},friend_heal={},
	hostile_send_damage={},hostile_recv_damage={},hostile_heal={},
	total_send_damage=0,total_recv_damage=0,total_heal=0}
end

--职业和名字放到一个表里
--current raw   减少一些计算
--current
--finished
--summary = finished + current
allbattle = {}
sumbattle = newbattle()
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

--要支持从头构建 逐条合并 两的成品合并
--friend_send_damage:友方造成伤害 友方造成伤害速率
function friend_damage(battle)
	allitems = meter.clonetable(battle.friend_send_damage)
	table.sort(allitems, function(a, b) return a.source_tid < b.source_tid end)
	local alldata = {}
	local currid = 0
	local currdamage
	local firsttime
	local lasttime
	local item
	for _,v in ipairs(allitems) do
		if v.source_tid ~= currid then
			if currid ~= 0 then
				item.occu = 1
				item.name = 'what'
				item.damage = currdamage
				item.damage_ratio = currdamage / battle.total_send_damage * 100
				item.active_seconds = lasttime - firsttime
				item.period_seconds = nowtime() - battle.begintime
				item.active_ratio = item.period_seconds<=0 and 0 or item.active_seconds / item.period_seconds
				item.damage_rate = item.active_seconds<=0 and 0 or currdamage / item.active_seconds
				alldata[currid] = item
			end
			currid = v.source_tid
			currdamage = 0
			firsttime = 7952313600000
			lasttime = 0
			item = {skillset={},targetset={}}
		end
		currdamage = currdamage + v.value
		if firsttime>v.time then firsttime = v.time end
		if lasttime<v.time then lasttime = v.time end
	end
	if currid ~= 0 then
		item.occu = 1
		item.name = 'what'
		item.damage = currdamage
		item.damage_ratio = currdamage / battle.total_send_damage * 100
		item.active_seconds = lasttime - firsttime
		item.period_seconds = nowtime() - battle.begintime
		item.active_ratio = item.period_seconds<=0 and 0 or item.active_seconds / item.period_seconds
		item.damage_rate = item.active_seconds<=0 and 0 or currdamage / item.active_seconds
		alldata[currid] = item
	end

	--伤害 比例 最大 最小 平均 暴击 命中
	local skillset
	--伤害 比例
	local targetset

	meter.dump(alldata, 'alldata=')
end
