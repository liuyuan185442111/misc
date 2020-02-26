function death_record_add_activity(death_record, tid, is_operator_player, operator, skillid, delta, hp)
	local midresult = death_record.midresult
	if midresult == nil then
		midresult = {}
		death_record.midresult = midresult
	end
	local record = midresult[tid]
	if record == nil then
		record = {
			id=tid,count=0,occu=getroleoccu(tid),name=getrolename(tid),
			curr_activity_NS=skada.queue.new(2),death_activity={},
		}
		midresult[tid] = record
	end
	record.curr_activity_NS:push({time=nowtime(),
		operator=is_operator_player and getrolename(operator) or getnpcname(operator),
		skillid=skillid,delta=delta,ratio=hp/getrolemaxhp(tid)})
	if hp <= 0 then
		death_record.OK = nil
		death_record.count = death_record.count == nil and 1 or death_record.count + 1
		record.count = record.count + 1
		local curr_activity = record.curr_activity_NS
		record.curr_activity_NS:clear()
		local basetime = curr_activity:back().time
		for i = curr_activity.left, curr_activity.right do
			local temp = curr_activity[i]
			temp.time = temp.time - basetime
			table.insert(record.death_activity, temp)
		end
		table.insert(record.death_activity, {time=basetime,operator=getrolename(tid),skillid=0,delta=0,ratio=0})
	end
end

--结果存储在death_record.result中, 返回false表示上次调用后结果未发生变化
function death_record_cal_death(death_record)
	if death_record.OK then
		return false
	end
	if death_record.count == nil then
		death_record.count = 0
		death_record.result = {}
		death_record.OK = true
		return true
	end
	local result = death_record.result
	if result == nil then
		result = {}
		death_record.result = result
	end
	for _,v in pairs(death_record.midresult) do
		table.insert(result, v)
	end
	table.sort(result, function(a,b) return a.count>b.count end)
	death_record.OK = true
	return true
end

function death_record_finish_battle(death_record)
	death_record_cal_death(death_record)
	death_record.midresult = nil
	for _,v in pairs(death_record.result) do
		v.curr_activity_NS = nil
	end
end

--debug
local function test()
	dofile('common.lua')
	dofile('port.lua')
	local death_record = {}
	local n=8
	death_record_add_activity(death_record,n,true,1,11,21,22)
	death_record_add_activity(death_record,n,true,2,11,22,44)
	death_record_add_activity(death_record,n,true,2,11,22,44)
	death_record_add_activity(death_record,n,true,3,11,-23,21)
	death_record_add_activity(death_record,n,true,4,11,24,45)
	death_record_add_activity(death_record,n,true,5,11,-45,0)

	n=4
	death_record_add_activity(death_record,n,true,1,12,21,22)
	death_record_add_activity(death_record,n,true,2,12,22,44)
	death_record_add_activity(death_record,n,true,2,12,22,44)
	death_record_add_activity(death_record,n,true,3,12,-23,21)
	death_record_add_activity(death_record,n,true,4,12,24,45)
	death_record_add_activity(death_record,n,true,5,11,-45,0)

	death_record_cal_death(death_record)
	death_record_finish_battle(death_record)
	print(skada.dump(death_record))
end
--test()
