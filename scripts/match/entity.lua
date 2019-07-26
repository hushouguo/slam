
--
------------------- Entity class -------------------
--

Entity = {
    copy = nil, -- reference to Copy instance    

	id = nil, -- entity.id
	baseid = nil, -- entity.baseid
	base = nil, -- {field_name=field_value}, related entity.xls
	ai = nil,
	random_func = nil,
	
    master = nil, -- reference to entity of master
    side = nil,
    
    record = nil, -- record table
    
    createtime = nil,
    seed = nil,
    copy_layer = nil, -- current layer of copy
    copy_seed = nil, -- current seed of copy
    newbie = nil,
    death = nil, -- state of owner
    coord = nil, -- coord: {x = ?, y = ?}
    
    pack = nil, -- pack object

    
	constructor = function(self, copy, entityid, side, master, span)
	    self.copy = copy

		self.id = entityid
		self.baseid = cc.GetBaseid(entityid)
		self.base = cc.LookupTable("Entity", self.baseid)
		assert(self.base ~= nil)
		if self.base.category == EntityCategory.MONSTER then
		    self.ai = Monster:new(self)
		end
		self.random_func = NewRandom(os.time())

	    self.master = master
	    self.side = side	    
        assert(self.side == Side.ALLIES or self.side == Side.ENEMY)

		--TODO: self.record = record.unserialize(self)
		if self.record == nil then
    	    self.createtime = os.time()
    	    self.seed = Seed(self.id)
    	    self.copy_layer = 1
    	    self.newbie = true -- never enter any copy
    	else
    	    self.createtime = self.record.createtime
    	    self.seed = self.record.seed
    	    self.copy_layer = self.record[tostring(self.copy.baseid)] == nil and 1 or self.record[tostring(self.copy.baseid)].current_layer
    	    self.newbie = self.record[tostring(self.copy.baseid)] == nil -- never enter this copy
	    end
    	self.copy_seed = self.seed + self.copy.baseid
    	self.death = false
    	self.coord = nil
	    		
	    cc.WriteLog(string.format("++++++++++ entity: %d,%d,%s, newbie: %s, create", self.id, self.baseid, self.base.name.cn, tostring(self.newbie)))

	    self.pack = self:pack_init(span)
        self.pack:notify_client()
	end,

	destructor = function(self)
	    if self.master ~= nil then
    		cc.WriteLog(string.format("entity: %d,%d,%s, destroy puppet: %d, %d, %s", 
    		    self.master.id, self.master.baseid, self.master.base.name.cn,
    		    self.id, self.baseid, self.base.name.cn))	    
	    else
	        cc.WriteLog(string.format("---------- entity: %d,%d,%s, destroy", self.id, self.baseid, self.base.name.cn))
	    end

	    if self.ai ~= nil then
	        self.ai:destructor()
	        self.ai = nil
	    end

	    -- cause all of puppets is reference, destructor puppets here, but not pack:destructor
	    for _, target in pairs(self.pack.puppets) do 
	        if target.id ~= self.id then
	            target:destructor() 
	        end
	    end
	    table.clear(self.pack.puppets)	    
	    for _, target in pairs(self.pack.placeholders) do 
	        if target.id ~= self.id then
	            target:destructor() 
	        end
	    end
	    table.clear(self.pack.placeholders)
	    	    
	    self.pack:destructor()
	    self.pack = nil

	    cc.EntityDestroy(self.id)
	end,
	
	update = function(self, delta)
		if self.ai ~= nil then self.ai:update(delta) end
	end
}

function Entity:new(copy, entityid, side, master, span)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, entityid, side, master, span)
	return object
end

----------------------------------------------------- init pack -----------------------------------------------------

-- pack pack_init(span)
function Entity:pack_init(span)
    local pack = Pack:new(self, span)
        
    if self.newbie then
    	-- init base property
        pack.hp = self.base.inithp
        pack.mp = self.base.round_mp
        pack.maxhp = self.base.maxhp
        pack.maxmp = self.base.round_mp
        pack.strength = self.base.strength
        pack.armor = self.base.armor
    	pack.shield = self.base.shield
    	pack.weakness = 1

        -- init match parameters
        pack.round_cards = self.base.round_cards
        pack.max_hold_cards = self.base.max_hold_cards
        
        pack.stack_deal = {}
        pack.stack_discard = {}
        pack.stack_exhaust = {}
        pack.stack_hold = {}

        -- init money
    	pack.gold = self.base.init_gold

    	-- init cards
    	pack.cards = {}
    	for _, card_baseid in pairs(self.base.init_cards) do
            local newcard = Card:new(self, card_baseid)
            assert(pack.cards[newcard.id] == nil)
            pack.cards[newcard.id] = newcard
    	end        	

    	-- init items
    	pack.items = {}
    	for item_baseid, item_number in pairs(self.base.init_items) do
            local newitem = nil
            for _, item in pairs(pack.items) do
                if item.baseid == item_baseid then 
                    newitem = item 
                    break
                end
            end                    
            if newitem ~= nil then -- overlap number of item
                item_number = item_number + newitem.number
                newitem:set_number(newitem.number + item_number)
                item_number = item_number - newitem.number
            end

            while item_number > 0 and table.size(pack.items) < self.base.max_pack_slots do
                local newitem = Item:new(self, item_baseid, item_number)
                assert(pack.items[newitem.id] == nil)
                pack.items[newitem.id] = newitem
                item_number = item_number - newitem.number
                if newitem.base.category == ItemCategory.USABLE then break end        
            end
    	end

        -- init equips
        pack.equips = {}

    	-- init puppets
    	pack.puppets = {}
    	pack.placeholders = {}				
		pack.placeholders[Placeholder.FRONT] = self -- init master placeholder
		for target_baseid, placeholder in pairs(self.base.init_puppets) do
		    assert(target_baseid ~= self.baseid) -- avoid dead-cycle
            local target = Entity:new(self.copy, cc.EntityNew(target_baseid), self.side, self, pack.span) -- use owner.side
		    if placeholder == Placeholder.NONE then
                assert(pack.puppets[target.id] == nil) 
                pack.puppets[target.id] = target    
            else
                assert(placeholder == Placeholder.FRONT or placeholder == Placeholder.MIDDLE or placeholder == Placeholder.BACK)
                assert(pack.placeholders[placeholder] == nil)
                pack.placeholders[placeholder] = target
            end
        end
    	
        -- init buffs
        pack.buffs = {}
        for buff_baseid, buff_layers in pairs(self.base.init_buffs) do
            local newbuff = nil
        	for _, buff in pairs(pack.buffs) do
        		if buff.baseid == buff_baseid then 
        		    newbuff = buff
        		    break
                end
        	end
        	if newbuff ~= nil then
        		newbuff:layers_add(buff_layers)
        	else
        	    newbuff = Buff:new(self, cc.BuffAdd(self.id, buff_baseid, buff_layers), buff_baseid, buff_layers)
        		assert(pack.buffs[newbuff.id] == nil)
        		pack.buffs[newbuff.id] = newbuff
        	end                    
        end
    else
        -- restore data by entity.record
        local t = self.record[tostring(self.copy.baseid)]
        
        -- TODO: events_accomplish

    	-- init base property
        pack.hp = t.pack.hp
        pack.mp = t.pack.mp
        pack.maxhp = t.pack.maxhp
        pack.maxmp = t.pack.maxmp
        pack.strength = t.pack.strength
        pack.armor = t.pack.armor
    	pack.shield = t.pack.shield
    	pack.weakness = t.pack.weakness

        -- init match parameters
        pack.round_cards = t.pack.round_cards
        pack.max_hold_cards = t.pack.max_hold_cards
        
        pack.stack_deal = {}
        pack.stack_discard = {}
        pack.stack_exhaust = {}
        pack.stack_hold = {}

        pack.gold = t.pack.gold 

    	-- init cards
    	pack.cards = {}
    	for _, card_baseid in pairs(t.pack.cards) do
            local newcard = Card:new(self, card_baseid)
            assert(pack.cards[newcard.id] == nil)
            pack.cards[newcard.id] = newcard
    	end

    	-- init items
    	pack.items = {}
    	for item_baseid_str, item_number in pairs(t.pack.items) do
    	    local item_baseid = tonumber(item_baseid_str)
            local newitem = nil
            for _, item in pairs(pack.items) do
                if item.baseid == item_baseid then 
                    newitem = item 
                    break
                end
            end                    
            if newitem ~= nil then -- overlap number of item
                newitem:set_number(newitem.number + item_number)
            else
                newitem = Item:new(self, item_baseid, item_number)
                assert(pack.items[newitem.id] == nil)
                pack.items[newitem.id] = newitem
            end
    	end

        -- init equips
        pack.equips = {}

    	-- init puppets
        -- TODO: 宠物的属性可能会发生变化
    	pack.puppets = {}
    	for _, target_baseid in pairs(t.pack.puppets) do
    	    local target = Entity:new(self.copy, cc.EntityNew(target_baseid), self.side, self, pack.span)
    	    pack.puppets[target.id] = target
    	end
    	
    	pack.placeholders = {}
    	for placeholder, target_baseid in pairs(t.pack.placeholders) do
    	    local target = Entity:new(self.copy, cc.EntityNew(target_baseid), self.side, self, pack.span)
    	    pack.placeholders[placeholder] = target
    	end
    	        	
        -- init buffs
        pack.buffs = {}
        for buff_baseid_str, buff_layers in pairs(t.pack.buffs) do
            local buff_baseid = tonumber(buff_baseid_str)
            local newbuff = nil
        	for _, buff in pairs(pack.buffs) do
        		if buff.baseid == buff_baseid then 
        		    newbuff = buff
        		    break
                end
        	end
        	if newbuff ~= nil then
        		newbuff:layers_add(buff_layers)
        	else
        	    newbuff = Buff:new(self, cc.BuffAdd(self.id, buff_baseid, buff_layers), buff_baseid, buff_layers)
        		assert(pack.buffs[newbuff.id] == nil)
        		pack.buffs[newbuff.id] = newbuff
        	end                    
        end
    end
    return pack
end

----------------------------------------------------- change span ---------------------------------------------------

function Entity:enter_copy(copy)
    assert(self.copy.id == copy.id)
    cc.WriteLog(string.format("entity: %d enter copy:%d,%s, newbie: %s", self.id, copy.baseid, copy.base.name.cn, tostring(self.newbie)))
    self.pack:notify_client()
end

function Entity:exit_copy(copy)
    assert(self.copy.id == copy.id)
    cc.WriteLog(string.format("entity: %d exit copy:%d,%s, newbie: %s", self.id, copy.baseid, copy.base.name.cn, tostring(self.newbie)))
end

function Entity:enter_scene(scene)
    cc.WriteLog(string.format("entity: %d enter scene:%d,%d,%s", self.id, scene.id, scene.baseid, scene.base.name.cn))
end

function Entity:exit_scene(scene)
    cc.WriteLog(string.format("entity: %d exit scene:%d,%d,%s", self.id, scene.id, scene.baseid, scene.base.name.cn))
end

-- b ->>> c
function Entity:enter_match(match)
    assert(self.__pack == nil)
    assert(self.span ~= Span.MATCH)
    self.__pack = self.pack
    self.pack = Pack:new(self, Span.MATCH)
    self.__pack:delivery(self.pack)
    
    if self.pack.hp == 0 then 
        self:to_die()
    else
        -- cleanup buffs
        for buffid, _ in pairs(self.pack.buffs) do cc.BuffRemove(self.id, buffid) end
    
        self.pack:notify_client()
        
        -- add pack.cards to stack deal
        for cardid, card in pairs(self.pack.cards) do
        	self:stack_deal_insert(card)
        end
    end
    
    cc.WriteLog(string.format("entity: %d enter match", self.id))
end

-- b <<<- c
function Entity:exit_match(match)
    assert(self.__pack ~= nil)
    self.pack:delivery(self.__pack) -- b <<<- c

	-- cleanup stack_XXXX
    for cardid, _ in pairs(self.pack.stack_deal) do cc.StackDealRemove(self.id, cardid) end
    for cardid, _ in pairs(self.pack.stack_hold) do cc.StackHoldRemove(self.id, cardid) end
    for cardid, _ in pairs(self.pack.stack_discard) do cc.StackDiscardRemove(self.id, cardid) end
    for cardid, _ in pairs(self.pack.stack_exhaust) do cc.StackExhaustRemove(self.id, cardid) end

    -- cleanup buffs
    for buffid, _ in pairs(self.pack.buffs) do cc.BuffRemove(self.id, buffid) end

    self.pack:destructor()
    self.pack = self.__pack  -- reset to backup pack
    self.__pack = nil

    self.pack:notify_client()
    
    cc.WriteLog(string.format("entity: %d exit match", self.id))
end


----------------------------------------------------- basic method --------------------------------------------------

function Entity:to_die()
    if self.death then 
        cc.WriteLog(string.format("entity: %d already is death, func: %s", self.id, Function()))
        return 
    end -- perhaps take multiple damage in one round
    self.death = true
    cc.WriteLog(string.format("entity: %d,%d die", self.id, self.baseid))
    cc.Die(self.id)
    self.copy.scene.match:die_entity(self.id)
end

function Entity:hp_add(value)
	if value == 0 then return end
    self.pack.hp = self.pack.hp + value
    if self.pack.hp < 0 then self.pack.hp = 0 end
    if self.pack.hp > self.pack.maxhp then self.pack.hp = self.pack.maxhp end
    cc.SetCurHP(self.id, self.pack.hp)
    cc.WriteLog(string.format("entity: %d, %s, add hp value: %d, hp: %d", self.id, self.base.name.cn, value, self.pack.hp))
    if value < 0 then
        self:breakpoint(nil, nil, ENTITY_SUFFER_DAMAGE)
    end
    if self.pack.hp == 0 then self:to_die() end
end

function Entity:maxhp_add(value)
	if value == 0 then return end
    self.pack.maxhp = self.pack.maxhp + value
    if self.pack.maxhp < 0 then self.pack.maxhp = 0 end
    assert(self.pack.maxhp > 0)
    cc.SetMaxHP(self.id, self.pack.maxhp)
    cc.WriteLog(string.format("entity: %d, %s, add maxhp value: %d, maxhp: %d", self.id, self.base.name.cn, value, self.pack.maxhp))
end

function Entity:mp_add(value)	
	if value == 0 then return end
	assert(self.pack.mp >= 0)
	self.pack.mp = self.pack.mp + value
	if self.pack.mp < 0 then self.pack.mp = 0 end
	cc.SetCurMP(self.id, self.pack.mp)
	cc.WriteLog(string.format("entity: %d, %s, add mp value: %d, mp: %d", self.id, self.base.name.cn, value, self.pack.mp))
end

function Entity:strength_add(value)	
	if value == 0 then return end
	assert(self.pack.strength >= 0)
	self.pack.strength = self.pack.strength + value
	if self.pack.strength < 0 then self.pack.strength = 0 end	
	cc.SetStrength(self.id, self.pack.strength)
	cc.WriteLog(string.format("entity: %d, %s, add strength value: %d, strength: %d", self.id, self.base.name.cn, value, self.pack.strength))
end

function Entity:armor_add(value)
	if value == 0 then return end
	assert(self.pack.armor >= 0)
	self.pack.armor = self.pack.armor + value
	if self.pack.armor < 0 then self.pack.armor = 0 end	
	cc.SetArmor(self.id, self.pack.armor)
	cc.WriteLog(string.format("entity: %d, %s, add armor value: %d, armor: %d", self.id, self.base.name.cn, value, self.pack.armor))
end

function Entity:shield_add(value)
	if value == 0 then return end
	assert(self.pack.shield >= 0)
	self.pack.shield = self.pack.shield + value
	if self.pack.shield < 0 then self.pack.shield = 0 end	
	--cc.SetShield(self.id, self.pack.shield)
	cc.WriteLog(string.format("entity: %d, %s, add shield value: %d, shield: %d", self.id, self.base.name.cn, value, self.pack.shield))
end

--
-- stack deal
--
function Entity:stack_deal_insert(card)
	assert(self.pack.stack_deal[card.id] == nil)
	self.pack.stack_deal[card.id] = card
	cc.StackDealAdd(self.id, card.id)
end
function Entity:stack_deal_remove(cardid)
	assert(self.pack.stack_deal[cardid] ~= nil)
	self.pack.stack_deal[cardid] = nil
	cc.StackDealRemove(self.id, cardid)
end

--
-- stack discard
--
function Entity:stack_discard_insert(card)
	assert(self.pack.stack_discard[card.id] == nil)
	self.pack.stack_discard[card.id] = card
	cc.StackDiscardAdd(self.id, card.id)
end
function Entity:stack_discard_remove(cardid)
	assert(self.pack.stack_discard[cardid] ~= nil)
	self.pack.stack_discard[cardid] = nil
	cc.StackDiscardRemove(self.id, cardid)
end

--
-- stack exhaust
--
function Entity:stack_exhaust_insert(card)
	assert(self.pack.stack_exhaust[card.id] == nil)
	self.pack.stack_exhaust[card.id] = card
	cc.StackExhaustAdd(self.id, card.id)
end
function Entity:stack_exhaust_remove(cardid)
	assert(self.pack.stack_exhaust[cardid] ~= nil)
	self.pack.stack_exhaust[cardid] = nil
	cc.StackExhaustRemove(self.id, cardid)
end

--
-- stack hold
--
function Entity:stack_hold_insert(card)
	assert(self.pack.stack_hold[card.id] == nil)
	self.pack.stack_hold[card.id] = card
	cc.StackHoldAdd(self.id, card.id)
end
function Entity:stack_hold_remove(cardid)
	assert(self.pack.stack_hold[cardid] ~= nil)
	self.pack.stack_hold[cardid] = nil
	cc.StackHoldRemove(self.id, cardid)
end
function Entity:stack_hold_retrieve()
	for cardid in pairs(self.pack.stack_hold) do return cardid end
	return nil
end

--
-- new card and put it into stack_hold
--
function Entity:stack_hold_newone(card_baseid)
	local card = Card:new(self, card_baseid)
	self:stack_hold_insert(card)
	cc.WriteLog(string.format("entity: %d, newone card: %d,%d", self.id, card.id, card.baseid))
	return card
end

--
-- shuffle stack_discard to stack_deal
--
function Entity:shuffle_stackdiscard()
	for cardid, card in pairs(self.pack.stack_discard) do
		self:stack_deal_insert(card)
		self:stack_discard_remove(cardid)
	end
end

--
-- shuffle stack_exhaust to stack_deal
--
function Entity:shuffle_stackexhaust()
	for cardid, card in pairs(self.pack.stack_exhaust) do
		self:stack_deal_insert(card)
		self:stack_exhaust_remove(cardid)
	end
end

--
-- Buff
-- 
function Entity:buff_find(buff_baseid)
	for _, buff in pairs(self.pack.buffs) do
		if buff.baseid == buff_baseid then return buff end
	end
	return nil
end
function Entity:buff_add(buff_baseid, buff_layers)
	local buff = self:buff_find(buff_baseid)
	if buff ~= nil then
		buff:layers_add(buff_layers)
	else
	    self:breakpoint(nil, nil, BUFF_ADD_A)
		local buffid = cc.BuffAdd(self.id, buff_baseid, buff_layers)
		assert(self.pack.buffs[buffid] == nil)
		self.pack.buffs[buffid] = Buff:new(self, buffid, buff_baseid, buff_layers)
	    self:breakpoint(nil, nil, BUFF_ADD_Z)
	end
end
function Entity:buff_remove(buffid)
	assert(self.pack.buffs[buffid] ~= nil)
	self:breakpoint(nil, nil, BUFF_REMOVE_A)
	self.pack.buffs[buffid]:destructor()
	self.pack.buffs[buffid] = nil
	cc.BuffRemove(self.id, buffid)
	self:breakpoint(nil, nil, BUFF_REMOVE_Z)
end
function Entity:buff_clear()
	for buffid, _ in pairs(self.pack.buffs) do
	    self:buff_remove(buffid)
	end
end

--
-------------------------- cards -----------------------------
--

--
-- card add_card(card_baseid)
--
function Entity:add_card(card_baseid)
    local card = Card:new(self, card_baseid)
    assert(self.pack.cards[card.id] == nil)
    self.pack.cards[card.id] = card
    cc.WriteLog(string.format("entity: %d add card: %d,%d to Entity", self.id, card.id, card.baseid))
    cc.BagAddCard(self.id, card.id)
    return card
end

--
-- void remove_card(cardid)
--
function Entity:remove_card(cardid)
    assert(self.pack.cards[cardid] ~= nil)
    self.pack.cards[cardid] = nil
    cc.WriteLog(string.format("entity: %d remove card: %d from Entity", self.id, cardid))
    cc.BagRemoveCard(self.id, cardid)
end

--
-------------------------- items -----------------------------
--


--
-- item find_item(item_baseid)
--
function Entity:find_item_by_baseid(item_baseid)
    for _, item in pairs(self.pack.items) do
        if item.baseid == item_baseid then return item end
    end
    return nil
end

--
-- void add_item(item_baseid, item_number)
--
function Entity:add_item(item_baseid, item_number)
    local item = self:find_item_by_baseid(item_baseid)
    if item ~= nil then -- overlap number of item
        item_number = item_number + item.number
        item:set_number(item.number + item_number)
        cc.BagSetItemNumber(self.id, item.id, item.number)
        item_number = item_number - item.number
    end

    while item_number > 0 and table.size(self.pack.items) < self.base.max_pack_slots do
        local item = Item:new(self, item_baseid, item_number)
        assert(self.pack.items[item.id] == nil)
        self.pack.items[item.id] = item
        cc.WriteLog(string.format("entity: %d add item: %d,%d to Entity", self.id, item.id, item.baseid))
        cc.BagAddItem(self.id, item.id)
        item_number = item_number - item.number
        if item.base.category == ItemCategory.USABLE then break end        
    end
end

--
-- void remove_item(itemid, item_number)
--
function Entity:remove_item(itemid, item_number)
    local item = self.pack.items[itemid]
    assert(item ~= nil)
    item:set_number(item.number - item_number)
    if item.number > 0 then
        cc.BagSetItemNumber(self.id, item.id, item.number)
    else
        self.pack.items[itemid] = nil
        cc.WriteLog(string.format("entity: %d remove item: %d from Entity", self.id, itemid))
        cc.BagRemoveItem(self.id, itemid)
    end
end

--
-------------------------- equips -----------------------------
--

--
-- item add_equip(itemid, slot)
--
function Entity:add_equip(itemid, slot)
    local equip = self.pack.items[itemid]
    assert(equip ~= nil and equip.base.category == ItemCategory.EQUIPABLE)
    if slot == nil then
        slot = table.size(self.pack.equips) + 1
    end
    self:remove_equip(slot)
    self.pack.items[equip.id] = nil
    cc.BagRemoveItem(self.id, equip.id)
    self.pack.equips[slot] = equip
    cc.EquipBarAdd(self.id, slot, equip.id)
    cc.WriteLog(string.format("entity: %d add equip: %d,%d to slot: %d", self.id, equip.id, equip.baseid, slot))
    return item
end

--
-- bool remove_equip(slot)
--
function Entity:remove_equip(slot)
    local equip = self.pack.equips[slot]
    if equip == nil then return false end
    assert(self.pack.items[equip.id] == nil)
    self.pack.items[equip.id] = equip
    cc.BagAddItem(self.id, equip.id)
    self.pack.equips[slot] = nil
    cc.EquipBarRemove(self.id, slot)
    cc.WriteLog(string.format("entity: %d remove equip: %d from slot: %d", self.id, equip.id, slot))
end

--
-------------------------- puppets -----------------------------
--

--
-- target_entity add_puppet(target_baseid)
--
function Entity:add_puppet(target_baseid)
    local target = Entity:new(self.copy, cc.EntityNew(target_baseid), self.side, self, self.pack.span) -- use owner.side
    assert(self.pack.puppets[target.id] == nil)
    self.pack.puppets[target.id] = target    
    cc.BagAddPuppet(self.id, target.id)
    cc.WriteLog(string.format("entity: %d add puppet: %d,%d,%s to Entity", self.id, target.id, target.baseid, target.base.name.cn))
    return target
end

--
-- bool remove_puppet(target_entityid)
--
function Entity:remove_puppet(target_entityid)    
    local target = self.pack.puppets[target_entityid]
    if target == nil then
        cc.WriteLog(string.format(">>>>>> not found puppet: %d, func: %s", target_entityid, Function()))
        return false
    end    

    -- remove puppet
    self.pack.puppets[target.id] = nil
    cc.BagRemovePuppet(self.id, target.id)
    
    target:destructor()
    cc.WriteLog(string.format("entity: %d remove puppet: %d,%d,%s from Entity", self.id, target.id, target.baseid, target.base.name.cn))

    return true
end

--
-- bool arrange_placeholder(target_entityid, placeholder)
--
function Entity:arrange_placeholder(target_entityid, placeholder)
    cc.WriteLog(string.format("entity: %d, arrange placeholder, target_entityid: %d, placeholder: %d", self.id, target_entityid, placeholder))

    -- check target_entityid is valid
    if target_entityid == nil then
        cc.WriteLog(string.format(">>>>>>> target_entityid is nil, func: %s", Function()))
        return false
    end

    -- check placeholder is legal
    local t = {
        [Placeholder.NONE] = true, [Placeholder.FRONT] = true, [Placeholder.MIDDLE] = true, [Placeholder.BACK] = true
    }
    if t[placeholder] == nil then
        cc.WriteLog(string.format(">>>>>> entity: %d, illegal placeholder: %d, func: %s", self.id, placeholder, Function()))
        return false
    end

    -- take back
    if target_entityid == 0 then
        local target = self.pack.placeholders[placeholder]
        assert(target ~= nil, 'placeholder: ' .. tostring(placeholder))

        if target.id == self.id then
            cc.WriteLog(string.format(">>>>>> entity: %d, take back master not allow, func: %s", self.id, Function()))
            return false
        end
        
        self.pack.placeholders[placeholder] = nil
        cc.BagArrangePlaceholder(self.id, 0, placeholder) -- clear placeholder
        
        assert(self.pack.puppets[target.id] == nil)
        self.pack.puppets[target.id] = target
        cc.BagAddPuppet(self.id, target.id) -- set puppets
        
        return true
    end

    if placeholder == Placeholder.NONE then
        cc.WriteLog(string.format(">>>>>> entity: %d, move to Placeholder.NONE not allow, func: %s", self.id, Function()))
        return false
    end

    -- move to placeholders from puppets
    local target = self.pack.puppets[target_entityid]
    if target ~= nil then
        assert(target.id ~= self.id) -- self should not be in the pack.puppets

        local target_old = self.pack.placeholders[placeholder]
        if target_old ~= nil and target_old.id == self.id then
            cc.WriteLog(string.format(">>>>>> entity: %d, move master not allow, placeholder: %d, func: %s", self.id, placeholder, Function()))
            return false
        end

        self.pack.puppets[target_entityid] = nil
        cc.BagRemovePuppet(self.id, target.id) -- clear puppets

        if target_old ~= nil then            
            self.pack.placeholders[placeholder] = nil
            cc.BagArrangePlaceholder(self.id, 0, placeholder) -- clear placeholder

            assert(self.pack.puppets[target_old.id] == nil)
            self.pack.puppets[target_old.id] = target_old
            cc.BagAddPuppet(self.id, target_old.id) -- set puppets
        end

        self.pack.placeholders[placeholder] = target
        cc.BagArrangePlaceholder(self.id, target.id, placeholder) -- set placeholder

        return true
    end

    -- swap placeholder
    target = nil
    target_placeholder = nil
    for slot, entity in pairs(self.pack.placeholders) do
        if entity.id == target_entityid then
            target = entity
            target_placeholder = slot
            break
        end
    end

    if target == nil or target_placeholder == nil then
        cc.WriteLog(string.format(">>>>>> entity: %d, target: %s not found or placeholder: %s not found, func: %s", self.id, tostring(target_entityid), tostring(target_placeholder), Function()))
        return false
    end
    
    self.pack.placeholders[target_placeholder] = nil
    cc.BagArrangePlaceholder(self.id, 0, target_placeholder) -- clear placeholders
    
    local target_old = self.pack.placeholders[placeholder]
    if target_old ~= nil then
        self.pack.placeholders[placeholder] = nil
        cc.BagArrangePlaceholder(self.id, 0, placeholder) -- clear placeholders

        self.pack.placeholders[target_placeholder] = target_old
        cc.BagArrangePlaceholder(self.id, target_old.id, target_placeholder) -- set placeholders
    end

    self.pack.placeholders[placeholder] = target
    cc.BagArrangePlaceholder(self.id, target.id, placeholder) -- set placeholders
    
    return true
end

--
-------------------------- gold -----------------------------
--

--
-- void add_gold(value)
--
function Entity:add_gold(value)
    assert(self.pack.gold >= 0)
    local gold_old = self.pack.gold
    self.pack.gold = self.pack.gold + value
    if self.pack.gold < 0 then self.pack.gold = 0 end
    cc.WriteLog(string.format("entity: %d, add_gold: %d, gold: %d", self.id, value, self.pack.gold))
    cc.BagSetGold(self.id, self.pack.gold, gold_old)
end

----------------------------------------------------- match --------------------------------------------------

function Entity:breakpoint(card, pick_entityid, bp)
	local pick_entity = pick_entityid ~= nil and self.copy.scene.match.entities[pick_entityid] or nil
	if card ~= nil and card.base.enable_script then
		card.script_func(self, card, nil, pick_entity, bp)
	end
	for _, buff in pairs(self.pack.buffs) do
		if buff.base.enable_script then
			buff.script_func(self, card, buff, pick_entity, bp)
		end
	end
end

--
-- void round_begin()
--
function Entity:round_begin()
    -- for puppet, perhaps already is death in the round_begin
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return
    end
    
	cc.WriteLog(string.format("*************************************** 回合: %d, entity: %d ******************************", self.copy.scene.match.round_total, self.id))
    
	self:breakpoint(nil, nil, BreakPoint.ROUND_BEGIN_A)
	
	--
	-- resume mp
	--
	assert(self.pack.mp >= 0 and self.base.round_mp > 0)
	if self.pack.mp < self.base.round_mp then
		self:mp_add(self.base.round_mp - self.pack.mp)
	end

	self:breakpoint(nil, nil, BreakPoint.ROUND_BEGIN_Z)
	
	--
	-- deal card
	--
	self:card_deal()

	cc.WriteLog(string.format("entity: %d,%d,%s card_deal end", self.id, self.baseid, self.base.name.cn))
    for _, card in pairs(self.pack.stack_hold) do 
        cc.WriteLog(string.format("      card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
    end	
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
    
	local round_cards = self.pack.round_cards
	assert(round_cards > 0)
	cc.WriteLog(string.format("[%s:%d:%d] 发牌", self.base.name.cn, self.id, self.baseid))
	
	while round_cards > 0 do
		if table.size(self.pack.stack_deal) == 0 then
			self:shuffle_stackdiscard() -- stack_deal not enough for round_cards
		end
		if table.size(self.pack.stack_deal) == 0 then
		    -- cc.WriteLog(string.format(">>>>>>> entity: %d, cards not enough for cardDeal, func: %s", self.id, Function()))
			break
		end
		local cardid = table.random(self.pack.stack_deal, table.size(self.pack.stack_deal), self.random_func)
		assert(cardid ~= nil and self.pack.stack_deal[cardid] ~= nil)
		local card = self.pack.stack_deal[cardid]
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
	while draw_cards > 0 and table.size(self.pack.stack_deal) > 0 do
		local cardid = table.random(self.pack.stack_deal, table.size(self.pack.stack_deal), self.random_func)
		assert(cardid ~= nil and self.pack.stack_deal[cardid] ~= nil)
		self:stack_hold_insert(self.pack.stack_deal[cardid])
		self:stack_deal_remove(cardid)
		draw_cards = draw_cards - 1
	end
end

--
-- void card_draw_from_stackdiscard(draw_cards)
--
function Entity:card_draw_from_stackdiscard(draw_cards)
	while draw_cards > 0 and table.size(self.pack.stack_discard) > 0 do
		local cardid = table.random(self.pack.stack_discard, table.size(self.pack.stack_discard), self.random_func)
		assert(cardid ~= nil and self.pack.stack_discard[cardid] ~= nil)
		self:stack_hold_insert(self.pack.stack_discard[cardid])
		self:stack_discard_remove(cardid)
		draw_cards = draw_cards - 1
	end
end

--
-- void card_draw_from_stackexhaust(draw_cards)
--
function Entity:card_draw_from_stackexhaust(draw_cards)
	while draw_cards > 0 and table.size(self.pack.stack_exhaust) > 0 do
		local cardid = table.random(self.pack.stack_exhaust, table.size(self.pack.stack_exhaust), self.random_func)
		assert(cardid ~= nil and self.pack.stack_exhaust[cardid] ~= nil)
		self:stack_hold_insert(self.pack.stack_exhaust[cardid])
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

	local card = self.pack.stack_hold[cardid]
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
	if self.pack.mp < card.base.cost_mp then
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

	local card = self.pack.stack_hold[cardid]
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
	if self.pack.mp < card.base.cost_mp then 
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

	local card = self.pack.stack_hold[cardid]
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

	local card = self.pack.stack_hold[cardid]
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
	while table.size(self.pack.stack_hold) > 0 and table.size(self.pack.stack_hold) > self.pack.max_hold_cards do
		local cardid = self:stack_hold_retrieve()
		self:card_discard(cardid, true)
	end
	
	self:breakpoint(nil, nil, BreakPoint.ROUND_END_Z)
end


----------------------------------------------------- event --------------------------------------------------

--
-- void accomplish_event(event)
--
function Entity:accomplish_event(event)
    cc.WriteLog(string.format("entity:%d accomplish event: %d, %d", self.id, event.id, event.baseid))    
--TODO:    record.copy_events_accomplish_add(self.record, self.copy.baseid, self.copy:current_layer(), event.baseid)
    cc.AccomplishEvent(self.id, self.copy.scene.id, event.id)
end

--
-- bool event_reward(args, reward_index, entry_index)
--
function Entity:event_reward(args, reward_index, entry_index)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end
    
    if args[reward_index] == nil then -- entity cancel reward
        cc.WriteLog(string.format(">>>>>>> args.rewards not reward_index: %s, func: %s", tostring(reward_index), Function()))
        return false
    end
    
    local reward = args[reward_index] 
    
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

		if table.size(self.pack.cards) >= self.base.max_cards then
			cc.WriteLog(string.format(">>>>>>> pack.cards is full, max: %d, func: %s", self.base.max_cards, Function()))
			return false
		end

        cc.WriteLog(string.format("entity: %d, reward card: %d,%s", self.id, card_baseid, card_base.name.cn))
        self:add_card(card_baseid)
        return true
    end
    
    local function reward_items(self, items, entry_index)
        if items[entry_index] == nil then
            cc.WriteLog(string.format(">>>>>>> rewards.items not entry_index: %s, func: %s", tostring(entry_index), Function()))
            return false
        end
        local item_baseid = entry_index
        local item_number = items[entry_index]
        local item_base = cc.LookupTable("Item", item_baseid)
        if item_base == nil then
            cc.WriteLog(string.format(">>>>>>> reward found illegal item_baseid: %s, func: %s", tostring(item_baseid), Function()))
            return false
        end

		if table.size(self.pack.items) >= self.base.max_pack_slots then
			cc.WriteLog(string.format(">>>>>>> pack.items is full, max: %d, func: %s", self.base.max_pack_slots, Function()))
			return false
		end

        cc.WriteLog(string.format("entity: %d, reward item: %d,%s, number: %d", self.id, item_baseid, item_base.name.cn, item_number))
        self:add_item(item_baseid, item_number)
        return true
    end

    local function reward_puppets(self, puppets, entry_index)
        if puppets[entry_index] == nil then
            cc.WriteLog(string.format(">>>>>>> rewards.puppets not entry_index: %s, func: %s", tostring(entry_index), Function()))
            return false
        end
        local entity_baseid = puppets[entry_index]
        local entity_base = cc.LookupTable("Entity", entity_baseid)
        if entity_base == nil then
            cc.WriteLog(string.format(">>>>>>> reward found illegal entity_baseid: %s, func: %s", tostring(entity_baseid), Function()))
            return false
        end
        local size = table.size(self.pack.puppets)
		if size >= self.base.max_puppets then
            cc.WriteLog(string.format(">>>>>>> pack.puppets is full, max: %d, func: %s", self.base.max_puppets, Function()))
            return false
        end
        cc.WriteLog(string.format("entity: %d, reward puppet: %d,%s", self.id, entity_baseid, entity_base.name.cn))
        self:add_puppet(entity_baseid)
        return true
    end
    
    if reward.cards ~= nil then
        return reward_cards(self, reward.cards, entry_index)
    elseif reward.items ~= nil then
        return reward_items(self, reward.items, entry_index)
    elseif reward.gold ~= nil then
        cc.WriteLog(string.format("entity: %d, reward gold: %d", self.id, reward.gold))
        self:add_gold(reward.gold)
    elseif reward.puppets ~= nil then
        return reward_puppets(self, reward.puppets, entry_index)
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
        return false
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

    if self.pack.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.pack.gold, price_gold, Function()))
        return false
    end

	if table.size(self.pack.cards) >= self.base.max_cards then
		cc.WriteLog(string.format(">>>>>>> pack.cards is full, max: %d, func: %s", self.base.max_cards, Function()))
		return false
	end

    self:add_gold(-price_gold)    
    self:add_card(card_baseid)    
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
        return false
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

    if self.pack.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.pack.gold, price_gold, Function()))
        return false
    end

	if table.size(self.pack.items) >= self.base.max_pack_slots then
		cc.WriteLog(string.format(">>>>>>> pack.items is full, max: %d, func: %s", self.base.max_pack_slots, Function()))
		return false
	end

    self:add_gold(-price_gold)    
    self:add_item(item_baseid, 1)    
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
        return false
    end

    if args.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> args.price_gold is nil, func: %s", Function()))
        return false
    end

    local card = self.pack.cards[cardid]
    if card == nil then
        cc.WriteLog(string.format(">>>>>>> not found cardid: %d, func: %s", cardid, Function()))
        return false
    end

    local price_gold = args.price_gold
    -- TODO: discount

    if self.pack.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.pack.gold, price_gold, Function()))
        return false
    end

    self:add_gold(-price_gold)
    
    self:remove_card(cardid)
    card:destructor()
    
    cc.WriteLog(string.format("entity: %d, destroy card: %d,%d,%s, cost gold: %d", self.id, 
        card.id, card.baseid, card.base.name.cn, price_gold
    ))

    return true
end

--
-- bool levelup_card(cardid, args)
--
function Entity:levelup_card(cardid, args)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

    if args.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> args.price_gold is nil, func: %s", Function()))
        return false
    end

    local card_old = self.pack.cards[cardid]
    if card_old == nil then
        cc.WriteLog(string.format(">>>>>>> not found cardid: %d, func: %s", cardid, Function()))
        return false
    end

    local price_gold = args.price_gold
    -- TODO: discount

    if self.pack.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.pack.gold, price_gold, Function()))
        return false
    end

    local card_baseid_new = card_old.base.levelup_baseid
	if card_baseid_new == nil then
        cc.WriteLog(string.format(">>>>>>> card_old.levelup_baseid is nil: %d, func: %s", card_old.baseid, Function()))
        return false 
	end

    if card_baseid_new == 0 then
        cc.WriteLog(string.format(">>>>>>> card already is top level, func: %s", Function()))
        return false 
    end
    
    if cc.LookupTable("Card", card_baseid_new) == nil then 
        cc.WriteLog(string.format(">>>>>>> not exist levelup_baseid: %d card, func: %s", card_baseid_new, Function()))
        return false 
    end
    
    self:remove_card(cardid)
    card_old:destructor()
    
    local card_new = self:add_card(card_baseid_new)
    cc.WriteLog(string.format("entity: %d, levelup card: %d,%d,%s to card: %d,%d,%s, cost gold: %d", self.id, 
        card_old.id, card_old.baseid, card_old.base.name.cn,
        card_new.id, card_new.baseid, card_new.base.name.cn, price_gold
    ))
    
    return true
end


--
-- bool levelup_puppet(target_entityid, args)
--
function Entity:levelup_puppet(target_entityid, args)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

    if target_entityid == nil then
        cc.WriteLog(string.format(">>>>>>> target_entityid is nil, func: %s", Function()))
        return false
    end

    if args.price_gold == nil then
        cc.WriteLog(string.format(">>>>>>> args.price_gold is nil, func: %s", Function()))
        return false
    end    

    local target_old = self.pack.puppets[target_entityid]
    if target_old == nil then
        cc.WriteLog(string.format(">>>>>>> target_entityid: %d not found, func: %s", target_entityid, Function()))
        return false
    end

    local target_baseid_new = target_old.base.levelup_baseid
	if target_baseid_new == nil then
        cc.WriteLog(string.format(">>>>>>> target.levelup_baseid is nil: %d, func: %s", target_old.baseid, Function()))
        return false 
	end

    if target_baseid_new == 0 then
        cc.WriteLog(string.format(">>>>>>> puppet already is top level, func: %s", Function()))
        return false 
    end
    
    if cc.LookupTable("Entity", target_baseid_new) == nil then 
        cc.WriteLog(string.format(">>>>>>> not exist levelup_baseid: %d entity, func: %s", target_baseid_new, Function()))
        return false 
    end

    local price_gold = args.price_gold
    -- TODO: discount

    if self.pack.gold < price_gold then
        cc.WriteLog(string.format(">>>>>>> not enough gold: %d, price: %d, func: %s", self.pack.gold, price_gold, Function()))
        return false
    end

    self:add_gold(-price_gold)
    
    local placeholder_old = self:retrieve_placeholder(target_entityid)
    
    self:remove_puppet(target_entityid)
    local target_new = self:add_puppet(target_baseid_new)
    cc.WriteLog(string.format("entity: %d, levelup puppet: %d,%d,%s to entity: %d,%d,%s, placeholder: %d, cost gold: %d", self.id, 
        target_old.id, target_old.baseid, target_old.base.name.cn,
        target_new.id, target_new.baseid, target_new.base.name.cn, placeholder_old, price_gold
    ))

    if placeholder_old ~= Placeholder.NONE then
        return self:arrange_placeholder(target_new.id, placeholder_old)
    end
    
    return true
end

--
-- bool destroy_puppet(target_entityid)
--
function Entity:destroy_puppet(target_entityid)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

    if target_entityid == nil then
        cc.WriteLog(string.format(">>>>>>> target_entityid is nil, func: %s", Function()))
        return false
    end
    
    return self:remove_puppet(target_entityid)
end

--
-- bool use_item(itemid)
--
function Entity:use_item(itemid)
    if self.death then
        cc.WriteLog(string.format(">>>>>>> entity: %d is die, func: %s", self.id, Function()))
        return false
    end

    local item = self.pack.items[itemid]
    -- check span
    if item.base.span ~= Span.NONE and item.base.span ~= self.pack.span then
        cc.WriteLog(string.format(">>>>>>> item.baseid: %d, item.span: %d, entity.span: %d func: %s", item.baseid, item.base.span, self.pack.span, Function()))
        return false
    end

    -- check use times
    if item.base.consumable_times ~= 0 and item.use_times >= item.base.consumable_times then
        cc.WriteLog(string.format(">>>>>>> item.baseid: %d, item.consumable_times: %d, item.use_times: %d func: %s", item.baseid, item.base.consumable_times, item.use_times, Function()))
        return false
    end
    
    if item.base.category == ItemCategory.USABLE then
    
        -- check number
        if item.number < 1 then
            cc.WriteLog(string.format(">>>>>>> item.baseid: %d, item.number: %s, func: %s", item.baseid, tostring(item.number), Function()))
            return false
        end

        -- reduce number
        self:remove_item(itemid, 1)

        -- call script_func
        assert(item.script_func ~= nil and type(item.script_func) == "function")
        item.script_func(self.id, item, self.seed)

        item.use_times = item.use_times + 1
        
    elseif item.base.category == ItemCategory.EQUIPABLE then

        -- check pack.quips is full
		if table.size(self.pack.equips) >= self.base.max_equip_slots then
			cc.WriteLog(string.format(">>>>>>> pack.equips is full, max: %d func: %s", self.base.max_equip_slots, Function()))
			return false
		end

        self:add_equip(item.id)
            
    elseif item.base.category == ItemCategory.MATERIAL then
    else
        cc.WriteLog(string.format(">>>>>>> item_baseid: %d, category: %s, func: %s", item.baseid, tostring(item.base.category), Function()))
        return false
    end
    
    return true
end

