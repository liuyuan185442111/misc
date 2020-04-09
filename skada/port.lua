--3 friendly  2 neutral  1 hostile  0 none
local function getcampinfo(xid)
	if xid > 100 then return 3 end
	if xid < 100 then return 1 end
	return math.random(4)
end

local function isplayer(xid)
	return true
end

local function is_self_or_teammate(xid, tid)
	if not isplayer(xid) then
		return false
	end
	return false
end

local lasttime = 0
local function nowtime()
	if os.time() <= lasttime then
		lasttime = lasttime + 1
	else
		lasttime = os.time()
	end
	return lasttime
end

local function isbaoji(flag)
	return (flag & 1) ~= 0
end
local function isshanduo(flag)
	return (flag & 2) ~= 0
end
local function isgedang(flag)
	return (flag & 4) ~= 0
end
local function ismingzhong(flag)
	return flag == 0
end

local function getrolename(tid)
	return 'what'
end
local function getroleinfo(tid)
	return getrolename(), 1
end
local nullname = {}
local function getroleinfo2(tid)
	return 'waht2', 0
end
local function getnpcname(tid)
	return 'npc'
end
local function getpawnname(isplayer, tid)
	return isplayer and getrolename(tid) or getnpcname(tid)
end
local function getskillname(skillid)
	return 'attack'
end

local function getrolemaxhp(tid)
	return 1
end

local function getrivalinfo(curr_tid, curr_level, target_xid, target_tid)
	return target_tid, 10, 'topic'
end

local function savedata(region, data)
	print(data)
	return true
end
local function loaddata(region)
	return '', false
end

------------------------------------------------------------
skada.getcampinfo = getcampinfo
skada.isplayer = isplayer
skada.is_self_or_teammate = is_self_or_teammate
skada.nowtime = nowtime
skada.isbaoji = isbaoji
skada.isshanduo = isshanduo
skada.isgedang = isgedang
skada.ismingzhong = ismingzhong
skada.getroleinfo = getroleinfo
skada.nullname = nullname
skada.getroleinfo2 = getroleinfo2
skada.getpawnname = getpawnname
skada.getskillname = getskillname
skada.getrolemaxhp = getrolemaxhp
skada.getrivalinfo = getrivalinfo
skada.savedata = savedata
skada.loaddata = loaddata
skada.MAX_BATTLES = 10
skada.MAX_DEATH_ACTIVITIES = 10
