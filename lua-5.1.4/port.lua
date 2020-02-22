#! /usr/bin/env lua

--3 friendly  2 neutral  1 hostile  0 none
function getcampinfo(xid)
	if xid < 100 then return 3 end
	if xid > 100 then return 1 end
	return math.random(4)
end

function isplayer(xid)
	return true
	--return #xid > 2 and xid[1] == '0' and x[2] == ':'
end

function isteammate(xid)
	if not isplayer(xid) then
		return false
	end
	return false
end

--boss*0x1000 玩家*0x0100 精英*0x0010 普通*0x0000
function getrivalinfo(curr_tid, curr_level, target_tid)
	if curr_tid == target_tid then
		return nil
	end
	local target_type, target_level = 'boss', 0
	if target_type == 'role' then
	else
		if target_type == 'boss' then
		elseif target_type == 'jing' then
		else
		end
	end
	if target_level > curr_level then
		return target_tid, target_level,
		target_type=='role' and getrolename(target_tid) or getnpcname(target_tid)
	end
end

local lasttime = 0
function nowtime()
	if os.time() <= lasttime then
		lasttime = lasttime + 1
	else
		lasttime = os.time()
	end
	return lasttime
end

function isbaoji(flag)
	return (flag & 1) ~= 0
end

--TODO 角色/npc/技能信息 可以考虑做缓存
--但角色信息变化时要对应更新 实现复杂一些
function getroleoccu(tid)
	return 1
end
function getrolename(tid)
	return 'what'
end
function getnpcname(tid)
	return 'npc'
end
function getskilloccu(skillid)
	return 2
end
function getskillname(skillid)
	return 'attack'
end

--根据属性广播消息 维护所有人的maxhp信息 因为c2s_damage里不包含maxhp信息
local role_maxhp_map = {}
function updaterolemaxhp(tid, maxhp)
	role_maxhp_map[tid] = maxhp
end
function getrolemaxhp(tid)
	return role_maxhp_map[tid] or 0
end
