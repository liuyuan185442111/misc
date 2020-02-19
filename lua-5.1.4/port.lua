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

function nowtime()
	return os.time()
end

function isbaoji(flag)
	return (flag & 1) == 0
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
