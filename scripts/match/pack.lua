
--
------------------- Pack class -------------------
--

Pack = {
    owner = nil, -- reference to entity of owner
    span = nil, -- span
    
	-- base property
    hp = nil,
    mp = nil,
    maxhp = nil,
    round_mp = nil, -- mp/round_mp
    strength = nil,
    armor = nil,
	shield = nil,
	weakness = nil, -- [0 ~ 1]

    -- match parameters
    round_cards = nil,
    max_hold_cards = nil,
    
    stack_deal = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_discard = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_exhaust = nil, -- {[cardid]->card, [cardid]->card, ...}
    stack_hold = nil, -- {[cardid]->card, [cardid]->card, ...}
    
    -- gold
	gold = nil, -- gold

	-- cards
	cards = nil, -- {[cardid]->card, ...}

	-- items
	items = nil, -- {[itemid]->item, ...}

    -- equips
    equips = nil, -- {[EquipPlaceholder.COPY_OUTSIDE]->{[slot]->item, ...}, [EquipPlaceholder.COPY_INSIDE]->{}}

	-- puppets
	puppets = nil, -- {[entityid]->entity, ...}
	placeholders = nil, -- {[1]->entity, ...}
	
    -- buffs
    buffs = nil, -- {[buffid]->buff, [buffid]->buff, ...}

    -- base value
    base_value = nil,
        
	constructor = function(self, entity, span)
	    self.owner = entity
	    self.span = span
	end,
	
	destructor = function(self)
	    for _, card in pairs(self.cards) do card:destructor() end
	    table.clear(self.cards)

	    for _, item in pairs(self.items) do item:destructor() end
	    table.clear(self.items)

	    for _, item in pairs(self.equips) do item:destructor() end
	    table.clear(self.equips)

	    table.clear(self.puppets) -- puppets is reference
	    table.clear(self.placeholders) -- placeholder is reference

	    for _, buff in pairs(self.buffs) do buff:destructor() end
	    table.clear(self.buffs)
	end
}

function Pack:new(entity, span)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity, span)
	return object
end

--
-- recalculate enhance value
--
function Pack:calc_enhance_value()
    local base = {
        hp = self.hp,
        mp = self.mp,
        maxhp = self.maxhp,
        maxmp = self.maxmp,
        strength = self.strength,
        armor = self.armor,
    	shield = self.shield,
    	weakness = self.weakness
    }

    -- calc base value
    if self.enhance_value ~= nil then
        assert(base.hp >= self.enhance_value.hp)
        base.hp = base.hp - self.enhance_value.hp
        assert(base.mp >= self.enhance_value.mp)
        base.mp = base.mp - self.enhance_value.mp
        assert(base.maxhp >= self.enhance_value.maxhp)
        base.maxhp = base.maxhp - self.enhance_value.maxhp
        assert(base.maxmp >= self.enhance_value.maxmp)
        base.maxmp = base.maxmp - self.enhance_value.maxmp
        assert(base.strength >= self.enhance_value.strength)
        base.strength = base.strength - self.enhance_value.strength
        assert(base.armor >= self.enhance_value.armor)
        base.armor = base.armor - self.enhance_value.armor
        assert(base.shield >= self.enhance_value.shield)
        base.shield = base.shield - self.enhance_value.shield
        assert(base.weakness >= self.enhance_value.weakness)
        base.weakness = base.weakness - self.enhance_value.weakness
    end

    -- clear enhance value
    self.enhance_value = {
        hp = 0, -- 10, 10%
        mp = 0,
        maxhp = 0,
        round_mp = 0,
        strength = 0,
        armor = 0,
        shield = 0,
        weakness = 0,
        buffs = {}
    }

    for _, equip in pairs(self.equips) do
        assert(equip.base.category == ItemCategory.EQUIPABLE)
        
        -- call script_func
        assert(equip.script_func ~= nil and type(equip.script_func) == "function")
        local t = equip.script_func(self.owner.id, equip, self.owner.seed)
        assert(t ~= nil, 'equip: ' .. tostring(equip.baseid))

        self.enhance_value.hp = self.enhance_value.hp + t.hp
        self.enhance_value.mp = self.enhance_value.mp + t.mp
        self.enhance_value.maxhp = self.enhance_value.maxhp + t.maxhp
        self.enhance_value.hp = self.enhance_value.hp + t.hp
        self.enhance_value.hp = self.enhance_value.hp + t.hp
        self.enhance_value.hp = self.enhance_value.hp + t.hp
        self.enhance_value.hp = self.enhance_value.hp + t.hp
        self.enhance_value.hp = self.enhance_value.hp + t.hp
    end
end

--
-- void delivery(pack, span)
--
function Pack:delivery(pack)
    cc.WriteLog(string.format("entity: %d, span delivery from %d to %d", self.owner.id, self.span, pack.span))
    assert(self.span ~= pack.span) -- forbidden
    if self.span > pack.span then
        -- a ->>> b, b ->>> c
        pack.hp = self.hp
        pack.mp = self.mp
        pack.maxhp = self.maxhp
        pack.maxmp = self.maxmp
        pack.strength = self.strength
        pack.armor = self.armor
        pack.shield = self.shield
        pack.weakness = self.weakness

        pack.round_cards = self.round_cards
        pack.max_hold_cards = self.max_hold_cards

        -- don't delivery stack_XXXX
        pack.stack_deal = {}
        pack.stack_discard = {}
        pack.stack_exhaust = {}
        pack.stack_hold = {}

        pack.gold = self.gold

        -- cards
        pack.cards = {}
        for _, card in pairs(self.cards) do
            local newcard = Card:new(pack.owner, card.baseid)
            pack.cards[newcard.id] = newcard
        end

        -- items
        pack.items = {}
        for _, item in pairs(self.items) do
            local newitem = Item:new(pack.owner, item.baseid, item.number)
            pack.items[newitem.id] = newitem
        end

        -- equips
        pack.equips = {}
        for slot, item in pairs(self.equips) do
            local newitem = Item:new(pack.owner, item.baseid, item.number)            
            pack.equips[slot] = newitem
        end

    	-- puppets
    	pack.puppets = {}
    	for _, entity in pairs(self.puppets) do    	    
            pack.puppets[entity.id] = entity -- delivery puppets by reference
    	end

        -- placeholder for puppets
        pack.placeholders = {}
        for slot, entity in pairs(self.placeholders) do
            pack.placeholders[slot] = entity -- delivery placeholder by reference
        end
            	
        -- buffs
        pack.buffs = {}
        -- TODO:
    else
        -- a <<<- b, b <<<- c
        pack.hp = self.hp
    end
end

--
-- void notify_client()
--
function Pack:notify_client()

    cc.BagClearCard(self.owner.id)
    cc.BagClearItem(self.owner.id)
    cc.BagClearPuppet(self.owner.id)
    cc.BagClearEquip(self.owner.id)

    -- notify base property
    cc.SetCurHP(self.owner.id, self.hp)
    cc.SetCurMP(self.owner.id, self.mp)
    cc.SetMaxHP(self.owner.id, self.maxhp)
    cc.SetMaxMP(self.owner.id, self.maxmp)
    cc.SetStrength(self.owner.id, self.strength)
    cc.SetArmor(self.owner.id, self.armor)
-- TODO: pack.shield = self.shield
-- TODO: pack.weakness = self.weakness
    cc.BagSetGold(self.owner.id, self.gold, self.gold)

    -- notify cards
    for _, card in pairs(self.cards) do cc.BagAddCard(self.owner.id, card.id) end
    
    -- notify items
    for _, item in pairs(self.items) do cc.BagAddItem(self.owner.id, item.id) end

    -- notify equips
    for slot, item in pairs(self.equips) do cc.EquipBarAdd(self.owner.id, item.id, slot) end

    -- notify puppets
    for _, target in pairs(self.puppets) do cc.BagAddPuppet(self.owner.id, target.id) end

    -- notify placeholder
    for placeholder, target in pairs(self.placeholders) do 
        cc.BagArrangePlaceholder(self.owner.id, target.id, placeholder) 
    end
    
    -- notify buffs
    for _, buff in pairs(self.buffs) do
        cc.BuffAdd(self.owner.id, buff.baseid, buff.layers)
    end
end

