
--
------------------- Obstacle class -------------------
--

Obstacle = {
	id = nil, -- obstacle.id
	baseid = nil, -- obstacle.baseid
	base = nil, -- {field_name=field_value}, related obstacle.xls
	coord = nil, -- {x = ?, y = ?}

	objectCategory = nil, -- GridObjectCategory

	constructor = function(self, obstacleid, obstacle_baseid, coord)
		self.id = obstacleid
		self.baseid = obstacle_baseid
		self.base = cc.LookupTable("Obstacle", obstacle_baseid)
		assert(self.base ~= nil)
		self.coord = coord
	    self.objectCategory = GridObjectCategory.OBSTACLE
	end
}

function Obstacle:new(obstacleid, obstacle_baseid, coord)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(obstacleid, obstacle_baseid, coord)
	return object
end

