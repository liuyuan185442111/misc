--以下三个函数都对表death_record进行操作，初始状态death_record可以并且最好是一个空表

--tid 行为接受对象的tid  operator 行为发起者的tid  is_operator_player 行为发起者是否为玩家
--delta 血量的变化，为正表示加血，为负表示掉血  hp 行为接受对象的最终血量
local function add_death_activity(death_record, tid, is_operator_player, operator, skillid, delta, hp)
	local midresult = death_record.midresult
	if midresult == nil then
		midresult = {}
		death_record.midresult = midresult
	end
	local record = midresult[tid]
	if record == nil then
		record = {
			--count是死亡计数
			id=tid, count=0, occu=skada.getroleoccu(tid), name=skada.getrolename(tid),
			--curr_activity_NS临时记录事件，对象死亡时将其放入death_activity, 作为其死亡前的一些事件记录
			curr_activity_NS=skada.queue.new(skada.MAX_DEATH_ACTIVITIES), death_activity={}
		}
		midresult[tid] = record
	end
	record.curr_activity_NS:push({time=skada.nowtime(),
		name=is_operator_player and skada.getrolename(operator) or skada.getnpcname(operator),
		skillid=skillid, delta=delta, ratio=hp/skada.getrolemaxhp(tid)})
	if hp <= 0 then --认为对象死亡
		death_record.OK = nil
		death_record.count = (death_record.count == nil) and 1 or (death_record.count + 1)
		record.count = record.count + 1
		local curr_activity = record.curr_activity_NS
		record.curr_activity_NS:clear()
		local dietime = curr_activity:back().time
		for i = curr_activity.left, curr_activity.right do
			local temp = curr_activity[i]
			temp.time = temp.time - dietime
			table.insert(record.death_activity, temp)
		end
		--最后放入一条死亡记录，这里name表示死亡者的名字
		table.insert(record.death_activity, {time=dietime,name=skada.getrolename(tid),skillid=0,delta=0,ratio=0})
	end
end

--结果存储在death_record.result中，结果按死亡次数排序
--返回false表示上次调用后结果未发生变化
local function cal_death_record(death_record)
	if death_record.OK then
		return false
	end
	if death_record.count == nil then
		death_record.count = 0
		death_record.result = {}
		death_record.OK = true
		return true
	end
	death_record.result = skada.trans_table(death_record.midresult)
	table.sort(death_record.result, function(a,b) return a.count>b.count end)
	death_record.OK = true
	return true
end

--一场战斗结束的时候调用
local function finish_death_record(death_record)
	cal_death_record(death_record)
	death_record.midresult = nil
	for _,deadman in ipairs(death_record.result) do
		deadman.curr_activity_NS = nil
	end
end

--begin of test
local function test()
	if dofile then
		dofile('common.lua')
		dofile('port.lua')
	end
	local death_record = {}
	local n=8
	add_death_activity(death_record,n,true,1,11,21,22)
	add_death_activity(death_record,n,true,2,11,22,44)
	add_death_activity(death_record,n,true,2,11,22,44)
	add_death_activity(death_record,n,true,3,11,-23,21)
	add_death_activity(death_record,n,true,4,11,24,45)
	add_death_activity(death_record,n,true,5,11,-45,0)

	n=4
	add_death_activity(death_record,n,true,1,12,21,22)
	add_death_activity(death_record,n,true,2,12,22,44)
	add_death_activity(death_record,n,true,2,12,22,44)
	add_death_activity(death_record,n,true,3,12,-23,21)
	add_death_activity(death_record,n,true,4,12,24,45)
	add_death_activity(death_record,n,true,5,11,-45,0)

	cal_death_record(death_record)
	finish_death_record(death_record)
	print(skada.dump(death_record))
end

local args = {...}
if #args == 0 then
	test()
end
--end of test

skada.add_death_activity = add_death_activity
skada.finish_death_record = finish_death_record
