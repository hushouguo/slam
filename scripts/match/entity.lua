
--
------------------- Entity class -------------------
--

Entity = {
    copy = nil, -- reference to Copy instance
    
	id = nil, -- entity.id
	baseid = nil, -- entity.baseid
	base = nil, -- {field_name=field_value}, related entity.xls
	side = nil, -- Side.ALLIES, Side.ENEMY: allies, enemy
	ai = nil,
	random_func = nil,

    bag = nil, -- Bag object
    coord = nil, -- coord: {x = ?, y = ?}

    record = nil, -- record of entity
    
    ---------------------------------------
    round_cards = nil,
    max_hold_cards = nil,
    
    death = nil,

    hp = nil,
    mp = nil,
    maxhp = nil,
    strength = nil,
    armor = nil,
	shield = nil,
	weakness = nil, -- [0 ~ 1] 

    stack_deal = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_deal_size = nil,
    stack_discard = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_discard_size = nil,
    stack_exhaust = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_exhaust_size = nil,
    stack_hold = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_hold_size = nil,
    
    buffs = nil, -- {[buffid]->buff, [buffid]->buff, ...}
    buffs_size = nil,
    
    equip = nil, -- {[itemid]=item, ...}
    equip_size = nil,

	    
	constructor = function(self, copy, entityid, side)
	    self.copy = copy
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
            self.record = record.unserialize(entityid)
            assert(self.record ~= nil)
		end
		self.random_func = NewRandom(os.time())
		
		self.bag = Bag:new(self)
		self.coord = nil
        self.hp = self.base.inithp
	end,

	destructor = function(self)
	    self.bag:destructor()
	    self.bag = nil
	end,
    
    ---------------------------------------
    
	enter_match = function(self, match)
	    self.round_cards = self.base.round_cards
	    self.max_hold_cards = self.base.max_hold_cards	    
        assert(self.round_cards > 0)
        assert(self.max_hold_cards >= 0)
        self.death = false

	    -- NOTE: hold self.hp
	    
        -- for test, reset self.hp to inithp
	    -- self.hp = self.base.inithp
	    
        self.mp = self.base.round_mp
        self.maxhp = self.base.maxhp
		assert(self.hp > 0)
		assert(self.maxhp > 0)
		assert(self.hp <= self.maxhp)
		assert(self.mp > 0)		
		cc.SetCurHP(self.id, self.hp)
		cc.SetMaxHP(self.id, self.maxhp)
		cc.SetCurMP(self.id, self.mp)

        self.strength = self.base.strength
        self.armor = self.base.armor        
		self.shield = self.base.shield
        assert(self.strength >= 0)
        assert(self.armor >= 0)
		assert(self.shield >= 0)
        cc.SetStrength(self.id, self.strength)
        cc.SetArmor(self.id, self.armor)
		--cc.SetShield(self.id, self.shield)

		self.weakness = 1 

        self.stack_deal = {}
        self.stack_deal_size = 0
        self.stack_discard = {}
        self.stack_discard_size = 0
        self.stack_exhaust = {}
        self.stack_exhaust_size = 0
        self.stack_hold = {}
        self.stack_hold_size = 0

        -- add init_cards from bag.cards
		for cardid, card in pairs(self.bag.cards) do
			self:stack_deal_insert(Card:new(self, cardid, card.baseid))
		end
		-- assert(self.stack_deal_size > 0)
		if self.stack_deal_size == 0 then
			cc.WriteLog(string.format(">>>>>> entity: %d NO cards, func: %s", self.id, Function()))
		end
		--TODO: check: assert(self.stack_deal_size <= self.base.max_cards)

        self.buffs = {}
        self.buffs_size = 0

		-- add init_buffs
        for buff_baseid, buff_layers in pairs(self.base.init_buffs) do
            self:buff_add(buff_baseid, buff_layers)
        end
        
        self.equip = {}
        self.equip_size = 0
        
        cc.WriteLog(string.format("entity: %d enter match", self.id))
	end,

	exit_match = function(self, match)
	    local function cleanup_cards(self, t, from)
            for cardid, card in pairs(t) do
                if self.bag.cards[cardid] == nil then
            		cc.WriteLog(string.format("entity: %d, destroy card: %d,%d, from: %s", self.id, cardid, card.baseid, from))
                    cc.CardDestroy(self.id, cardid)
                end
            end
	    end
	    
	    cleanup_cards(self, self.stack_deal, 'stack_deal')
	    cleanup_cards(self, self.stack_discard, 'stack_discard')
	    cleanup_cards(self, self.stack_exhaust, 'stack_exhaust')
	    cleanup_cards(self, self.stack_hold, 'stack_hold')
	    
        cc.WriteLog(string.format("entity: %d exit match", self.id))
	end,

	enter_scene = function(self, scene)
        cc.WriteLog(string.format("entity: %d enter scene", self.id))
	end,

	exit_scene = function(self, scene)
        cc.WriteLog(string.format("entity: %d exit scene", self.id))
	end,
	
    die = function(self)
        --assert(not self.death)
		if self.death then return end -- perhaps take multiple damage in one round

        self.death = true
        cc.WriteLog(string.format("entity: %d,%d die", self.id, self.baseid))
        cc.Die(self.id)
        self.copy.scene.match:die_entity(self.id)
    end,

	hp_add = function(self, value)
		if value == 0 then return end
		self.hp = self.hp + value
		if self.hp < 0 then self.hp = 0 end
		if self.hp > self.maxhp then self.hp = self.maxhp end
		cc.SetCurHP(self.id, self.hp)
		cc.WriteLog(string.format("entity: %d, %s, add hp value: %d, hp: %d", self.id, self.base.name.cn, value, self.hp))
		if value < 0 then
		    self:breakpoint(nil, nil, ENTITY_SUFFER_DAMAGE)
		end
		if self.hp == 0 then self:die() end
	end,	
	
	maxhp_add = function(self, value)
		if value == 0 then return end
		self.maxhp = self.maxhp + value
		if self.maxhp < 0 then self.maxhp = 0 end
		assert(self.maxhp > 0)
		cc.SetMaxHP(self.id, self.maxhp)
		cc.WriteLog(string.format("entity: %d, %s, add maxhp value: %d, maxhp: %d", self.id, self.base.name.cn, value, self.maxhp))
	end,
		
	mp_add = function(self, value)
		if value == 0 then return end
		assert(self.mp >= 0)
		self.mp = self.mp + value
		if self.mp < 0 then self.mp = 0 end
 		cc.SetCurMP(self.id, self.mp)
		cc.WriteLog(string.format("entity: %d, %s, add mp value: %d, mp: %d", self.id, self.base.name.cn, value, self.mp))
	end,
		
	strength_add = function(self, value)
		if value == 0 then return end
		assert(self.strength >= 0)
		self.strength = self.strength + value
		if self.strength < 0 then self.strength = 0 end	
		cc.SetStrength(self.id, self.strength)
		cc.WriteLog(string.format("entity: %d, %s, add strength value: %d, strength: %d", self.id, self.base.name.cn, value, self.strength))
	end,
	
	armor_add = function(self, value)
		if value == 0 then return end
		assert(self.armor >= 0)
		self.armor = self.armor + value
		if self.armor < 0 then self.armor = 0 end	
		cc.SetArmor(self.id, self.armor)
		cc.WriteLog(string.format("entity: %d, %s, add armor value: %d, armor: %d", self.id, self.base.name.cn, value, self.armor))
	end,
	
	shield_add = function(self, value)
		if value == 0 then return end
		assert(self.shield >= 0)
		self.shield = self.shield + value
		if self.shield < 0 then self.shield = 0 end	
		--cc.SetShield(self.id, self.shield)
		cc.WriteLog(string.format("entity: %d, %s, add shield value: %d, shield: %d", self.id, self.base.name.cn, value, self.shield))
	end,
		
	--
	-- stack deal
	--
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
		local card = Card:new(self, cardid, card_baseid)
		self:stack_hold_insert(card)
		cc.WriteLog(string.format("entity: %d, newone card: %d,%d", self.id, cardid, card_baseid))
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
	end,


	--
	-- Buff
	-- 
	buff_find = function(self, buff_baseid)
		for _, buff in pairs(self.buffs) do
			if buff.baseid == buff_baseid then return buff end
		end
		return nil
	end,
	buff_add = function(self, buff_baseid, buff_layers)
		local buff = self:buff_find(buff_baseid)
		if buff ~= nil then
			buff:layers_add(buff_layers)
		else
		    self:breakpoint(nil, nil, BUFF_ADD_A)
			local buffid = cc.BuffAdd(self.id, buff_baseid, buff_layers)
			assert(self.buffs[buffid] == nil)
			self.buffs[buffid] = Buff:new(self, buffid, buff_baseid, buff_layers)
			self.buffs_size = self.buffs_size + 1
		    self:breakpoint(nil, nil, BUFF_ADD_Z)
		end
	end,
	buff_remove = function(self, buffid)
		assert(self.buffs_size > 0)
		assert(self.buffs[buffid] ~= nil)
		self:breakpoint(nil, nil, BUFF_REMOVE_A)
		self.buffs[buffid]:destructor()
		self.buffs[buffid] = nil
		self.buffs_size = self.buffs_size - 1
		cc.BuffRemove(self.id, buffid)
		self:breakpoint(nil, nil, BUFF_REMOVE_Z)
	end,

	update = function(self, delta)
		if self.ai ~= nil then self.ai:update(delta) end
	end
}

function Entity:new(copy, entityid, side)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, entityid, side)
	return object
end

----------------------------------------------------- match --------------------------------------------------

function Entity:breakpoint(card, pick_entityid, bp)
	local pick_entity = pick_entityid ~= nil and self.copy.scene.match.entities[pick_entityid] or nil
	if card ~= nil and card.base.enable_script then
		card.script_func(self, card, nil, pick_entity, bp)
	end
	for _, buff in pairs(self.buffs) do
		if buff.base.enable_script then
			buff.script_func(self, card, buff, pick_entity, bp)
		end
	end
end

--
-- void round_begin()
--
function Entity:round_begin()
    assert(not self.death)
    
	cc.WriteLog(string.format("*************************************** 回合: %d, entity: %d ******************************", self.copy.scene.match.round_total, self.id))
    
	self:breakpoint(nil, nil, BreakPoint.ROUND_BEGIN_A)
	
	--
	-- resume mp
	--
	assert(self.mp >= 0 and self.base.round_mp > 0)
	if self.mp < self.base.round_mp then
		self:mp_add(self.base.round_mp - self.mp)
	end

	self:breakpoint(nil, nil, BreakPoint.ROUND_BEGIN_Z)
	
	--
	-- deal card
	--
	self:card_deal()
end

--
-- void card_deal()
--
function Entity:card_deal()
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end
    
    self:breakpoint(nil, nil, BreakPoint.CARD_DEAL_A)
    
	local round_cards = self.round_cards
	assert(round_cards > 0)
	cc.WriteLog(string.format("[%s:%d:%d] 发牌", self.base.name.cn, self.id, self.baseid))
	
	while round_cards > 0 do
		assert(self.stack_deal_size >= 0)
		if self.stack_deal_size == 0 then
			self:shuffle_stackdiscard() -- stack_deal not enough for round_cards
		end
		--assert(self.stack_deal_size > 0)
		if self.stack_deal_size == 0 then
		    cc.WriteLog(string.format("entity: %d, cards not enough for cardDeal, func: %s", self.id, Function()))
			break
		end
		local cardid = table.random(self.stack_deal, self.stack_deal_size, self.random_func)
		assert(cardid ~= nil and self.stack_deal[cardid] ~= nil)
		local card = self.stack_deal[cardid]
		self:stack_hold_insert(card)
		self:stack_deal_remove(cardid)
		round_cards = round_cards - 1
		cc.WriteLog(string.format("    card: %s,%d,%d", card.base.name.cn, card.id, card.baseid))
	end
	
	self:breakpoint(nil, nil, BreakPoint.CARD_DEAL_Z)
end

--
-- void card_draw_from_stackdeal(draw_cards)
--
function Entity:card_draw_from_stackdeal(draw_cards)
	while draw_cards > 0 and self.stack_deal_size > 0 do
		local cardid = table.random(self.stack_deal, self.stack_deal_size, self.random_func)
		assert(cardid ~= nil and self.stack_deal[cardid] ~= nil)
		self:stack_hold_insert(self.stack_deal[cardid])
		self:stack_deal_remove(cardid)
		draw_cards = draw_cards - 1
	end
end

--
-- void card_draw_from_stackdiscard(draw_cards)
--
function Entity:card_draw_from_stackdiscard(draw_cards)
	while draw_cards > 0 and self.stack_discard_size > 0 do
		local cardid = table.random(self.stack_discard, self.stack_discard_size, self.random_func)
		assert(cardid ~= nil and self.stack_discard[cardid] ~= nil)
		self:stack_hold_insert(self.stack_discard[cardid])
		self:stack_discard_remove(cardid)
		draw_cards = draw_cards - 1
	end
end

--
-- void card_draw_from_stackexhaust(draw_cards)
--
function Entity:card_draw_from_stackexhaust(draw_cards)
	while draw_cards > 0 and self.stack_exhaust_size > 0 do
		local cardid = table.random(self.stack_exhaust, self.stack_exhaust_size, self.random_func)
		assert(cardid ~= nil and self.stack_exhaust[cardid] ~= nil)
		self:stack_hold_insert(self.stack_exhaust[cardid])
		self:stack_exhaust_remove(cardid)
		draw_cards = draw_cards - 1
	end
end

--
-- bool card_play_judge(cardid, pick_entityid)
--
function Entity:card_play_judge(cardid, pick_entityid)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

	local card = self.stack_hold[cardid]
	assert(card ~= nil)

    --
    -- check card is enable play
    --
	if not card.base.enable_play then
	    -- cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d play card not allow", self.id, card.baseid))
		return false
	end

	--
	-- check career
	--
	if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
	    -- cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, career dismatch", self.id, card.baseid))
		return false
	end

	--
	-- TODO: check gender

	--
	-- check require MP
	if self.mp < card.base.cost_mp then
	    -- cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, mp not enough", self.id, card.baseid))
		return false
	end

	--
	-- check target type
	if pick_entityid ~= nil then
		local targets = CardGetTargets(self, card, pick_entityid)
		return targets ~= nil and targets[pick_entityid] ~= nil 
	end

	return true
end


--
-- bool card_play(cardid, pick_entityid)
--
function Entity:card_play(cardid, pick_entityid)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

	local card = self.stack_hold[cardid]
	if card == nil then
	    cc.WriteLog(string.format(">>>>>>> entity: %d, cardid: %d not exist, func: %s", self.id, cardid, Function()))
		return false
	end	

	if not card.base.enable_play then
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d play card not allow, func: %s", self.id, card.baseid, Function()))
		return false
	end
	
    self:breakpoint(card, pick_entityid, BreakPoint.CARD_PLAY_A)
    
	if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, career dismatch, func: %s", self.id, card.baseid, Function()))
		return false
	end

	-- TODO: check gender

	--
	-- cost mp
	if self.mp < card.base.cost_mp then 
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, mp not enough, func: %s", self.id, card.baseid, Function()))
	    return false
	end
	self:mp_add(-card.base.cost_mp)

	--
	-- resume mp
	if card.base.resume_mp ~= nil and card.base.resume_mp ~= 0 then
		self:mp_add(card.base.resume_mp)
	end

	cc.WriteLog(string.format("[%s:%d:%d] 出牌    ---> card: [%s:%d:%d]", 
	    self.base.name.cn, self.id, self.baseid,
	    card.base.name.cn, card.id, card.baseid
	    ))

	--
	-- remove from stack_hold
	self:stack_hold_remove(card.id)

    self:breakpoint(card, pick_entityid, BreakPoint.CARD_PLAY_Z)

	--
	-- decide to into discard, exhaust or destroy
	if card.base.into_stackdiscard then self:stack_discard_insert(card)			
	elseif card.base.into_stackexhaust then	self:stack_exhaust_insert(card) 
	--elseif card.base.into_destroy then
	else
    	cc.WriteLog(string.format("[%s:%d:%d] 销毁卡牌     , card: [%s:%d:%d]", 
    	    self.base.name.cn, self.id, self.baseid,
    	    card.base.name.cn, card.id, card.baseid
    	    ))
		cc.CardDestroy(self.id, cardid)
	end

	return true
end

--
-- bool card_discard_judge(cardid)
--
function Entity:card_discard_judge(cardid)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

	local card = self.stack_hold[cardid]
	assert(card ~= nil)
	
	--
	-- check card is allow to discard
	--
	if not card.base.enable_discard then
	    -- cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, disable discard", self.id, card.baseid))
		return false
	end

	return true
end

--
-- void card_discard(cardid, passive)
--
function Entity:card_discard(cardid, passive)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

	local card = self.stack_hold[cardid]
	if card == nil then
	    cc.WriteLog(string.format(">>>>>>> entity: %d, cardid: %d not found, func: %s", self.id, cardid, Function()))
		return
	end		

	--
	-- check card is allow to discard
	--
	if not card.base.enable_discard then
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, discard card not allow, func: %s", self.id, card.baseid, Function()))
		--
		-- in PASSIVE mode, force to discard or destroy this card
		if not passive then return end
	end

	self:breakpoint(card, nil, BreakPoint.CARD_DISCARD_A)

	--
	-- Settle when discarding card not in PASSIVE mode
	--if not passive and card.base.settle_discard then
	--	if self.match.isdone then return end -- check match done
	--end

    cc.WriteLog(string.format("[%s:%d:%d] 弃牌     , card: [%s:%d:%d]", 
        self.base.name.cn, self.id, self.baseid,
        card.base.name.cn, card.id, card.baseid
        ))
	
	--
	-- remove from stack_hold
	self:stack_hold_remove(card.id)

	self:breakpoint(card, nil, BreakPoint.CARD_DISCARD_Z)
	
	--
	-- decide to into discard, exhaust or destroy
	if card.base.into_stackdiscard then self:stack_discard_insert(card)			
	elseif card.base.into_stackexhaust then	self:stack_exhaust_insert(card) 
	--elseif card.base.into_destroy then
	else
		cc.CardDestroy(self.id, cardid)
	end
end

--
-- void round_end()
--
function Entity:round_end()
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    self:breakpoint(nil, nil, BreakPoint.ROUND_END_A)

	--
	-- discard extra cards
	--
	while self.stack_hold_size > 0 and self.stack_hold_size > self.max_hold_cards do
		local cardid = self:stack_hold_retrieve()
		self:card_discard(cardid, true)
	end
	
	self:breakpoint(nil, nil, BreakPoint.ROUND_END_Z)
end


----------------------------------------------------- event --------------------------------------------------
--
-- bool event_reward(args, reward_index, entry_index)
--
function Entity:event_reward(args, reward_index, entry_index)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    if args.rewards == nil then
        cc.WriteLog(string.format(">>>>>>> args.rewards is nil, func: %s", Function()))
        return false
    end
    
    if args.rewards[reward_index] == nil then -- entity cancel reward
        cc.WriteLog(string.format(">>>>>>> args.rewards not reward_index: %s, func: %s", tostring(reward_index), Function()))
        return false
    end
    
    local reward = args.rewards[reward_index] 
    
    local function reward_cards(self, cards, entry_index)
        if cards[entry_index] == nil then
            cc.WriteLog(string.format(">>>>>>> reward.cards not entry_index: %s, func: %s", tostring(entry_index), Function()))
            return false
        end
        local card_baseid = cards[entry_index]
        local card_base = cc.LookupTable("Card", card_baseid)
        if card_base == nil then
            cc.WriteLog(string.format(">>>>>>> reward found illegal card_baseid: %s, func: %s", tostring(card_baseid), Function()))
            return false
        end
        cc.WriteLog(string.format("reward card: %d", card_baseid))
        self.bag:add_card(card_baseid)
        cc.WriteLog(string.format("entity: %d, reward card: %d,%s", self.id, card_baseid, card_base.name.cn))
    end
    
    local function reward_items(self, items, entry_index)
        if items[entry_index] == nil then
            cc.WriteLog(string.format(">>>>>>> rewards.items not entry_index: %s, func: %s", tostring(card_baseid), Function()))
            return false
        end
        local item_baseid = entry_index
        local item_number = items[entry_index]
        local item_base = cc.LookupTable("Item", item_baseid)
        if item_base == nil then
            cc.WriteLog(string.format(">>>>>>> reward found illegal item_baseid: %s, func: %s", tostring(item_baseid), Function()))
            return false
        end
        cc.WriteLog(string.format("reward item: %d, number: %d", item_baseid, item_number))
        self.bag:add_item(item_baseid, item_number)
        cc.WriteLog(string.format("entity: %d, reward item: %d,%s", self.id, item_baseid, item_base.name.cn))
    end
    
    if reward.cards ~= nil then
        return reward_cards(self, reward.cards, entry_index)
    elseif reward.items ~= nil then
        return reward_items(self, reward.items, entry_index)
    elseif reward.gold ~= nil then
        cc.WriteLog(string.format("entity: %d, reward gold: %d", self.id, reward.gold))
        self.bag:add_gold(reward.gold)
    else
        cc.WriteLog(string.format(">>>>>>> unhandle reward, func: %s", Function()))
        table.dump(reward, 'reward')
        return false
    end
    
    return true
end

--
-- bool purchase_card(card_baseid, args)
--
function Entity:purchase_card(card_baseid, args)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    if args.cards == nil then
        cc.WriteLog(string.format(">>>>>>> args.cards is nil, func: %s", Function()))
        return false
    end

    local function check_card_baseid(args, card_baseid)
        for _, t in pairs(args.cards) do
            if t.card_baseid == card_baseid then return true end
        end
        return false
    end

    if not check_card_baseid(args, card_baseid) then
        cc.WriteLog(string.format(">>>>>>> not found card_baseid: %d in args, func: %s", card_baseid, Function()))
        return false
    end

    local card_base = cc.LookupTable("Card", card_baseid)
    if card_base == nil or card_base.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> illegal card_baseid: %d, func: %s", card_baseid, Function()))
        return false
    end

    local price_gold = card_base.price_gold
    -- TODO: discount

    if self.bag.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.bag.gold, price_gold, Function()))
        return false
    end

    self.bag:add_gold(-price_gold)
    self.bag:add_card(card_baseid, 1)
    cc.WriteLog(string.format("entity: %d, buy card: %d,%s, cost gold: %d", self.id, 
        card_baseid, card_base.name.cn, price_gold
    ))

    return true
end


--
-- bool purchase_item(item_baseid, args)
--
function Entity:purchase_item(item_baseid, args)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    if args.items == nil then
        cc.WriteLog(string.format(">>>>>>> args.items is nil, func: %s", Function()))
        return false
    end

    local function check_item_baseid(args, item_baseid)
        for _, t in pairs(args.items) do
            if t.item_baseid == item_baseid then return true end
        end
        return false
    end

    if not check_item_baseid(args, item_baseid) then
        cc.WriteLog(string.format(">>>>>>> not found item_baseid: %d in args, func: %s", item_baseid, Function()))
        return false
    end

    local item_base = cc.LookupTable("Item", item_baseid)
    if item_base == nil or item_base.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> illegal item_baseid: %d, func: %s", item_baseid, Function()))
        return false
    end

    local price_gold = item_base.price_gold
    -- TODO: discount

    if self.bag.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.bag.gold, price_gold, Function()))
        return false
    end

    self.bag:add_gold(-price_gold)
    self.bag:add_item(item_baseid, 1)
    cc.WriteLog(string.format("entity: %d, buy item: %d,%s, cost gold: %d", self.id, 
        item_baseid, item_base.name.cn, price_gold
    ))

    return true
end

--
-- bool destroy_card(cardid, args)
--
function Entity:destroy_card(cardid, args)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    if args.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> args.price_gold is nil, func: %s", Function()))
        return false
    end

    local card = self.bag.cards[cardid]
    if card == nil then
        cc.WriteLog(string.format(">>>>>>> not found cardid: %d, func: %s", cardid, Function()))
        return false
    end

    local price_gold = args.price_gold
    -- TODO: discount

    if self.bag.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.bag.gold, price_gold, Function()))
        return false
    end

    self.bag:add_gold(-price_gold)
    self.bag:remove_card(cardid)
    cc.WriteLog(string.format("entity: %d, destroy card: %d,%d,%s, cost gold: %d", self.id, 
        card.id, card.baseid, card.base.name.cn, price_gold
    ))

    cc.CardDestroy(self.id, cardid)

    return true
end

--
-- bool levelup_card(cardid, args)
--
function Entity:levelup_card(cardid, args)    
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end

    if args.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> args.price_gold is nil, func: %s", Function()))
        return false
    end

    local card_old = self.bag.cards[cardid]
    if card_old == nil then
        cc.WriteLog(string.format(">>>>>>> not found cardid: %d, func: %s", cardid, Function()))
        return false
    end

    local price_gold = args.price_gold
    -- TODO: discount

    if self.bag.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.bag.gold, price_gold, Function()))
        return false
    end

    local card_baseid_new = card_old.baseid + 1
    if cc.LookupTable("Card", card_baseid_new) == nil then 
        cc.WriteLog(string.format(">>>>>>> not exist next level: %d card, func: %s", card_baseid_new, Function()))
        return false 
    end
    
    self.bag:remove_card(cardid)
    local card_new = self.bag:add_card(card_baseid_new)
    cc.WriteLog(string.format("entity: %d, levelup card: %d,%d,%s to card: %d,%d,%s, cost gold: %d", self.id, 
        card_old.id, card_old.baseid, card_old.base.name.cn,
        card_new.id, card_new.baseid, card_new.base.name.cn, price_gold
    ))
    
    return true
end

