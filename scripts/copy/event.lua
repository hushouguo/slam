--
------------------- Event class -------------------
--

Event = {
	id = nil, -- event.id
	baseid = nil, -- event.baseid
	base = nil, -- {field_name=field_value}, related event.xls
	script_func = nil, -- script for func
	coord = nil, -- {x = ?, y = ?}

	objectCategory = nil, -- GridObjectCategory

	constructor = function(self, eventid, event_baseid, coord)
		self.id = eventid
		self.baseid = event_baseid
		self.base = cc.LookupTable("Event", event_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.coord = coord
	    self.objectCategory = GridObjectCategory.EVENT
	end
}

function Event:new(eventid, event_baseid, coord)
	local event = {}
	self.__index = self -- event.__index = function(key) return event[key] end
	setmetatable(event, self)
	event:constructor(eventid, event_baseid, coord)
	return event
end


