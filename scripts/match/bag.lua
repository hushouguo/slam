
--
------------------- Bag class -------------------
--

Bag = {
	entity = nil, -- reference to entity instance

	gold = nil, -- gold
	cards = nil, -- {[cardid]->card, ...}
	items = nil, -- {[itemid]->item, ...}
	buffs = nil, -- {[buffid]->buff, ...}

	constructor = function(self, entity)
		self.entity = entity

		self.gold = entity.base.init_gold
		if self.gold < 0 then self.gold = 0 end

		self.cards = {}
		for _, card_baseid in pairs(entity.base.init_cards) do
		    self:add_card(card_baseid)
		end

		self.items = {}
		-- TODO: init items

		self.buffs = {}
		-- TODO: init buffs
	end,

	destructor = function(self)
	    for cardid, _ in pairs(self.cards) do
	        self:remove_card(cardid)
	    end

	    --TODO: remove items
	    --TODO: remove buffs
	end
}

function Bag:new(entity)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity)
	return object
end

function Bag:add_card(card_baseid)
    local cardid = cc.CardNew(self.entity.id, card_baseid)
    assert(self.cards[cardid] == nil)
    local card = Card:new(cardid, card_baseid)
    self.cards[cardid] = card
    cc.WriteLog(string.format("entity: %d add card: %d,%d to Bag", self.entity.id, card.id, card.baseid))
    cc.BagAddCard(self.entity.id, cardid)
end

function Bag:remove_card(cardid)
    assert(self.cards[cardid] ~= nil)
    self.cards[cardid] = nil
    cc.WriteLog(string.format("entity: %d remove card: %d from Bag", self.entity.id, cardid))
    cc.BagRemoveCard(self.entity.id, cardid)
end

function Bag:add_item(item_baseid, number)
    cc.WriteLog("Bag:add_item not implement yet")
    -- TODO: notify client ??
end

function Bag:remove_item(itemid)
    cc.WriteLog("Bag:remove_item not implement yet")
    -- TODO: notify client ??
end

function Bag:add_gold(value)
    assert(self.gold >= 0)
    local gold_old = self.gold
    self.gold = self.gold + value
    if self.gold < 0 then self.gold = 0 end
    cc.WriteLog(string.format("entity: %d add_gold: %d, gold: %d", self.entity.id, value, self.gold))
    cc.BagSetGold(self.entity.id, self.gold, gold_old)
end


