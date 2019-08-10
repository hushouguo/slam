
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

	-- discount
	discount = nil, -- discount

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
-- card_find
-- 
function Pack:card_find(card_baseid)
	for _, card in pairs(self.cards) do
		if card.baseid == card_baseid then return card end
	end
	return nil
end

--
-- void delivery(pack, span)
--
function Pack:delivery(pack)
    cc.ScriptDebugLog(string.format("entity: %d, span delivery from %d to %d", self.owner.id, self.span, pack.span))
    assert(self.span ~= pack.span) -- forbidden
    if self.span > pack.span then
        -- a ->>> b, b ->>> c
        pack.hp = self.hp
        pack.mp = self.mp
        pack.maxhp = self.maxhp
        pack.round_mp = self.round_mp
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
        pack.maxhp = self.maxhp
        if pack.hp < 0 then pack.hp = 0 end
        if pack.hp > pack.maxhp then pack.hp = pack.maxhp end
    end
end

--
-- void notify_client()
--
function Pack:notify_client()

    cc.ScriptBagClearCard(self.owner.id)
    cc.ScriptBagClearItem(self.owner.id)
    cc.ScriptBagClearPuppet(self.owner.id)
    cc.ScriptBagClearEquip(self.owner.id)
	-- TODO: ClearBuff

    -- notify base property
    cc.ScriptSetCurHP(self.owner.id, self.hp)
    cc.ScriptSetCurMP(self.owner.id, self.mp)
    cc.ScriptSetMaxHP(self.owner.id, self.maxhp)
    cc.ScriptSetMaxMP(self.owner.id, self.round_mp)
    cc.ScriptSetStrength(self.owner.id, self.strength)
    cc.ScriptSetArmor(self.owner.id, self.armor)
-- TODO: pack.shield = self.shield
-- TODO: pack.weakness = self.weakness
    cc.ScriptBagSetGold(self.owner.id, self.gold, self.gold)

    -- notify cards
    for _, card in pairs(self.cards) do cc.ScriptBagAddCard(self.owner.id, card.id) end
    
    -- notify items
    for _, item in pairs(self.items) do cc.ScriptBagAddItem(self.owner.id, item.id) end

    -- notify equips
    for slot, item in pairs(self.equips) do cc.ScriptEquipBarAdd(self.owner.id, slot, item.id) end

    -- notify puppets
    for _, target in pairs(self.puppets) do cc.ScriptBagAddPuppet(self.owner.id, target.id) end

    -- notify placeholder
    for placeholder, target in pairs(self.placeholders) do 
        cc.ScriptBagArrangePlaceholder(self.owner.id, target.id, placeholder) 
    end
    
    -- notify buffs
    for _, buff in pairs(self.buffs) do
        cc.ScriptBuffAdd(self.owner.id, buff.baseid, buff.layers)
    end
end


--
-- void dump()
--
function Pack:dump()
    cc.ScriptDebugLog(string.format("owner: %d,%d,%s pack", self.owner.id, self.owner.baseid, self.owner.base.name.cn))
    cc.ScriptDebugLog(string.format("    span: %d", self.span))
    cc.ScriptDebugLog(string.format("    hp: %d", self.hp))
    cc.ScriptDebugLog(string.format("    mp: %d", self.mp))
    cc.ScriptDebugLog(string.format("    maxhp: %d", self.maxhp))
    cc.ScriptDebugLog(string.format("    round_mp: %d", self.round_mp))
    cc.ScriptDebugLog(string.format("    strength: %d", self.strength))
    cc.ScriptDebugLog(string.format("    armor: %d", self.armor))
    cc.ScriptDebugLog(string.format("    shield: %d", self.shield))
    cc.ScriptDebugLog(string.format("    weakness: %d", self.weakness))
    cc.ScriptDebugLog(string.format("    round_cards: %d", self.round_cards))
    cc.ScriptDebugLog(string.format("    max_hold_cards: %d", self.max_hold_cards))
    cc.ScriptDebugLog(string.format("    stack_deal.size: %d", table.size(self.stack_deal)))
    cc.ScriptDebugLog(string.format("    stack_discard.size: %d", table.size(self.stack_discard)))
    cc.ScriptDebugLog(string.format("    stack_exhaust.size: %d", table.size(self.stack_exhaust)))
    cc.ScriptDebugLog(string.format("    stack_hold.size: %d", table.size(self.stack_hold)))
    cc.ScriptDebugLog(string.format("    gold: %d", self.gold))
    cc.ScriptDebugLog(string.format("    cards:"))
    for _, card in pairs(self.cards) do
    cc.ScriptDebugLog(string.format("        card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
    end
    
    cc.ScriptDebugLog(string.format("    items:"))
    for _, item in pairs(self.items) do
    cc.ScriptDebugLog(string.format("        item: %d,%d,%s", item.id, item.baseid, item.base.name.cn))
    end
    
    cc.ScriptDebugLog(string.format("    equips:"))
    for slot, equip in pairs(self.equips) do
    cc.ScriptDebugLog(string.format("        slot: %d, equip: %d,%d,%s", slot, equip.id, equip.baseid, equip.base.name.cn))
    end
    
    cc.ScriptDebugLog(string.format("    puppets:"))
    for _, target in pairs(self.puppets) do
    cc.ScriptDebugLog(string.format("        puppet: %d,%d,%s", target.id, target.baseid, target.base.name.cn))
    end
    
    cc.ScriptDebugLog(string.format("    placeholders:"))
    for slot, target in pairs(self.placeholders) do
    cc.ScriptDebugLog(string.format("        slot: %d, target: %d,%d,%s", slot, target.id, target.baseid, target.base.name.cn))
    end
    
    cc.ScriptDebugLog(string.format("    buffs:"))
    for _, buff in pairs(self.buffs) do
    cc.ScriptDebugLog(string.format("        buff: %d,%d,%s", buff.id, buff.baseid, buff.base.name.cn))
    end
end

