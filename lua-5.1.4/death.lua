--death_record的初始状态可以并且最好是一个空表

--每次友方的活动(加血或掉血)都记入当前战斗的死亡管理里
--tid 行为接受对象的tid  operator 行为发起者的tid  is_operator_player 行为发起者是否为玩家
--delta 血量的变化，为正表示加血，为负表示掉血  hp 行为接受对象的最终血量
local function add_death_activity(tid, is_operator_player, operator, skillid, delta, hp)
	local death_record = currbattle.death_record
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
			--临时记录事件，对象死亡时将其放入death_activity, 作为其死亡前的一些事件记录
			temp_activities=skada.queue.new(skada.MAX_DEATH_ACTIVITIES), death_activity={}
		}
		midresult[tid] = record
	end
	record.temp_activities:push({time=skada.nowtime(),
		name=is_operator_player and skada.getrolename(operator) or skada.getnpcname(operator),
		skillid=skillid, delta=delta, ratio=hp/skada.getrolemaxhp(tid)})
	if hp <= 0 then --认为对象死亡
		death_record.OK = nil
		death_record.count = (death_record.count == nil) and 1 or (death_record.count + 1)
		record.count = record.count + 1
		local curr_activity = record.temp_activities
		record.temp_activities = skada.queue.new(skada.MAX_DEATH_ACTIVITIES)
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
local function cal_death_record()
	local death_record = currbattle.death_record
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
local function finish_death_record()
	local death_record = currbattle.death_record
	cal_death_record(death_record)
	death_record.midresult = nil
	for _,deadman in ipairs(death_record.result) do
		deadman.temp_activities = nil
	end
end

--将所有战斗的数据做个合并，这里忽略了死前活动记录
function cal_death_sum(death_record)
	if death_record.OK then
		return false
	end
	local count = 0
	local result = {}
	for _,battle in ipairs(allbattle) do
		for _,v in ipairs(battle.death_record.result) do
			if result[v.id] == nil then
				result[v.id] = {id=v.id, count=v.count, occu=v.occu, name=v.name, death_activity={}}
			else
				result[v.id].count = result[v.id].count + v.count
			end
		end
	end
	death_record.count = count
	death_record.result = result
	death_record.OK = true
	return true
end

local function cal_death(battle)
	if battle == currbattle then
		return cal_death_record(currbattle.death_record)
	elseif battle == sumbattle then
		return cal_death_sum(sumbattle.death_record)
	end
	return true
end

skada.add_death_activity = add_death_activity
skada.finish_death_record = finish_death_record
skada.cal_death = cal_death
