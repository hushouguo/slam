--
-- match.lua
--

<<<<<<< HEAD
require('common')
require('tools')
require('buff')
require('card')
require('entity')
require('monster')

--
------------------- Match class -------------------
=======

local host = _G
local cc = {}

--
------------------- HOST APIs Wrapper -------------------
--

local function api_host_wrapper()
	cc.WriteLog = host.WriteLog
	cc.LookupTable = host.LookupTable
	cc.Milliseconds = host.Milliseconds

	cc.GetBaseid = host.GetBaseid
	cc.GetBuildCards = host.GetBuildCards
	
	cc.StackDealAdd = host.StackDealAdd
	cc.StackDealRemove = host.StackDealRemove
	cc.StackDiscardAdd = host.StackDiscardAdd
	cc.StackDiscardRemove = host.StackDiscardRemove
	cc.StackExhaustAdd = host.StackExhaustAdd
	cc.StackExhaustRemove = host.StackExhaustRemove
	cc.StackHoldAdd = host.StackHoldAdd
	cc.StackHoldRemove = host.StackHoldRemove

	cc.CardNew = host.CardNew
	cc.CardDestroy = host.CardDestroy

	cc.SetCurHP = host.SetCurHP
	cc.SetCurMP = host.SetCurMP
	cc.SetMaxHP = host.SetMaxHP
	cc.SetMaxMP = host.SetMaxMP
	--cc.SetStrength = host.SetStrength
	--cc.SetArmor = host.SetArmor
	--cc.SetShield = host.SetShield
	cc.SetStrength = function(entityid, value) 
		print("entity: " .. entityid .. ", SetStrength: " .. value)
	end
	cc.SetArmor = function(entityid, value) 
		print("entity: " .. entityid .. ", SetArmor: " .. value)
	end
	cc.SetShield = function(entityid, value) 
		print("entity: " .. entityid .. ", SetShield: " .. value)
	end

	cc.BuffAdd = host.BuffAdd
	cc.BuffRemove = host.BuffRemove

	cc.Damage = host.Damage
	cc.Die = host.Die
	cc.Relive = host.Relive

	cc.MatchEnd = host.MatchEnd
end

local runassingle = true
local runasmultiple = false
local runonclient = true
local runonserver = false

--
------------------- table helper function -------------------
--

--
-- dump table
table.dump = function(t, prefix)  
    local dump_cache = {}
    local function sub_dump(t, indent)
        if dump_cache[tostring(t)] then
            cc.WriteLog(indent .. "*" .. tostring(t))
        else
            dump_cache[tostring(t)] = true
            if type(t) == "table" then
                for pos, val in pairs(t) do
                    if type(val) == "table" then
                        cc.WriteLog(indent .. "[" .. pos .. "] => " .. tostring(t) .. " {")
                        sub_dump(val, indent .. string.rep(" ", string.len(pos) + 8))
                        cc.WriteLog(indent .. string.rep(" ", string.len(pos) + 6) .. "}")
                    elseif type(val) == "string" then
                        cc.WriteLog(indent .. "[" .. pos .. '] => "' .. val .. '"')
                    else
                        cc.WriteLog(indent .. "[" .. pos .. "] => " .. tostring(val))
                    end
                end
            else
                cc.WriteLog(indent .. tostring(t))
            end
        end
    end
    if prefix ~= nil then cc.WriteLog("table.dump: " .. tostring(prefix)) end
    if type(t) == "table" then
        cc.WriteLog(tostring(t).." {")
        sub_dump(t, "  ")
        cc.WriteLog("}")
    else
        sub_dump(t, "  ")
    end
--    cc.WriteLog()
end


>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
--

<<<<<<< HEAD
local Match = {
	isstart = false, -- indicate whether the match has started
	isdone = false, -- indicate whether the match has done
	
	--
	-- entity instances
	entities = {}, -- key: entityid, value: Entity instance
	entity_size = function(self, side)
		local size = 0
		for _, entity in pairs(self.entities) do
			if entity.side == side then size = size + 1 end
		end
		return size
	end,
	dead_entities = {}, -- key: entityid, value: Entity instance
=======
--
-- get size of table
table.size = function(t)
	assert(type(t) == "table")
	local size = 0
	for _, v in pairs (t) do 
		if v ~= nil then size = size + 1 end
	end
	return size
end
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e

	--
	-- round list
	round_total = 0, -- 
	round_entityid = nil,
	round_side = Side.ALLIES,
	init_round_list = function(self)
		self.round_total = 0
		self.round_entityid = nil
		self.round_side = Side.ALLIES
		for _, entity in pairs(self.entities) do
			entity.round_total = 0
		end
	end,		
	next_round_entity = function(self)
		self.round_total = self.round_total + 1
		local round_entity = nil
		local round_total = 0
		for _, entity in pairs(self.entities) do
			if entity.side == self.round_side and 
				(round_entity == nil or entity.round_total < round_total) 
			then
				round_entity = entity
				round_total = entity.round_total
			end
		end
		assert(round_entity ~= nil and not round_entity.death)
		round_entity.round_total = round_entity.round_total + 1
		self.round_entityid = round_entity.id
		self.round_side = (self.round_side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
		Debug(nil, nil, nil, "*************************************** 回合: " .. self.round_total .. ", entity: " .. self.round_entityid .. " ******************************")
		return self.round_entityid
	end,
	remove_round_entity = function(self, entityid)
	    assert(entityid ~= nil)
		assert(self.entities[entityid] == nil)
		assert(self.dead_entities[entityid] ~= nil)
	    Debug(nil, nil, nil, "remove round entityid: " .. tostring(entityid))
	end,

<<<<<<< HEAD
	--
	-- bool card_play_judge(entityid, cardid, pick_entityid)
	--
	card_play_judge = function(self, entityid, cardid, pick_entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end
		
		return self.entities[entityid]:card_play_judge(cardid, pick_entityid)
	end,
	
	--
	-- bool card_play(entityid, cardid, pick_entityid)
	--
	card_play = function(self, entityid, cardid, pick_entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_play(cardid, pick_entityid)
	end,

	--
	-- bool card_discard_judge(entityid, cardid)
	--
	card_discard_judge = function(self, entityid, cardid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_discard_judge(cardid)
	end,
	
	--
	-- bool card_discard(entityid, cardid)
	--
	card_discard = function(self, entityid, cardid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_discard(cardid, false)
	end,

	--
	-- void abort()
	-- 
	abort = function(self, entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		self:done(self.entities[entityid].side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
	end,	

	--
	-- void update()
	--
	update = function(self, delta)
		if not self.isstart or self.isdone then
			--Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		
		for _, entity in pairs(self.entities) do
			entity:update(delta)
		end
=======
--
------------------- enum declare -------------------
--


--
-- EntityGender
-- 
local EntityGender = {
	NONE = 0, MALE = 1, FEMALE = 2
}

--
-- EntityCareer
--
local EntityCareer = {
	
}

--
-- EntityCategory
--
local EntityCategory = {
	NONE = 0, PLAYER = 1, MONSTER = 2
}

--
-- CardQuality
--
local CardQuality = {
}

--
-- CardCategory
--
local CardCategory = {
	NONE				=	0,	-- 无种类
	AGGRESSIVE			=	1,	-- 攻击类
	SPELL				=	2,	-- 技能类
	ABILITY				=	3,	-- 能力类
	DUMMY				=	4,	-- 状态类
	CURSE				=	5,	-- 诅咒类
	EQUIPMENT			=	6,	-- 装备类
	PUPPET 				=	7,	-- 随从类	
}

--
-- TargetType
--
local TargetType = {
	NONE				=	0,	-- 无需目标
	PICK_SINGLE_ALLIES	=	1,	-- 对自选单个本方目标
	PICK_SINGLE_ENEMY	=	2,	-- 对自选单个敌方目标
	SELF				=	3,	-- 对自己
	SELF_PUPPET			=	4,	-- 对自己的随从
	ALLIES_ALL			=	5,	-- 对所有盟友
	ALLIES_PUPPET_ALL	=	6,	-- 对所有盟友随从
	ENEMY_ALL			=	7,	-- 对所有敌人
	ENEMY_PUPPET_ALL	=	8,	-- 对所有敌人随从
	ENEMY_RANDOM		=	9,	-- 对随机一个敌人
	ALL 				=	10,	-- 对全体
}

--
-- DamageType
--
local DamageType = {
	NONE = 0, PHYSICAL = 1, SPELL = 2
}

--
-- SurviveTimeType
--
local SurviveTimeType = {
	NONE = 0, ROUND = 1, TIMES = 2
}

--
-- SettleType
--
local SettleType = {
	NONE = 0, ROUND_BEGIN = 1, ROUND_END = 2
}

--
-- DefenseDamageType
--
local DefenseDamageType = {
	NONE = 0, HP = 1, MP = 2
}

--
-- Side
--
local Side = {
	ALLIES = 0, ENEMY = 1
}


--
-- SettlePoint
--
local SettlePoint = {
	PLAYING	= 0, EQUIPING = 1, DISCARDING = 2
}



--
-- match instance
local match = nil


--
------------------- AI functions -------------------
--

local function ai_monster(entity)
	if not match.isstart or match.isdone then return end
	if match.round_entityid ~= entity.id then return end
	if entity.stack_hold_size == 0 then  -- TODO: check MP is enough
		match:round_end(entity.id)
	else
		for cardid in pairs(entity.stack_hold) do
			match:card_play(entity.id, cardid)
			return	-- only play one card for every update
		end
	end
end


--
------------------- Card class -------------------
--

local Card = {
	id = nil, -- card.id
	baseid = nil, -- card.baseid
	base = nil, -- {field_name=field_value}, related entity.xls

	constructor = function(self, cardid, card_baseid)
		self.id = cardid
		self.baseid = card_baseid
		self.base = cc.LookupTable("card", card_baseid)
		assert(self.base ~= nil)
	end
}

function Card:new(cardid, card_baseid)
	local card = {}
	self.__index = self -- Card.__index = function(key) return Card[key] end
	setmetatable(card, self)
	card:constructor(cardid, card_baseid)
	return card
end


--
------------------- Entity class -------------------
--

local Entity = {
	id = nil, -- entity.id
	baseid = nil, -- entity.baseid
	base = nil, -- {field_name=field_value}, related entity.xls
	side = nil, -- Side.ALLIES, Side.ENEMY: allies, enemy
	ai = nil,

	--
	-- attribute value 
	--
	hp = nil,
	hp_modify = function(self, value)
		if value == 0 then return end
		assert(self.hp > 0)
		self.hp = self.hp + value
		if self.hp < 0 then self.hp = 0 end
		cc.SetCurHP(self.id, self.hp)
		if self.hp == 0 then
			match:check_done()
		end
	end,
		
	mp = nil,
	mp_modify = function(self, value)
		if value == 0 then return end
		assert(self.mp > 0)
		self.mp = self.mp + value
		if self.mp < 0 then self.mp = 0 end
 		cc.SetCurMP(self.id, self.mp)
	end,
		
	strength = nil,
	strength_modify = function(self, value)
		if value == 0 then return end
		assert(self.strength >= 0)
		self.strength = self.strength + value
		if self.strength < 0 then self.strength = 0 end	
		cc.SetStrength(self.id, self.strength)
	end,
	
	armor = nil,
	armor_modify = function(self, value)
		if value == 0 then return end
		assert(self.armor >= 0)
		self.armor = self.armor + value
		if self.armor < 0 then self.armor = 0 end	
		cc.SetArmor(self.id, self.armor)
	end,
	
	shield = nil,
	shield_modify = function(self, value)
		if value == 0 then return end
		assert(self.shield >= 0)
		self.shield = self.shield + value
		if self.shield < 0 then self.shield = 0 end	
		cc.SetShield(self.id, self.shield)
	end,
	
	weakness = 1, -- [0 ~ 1]

	--
	-- stack deal
	--
	stack_deal = nil, -- {[cardid]->card, [cardid]->card, ...}
	stack_deal_size = 0,
	stack_deal_insert = function(self, card)
		assert(self.stack_deal[card.id] == nil)
		self.stack_deal[card.id] = card
		self.stack_deal_size = self.stack_deal_size + 1
		cc.StackDealAdd(self.id, card.id)
	end,
	stack_deal_remove = function(self, cardid)
		assert(self.stack_deal[cardid] ~= nil)
		self.stack_deal[cardid] = nil
		self.stack_deal_size = self.stack_deal_size - 1
		assert(self.stack_deal_size >= 0)
		cc.StackDealRemove(self.id, cardid)
	end,

	--
	-- stack discard
	--
	stack_discard = nil, -- {[cardid]->card, [cardid]->card, ...}
	stack_discard_size = 0,
	stack_discard_insert = function(self, card)
		assert(self.stack_discard[card.id] == nil)
		self.stack_discard[card.id] = card
		self.stack_discard_size = self.stack_discard_size + 1
		cc.StackDiscardAdd(self.id, card.id)
	end,
	stack_discard_remove = function(self, cardid)
		assert(self.stack_discard[cardid] ~= nil)
		self.stack_discard[cardid] = nil
		self.stack_discard_size = self.stack_discard_size - 1
		assert(self.stack_discard_size >= 0)
		cc.StackDiscardRemove(self.id, cardid)
	end,

	--
	-- stack exhaust
	--
	stack_exhaust = nil, -- {[cardid]->card, [cardid]->card, ...}
	stack_exhaust_size = 0,
	stack_exhaust_insert = function(self, card)
		assert(self.stack_exhaust[card.id] == nil)
		self.stack_exhaust[card.id] = card
		self.stack_exhaust_size = self.stack_exhaust_size + 1
		cc.StackExhaustAdd(self.id, card.id)
	end,
	stack_exhaust_remove = function(self, cardid)
		assert(self.stack_exhaust[cardid] ~= nil)
		self.stack_exhaust[cardid] = nil
		self.stack_exhaust_size = self.stack_exhaust_size - 1
		assert(self.stack_exhaust_size >= 0)
		cc.StackExhaustRemove(self.id, cardid)
	end,

	--
	-- stack hold
	--
	stack_hold = nil, -- {[cardid]->card, [cardid]->card, ...}
	stack_hold_size = 0,
	stack_hold_insert = function(self, card)
		assert(self.stack_hold[card.id] == nil)
		self.stack_hold[card.id] = card
		self.stack_hold_size = self.stack_hold_size + 1
		cc.StackHoldAdd(self.id, card.id)
	end,
	stack_hold_remove = function(self, cardid)
		assert(self.stack_hold[cardid] ~= nil)
		self.stack_hold[cardid] = nil
		self.stack_hold_size = self.stack_hold_size - 1
		assert(self.stack_hold_size >= 0)
		cc.StackHoldRemove(self.id, cardid)
	end,

	--
	-- dump stack_discard to stack_deal
	--
	stack_discard_shuffle = function(self)
		cc.WriteLog("entity: " .. self.id .. " stack_discard_shuffle")

		for cardid, card in pairs(self.stack_discard) do
			assert(self.stack_deal[cardid] == nil)
			self.stack_deal[cardid] = card
			cc.StackDealAdd(self.id, cardid)

			self.stack_discard[cardid] = nil
			cc.StackDiscardRemove(self.id, cardid)
		end

		self.stack_deal_size = table.size(self.stack_deal)
		self.stack_discard_size = table.size(self.stack_discard)
		assert(self.stack_discard_size == 0)
	end,
	
	--
	-- dump stack_exhaust to stack_deal
	--
	stack_exhaust_shuffle = function(self)
		cc.WriteLog("entity: " .. self.id .. " stack_discard_shuffle")

		for cardid, card in pairs(self.stack_exhaust) do
			assert(self.stack_deal[cardid] == nil)
			self.stack_deal[cardid] = card
			cc.StackDealAdd(self.id, cardid)

			self.stack_exhaust[cardid] = nil
			cc.StackExhaustRemove(self.id, cardid)
		end

		self.stack_deal_size = table.size(self.stack_deal)
		self.stack_exhaust_size = table.size(self.stack_exhaust)
		assert(self.stack_exhaust_size == 0)
	end,

	--
	-- void round_begin()
	--
	round_begin = function(self)
		cc.WriteLog("entity: " .. self.id .. " round begin")
		--
		-- resume mp
		assert(self.mp >= 0 and self.base.round_mp > 0)
		if self.mp < self.base.round_mp then
			self:mp_modify(self.base.round_mp - self.mp)
		end
		--
		-- deal card
		self:card_deal()
	end,
	
	--
	-- void card_deal()
	--
	card_deal = function(self)
		function random_stack_deal(self)
			assert(self.stack_deal_size > 0)
			local n = math.random(0, self.stack_deal_size - 1)
			for cardid in pairs(self.stack_deal) do
				if n == 0 then return cardid end
				n = n - 1
			end
			return nil
		end

		local round_cards = self.base.round_cards -- read from conf
		assert(round_cards ~= nil and round_cards > 0)		
		if self.stack_deal_size < round_cards then
			self:stack_discard_shuffle() -- stack_deal not enough for round_cards
		end
		assert(self.stack_deal_size >= round_cards)
		
		while round_cards > 0 do
			local cardid = random_stack_deal(self)
			assert(cardid ~= nil and self.stack_deal[cardid] ~= nil)
			self:stack_hold_insert(self.stack_deal[cardid])
			self:stack_deal_remove(cardid)
			round_cards = round_cards - 1
		end
	end,

	--
	-- switch case different target type
	--
	switch_target_type = {
		[TargetType.NONE] = function(self, card, pick_entityid, settle_point)
			return {} -- no target
		end,
		[TargetType.PICK_SINGLE_ALLIES] = function(self, card, pick_entityid, settle_point)
			--TODO: check pick_entityid is allies
			return { pick_entityid }
		end,
		[TargetType.PICK_SINGLE_ENEMY] = function(self, card, pick_entityid, settle_point)
			--TODO: check pick_entityid is enemy
			return { pick_entityid }
		end,
		[TargetType.SELF] = function(self, card, pick_entityid, settle_point)
			return { self.id }
		end,
		[TargetType.SELF_PUPPET] = function(self, card, pick_entityid, settle_point)
			-- TODO: return puppet of mine
			return {}
		end,
		[TargetType.ALLIES_ALL] = function(self, card, pick_entityid, settle_point)
			local targets = {}
			for entityid, entity in pairs(match.entities) do
				if entity.side == self.side then table.insert(targets, entityid) end
			end
			return targets
		end,
		[TargetType.ALLIES_PUPPET_ALL] = function(self, card, pick_entityid, settle_point)
			-- TODO: return all puppets of allies
			return {}
		end,
		[TargetType.ENEMY_ALL] = function(self, card, pick_entityid, settle_point)
			local targets = {}
			for entityid, entity in pairs(match.entities) do
				if entity.side ~= self.side then table.insert(targets, entityid) end
			end
			return targets
		end,
		[TargetType.ENEMY_PUPPET_ALL] = function(self, card, pick_entityid, settle_point)
			-- TODO: return all puppets of enemy
			return {}
		end,
		[TargetType.ENEMY_RANDOM] = function(self, card, pick_entityid, settle_point)
			local targets = {}
			for entityid, entity in pairs(match.entities) do
				if entity.side ~= self.side then table.insert(targets, entityid) end
			end
			local n = math.random(0, table.size(targets) - 1)
			for _, entityid in pairs(targets) do
				if n == 0 then return { entityid } end
				n = n - 1
			end
			return {}
		end,
		[TargetType.ALL] = function(self, card, pick_entityid, settle_point)
			local targets = {}
			for entityid, entity in pairs(match.entities) do
				table.insert(targets, entityid)
			end
			return targets
		end,
	},


	--
	-- void card_settle_damage(card, target_entityid, settle_point)
	--
	card_settle_damage = function(self, card, target_entityid)
		local target = match.entities[target_entityid]
		assert(target ~= nil)

		--
		-- damage
		local switch_damage_type = {
			[DamageType.NONE] = function(card, self, target)
				-- no damage
			end,
			[DamageType.PHYSICAL] = function(card, self, target)
				assert(card.base.damage_value ~= nil)
				local damage = (card.base.damage_value + self.strength) * self.weakness
				damage = math.floor(damage + 0.5)
				cc.Damage(target.id, damage)
				if damage < target.armor then
					target:armor_modify(-damage)
				else
					damage = damage - target.armor
					target:armor_modify(-target.armor)
					target:hp_modify(-damage)
				end
			end,
			[DamageType.SPELL] = function(card, self, target)
				assert(card.base.damage_value ~= nil)
				local damage = (card.base.damage_value + self.strength) * self.weakness
				damage = math.floor(damage + 0.5)
				cc.Damage(target.id, damage)
				if damage < target.shield then
					target:shield_modify(-damage)
				else
					damage = damage - target.shield
					target:shield_modify(-target.shield)
					target:hp_modify(-damage)
				end
			end,
		}
		assert(card.base.damage_type ~= nil)
		assert(switch_damage_type[card.base.damage_type] ~= nil)
		switch_damage_type[card.base.damage_type](card, self, target)
		if match.isdone then return end -- match done
		
		--
		-- effect hp
		assert(card.base.effect_hp ~= nil)
		target:hp_modify(card.base.effect_hp)
		if match.isdone then return end -- match done

		--
		-- effect mp
		assert(card.base.effect_mp ~= nil)
		target:mp_modify(card.base.effect_mp)

		--
		-- effect strength
		assert(card.base.effect_strength ~= nil)
		target:strength_modify(card.base.effect_strength)
		
		--
		-- effect armor
		assert(card.base.effect_armor ~= nil)
		target:armor_modify(card.base.effect_armor)

		--
		-- effect shield
		assert(card.base.effect_shield ~= nil)
		target:shield_modify(card.base.effect_shield)

		--
		-- TODO: effect buff		
	end,

	--
	-- void card_settle(card, target_entityid, settle_point)
	--
	card_settle = function(self, card, pick_entityid, settle_point)
		--
		-- cost mp
		assert(card.base.cost_mp ~= nil)
		assert(self.mp >= card.base.cost_mp)
		self:mp_modify(-card.base.cost_mp)

		--
		-- choose target
		assert(card.base.require_target ~= nil)
		assert(self.switch_target_type[card.base.require_target] ~= nil)
		local targets = self.switch_target_type[card.base.require_target](self, card, pick_entityid, settle_point)
		--table.dump(targets, "choose target")

		--
		-- damage settle
		for _, target_entityid in pairs(targets) do
			local settle_times = card.base.settle_times
			assert(settle_times ~= nil)
			while settle_times > 0 do
				self:card_settle_damage(card, target_entityid)
				if match.isdone then return end -- match done
				settle_times = settle_times - 1
			end
		end

		--
		-- TODO: draw_stackdeal

		--
		-- TODO: draw_stackdiscard

		--
		-- TODO: draw_stackexhaust

		--
		-- TODO: shuffle_stackdiscard

		--
		-- TODO: shuffle_stackexhaust


		--
		-- enable script
		assert(card.base.enable_script ~= nil)
		if card.base.enable_script then
			--TODO: loadstring(card.base.script)
		end
	end,
	
	--
	-- void card_play(cardid, target_entityid)
	--
	card_play = function(self, cardid, pick_entityid)
		--
		----------- Precondition ---------
		--
	
		--
		-- check cardid is mine
		local card = self.stack_hold[cardid]
		assert(card ~= nil)

		cc.WriteLog("entity: " .. self.id .. " play card: " .. cardid .. ", baseid: " .. card.baseid)

		--
		-- check career
		assert(card.base.require_career ~= nil and self.base.career ~= nil)
		if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
			cc.WriteLog("card: " .. card.baseid .. " require career: " .. card.base.require_career .. ", self: " .. self.base.career)
			return 
		end

		--
		-- TODO: check gender

		--
		-- check cost MP
		assert(card.base.cost_mp ~= nil)
		if self.mp < card.base.cost_mp then
			cc.WriteLog("card: " .. card.baseid .. " cost_mp: " .. card.base.cost_mp .. ", self: " .. self.mp)
			return 
		end

		--
		----------- Settle ---------
		--

		--
		-- Settle when playing card
		assert(card.base.enable_play ~= nil)
		if card.base.enable_play and card.base.settle_play then
			assert(card.base.settle_play ~= nil)
			if card.base.settle_play then
				self:card_settle(card, pick_entityid, SettlePoint.PLAYING)
				if match.isdone then return end -- match done
			end
		end

		--
		-- Equip & Settle when equiping card
		assert(card.base.enable_equip ~= nil)
		if card.base.enable_equip and card.base.settle_equip then
			-- TODO: equip card
			assert(card.base.settle_equip ~= nil)
			if card.base.settle_equip then
				self:card_settle(card, pick_entityid, SettlePoint.EQUIPING)
				if match.isdone then return end -- match done
			end
		end


		--
		----------- discard card ---------
		--

		assert(card.base.enable_discard ~= nil)
		if card.base.enable_discard then
			--
			-- Settle when discarding card
			assert(card.base.settle_discard ~= nil)
			if card.base.settle_discard then
				self:card_settle(card, pick_entityid, SettlePoint.DISCARDING)
				if match.isdone then return end -- match done
			end
			
			--
			-- decide to into discard, exhaust or equip
			assert(card.base.into_stackdiscard ~= nil and card.base.into_stackexhaust ~= nil and card.base.into_destroy ~= nil)
			if card.base.into_stackdiscard then 
				self:stack_discard_insert(card)			
			elseif card.base.into_stackexhaust then
				self:stack_exhaust_insert(card) 
			--elseif card.base.into_destroy then
			else
				-- TODO: destroy this card
			end

			--
			-- remove from stack_hold
			self:stack_hold_remove(card.id)
		end
	end,

	--
	-- void card_discard(card_baseid)
	--
	card_discard = function(self, cardid)
	end,

	--
	-- void round_end()
	--
	round_end = function(self)
		cc.WriteLog("entity: " .. self.id .. " round end")
	end,
	
	buff = nil, -- {buff_baseid, buff_baseid, ...}
	buff_add = function(self, buff_baseid)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
	end,

	--
	-- void round_end(entityid)
	--
	round_end = function(self, entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return
		end

		self.entities[entityid]:round_end()
				
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()			
	end,

	--
	-- void prepare()
	--
	prepare = function(self)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		Debug(nil, nil, nil, "prepare")
	end,

	--
<<<<<<< HEAD
	-- void add_member(entityid, side)
	--
	add_member = function(self, entityid, side)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end

		if self.entities[entityid] ~= nil then
			Error(nil, nil, nil, "entityid: " .. tostring(entityid) .. " already exit")
			return
		end
		
		self.entities[entityid] = Entity:new(entityid, side)
		--table.dump(entity)
		Debug(nil, nil, nil, "add member: " .. tostring(entityid) .. ", side: " .. tostring(side))
	end,

	--
	-- void start()
	--
	start = function(self)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		
		if self:entity_size(Side.ALLIES) < 1 or self:entity_size(Side.ENEMY) < 1 then
			Error(nil, nil, nil, "ALLIES size: " .. self:entity_size(Side.ALLIES) .. ", ENEMY size: " .. self:entity_size(Side.ENEMY))
			return
		end

		self.isstart = true
		self.isdone = false
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()
	end,

    --
    -- void die_entity(entityid)
    --
    die_entity = function(self, entityid)
        assert(self.entities[entityid] ~= nil)
        local entity = self.entities[entityid]
        assert(entity.death)
        self.entities[entityid] = nil
        assert(self.dead_entities[entityid] == nil)
        self.dead_entities[entityid] = entity
        self:check_done()
        if not self.isdone then
            self:remove_round_entity(entityid)
        end
    end,
    
	--
	-- bool check_done()
	--
	check_done = function(self)
		local sides = {
			[Side.ALLIES] = 0,
			[Side.ENEMY] = 0
		}
		
		for _, entity in pairs(self.entities) do
			assert(sides[entity.side] ~= nil)
			if not entity.death then sides[entity.side] = sides[entity.side] + 1 end
		end
		
		assert(sides[Side.ALLIES] > 0 or sides[Side.ENEMY] > 0)
		if sides[Side.ALLIES] > 0 and sides[Side.ENEMY] > 0 then return false end

		self:done(sides[Side.ALLIES] > 0 and Side.ALLIES or Side.ENEMY)
		return true
	end,

	--
	-- void done(side_victory)
	-- 
	done = function(self, side_victory)
		assert(self.isstart and not self.isdone)
=======
	-- update
	--
	update = function(self, delta)
		if self.base.category == EntityCategory.MONSTER or self.ai ~= nil then
			self:ai()
		end
	end,
	
	--
	-- arg: entity_host is userdata, related with entity of host layer
	--
	constructor = function(self, entityid, side)
		self.id = entityid
		self.baseid = cc.GetBaseid(entityid)
		self.base = cc.LookupTable("entity", self.baseid)
		assert(self.base ~= nil)
		self.side = side
		assert(self.side == Side.ALLIES or self.side == Side.ENEMY)
		self.ai = ai_monster
		
		self.stack_deal = {}
		self.stack_deal_size = 0
		
		--
		-- GetBuildCards: {{cardid = card_baseid}, ...}
		--
		local cards = cc.GetBuildCards(entityid)
		assert(cards ~= nil and type(cards) == "table")
		for cardid, card_baseid in pairs(cards) do
			assert(self.stack_deal[cardid] == nil)
			self:stack_deal_insert(Card:new(cardid, card_baseid))
		end
		--table.dump(self.stack_deal)
		assert(self.stack_deal_size > 0)
		
		self.stack_discard = {}
		self.stack_discard_size = 0
		
		self.stack_exhaust = {}
		self.stack_exhaust_size = 0
		
		self.stack_hold = {}
		self.stack_hold_size = 0
		
		self.buff = {}
		self.equip = {}
		self.puppet = {}

		--
		-- init attribute value
		--
		self.hp = self.base.round_hp
		assert(self.hp > 0)
		self.mp = self.base.round_mp
		assert(self.mp > 0)
		self.strength = self.base.strength
		assert(self.strength >= 0)
		self.armor = self.base.armor
		assert(self.armor >= 0)
		self.shield = self.base.shield
		assert(self.shield >= 0)
		self.weakness = 1 -- [0 ~ 1]
	end
}

function Entity:new(entityid, side)
	local entity = {}
	self.__index = self -- Entity.__index = function(key) return Entity[key] end
	setmetatable(entity, self)
	entity:constructor(entityid, side)
	return entity
end


--
------------------- Match class -------------------
--

local Match = {
	isstart = false, -- indicate whether the match has started
	isdone = false, -- indicate whether the match has done
	
	--
	-- entity instances
	entities = {}, -- key: entityid, value: Entity instance

	--
	-- round list
	round_total = 0, -- 
	round_entityid = nil,
	round_list_index = 0,
	round_list = {}, -- key: integer, start from 0, value: entityid
	init_round_list = function(self)
		function list_exist(self, entityid)
			for _, value in pairs(self.round_list) do
				if value == entityid then return true end
			end
			return false
		end
		
		local side = Side.ALLIES -- first side default is allies side
		local len = table.size(self.entities)
		local index = self.round_list_index
		
		table.clear(self.round_list)
		while table.size(self.round_list) < len do
			for entityid, entity in pairs(self.entities) do
				if entity.side == side and list_exist(self, entityid) == false then
					assert(self.round_list[index] == nil)
					self.round_list[index] = entityid
					index = index + 1
					side = (side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
					break
				end
			end
		end

		table.dump(self.round_list, "round list")
	end,		
	next_round_entity = function(self)
		self.round_total = self.round_total + 1
		if self.round_entityid ~= nil then
			self.round_list_index = self.round_list_index + 1
			self.round_list_index = self.round_list_index % table.size(self.round_list)
		end
		self.round_entityid = self.round_list[self.round_list_index]
		cc.WriteLog("************* Round: " .. self.round_total .. ", entity: " .. self.round_entityid .. " *************")
		return self.round_entityid
	end,

	--
	-- void card_play(entityid, card_baseid)
	--
	card_play = function(self, entityid, cardid, pick_entityid)
		self.entities[entityid]:card_play(cardid, pick_entityid)	
	end,

	--
	-- void card_discard(entityid, card_baseid)
	--
	card_discard = function(self, entityid, cardid)
		self.entities[entityid]:card_discard(cardid)	
	end,

	--
	-- void abort()
	-- 
	abort = function(self)
	end,	

	--
	-- void update()
	--
	update = function(self, delta)
		--cc.WriteLog("update: " .. delta)
		for _, entity in pairs(self.entities) do
			entity:update(delta)
		end		
	end,

	--
	-- void round_end(entityid)
	--
	round_end = function(self, entityid)
		assert(entityid == self.round_entityid)
		local entity = self.entities[entityid]
		entity:round_end()
		
		--cc.WriteLog("=============== Round: " .. self.round_total .. " end")

		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()
	end,

	--
	-- void prepare()
	--
	prepare = function(self)
		cc.WriteLog("match prepare")
	end,

	--
	-- void add_member(entityid, side)
	--
	add_member = function(self, entityid, side)
		assert(self.entities[entityid] == nil)
		self.entities[entityid] = Entity:new(entityid, side)
		--table.dump(entity)
	end,

	--
	-- void start()
	--
	start = function(self)
		assert(table.size(self.entities) >= 2)
		self.isstart = true
		self:init_round_list()
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()
	end,

	--
	-- bool check_done()
	--
	check_done = function(self)
		local sides = {
			[Side.ALLIES] = 0,
			[Side.ENEMY] = 0
		}
		
		for _, entity in pairs(self.entities) do
			assert(sides[entity.side] ~= nil)
			if entity.hp > 0 then sides[entity.side] = sides[entity.side] + 1 end
		end
		
		assert(sides[Side.ALLIES] > 0 or sides[Side.ENEMY] > 0)
		if sides[Side.ALLIES] > 0 and sides[Side.ENEMY] > 0 then return false end

		self:done(sides[Side.ALLIES] > 0 and Side.ALLIES or Side.ENEMY)
		return true
	end,

	--
	-- void done(side_victory)
	-- 
	done = function(self, side_victory)
		assert(self.isstart and not self.isdone)		
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
		-- notify allies members
		for _, entity in pairs(self.entities) do
			if entity.side == Side.ALLIES then
				cc.MatchEnd(entity.id, side_victory == entity.side)
			end
		end
		self.isdone = true
<<<<<<< HEAD
		self.isstart = false
		Debug(nil, nil, nil, "比赛结束, 胜利方: " .. (side_victory == Side.ALLIES and "allies" or "enemy"))
	end,	
    
=======
		cc.WriteLog("match done, victory: " .. side_victory)
	end,	

>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
	constructor = function(self)
		self.isstart = false
		self.isdone = false
		table.clear(self.entities)
<<<<<<< HEAD
		self:init_round_list()
=======
		self.round_list_index = 0
		table.clear(self.round_list)
		self.round_entityid = nil
		self.round_total = 0
		math.randomseed(os.time())
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
	end
}

function Match:new()	
	local match = {}
	self.__index = self -- Match.__index = function(key) return Match[key] end
	setmetatable(match, self)
	match:constructor()
	return match
end


--
------------------- lua APIs -------------------
--

--
-- prepare match
function lua_entry_match_prepare()
<<<<<<< HEAD
	g_match = Match:new()
	g_match:prepare()
=======
	api_host_wrapper()
	match = Match:new()
	match:prepare()
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- add member of allies to match
function lua_entry_add_member(entityid)
<<<<<<< HEAD
	assert(g_match)
	g_match:add_member(entityid, Side.ALLIES)
=======
	assert(match)
	assert(not match.isstart and not match.isdone)
	match:add_member(entityid, Side.ALLIES)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- add member of enemy to match
function lua_entry_add_opponent(entityid)
<<<<<<< HEAD
	assert(g_match)
	g_match:add_member(entityid, Side.ENEMY)
=======
	assert(match)
	assert(not match.isstart and not match.isdone)
	match:add_member(entityid, Side.ENEMY)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- start match
function lua_entry_match_start()
<<<<<<< HEAD
	assert(g_match)
	g_match:start()
end

--
-- the play if can play a card
function lua_entry_card_play_judge(entityid, cardid, pick_entityid)
	assert(g_match)
	return g_match:card_play_judge(entityid, cardid, pick_entityid)
=======
	assert(match)
	assert(not match.isstart and not match.isdone)	
	match:start()
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- the player play a card
function lua_entry_card_play(entityid, cardid, pick_entityid)
<<<<<<< HEAD
	assert(g_match)
	g_match:card_play(entityid, cardid, pick_entityid)
end

--
-- the player if can discard a card
function lua_entry_card_discard_judge(entityid, cardid)
	assert(g_match)
	return g_match:card_discard_judge(entityid, cardid)
=======
	assert(match)
	assert(match.isstart and not match.isdone)
	assert(entityid == match.round_entityid)
	match:card_play(entityid, cardid, pick_entityid)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- the player discard a card
function lua_entry_card_discard(entityid, cardid)
<<<<<<< HEAD
	assert(g_match)
	g_match:card_discard(entityid, cardid)
=======
	assert(match)
	assert(match.isstart and not match.isdone)
	assert(entityid == match.round_entityid)
	match:card_discard(entityid, cardid)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- the player end this round
function lua_entry_round_end(entityid)
<<<<<<< HEAD
	assert(g_match)
	g_match:round_end(entityid)
=======
	assert(match)
	assert(match.isstart and not match.isdone)
	assert(entityid == match.round_entityid)
	match:round_end(entityid)
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end

--
-- call me for every frame
function lua_entry_update(delta)
<<<<<<< HEAD
	if g_match ~= nil then g_match:update(delta) end
=======
	if not match then return end
	if not match.isstart or match.isdone then return end
	match:update(delta)
end


--
------------------- unittest -------------------
--

local function unittest()
	host.LookupTable = function(table_name, baseid)
		if table_name == "entity" then
			local entities = {
				[1001] = {
					baseid = 1001,
					career = 0,
					category = 1,
					level = 0,
					levelup_baseid = 0,
					round_hp = 100,
					round_mp = 3,
					strength = 0,
					armor = 0,
					shield = 0,
					init_cards = {10000,10010,10011,10020,10021,10030,10031,10040,10041,10050},
					max_cards = 200,
					init_buffs = {},
					round_cards = 2,
					max_hold_cards = 0,
					carry_puppets = 0,
					equip_slot = 0,
				},
				[1002] = {
					baseid = 1002,
					career = 0,
					category = 2,
					level = 0,
					levelup_baseid = 0,
					round_hp = 10,
					round_mp = 1,
					strength = 0,
					armor = 0,
					shield = 0,
					init_cards = {10010,10030},
					max_cards = 0,
					init_buffs = {},
					round_cards = 1,
					max_hold_cards = 0,
					carry_puppets = 0,
					equip_slot = 0,
				}
			}
			return entities[baseid]
		elseif table_name == "card" then
			local cards = {
				[10000] = {
					baseid = 10000,
					quality = 0,
					level = 1,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 9,
					damage_type = 1,
					damage_value = 10,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10010] = {
					baseid = 10010,
					quality = 0,
					level = 1,
					levelup_baseid = 10011,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 2,
					damage_type = 9,
					damage_value = 6,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10011] = {
					baseid = 10011,
					quality = 0,
					level = 2,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 2,
					damage_type = 9,
					damage_value = 6,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10020] = {
					baseid = 10020,
					quality = 0,
					level = 1,
					levelup_baseid = 10021,
					require_career = 0,
					require_gender = 0,
					cost_mp = 2,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 9,
					damage_type = 1,
					damage_value = 8,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10021] = {
					baseid = 10021,
					quality = 0,
					level = 2,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 2,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 9,
					damage_type = 1,
					damage_value = 10,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10030] = {
					baseid = 10030,
					quality = 0,
					level = 1,
					levelup_baseid = 10031,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 3,
					damage_type = 0,
					damage_value = 0,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 5,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10031] = {
					baseid = 10031,
					quality = 0,
					level = 2,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 3,
					damage_type = 0,
					damage_value = 0,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 8,
					effect_shield = 0,
					effect_buff = {},					
					draw_stackdeal = 0,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10040] = {
					baseid = 10040,
					quality = 0,
					level = 2,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 9,
					damage_type = 1,
					damage_value = 9,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 1,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10041] = {
					baseid = 10041,
					quality = 0,
					level = 2,
					levelup_baseid = 0,
					require_career = 0,
					require_gender = 0,
					cost_mp = 1,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 9,
					damage_type = 1,
					damage_value = 10,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 0,
					effect_shield = 0,
					effect_buff = {},					
					draw_stackdeal = 2,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
				[10050] = {
					baseid = 10050,
					quality = 0,
					level = 1,
					levelup_baseid = 10051,
					require_career = 0,
					require_gender = 0,
					cost_mp = 0,
					enable_play = true,
					enable_equip = false,
					enable_discard = true,
					settle_play = true,
					settle_discard = false,
					settle_equip = false,
					settle_times = 1,
					into_stackdiscard = true,
					into_stackexhaust = false,
					into_destroy = false,
					require_target = 3,
					damage_type = 0,
					damage_value = 0,
					effect_hp = 0,
					effect_mp = 0,
					effect_strength = 0,
					effect_armor = 8,
					effect_shield = 0,
					effect_buff = {},
					draw_stackdeal = 1,
					draw_stackdiscard = 0,
					draw_stackexhaust = 0,
					shuffle_discardstack = false,
					shuffle_exhauststack = false,
					enable_script = false,
					script = "",
				},
			}
			return cards[baseid]
		end
	end
	host.WriteLog = function(content) print(content) end
	host.Milliseconds = function() return os.time() end

	host.StackDealAdd = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackDealAdd: " .. cardid)
	end
	host.StackDealRemove = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackDealRemove: " .. cardid)
	end
	host.StackDiscardAdd = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackDiscardAdd: " .. cardid)
	end
	host.StackDiscardRemove = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackDiscardRemove: " .. cardid)
	end
	host.StackExhaustAdd = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackExhaustAdd: " .. cardid)
	end
	host.StackExhaustRemove = function(entityid, cardid) 
		cc.WriteLog("entity: " .. entityid .. " StackExhaustRemove: " .. cardid)
	end

	host.SetCurHP = function(entityid, value) 
		cc.WriteLog("entity: " .. entityid .. " SetCurHP: " .. value)
	end
	host.SetCurMP = function(entityid, value) 
		cc.WriteLog("entity: " .. entityid .. " SetCurMP: " .. value)
	end
	host.SetMaxHP = function(entityid, value) 
		cc.WriteLog("entity: " .. entityid .. " SetMaxHP: " .. value)
	end
	host.SetMaxMP = function(entityid, value) 
		cc.WriteLog("entity: " .. entityid .. " SetMaxMP: " .. value)
	end	

	host.Damage = function(entityid, value) 
		cc.WriteLog("entity: " .. entityid .. " Damage: " .. value)
	end
	host.Die = function(entityid) 
		cc.WriteLog("entity: " .. entityid .. " Die")
	end
	host.Relive = function(entityid) end
	host.MatchEnd = function(entityid, victory) 
		cc.WriteLog("entity: " .. entityid .. " victory: " .. (victory and "true" or "false"))
	end

	local Member = {
		stack_hold = {},

		id = nil,
		baseid = nil,
		career = 1,
		cards = nil,

		constructor = function(self, id, baseid, cards)
			self.id = id
			self.baseid = baseid
			self.cards = cards
			self.stack_hold = {}
		end
	}
	function Member:new(id, baseid, cards)
		local member = {}
		self.__index = self
		setmetatable(member, self)
		member:constructor(id, baseid, cards)
		return member
	end
	
	local members = {
		player = Member:new(1, 1001, {10000,10010,10011,10020,10021,10030,10031,10040,10041,10050}),
		monster = Member:new(2, 1002, {10010,10030})
	}

	host.GetBaseid = function(entityid) 
		return entityid == members.player.id and members.player.baseid or members.monster.baseid
	end
	host.GetBuildCards = function(entityid) 
		return entityid == members.player.id and members.player.cards or members.monster.cards
	end

	host.StackHoldAdd = function(entityid, cardid)
		if entityid == members.player.id then
			members.player.stack_hold[cardid] = cardid
		end
		cc.WriteLog("entity: " .. entityid .. " HoldAdd cardid: " .. cardid)
	end
	host.StackHoldRemove = function(entityid, cardid)
		if entityid == members.player.id then
			members.player.stack_hold[cardid] = nil
		end		
		cc.WriteLog("entity: " .. entityid .. " HoldRemove cardid: " .. cardid)
	end

	lua_entry_match_prepare()
	lua_entry_add_member(members.player.id)
	lua_entry_add_opponent(members.monster.id)
	lua_entry_match_start()

	local function Sleep(n)
		os.execute("sleep " .. n)
	end

	while true do
		Sleep(1)
		if table.size(members.player.stack_hold) > 0 then
			for cardid in pairs(members.player.stack_hold) do
				lua_entry_card_play(members.player.id, cardid)
				if match.isdone then os.exit(0) end
			end
			table.clear(members.player.stack_hold)
			lua_entry_round_end(members.player.id)
		end
		lua_entry_update(1000)
	end
>>>>>>> c421f7cbc04964dc6894a3ad1e42dd690f8b993e
end
unittest()

