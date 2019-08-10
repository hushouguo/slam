
--
------------------- Obstacle class -------------------
--

Obstacle = {
    copy = nil, -- reference to copy instance
    
	id = nil, -- obstacle.id
	baseid = nil, -- obstacle.baseid
	base = nil, -- {field_name=field_value}, related obstacle.xls
	coord = nil, -- {x = ?, y = ?}

	objectCategory = nil, -- GridObjectCategory

	constructor = function(self, copy, obstacle_baseid, coord)
	    self.copy = copy
		self.id = cc.ObstacleNew(obstacle_baseid)
		self.baseid = obstacle_baseid
		self.base = cc.LookupTable("Obstacle", obstacle_baseid)
		assert(self.base ~= nil)
		self.coord = coord
	    self.objectCategory = GridObjectCategory.OBSTACLE
	end,

	destructor = function(self)
	    cc.ObstacleDestroy(self.id)
	end
}

function Obstacle:new(copy, obstacle_baseid, coord)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, obstacle_baseid, coord)
	return object
end

