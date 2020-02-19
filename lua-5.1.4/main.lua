#! /bin/env lua

skada = {}

--所有初始数据和排序后的数据都不导出
function export_data()
	finish_battle()
	local temp = {}
	for _,v in ipairs(allbattle) do
		local t = {}
		t.count = v.count
		t.begintime = v.begintime
		t.endtime = v.endtime
		t.total_send_damage = v.total_send_damage
		t.total_recv_damage = v.total_recv_damage
		t.total_heal = v.total_heal
		--t.twd_summary = v.twd_summary
		t.fsd_summary = v.fsd_summary
		--t.frd_summary1 = v.frd_summary1
		--t.frd_summary2 = v.frd_summary2
		--t.fh_summary1 = v.fh_summary1
		--t.frd_summary2 = v.fh_summary2
		--t.hsd_summary = v.hsd_summary
		--t.hrd_summary = v.hrd_summary
		--t.hh_summary1 = v.hh_summary1
		--t.hh_summary2 = v.hh_summary2

		t.fsd_sort1 = v.fsd_sort1
		t.fsd_sort2 = v.fsd_sort2
		table.insert(temp, t)
	end
	print(skada.dump(temp, 'allbattle='))
end

--for debug
function export_sumbattle()
	cal_sumbattle()
	local t = {}
	local v = sumbattle
	t.count = v.count
	t.begintime = v.begintime
	t.endtime = v.endtime
	t.total_send_damage = v.total_send_damage
	t.total_recv_damage = v.total_recv_damage
	t.total_heal = v.total_heal

	--t.twd_summary = v.twd_summary
	t.fsd_summary = v.fsd_summary
	t.fsd_sort1 = v.fsd_sort1
	t.fsd_sort2 = v.fsd_sort2
	--t.frd_summary1 = v.frd_summary1
	--t.frd_summary2 = v.frd_summary2
	--t.fh_summary1 = v.fh_summary1
	--t.frd_summary2 = v.fh_summary2
	--t.hsd_summary = v.hsd_summary
	--t.hrd_summary = v.hrd_summary
	--t.hh_summary1 = v.hh_summary1
	--t.hh_summary2 = v.hh_summary2

	print(skada.dump(t, 'sumbattle='))
end

--load(str)()
