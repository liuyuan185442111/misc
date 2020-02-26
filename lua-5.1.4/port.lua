--3 friendly  2 neutral  1 hostile  0 none
local function getcampinfo(xid)
	if xid < 100 then return 3 end
	if xid > 100 then return 1 end
	return math.random(4)
end

local function isplayer(xid)
	return true
end

function isteammate(xid, tid)
	if not isplayer(xid) then
		return false
	end
	return false
end

function getrivalinfo(curr_tid, curr_level, target_xid, target_tid)
	return target_tid, 10, 'what'
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

function getroleoccu(tid)
	return 1
end
function getrolename(tid)
	return 'what'
end
function getnpcname(tid)
	return 'npc'
end
function getskillname(skillid)
	return 'attack'
end

function getrolemaxhp(tid)
	return 1
end

skada.getcampinfo = getcampinfo
skada.isplayer = isplayer
