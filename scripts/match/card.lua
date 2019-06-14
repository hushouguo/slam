
--
------------------- Card class -------------------
--

Card = {
	id = nil, -- card.id
	baseid = nil, -- card.baseid
	base = nil, -- {field_name=field_value}, related card.xls
	func_script = nil, -- script for func

	constructor = function(self, cardid, card_baseid)
		self.id = cardid
		self.baseid = card_baseid
		self.base = cc.LookupTable("Card", card_baseid)
		assert(self.base ~= nil)
		self.func_script = self.base.script -- loadstring(self.base.script)
		assert(self.func_script ~= nil and type(self.func_script) == "function")
	end
}

function Card:new(cardid, card_baseid)
	local card = {}
	self.__index = self -- Card.__index = function(key) return Card[key] end
	setmetatable(card, self)
	card:constructor(cardid, card_baseid)
	return card
end
