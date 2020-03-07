allbattle = {}
currbattle = nil
sumbattle = nil

--认为友方只能是玩家
local function newbattle()
	return {
		count=0, --有效记录数目
		begintime=skada.nowtime(),

		--给用户的战斗的标识
		rival_tid=0,
		rival_level=-1,
		rival_title='当前',

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

		--如果"_sort"系列成员不存盘，重新加载后会重新计算，用sort_ok来避免重复计算
		--如果"_sort"系列成员存盘，就不需要sort_ok了，但导出文件的大小可能会膨胀2倍
		sort_ok={},

		death_record={}, --死亡及生前事件记录
	}
end

local function newsumbattle()
	local begintime,finishtime,count = math.maxinteger,0,0
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
	temp.rival_title = '总计'
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

local function finish_battle()
	--当前战斗没数据或已保存过了
	if currbattle.count == 0 or currbattle.finishtime then
		return
	end
	currbattle.finishtime = skada.nowtime()
	skada.cal_currbattle()
	skada.finish_death_record()
	table.insert(allbattle, 1, currbattle)
	if #allbattle > skada.MAX_BATTLES then
		table.remove(allbattle)
	end
	sumbattle = newsumbattle()
	skada.export_allbattle()
end

local function begin_battle()
	finish_battle()
	currbattle = newbattle()
end

local function onlogin()
	skada.import_allbattle()
	for _,battle in ipairs(allbattle) do
		battle.sort_ok = {}
	end
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
		skada.add_death_activity(target_tid, true, source_tid, skillid, -value, lastvalue)
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
			skada.add_death_activity(target_tid, skada.isplayer(source_xid), source_tid, skillid, -value, lastvalue)
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
			skada.add_death_activity(target_tid, skada.isplayer(source_xid), source_tid, skillid, value, lastvalue)
		else --hostile
			table.insert(currbattle.hostile_heal, item)
			currbattle.total_herecv_damage = currbattle.total_herecv_damage + value
			discard_record = false
		end
	end
	if not discard_record then
		currbattle.count = currbattle.count + 1
	else
		print('skada: discard_record from '..source_tid..' using skill '..skillid)
	end
end

local function protect_battles(battle_ids)
	for _,battle in ipairs(allbattle) do
		battle.protected = nil
	end
	for _,id in ipairs(battle_ids) do
		if id > 0 and id <= #allbattle then
			allbattle[id].protected = true
		end
	end
end

--即使被保护也会被删除
local function rm_a_battle(battle_id)
	if battle_id < 1 or battle_id > #allbattle then
		return false
	end
	table.remove(allbattle, battle_id)
	sumbattle = newsumbattle()
	skada.export_allbattle()
	return true
end

--被保护的不会被删除
local function rm_all_battles()
	local oldsize = #allbattle
	local temp = {}
	for _,battle in ipairs(allbattle) do
		if battle.protected then
			table.insert(temp, battle)
		end
	end
	allbattle = temp
	if #allbattle ~= oldsize then
		sumbattle = newsumbattle()
		skada.export_allbattle()
	end
end

local function cal_currbattle()
	skada.cal_fsd_curr()
	local sort_ok = currbattle.sort_ok
	for i=1,skada.MODE_SIZE do
		table.insert(sort_ok, true)
	end
end

onlogin()

------------------------------------------------------------
--以下给数据生产者用
skada.onlogin = onlogin
skada.begin_battle = begin_battle
skada.finish_battle = finish_battle
skada.add_damage_or_heal = add_damage_or_heal
--以下给数据消费者用
skada.protect_battles = protect_battles
skada.rm_a_battle = rm_a_battle
skada.rm_all_battles = rm_all_battles
skada.cal_currbattle = cal_currbattle
