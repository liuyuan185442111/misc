LONG_TIME_LATER = 7952313600000

--认为友方只能是玩家
local function newbattle()
	return {
		count=0, --有效记录数目
		begintime=skada.nowtime(),

		--用来标识本场战斗
		rival_tid=0,
		rival_level=-1,
		rival_title='总计',

		total_wrong_damage=0,
		team_wrong_damage={}, --队友误伤
		twd_summary={}, --以攻击者tid分组
		twd_sort={}, --以伤害量排序

		total_wesend_damage=0,
		friend_send_damage={}, --友方造成伤害
		fsd_summary={}, --以攻击者tid分组
		fsd_sort1={}, --以伤害量排序
		fsd_sort2={}, --以每秒伤害排序

		total_werecv_damage=0,
		friend_recv_damage={}, --友方受到伤害
		frd_summary1={}, --以被攻击者tid分组
		frd_sort1={}, --以伤害量排序
		frd_summary2={}, --以skillid分组
		frd_sort2={}, --以伤害量排序

		total_wereal_heal=0,
		total_weover_heal=0,
		friend_heal={}, --友方治疗
		fh_summary1={}, --以治疗者tid分组
		fh_sort1={}, --以有效治疗量排序
		fh_sort2={}, --以过量治疗量排序
		fh_sort3={}, --以总治疗量排序
		fh_summary2={}, --以被治疗者tid分组
		fh_sort4={}, --以有效被治疗量排序

		total_hesend_damage=0,
		hostile_send_damage={}, --敌对造成伤害
		hsd_summary={}, --以攻击者tid分组
		hsd_sort={}, --以伤害量排序

		total_herecv_damage=0,
		hostile_recv_damage={}, --敌对受到伤害
		hrd_summary={}, --以被攻击者tid分组
		hrd_sort={}, --以伤害量排序

		total_hereal_heal=0,
		hostile_heal={}, --敌对治疗
		hh_summary1={}, --以治疗者tid分组
		hh_sort1={}, --以有效治疗量排序
		hh_summary2={}, --以被治疗者tid分组
		hh_sort2={}, --以有效被治疗量排序

		death_record = {}, --死亡及生前事件记录，参见death.lua

		--如果_sort系列成员不存盘，重新加载后在会重新计算，用sort_ok来避免重复计算
		--如果_sort系列成员存盘的话就不需要sort_ok了，但导出文件的大小可能会膨胀2倍
		sort_ok = {},
	}
end

local function newsumbattle()
	local begintime,finishtime,count = LONG_TIME_LATER,0,0
	local total_wesend_damage,total_werecv_damage,total_wereal_heal = 0,0,0
	local total_hesend_damage,total_herecv_damage,total_hereal_heal = 0,0,0
	local total_wrong_damage,total_weover_heal = 0,0
	for _,battle in ipairs(allbattle) do
		begintime = math.min(begintime, battle.begintime)
		finishtime = math.max(finishtime, battle.finishtime)
		count = count + battle.count
		total_wesend_damage = total_wesend_damage + battle.total_wesend_damage
		total_werecv_damage = total_werecv_damage + battle.total_werecv_damage
		total_wereal_heal = total_wereal_heal + battle.total_wereal_heal
		total_hesend_damage = total_hesend_damage + battle.total_hesend_damage
		total_herecv_damage = total_herecv_damage + battle.total_herecv_damage
		total_hereal_heal = total_hereal_heal + battle.total_hereal_heal
		total_wrong_damage = total_wrong_damage + battle.total_wrong_damage
		total_weover_heal = total_weover_heal + battle.total_weover_heal
	end
	local temp = newbattle()
	temp.begintime = begintime
	temp.finishtime = finishtime
	temp.count = count
	temp.total_wesend_damage = total_wesend_damage
	temp.total_werecv_damage = total_werecv_damage
	temp.total_wereal_heal = total_wereal_heal
	temp.total_hesend_damage = total_hesend_damage
	temp.total_herecv_damage = total_herecv_damage
	temp.total_hereal_heal = total_hereal_heal
	temp.total_wrong_damage = total_wrong_damage
	temp.total_weover_heal = total_weover_heal
	return temp
end

allbattle = {}
currbattle = newbattle()
--只要allbattle发生变化，sumbattle就要重新new一个
sumbattle = newsumbattle()

local function finish_battle()
	--当前战斗没数据
	if currbattle.count == 0 then
		return
	end
	--当前战斗已存过了
	if currbattle.finishtime then
		return
	end
	currbattle.finishtime = skada.nowtime()
	skada.cal_currbattle()
	skada.finish_death_record(currbattle.death_record)
	table.insert(allbattle, 1, currbattle)
	currbattle = newbattle()
	if #allbattle > skada.MAX_BATTLES then
		table.remove(allbattle)
	end
	sumbattle = newsumbattle()
	skada.export_allbattle()
end

local function begin_battle()
	if currbattle.count == 0 then
		return
	end
	if not currbattle.finishtime then
		finish_battle()
	end
	--TODO 这个地方的处理是有问题的 应该分为非统计中 统计中
	currbattle = newbattle()
end

local function onlogin()
	skada.import_allbattle()
	currbattle = newbattle()
	sumbattle = newsumbattle()
end

--lastvalue 受伤害者或被治疗者的当前血量
local function add_damage_or_heal(source_xid, target_xid, source_tid, target_tid, skillid, flags, isdamage, value, overvalue, lastvalue)
	local item = {
		source_xid=source_xid,target_xid=target_xid,source_tid=source_tid,target_tid=target_tid,
		isdamage=isdamage,value=value,overvalue=overvalue,
		skillid=skillid,flags=flags,time=skada.nowtime()
	}
	--source和target是host或host的队友
	if isdamage and skada.is_self_or_teammate(source_xid, source_tid) and skada.is_self_or_teammate(target_xid, target_tid) then
		table.insert(currbattle.team_wrong_damage, item)
		currbattle.total_wrong_damage = currbattle.total_wrong_damage + value
		skada.add_death_activity(currbattle.death_record, target_tid, true, source_tid, skillid, -value, lastvalue)
		currbattle.count = currbattle.count + 1
		return
	end
	local discard_record = true
	if isdamage then
		local source_camp = skada.getcampinfo(source_xid)
		local target_camp = skada.getcampinfo(target_xid)
		local source_friend = source_camp~=1
		local source_hostile = source_camp==1
		local target_friend = target_camp~=1
		local target_hostile = target_camp==1
		if source_friend and target_hostile then
			if skada.isplayer(source_xid) then
				table.insert(currbattle.friend_send_damage, item)
				currbattle.total_wesend_damage = currbattle.total_wesend_damage + value
			end
			table.insert(currbattle.hostile_recv_damage, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			local a,b,c = skada.getrivalinfo(currbattle.rival_tid, currbattle.rival_level, target_xid, target_tid)
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
			local a,b,c = skada.getrivalinfo(currbattle.rival_tid, currbattle.rival_level, source_xid, source_tid)
			if a then
				currbattle.rival_tid,currbattle.rival_level,currbattle.rival_title = a,b,c
			end
			skada.add_death_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, -value, lastvalue)
			discard_record = false
		end
	else
		if skada.getcampinfo(target_xid)~=1 then --friend
			if skada.isplayer(source_xid) and skada.isplayer(target_xid) then
				table.insert(currbattle.friend_heal, item)
				currbattle.total_wereal_heal = currbattle.total_wereal_heal + value
				currbattle.total_weover_heal = currbattle.total_weover_heal + overvalue
				discard_record = false
			end
			skada.add_death_activity(currbattle.death_record, target_tid, skada.isplayer(source_xid), source_tid, skillid, value, lastvalue)
		else --hostile
			table.insert(currbattle.hostile_heal, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			discard_record = false
		end
	end
	if not discard_record then
		currbattle.count = currbattle.count + 1
	else
		print('discard_record: from '..source_tid..' using skill '..skillid)
	end
end

--------------------------------------------------------------

_G['onlogin'] = onlogin
_G['begin_battle'] = begin_battle
_G['finish_battle'] = finish_battle
_G['add_damage_or_heal'] = add_damage_or_heal
