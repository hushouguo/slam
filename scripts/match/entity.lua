
--
------------------- Entity class -------------------
--

Entity = {
	id = nil, -- entity.id
	baseid = nil, -- entity.baseid
	base = nil, -- {field_name=field_value}, related entity.xls
	side = nil, -- Side.ALLIES, Side.ENEMY: allies, enemy
	ai = nil,
	random_func = nil,
	round_total = 0,

    round_cards = nil,
    max_hold_cards = nil,

    --
    -- coord
    --
    coord = nil, -- {x = ? , y = ?}
    
    --
    -- death state
    --
	death = false,
	die = function(self)
	    assert(not self.death)
	    self.death = true
	    cc.Die(self.id)
		Debug(self, nil, nil, "Die")
	    g_match:die_entity(self.id)
	end,
	
	--
	-- attribute value 
	--
	hp = 0,
	hp_modify = function(self, value)
		if value == 0 then return end
		self.hp = self.hp + value
		if self.hp < 0 then self.hp = 0 end
		if self.hp > self.maxhp then self.hp = self.maxhp end
		cc.SetCurHP(self.id, self.hp)
		Debug(self, nil, nil, "hp: " .. tostring(self.hp) .. ", value: " .. tostring(value))
		if self.hp == 0 then self:die() end
	end,	

	maxhp = 0,
	maxhp_modify = function(self, value)
		if value == 0 then return end
		self.maxhp = self.maxhp + value
		if self.maxhp < 0 then self.maxhp = 0 end
		assert(self.maxhp > 0)
		cc.SetMaxHP(self.id, self.maxhp)
		Debug(self, nil, nil, "maxhp: " .. tostring(self.maxhp) .. ", value: " .. tostring(value))
	end,
		
	mp = 0,
	mp_modify = function(self, value)
		if value == 0 then return end
		assert(self.mp >= 0)
		self.mp = self.mp + value
		if self.mp < 0 then self.mp = 0 end
 		cc.SetCurMP(self.id, self.mp)
 		Debug(self, nil, nil, "mp: " .. tostring(self.mp) .. ", value: " .. tostring(value))
	end,
		
	strength = 0,
	strength_modify = function(self, value)
		if value == 0 then return end
		assert(self.strength >= 0)
		self.strength = self.strength + value
		if self.strength < 0 then self.strength = 0 end	
		cc.SetStrength(self.id, self.strength)
		Debug(self, nil, nil, "strength: " .. tostring(self.strength) .. ", value: " .. tostring(value))
	end,
	
	armor = 0,
	armor_modify = function(self, value)
		if value == 0 then return end
		assert(self.armor >= 0)
		self.armor = self.armor + value
		if self.armor < 0 then self.armor = 0 end	
		cc.SetArmor(self.id, self.armor)
		Debug(self, nil, nil, "armor: " .. tostring(self.armor) .. ", value: " .. tostring(value))
	end,
	
	shield = 0,
	shield_modify = function(self, value)
		if value == 0 then return end
		assert(self.shield >= 0)
		self.shield = self.shield + value
		if self.shield < 0 then self.shield = 0 end	
		cc.SetShield(self.id, self.shield)
		Debug(self, nil, nil, "shield: " .. tostring(self.shield) .. ", value: " .. tostring(value))
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
	stack_hold_retrieve = function(self)
		for cardid in pairs(self.stack_hold) do return cardid end
		return nil
	end,

	--
	-- new card and put it into stack_hold
	--
	stack_hold_newone = function(self, card_baseid)
		local cardid = cc.CardNew(self.id, card_baseid)
		assert(cardid ~= nil)
		local card = Card:new(cardid, card_baseid)
		assert(card ~= nil)
		self:stack_hold_insert(card)
		return card
	end,

	--
	-- shuffle stack_discard to stack_deal
	--
	shuffle_stackdiscard = function(self)
		for cardid, card in pairs(self.stack_discard) do
			self:stack_deal_insert(card)
			self:stack_discard_remove(cardid)
		end
		assert(self.stack_discard_size == 0)
		Debug(self, nil, nil, "shuffle from stackdiscard")
	end,
	
	--
	-- shuffle stack_exhaust to stack_deal
	--
	shuffle_stackexhaust = function(self)
		for cardid, card in pairs(self.stack_exhaust) do
			self:stack_deal_insert(card)
			self:stack_exhaust_remove(cardid)
		end
		assert(self.stack_exhaust_size == 0)
		Debug(self, nil, nil, "shuffle from stackexhaust")
	end,

	--
	-- void round_begin()
	--
	round_begin = function(self)
	    assert(not self.death)
		Breakpoint(self, nil, nil, BreakPoint.ROUND_BEGIN_A)
		
		--
		-- effect buff
		--
		for _, buff in pairs(self.buffs) do
			self:buff_handle(buff, BuffSettlePoint.ROUND_BEGIN)
		end
		
		--
		-- resume mp
		--
		assert(self.mp >= 0 and self.base.round_mp > 0)
		if self.mp < self.base.round_mp then
			self:mp_modify(self.base.round_mp - self.mp)
		end

		Breakpoint(self, nil, nil, BreakPoint.ROUND_BEGIN_Z)
		
		--
		-- deal card
		--
		self:card_deal()
	end,
	
	--
	-- void card_deal()
	--
	card_deal = function(self)
	    assert(not self.death)
	    Breakpoint(self, nil, nil, BreakPoint.CARD_DEAL_A)
		local round_cards = self.round_cards
		assert(round_cards > 0)
		local debugstring = "CardDeal: "
		while round_cards > 0 do
			assert(self.stack_deal_size >= 0)
			if self.stack_deal_size == 0 then
				self:shuffle_stackdiscard() -- stack_deal not enough for round_cards
			end
			--assert(self.stack_deal_size > 0)
			if self.stack_deal_size == 0 then
				Error(self, nil, nil, "cards not enough")
				break
			end
			local cardid = table.random(self.stack_deal, self.stack_deal_size, self.random_func)
			assert(cardid ~= nil and self.stack_deal[cardid] ~= nil)
			self:stack_hold_insert(self.stack_deal[cardid])
			self:stack_deal_remove(cardid)
			round_cards = round_cards - 1
			debugstring = debugstring .. " " .. tostring(self.stack_hold[cardid].baseid) .. "(" .. tostring(self.stack_hold[cardid].base.name.cn) .. ")"
		end
        Debug(self, nil, nil, debugstring)		
		Breakpoint(self, nil, nil, BreakPoint.CARD_DEAL_Z)
	end,

	--
	-- void card_draw_from_stackdeal(draw_cards)
	--
	card_draw_from_stackdeal = function(self, draw_cards)
		while draw_cards > 0 and self.stack_deal_size > 0 do
			local cardid = table.random(self.stack_deal, self.stack_deal_size, self.random_func)
			assert(cardid ~= nil and self.stack_deal[cardid] ~= nil)
			self:stack_hold_insert(self.stack_deal[cardid])
			self:stack_deal_remove(cardid)
			draw_cards = draw_cards - 1
		end
	end,
	
	--
	-- void card_draw_from_stackdiscard(draw_cards)
	--
	card_draw_from_stackdiscard = function(self, draw_cards)
		while draw_cards > 0 and self.stack_discard_size > 0 do
			local cardid = table.random(self.stack_discard, self.stack_discard_size, self.random_func)
			assert(cardid ~= nil and self.stack_discard[cardid] ~= nil)
			self:stack_hold_insert(self.stack_discard[cardid])
			self:stack_discard_remove(cardid)
			draw_cards = draw_cards - 1
		end
	end,

	--
	-- void card_draw_from_stackexhaust(draw_cards)
	--
	card_draw_from_stackexhaust = function(self, draw_cards)
		while draw_cards > 0 and self.stack_exhaust_size > 0 do
			local cardid = table.random(self.stack_exhaust, self.stack_exhaust_size, self.random_func)
			assert(cardid ~= nil and self.stack_exhaust[cardid] ~= nil)
			self:stack_hold_insert(self.stack_exhaust[cardid])
			self:stack_exhaust_remove(cardid)
			draw_cards = draw_cards - 1
		end
	end,

	--
	-- bool card_play_judge(cardid, pick_entityid)
	--
	card_play_judge = function(self, cardid, pick_entityid)	
		local card = self.stack_hold[cardid]
		assert(card ~= nil)

		--
		-- check career
		--
		if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
			Error(self, card, nil, "require career: " .. card.base.require_career .. ", self: " .. self.base.career)
			return false
		end

		--
		-- TODO: check gender

		--
		-- check require MP
		if self.mp < card.base.cost_mp then
			Error(self, card, nil, "cost_mp: " .. card.base.cost_mp .. ", self: " .. self.mp)
			return false
		end

		--
		-- check target type
		if pick_entityid ~= nil then
			local targets = CardGetTargets(self, card, pick_entityid)
			return targets ~= nil and targets[pick_entityid] ~= nil 
		end

		return true
	end,

	
	--
	-- bool card_play(cardid, pick_entityid)
	--
	card_play = function(self, cardid, pick_entityid)	
	    assert(not self.death)
		local card = self.stack_hold[cardid]
		--assert(card ~= nil)
		if card == nil then
			Error(self, nil, nil, "cardid: " .. tostring(cardid) .. " not exist when card_play")
			return
		end
	    Breakpoint(self, card, pick_entityid, BreakPoint.CARD_PLAY_A)		
		if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
			Error(self, card, nil, "require career: " .. card.base.require_career .. ", self: " .. self.base.career)
			return
		end

		-- TODO: check gender

		--
		-- cost mp
		if self.mp < card.base.cost_mp then	Error(self, card, nil, "not enough mp")	return end
		self:mp_modify(-card.base.cost_mp)

		--
		-- resume mp
		if card.base.resume_mp ~= nil and card.base.resume_mp ~= 0 then
			self:mp_modify(card.base.resume_mp)
		end

		Debug(self, card, nil, "PlayCard")

        Breakpoint(self, card, pick_entityid, BreakPoint.CARD_PLAY_Z)

		--
		-- remove from stack_hold
		self:stack_hold_remove(card.id)

		--
		-- decide to into discard, exhaust or destroy
		if card.base.into_stackdiscard then self:stack_discard_insert(card)			
		elseif card.base.into_stackexhaust then	self:stack_exhaust_insert(card) 
		--elseif card.base.into_destroy then
		else
			Debug(self, card, nil, "CardDestroy")
			cc.CardDestroy(self.id, cardid)
		end
	end,

	--
	-- bool card_discard_judge(cardid)
	--
	card_discard_judge = function(self, cardid)
		local card = self.stack_hold[cardid]
		assert(card ~= nil)
		
		--
		-- check card is allow to discard
		--
		if not card.base.enable_discard then
			Error(self, card, nil, "discard card: " .. card.baseid .. " not allow")
			return false
		end

		return true
	end,
	
	--
	-- void card_discard(cardid, passive)
	--
	card_discard = function(self, cardid, passive)
	    assert(not self.death)
		local card = self.stack_hold[cardid]
		--assert(card ~= nil)
		if card == nil then
			Error(self, nil, nil, "cardid: " .. tostring(cardid) .. " not exist when card_discard")
			return
		end		
		Breakpoint(self, card, nil, BreakPoint.CARD_DISCARD_A)

		--
		-- check card is allow to discard
		--
		if not card.base.enable_discard then
			Error(self, card, nil, "discard card: " .. card.baseid .. " not allow")
			--
			-- in PASSIVE mode, force to discard or destroy this card
			if not passive then return end
		end

		--
		-- Settle when discarding card not in PASSIVE mode
		--if not passive and card.base.settle_discard then
		--	if g_match.isdone then return end -- check match done
		--end
		
		Debug(self, card, nil, "DiscardCard")

		--
		-- remove from stack_hold
		self:stack_hold_remove(card.id)

		--
		-- decide to into discard, exhaust or destroy
		if card.base.into_stackdiscard then self:stack_discard_insert(card)			
		elseif card.base.into_stackexhaust then	self:stack_exhaust_insert(card) 
		--elseif card.base.into_destroy then
		else
			cc.CardDestroy(self.id, cardid)
		end

		Breakpoint(self, card, nil, BreakPoint.CARD_DISCARD_Z)
	end,

	--
	-- void round_end()
	--
	round_end = function(self)
	    assert(not self.death)
	    Breakpoint(self, nil, nil, BreakPoint.ROUND_END_A)

		--
		-- discard extra cards
		--
		while self.stack_hold_size > 0 and self.stack_hold_size > self.max_hold_cards do
			local cardid = self:stack_hold_retrieve()
			self:card_discard(cardid, true)
		end
		
		--
		-- effect buff
		--
		for _, buff in pairs(self.buffs) do
			self:buff_handle(buff, BuffSettlePoint.ROUND_END)
		end

		Breakpoint(self, nil, nil, BreakPoint.ROUND_END_Z)
	end,

	--
	-- Buff
	-- 
	buffs = nil, -- {[buffid]->buff, [buffid]->buff, ...}
	buffs_size = 0,
	buff_find = function(self, buff_baseid)
		for _, buff in pairs(self.buffs) do
			if buff.baseid == buff_baseid then return buff end
		end
		return nil
	end,
	buff_add = function(self, buff_baseid, buff_layers)
		local buff = self:buff_find(buff_baseid)
		if buff ~= nil then
			buff:layers_modify(buff_layers)
			Debug(self, nil, buff, "UpdateLayers")
		else
			local buffid = cc.BuffAdd(self.id, buff_baseid, buff_layers)
			assert(buffid ~= nil)
			assert(self.buffs[buffid] == nil)
			self.buffs[buffid] = Buff:new(self, buffid, buff_baseid, buff_layers)
			self.buffs_size = self.buffs_size + 1
			Debug(self, nil, self.buffs[buffid], "BuffAdd")
		end
	end,
	buff_remove = function(self, buffid)
		assert(self.buffs_size > 0)
		assert(self.buffs[buffid] ~= nil)
		self.buffs[buffid] = nil
		self.buffs_size = self.buffs_size - 1
		cc.BuffRemove(self.id, buffid)
		Debug(self, nil, nil, "BuffRemove: " .. tostring(buffid))
	end,

	--
	-- void buff_handle(buff, BuffSettlePoint settle_point)
	--
	buff_handle = function(self, buff, settle_point)
		assert(buff)
		assert(self.buffs[buff.id] ~= nil)
		
		--
		-- check survive, BuffSettlePoint.NONE: settle right now
		--
		if (buff.base.survive_type == BuffSurviveType.NONE and settle_point == BuffSettlePoint.NONE) 
				or (buff.base.survive_type == BuffSurviveType.ROUND_BEGIN and settle_point == BuffSettlePoint.ROUND_BEGIN)
				or (buff.base.survive_type == BuffSurviveType.ROUND_END and settle_point == BuffSettlePoint.ROUND_END)
		then
			if buff.survive_value <= 0 then
				self:buff_remove(buff.id)
				return
			end
			buff.survive_value = buff.survive_value - 1 -- wait for next check
		end

		-- TODO: times_effect
	end,	

	--
	-- Equip
	--
	equip = nil, -- {[itemid]=item, ...}

	--
	-- Puppet
	--
	puppet = nil,		

	--
	-- update
	--
	update = function(self, delta)
		if self.ai ~= nil then self.ai:update(delta) end
	end,
	

	constructor = function(self, entityid, side)
		self.id = entityid
		self.baseid = cc.GetBaseid(entityid)
		self.base = cc.LookupTable("Entity", self.baseid)
		assert(self.base ~= nil)
		self.side = side
		assert(self.side == Side.ALLIES or self.side == Side.ENEMY)
		if self.base.category == EntityCategory.MONSTER then
			self.ai = Monster:new(self)
		else
			self.ai = nil
		end
		self.random_func = NewRandom(os.time())
		self.death = false
		self.round_total = 0
		
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
		--assert(self.stack_deal_size <= self.base.max_cards)
		
		self.stack_discard = {}
		self.stack_discard_size = 0
		
		self.stack_exhaust = {}
		self.stack_exhaust_size = 0
		
		self.stack_hold = {}
		self.stack_hold_size = 0
		
		--
		-- init buffs
		--
		self.buffs = {}
		self.buffs_size = 0
		self.buffid_base = 100

		self.equip = {}
		self.puppet = {}

        --
        --
        self.round_cards = self.base.round_cards
        assert(self.round_cards > 0)
        self.max_hold_cards = self.base.max_hold_cards
        assert(self.max_hold_cards >= 0)

		--
		-- init attribute value
		--
		self:maxhp_modify(self.base.maxhp)
		assert(self.maxhp > 0)
		self:hp_modify(self.base.inithp)
		assert(self.hp > 0)
		assert(self.hp <= self.maxhp)
		self:mp_modify(self.base.round_mp)
		assert(self.mp > 0)
		cc.SetMaxMP(self.id, self.base.round_mp)
		self:strength_modify(self.base.strength)
		assert(self.strength >= 0)
		self:armor_modify(self.base.armor)
		assert(self.armor >= 0)
		self:shield_modify(self.base.shield)
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
