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

function isbaoji()
	return true
end

--职业 玩家名字 怪物名字查询机制
