#! /usr/bin/env lua

--3 friendly  2 neutral  1 hostile  0 none
function campinfo(xid)
	if xid < 100 then return 3 end
	if xid > 100 then return 1 end
	return math.random(4)
end

function isplayer(xid)
	return true
end

function isteammate(xid)
	if not isplayer(xid) then
		return false
	end
	return false
end

function nowtime()
	return os.time()
end

function isbaoji(flag)
	return flag & 1
end

--职业 角色名字 npc名字 技能名字 可以考虑做缓存
--但角色名字职业变化时要对应更新 实现复杂一些
function getoccu()
	return 1
end
function getrolename()
	return 'what'
end

function getnpcname()
	return 'npc'
end
function getskillname()
	return 'attack'
end

function getentryinfo(tid, isplayer)
	if isplayer==false then
		return 0, getnpcname(tid)
	end
	return getoccu(tid), getrolename(tid)
end
