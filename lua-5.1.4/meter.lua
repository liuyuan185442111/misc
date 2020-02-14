#! /bin/env lua

math.randomseed(1001086)
local function isfriend(xid)
	return math.random(2)==1
end
local function ishostile(xid)
	return math.random(2)==1
end
local function isplayer(xid)
	return math.random(2)==1
end
local function isteammate(xid)
	if not isplayer(xid) then
		return false
	end
	return math.random(2)==1
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

local function clonetable(org)
	return {table.unpack(org)}
end

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
		if isfriend(source_xid) or isfriend(target_xid) then
			table.insert(currbattle.team_wrong_damage, item)
		end
		return
	end
	if isdamage then
		local source_friend = isfriend(source_xid)
		local source_hostile = ishostile(source_xid)
		local target_friend = isfriend(target_xid)
		local target_hostile = ishostile(target_xid)
		if source_friend and target_friend then
			if isplayer(source_xid) then --source是玩家
				table.insert(currbattle.friend_send_damage, item)
			end
			if target_xid then --target是玩家
				table.insert(currbattle.friend_recv_damage, item)
			end
			currbattle.total_send_damage = currbattle.total_send_damage + value
		end
		if source_friend and target_hostile then
			if source_xid then --source是玩家
				table.insert(currbattle.friend_send_damage, item)
			end
			table.insert(currbattle.hostile_recv_damage, item)
			currbattle.total_send_damage = currbattle.total_send_damage + value
		end
		if source_hostile and target_friend then
			table.insert(currbattle.hostile_send_damage, item)
			if target_xid then --target是玩家
				table.insert(currbattle.friend_recv_damage, item)
			end
			currbattle.total_recv_damage = currbattle.total_recv_damage + value
		end
		if source_hostile and target_hostile then
			table.insert(currbattle.hostile_send_damage, item)
			table.insert(currbattle.hostile_recv_damage, item)
			currbattle.total_recv_damage = currbattle.total_recv_damage + value
		end
	else
		if 1 then --source是友方
			table.insert(currbattle.friend_heal, item)
			currbattle.total_heal = currbattle.total_heal + value
		else --source是敌方
			table.insert(currbattle.hostile_heal, item)
		end
	end
end

--TODO
function friend_damage(battle)
	--先挑出友方来，按source_xid分组，每组按伤害量排序
	battle = clonespecificitems(battle, 'sourcecamp', 1)
	table.sort(battle, function(a, b) return a.source_xid < b.source_xid end)
	local flext = {}
	local currid = 0
	local damage = 0
	local item = {}
	for _,v in ipairs(battle) do
		if v.source_xid ~= currid then
			currid = v.source_xid
		end
			flext[v.source_xid] = {damage=0,item=v}
	end
	battle = clonetable(battle)
	local totol_damage=0
	for _,v in ipairs(battle) do
		totol_damage = totol_damage + v.damage
	end
	table.sort(battle,function(a,b) return a.damage < b.damage end)
	dump(battle)
end

----------------------------------------test
dofile('skada.lua')
begin_battle()
add_damage_or_heal(1,2,3,4,35,6,7,8,9)
add_damage_or_heal(1,2,3,4,25,6,7,8,9)
begin_battle()
add_damage_or_heal(1,2,3,4,5,6,7,8,9)
finish_battle()
dump(currbattle, 'curr=')
dump(allbattle, 'all=')
