#! /bin/env lua

meter = {}
dofile('common.lua')
dofile('port.lua')
dofile('damage.lua')

--所有排序后的数据都不导出, 下次导入重新计算
--所有初始数据表全部删除
function export_data()
	local temp = {}
	for _,v in ipairs(allbattle) do
		local t = {}
		t.count = v.count
		t.begintime = v.begintime
		t.endtime = v.endtime
		t.twd_summary = v.twd_summary
		t.total_send_damage = v.total_send_damage
		t.fsd_summary = v.fsd_summary
		t.total_recv_damage = v.total_recv_damage
		t.frd_summary1 = v.frd_summary1
		t.frd_summary2 = v.frd_summary2
		t.total_heal = v.total_heal
		t.fh_summary1 = v.fh_summary1
		t.frd_summary2 = v.fh_summary2
		t.hsd_summary = v.hsd_summary
		t.hrd_summary = v.hrd_summary
		t.hh_summary1 = v.hh_summary1
		t.hh_summary2 = v.hh_summary2
		t.team_wrong_damage = nil
		t.twd_sort = nil
		t.friend_send_damage = nil
		t.fsd_sort1 = nil
		t.fsd_sort2 = nil
		t.friend_recv_damage = nil
		t.frd_sort1 = nil
		t.frd_sort2 = nil
		t.friend_heal = nil
		t.fh_sort1 = nil
		t.fh_sort2 = nil
		t.fh_sort3 = nil
		t.fh_sort4 = nil
		t.hostile_send_damage = nil
		t.hsd_sort = nil
		t.hostile_recv_damage = nil
		t.hrd_sort = nil
		t.hostile_heal = nil
		t.hh_sort1 = nil
		t.hh_sort2 = nil
		t.fsd_sort1 = v.fsd_sort1
		t.fsd_sort2 = v.fsd_sort2
		table.insert(temp, t)
	end
	meter.dump(temp, 'data=')
end
