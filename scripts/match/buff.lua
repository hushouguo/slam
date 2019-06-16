
--
------------------- Buff class -------------------
--

Buff = {
	id = nil, -- buff.id
	baseid = nil, -- buff.baseid
	base = nil, -- {field_name=field_value}, related buff.xls
	entity = nil, -- related with entity object
	script_func = nil,
	
	layers = 0, -- buff layers
	layers_modify = function(self, value)
	    self.layers = self.layers + value
	    if self.layers < 0 then self.layers = 0 end
		if self.layers > self.base.max_layers then
			self.layers = self.base.max_layers
		end
		cc.BuffUpdateLayers(self.entity.id, self.id, self.layers)
	end,
	
	survive_value = 0, -- dynamic value
	survive_value_modify = function(self, value)
	    self.survive_value = self.survive_value + value
	    if self.survive_value < 0 then self.survive_value = 0 end
	end,

	constructor = function(self, entity, buffid, buff_baseid, layers)
		self.id = buffid
		self.baseid = buff_baseid
		self.base = cc.LookupTable("Buff", buff_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.entity = entity
		assert(self.entity ~= nil)
		
		self:layers_modify(layers)
		self:survive_value_modify(self.base.survive_value)
	end
}

function Buff:new(entity, buffid, buff_baseid, layers)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity, buffid, buff_baseid, layers)
	return object
end

