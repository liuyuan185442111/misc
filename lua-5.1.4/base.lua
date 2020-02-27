LONG_TIME_LATER = 7952313600000

--认为友方只能是玩家
local function newbattle()
	return {
		count=0,
		begintime=nowtime(),

		rival_tid=0,
		rival_level=0,
		rival_title='我们',

		total_wrong_damage=0,
		team_wrong_damage={}, --队友误伤
		twd_summary={}, --以tid分组
		twd_sort={}, --队友误伤排序

		total_wesend_damage=0,
		friend_send_damage={}, --友方造成伤害
		fsd_summary={}, --以tid分组
		fsd_sort1={}, --造成伤害排序
		fsd_sort2={}, --每秒伤害排序

		total_werecv_damage=0,
		friend_recv_damage={}, --友方受到伤害
		frd_summary1={}, --以tid分组
		frd_sort1={}, --受到伤害排序
		frd_summary2={}, --以skillid分组
		frd_sort2={}, --承受法术伤害排序

		total_wereal_heal=0,
		total_weover_heal=0,
		friend_heal={}, --友方治疗
		fh_summary1={}, --以治疗者tid分组
		fh_sort1={}, --有效治疗排序
		fh_sort2={}, --过量治疗排序
		fh_sort3={}, --总计治疗排序
		fh_summary2={}, --以被治疗者tid分组
		fh_sort4={}, --获得治疗排序

		total_hesend_damage=0,
		hostile_send_damage={}, --敌对造成伤害
		hsd_summary={}, --以tid分组
		hsd_sort={}, --敌对造成伤害排序

		total_herecv_damage=0,
		hostile_recv_damage={}, --敌对受到伤害
		hrd_summary={}, --以tid分组
		hrd_sort={}, --敌对受到伤害排序

		total_hereal_heal=0,
		hostile_heal={}, --敌对治疗
		hh_summary1={}, --以治疗者tid分组
		hh_sort1={}, --敌方治疗排序
		hh_summary2={}, --以被治疗者tid分组
		hh_sort2={}, --敌方获得治疗排序

		death_record = {}, --参见death.lua
	}
end

local function newsumbattle()
	local begintime,endtime,count,total_wesend_damage,total_werecv_damage,total_wereal_heal = LONG_TIME_LATER,0,0,0,0,0
	for _,v in ipairs(allbattle) do
		begintime = math.min(begintime, v.begintime)
		endtime = math.max(endtime, v.endtime)
		count = count + v.count
		total_wesend_damage = total_wesend_damage + v.total_wesend_damage
		total_werecv_damage = total_werecv_damage + v.total_werecv_damage
		total_wereal_heal = total_wereal_heal + v.total_wereal_heal
	end
	local temp = newbattle()
	temp.begintime = begintime
	temp.endtime = endtime
	temp.count = count
	temp.total_wesend_damage = total_wesend_damage
	temp.total_werecv_damage = total_werecv_damage
	temp.total_wereal_heal = total_wereal_heal
	return temp
end

allbattle = {}
currbattle = newbattle()
sumbattle = newsumbattle()

function begin_battle()
	if currbattle.count == 0 then
		return
	end
	if not currbattle.endtime then
		finish_battle()
	end
	currbattle = newbattle()
end

function finish_battle()
	if currbattle.count == 0 then
		return
	end
	if currbattle.endtime then
		return
	end
	currbattle.endtime = nowtime()
	cal_currbattle()
	death_record_finish_battle(currbattle.death_record)
	table.insert(allbattle, 1, currbattle)
	if #allbattle > 10 then
		table.remove(allbattle)
	end
	sumbattle = newsumbattle()
end

function add_damage_or_heal(source_xid,target_xid,source_tid,target_tid,isdamage,value,overvalue,skillid,flag,targethp)
	local item = {
		source_xid=source_xid,target_xid=target_xid,source_tid=source_tid,target_tid=target_tid,
		isdamage=isdamage,value=value,overvalue=overvalue,skillid=skillid,flag=flag,time=nowtime()
	}
	if isdamage and isteammate(source_xid, source_tid) and isteammate(target_xid, target_tid) then
		table.insert(currbattle.team_wrong_damage, item)
		currbattle.total_wrong_damage = currbattle.total_wrong_damage + value
		death_record_add_activity(currbattle.death_record, target_tid, true, source_tid, skillid, -value, targethp)
		currbattle.count = currbattle.count + 1
		return
	end
	local discard_record = true
	if isdamage then
		local source_camp = skada.getcampinfo(source_xid)
		local target_camp = skada.getcampinfo(target_xid)
		local source_friend = source_camp>1
		local source_hostile = source_camp==1
		local target_friend = target_camp>1
		local target_hostile = target_camp==1
		if source_friend and target_hostile then
			if skada.isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_wesend_damage = currbattle.total_wesend_damage + value
			end
			table.insert(currbattle.hostile_recv_damage, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			local a,b,c = getrivalinfo(currbattle.rival_tid, currbattle.rival_level)
			if a then
				currbattle.rival_tid,currbattle.rival_level,currbattle.rival_title = a,b,c
			end
			discard_record = false
		end
		if source_hostile and target_friend then
			table.insert(currbattle.hostile_send_damage, item)
			currbattle.total_hesend_damage = currbattle.total_hesend_damage + value
			if skada.isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_werecv_damage = currbattle.total_werecv_damage + value
			end
			local a,b,c = getrivalinfo(currbattle.rival_tid, currbattle.rival_level)
			if a then
				currbattle.rival_tid,currbattle.rival_level,currbattle.rival_title = a,b,c
			end
			death_record_add_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, -value, targethp)
			discard_record = false
		end
		--[[
		if source_friend and target_friend then
			if skada.isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_wesend_damage = currbattle.total_wesend_damage + value
			end
			if skada.isplayer(target_xid) then
				table.insert(currbattle.friend_recv_damage, item)
				currbattle.total_werecv_damage = currbattle.total_werecv_damage + value
			end
		end
		if source_hostile and target_hostile then
			table.insert(currbattle.hostile_send_damage, item)
			table.insert(currbattle.hostile_recv_damage, item)
		end
		]]
	else
		local camp = skada.getcampinfo(target_xid)
		if camp>1 then --friend
			if skada.isplayer(source_xid) and skada.isplayer(target_xid) then
				table.insert(currbattle.friend_heal, item)
				currbattle.total_wereal_heal = currbattle.total_wereal_heal + value
				currbattle.total_weover_heal = currbattle.total_weover_heal + overvalue
				discard_record = false
			end
			death_record_add_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, value, targethp)
		elseif camp==1 then --hostile
			table.insert(currbattle.hostile_heal, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			discard_record = false
		end
	end
	if not discard_record then
		currbattle.count = currbattle.count + 1
	end
end
