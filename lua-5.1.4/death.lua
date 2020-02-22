#! /bin/env lua
dofile('export.lua')
dofile('common.lua')
dofile('port.lua')

function death_record_add_activity(death_record, tid, operator, skillid, delta, hp)
	local record = death_record[tid]
	if record == nil then
		record = {id=tid,count=0,occu=getroleoccu(tid),name=getrolename(tid),curr_activity_NS=skada.queue.new(2),death_activity={}}
		death_record[tid] = record
	end
	record.curr_activity_NS:push({time=nowtime(),operator=tostring(operator),skillid=skillid,delta=delta,ratio=hp/100})
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
		table.insert(record.death_activity, {time=basetime,operator=tostring(tid),skillid=0,delta=0,ratio=0})
	end
end

--如果curr_activity_NS不能导出的话, 结束战斗是时候不调用该函数也无所谓
function death_record_finish_battle(death_record)
	for _,v in pairs(death_record) do
		v.curr_activity_NS = nil
	end
end

function death_record_get(death_record)
	if death_record.OK then
		return false
	end
	local result = death_record.result
	if result == nil then
		result = {}
		death_record.result = result
	end
	for k,v in pairs(death_record) do
		if type(k) == 'number' and v.count > 0 then
			table.insert(result, v)
		end
	end
	table.sort(result, function(a,b) return a.count>b.count end)
	death_record.OK = true
	return true
end

--debug
local death_record = {}
death_record_add_activity(death_record,8,1,11,21,22)
death_record_add_activity(death_record,8,2,11,22,44)
death_record_add_activity(death_record,8,2,11,22,44)
death_record_add_activity(death_record,8,3,11,-23,21)
death_record_add_activity(death_record,8,4,11,24,45)
death_record_add_activity(death_record,8,5,11,-45,0)

n=4
death_record_add_activity(death_record,n,1,12,21,22)
death_record_add_activity(death_record,n,2,12,22,44)
death_record_add_activity(death_record,n,2,12,22,44)
death_record_add_activity(death_record,n,3,12,-23,21)
death_record_add_activity(death_record,n,4,12,24,45)
death_record_add_activity(death_record,n,5,11,-45,0)

death_record_get(death_record)

print(skada.dump(death_record))
