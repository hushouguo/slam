--
------------------- Event class -------------------
--

Event = {
    copy = nil, -- reference to copy instance

	id = nil, -- event.id
	baseid = nil, -- event.baseid
	base = nil, -- {field_name=field_value}, related event.xls
	script_func = nil, -- script for func
	coord = nil, -- {x = ?, y = ?}

	objectCategory = nil, -- GridObjectCategory
    
	content = nil, -- event.script_func() ->
	trigger = nil,
	touch = nil,
	accomplish = nil,

	constructor = function(self, copy, event_baseid, coord)
	    self.copy = copy
		self.id = cc.EventNew(event_baseid)
		self.baseid = event_baseid
		self.base = cc.LookupTable("Event", event_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function", 'event_baseid: ' .. tostring(event_baseid))
		self.coord = coord
	    self.objectCategory = GridObjectCategory.EVENT
	    self.content = nil
	    self.trigger = false
	    self.touch = false
	    self.accomplish = false
	end,

	destructor = function(self)
	end
}

function Event:new(copy, event_baseid, coord)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, event_baseid, coord)
	return object
end


