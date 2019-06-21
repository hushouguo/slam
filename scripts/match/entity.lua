
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
    
    ---------------------------------------
    round_total = nil,    
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
    
    ---------------------------------------
    
	enter_match = function(self, match)
	    self.round_total = 0
	    self.round_cards = self.base.round_cards
	    self.max_hold_cards = self.base.max_hold_cards	    
        assert(self.round_cards > 0)
        assert(self.max_hold_cards >= 0)
        self.death = false

	    -- NOTE: hold self.hp
	    
        -- for test, reset self.hp to inithp
	    self.hp = self.base.inithp
	    
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
        
		for cardid, card in pairs(self.bag.cards) do
			self:stack_deal_insert(Card:new(cardid, card.baseid))
		end
		assert(self.stack_deal_size > 0)
		--TODO: check: assert(self.stack_deal_size <= self.base.max_cards)

        self.buffs = {}
        self.buffs_size = 0
        
        self.equip = {}
        self.equip_size = 0
	end,

	exit_match = function(self, match)
	end,

	enter_scene = function(self, scene)
	end,

	exit_scene = function(self, scene)
	end,
	
    die = function(self)
        --assert(not self.death)
		if self.death then return end -- perhaps take multiple damage in one round

        self.death = true
        cc.WriteLog(string.format("entity: %d,%d die", self.id, self.baseid))
        cc.Die(self.id)
        self.copy.scene.match:die_entity(self.id)
    end,

	hp_modify = function(self, value)
		if value == 0 then return end
		self.hp = self.hp + value
		if self.hp < 0 then self.hp = 0 end
		if self.hp > self.maxhp then self.hp = self.maxhp end
		cc.SetCurHP(self.id, self.hp)
		cc.WriteLog(string.format("entity: %d, modify hp value: %d, hp: %d", self.id, value, self.hp))
		if self.hp == 0 then self:die() end
	end,	
	
	maxhp_modify = function(self, value)
		if value == 0 then return end
		self.maxhp = self.maxhp + value
		if self.maxhp < 0 then self.maxhp = 0 end
		assert(self.maxhp > 0)
		cc.SetMaxHP(self.id, self.maxhp)
		cc.WriteLog(string.format("entity: %d, modify maxhp value: %d, maxhp: %d", self.id, value, self.maxhp))
	end,
		
	mp_modify = function(self, value)
		if value == 0 then return end
		assert(self.mp >= 0)
		self.mp = self.mp + value
		if self.mp < 0 then self.mp = 0 end
 		cc.SetCurMP(self.id, self.mp)
		cc.WriteLog(string.format("entity: %d, modify mp value: %d, mp: %d", self.id, value, self.mp))
	end,
		
	strength_modify = function(self, value)
		if value == 0 then return end
		assert(self.strength >= 0)
		self.strength = self.strength + value
		if self.strength < 0 then self.strength = 0 end	
		cc.SetStrength(self.id, self.strength)
		cc.WriteLog(string.format("entity: %d, modify strength value: %d, strength: %d", self.id, value, self.strength))
	end,
	
	armor_modify = function(self, value)
		if value == 0 then return end
		assert(self.armor >= 0)
		self.armor = self.armor + value
		if self.armor < 0 then self.armor = 0 end	
		cc.SetArmor(self.id, self.armor)
		cc.WriteLog(string.format("entity: %d, modify armor value: %d, armor: %d", self.id, value, self.armor))
	end,
	
	shield_modify = function(self, value)
		if value == 0 then return end
		assert(self.shield >= 0)
		self.shield = self.shield + value
		if self.shield < 0 then self.shield = 0 end	
		--cc.SetShield(self.id, self.shield)
		cc.WriteLog(string.format("entity: %d, modify shield value: %d, shield: %d", self.id, value, self.shield))
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
		local card = Card:new(cardid, card_baseid)
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
			buff:layers_modify(buff_layers)
		else
			local buffid = cc.BuffAdd(self.id, buff_baseid, buff_layers)
			assert(self.buffs[buffid] == nil)
			self.buffs[buffid] = Buff:new(self, buffid, buff_baseid, buff_layers)
			self.buffs_size = self.buffs_size + 1
		end
	end,
	buff_remove = function(self, buffid)
		assert(self.buffs_size > 0)
		assert(self.buffs[buffid] ~= nil)
		self.buffs[buffid] = nil
		self.buffs_size = self.buffs_size - 1
		cc.BuffRemove(self.id, buffid)
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

	update = function(self, delta)
		if self.ai ~= nil then self.ai:update(delta) end
	end,
	    
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
		end
		self.random_func = NewRandom(os.time())
		
		self.bag = Bag:new(self)
		self.coord = nil
        self.hp = self.base.inithp
	end,

	destructor = function(self)
	    local function cleanup_cards(self, t)
            for cardid, card in pairs(t) do
                if self.bag.cards[cardid] == nil then
            		cc.WriteLog(string.format("entity: %d, destroy card: %d,%d", self.id, cardid, card.baseid))
                    cc.CardDestroy(self.id, cardid)
                end
            end
	    end
	    
	    cleanup_cards(self, self.stack_deal)
	    cleanup_cards(self, self.stack_discard)
	    cleanup_cards(self, self.stack_exhaust)
	    cleanup_cards(self, self.stack_hold)
	    
	    self.bag:destructor()
	    self.bag = nil
	end
}

function Entity:new(copy, entityid, side)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, entityid, side)
	return object
end

function Entity:breakpoint(card, pick_entityid, bp)
	local pick_entity = pick_entityid ~= nil and self.match.entities[pick_entityid] or nil
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
	self:breakpoint(nil, nil, BreakPoint.ROUND_BEGIN_A)
	
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
    assert(not self.death)
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
		    cc.WriteLog(string.format("entity: %d, cards not enough for cardDeal", self.id))
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
end


--
-- bool card_play(cardid, pick_entityid)
--
function Entity:card_play(cardid, pick_entityid)	
    assert(not self.death)
	local card = self.stack_hold[cardid]
	--assert(card ~= nil)
	if card == nil then
	    cc.WriteLog(string.format(">>>>>>> entity: %d, cardid: %d not exist on card_play", self.id, cardid))
		return
	end	
	
    self:breakpoint(card, pick_entityid, BreakPoint.CARD_PLAY_A)
    
	if card.base.require_career ~= 0 and card.base.require_career ~= self.base.career then 
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, career dismatch", self.id, card.baseid))
		return
	end

	-- TODO: check gender

	--
	-- cost mp
	if self.mp < card.base.cost_mp then 
	    cc.WriteLog(string.format(">>>>>>> entity: %d, card_baseid: %d, mp not enough", self.id, card.baseid))
	    return 
	end
	self:mp_modify(-card.base.cost_mp)

	--
	-- resume mp
	if card.base.resume_mp ~= nil and card.base.resume_mp ~= 0 then
		self:mp_modify(card.base.resume_mp)
	end

	cc.WriteLog(string.format("[%s:%d:%d] 出牌    ---> card: [%s:%d:%d]", 
	    self.base.name.cn, self.id, self.baseid,
	    card.base.name.cn, card.id, card.baseid
	    ))

    self:breakpoint(card, pick_entityid, BreakPoint.CARD_PLAY_Z)

	--
	-- remove from stack_hold
	self:stack_hold_remove(card.id)

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
end

--
-- bool card_discard_judge(cardid)
--
function Entity:card_discard_judge(cardid)
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
end

--
-- void card_discard(cardid, passive)
--
function Entity:card_discard(cardid, passive)
    assert(not self.death)
	local card = self.stack_hold[cardid]
	--assert(card ~= nil)
	if card == nil then
		Error(self, nil, nil, "cardid: " .. tostring(cardid) .. " not exist when card_discard")
		return
	end		
	self:breakpoint(card, nil, BreakPoint.CARD_DISCARD_A)

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
	--	if self.match.isdone then return end -- check match done
	--end

    cc.WriteLog(string.format("[%s:%d:%d] 弃牌     , card: [%s:%d:%d]", 
        self.base.name.cn, self.id, self.baseid,
        card.base.name.cn, card.id, card.baseid
        ))
	
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

	self:breakpoint(card, nil, BreakPoint.CARD_DISCARD_Z)
end

--
-- void round_end()
--
function Entity:round_end()
    assert(not self.death)
    self:breakpoint(nil, nil, BreakPoint.ROUND_END_A)

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

	self:breakpoint(nil, nil, BreakPoint.ROUND_END_Z)
end

