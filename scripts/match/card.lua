
--
------------------- Card class -------------------
--

Card = {
	id = nil, -- card.id
	baseid = nil, -- card.baseid
	base = nil, -- {field_name=field_value}, related card.xls
	script_func = nil,

	constructor = function(self, cardid, card_baseid)
		self.id = cardid
		self.baseid = card_baseid
		self.base = cc.LookupTable("Card", card_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
	end
}

function Card:new(cardid, card_baseid)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(cardid, card_baseid)
	return object
end
