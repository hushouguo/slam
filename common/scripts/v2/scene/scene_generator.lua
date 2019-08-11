
--
------------------- Scene generator -------------------
--

--
-- void maps_dump()
--
function Scene:maps_dump()
	assert(self.entrymap ~= nil)
	local function dump_map(map, maps)
		cc.ScriptDebugLog(string.format("map: %d, layer: %d, locate: (%d,%d)", map.baseid, map.layer, map.locate.x, map.locate.y))
		map:tiles_dump()
		local t = {
			Direction.LEFT, Direction.RIGHT, Direction.UP, Direction.DOWN
		}
		for _, direction in pairs(t) do
			if map.neighbor[direction] ~= nil then
				table.insert(maps, map.neighbor[direction])
			end
		end
	end
	cc.ScriptDebugLog("Scene dump")
	local maps = {}
	table.insert(maps, self.entrymap)
	while table.size(maps) > 0 do
		local _, map = table.pop_front(maps)
		dump_map(map, maps)
	end
end

--
--  bool maps_generator(entityid)
--
function Scene:maps_generator(entityid)
	local function reset_seed(seed)
		local _, newseed = NewRandom(seed)(1, 10000)
		return newseed
	end

	local function create_newmap(self, maps, parent, entityid, events, seed, layer, exit_direction)
		local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, ReverseDirection(exit_direction))
		if parent ~= nil then
			assert(parent.neighbor[exit_direction] == nil)
			parent.neighbor[exit_direction] = newmap
		end
		table.insert(maps, newmap)
		return newmap
	end

	local layer = 1
	local seed = reset_seed(self.seed)
	local maps = {}

	
	-- copy.script_func(entityid, copy_baseid, copy_layers, randomseed)
	-- events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
	local events = self.copy.script_func(entityid, self.copy.baseid, layer, seed)
	if events == nil then
	    cc.ScriptErrorLog(string.format(">>>>>>>>>> copy: %d, layer: %d not exist", self.copy.baseid, layer))
	    return false
	end
	self.entrymap = create_newmap(self, maps, nil, entityid, events, seed, layer, RandomDirection())
	assert(self.entrymap ~= nil)


	while table.size(maps) > 0 do
		local _, map = table.pop_front(maps)
		assert(map ~= nil)

		layer = map.layer + 1
		local seed = reset_seed(self.seed)
	    local events = self.copy.script_func(entityid, self.copy.baseid, layer, seed)
	    if events == nil then 
			cc.ScriptErrorLog(string.format("copy: %d, layer: %d, no more map", self.copy.baseid, layer))
			return true 
		end

		cc.ScriptDebugLog(string.format("maps: %d, front map: %d,%d, layer: %d, pattern: %d", table.size(maps), map.id, map.baseid, layer, map.pattern))

	    local t = {
	    	--
	    	--  |
	    	--  |
	    	--  |
	    	--
			[MapPattern.I] 	= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--  ----
	    	--     |
	    	--     |
	    	--
			[MapPattern.L] 	= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
				elseif parent_map.entry_direction == Direction.LEFT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--  ----
	    	--  |   
	    	--  |  
	    	--
			[MapPattern.R] 	= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				elseif parent_map.entry_direction == Direction.RIGHT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--  -------
	    	--     |
	    	--     |
	    	--
			[MapPattern.LR] = function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				elseif parent_map.entry_direction == Direction.LEFT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				elseif parent_map.entry_direction == Direction.RIGHT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--     |
	    	--     |
	    	--  ---|
	    	--     |
	    	--     |
	    	--
			[MapPattern.LI] = function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
				elseif parent_map.entry_direction == Direction.LEFT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--  |
	    	--  |
	    	--  |---
	    	--  |
	    	--  |
	    	--
			[MapPattern.RI] = function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
				elseif parent_map.entry_direction == Direction.RIGHT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--     |
	    	--     |
	    	--  ---|---
	    	--     |
	    	--     |
	    	--
			[MapPattern.LRI]= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.DOWN then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				elseif parent_map.entry_direction == Direction.LEFT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				elseif parent_map.entry_direction == Direction.RIGHT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.DOWN)
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
			-----------------------------
	    	--
	    	--     |
	    	--     |
	    	--  ----
	    	--
			[MapPattern.RL]	= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.LEFT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.LEFT)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    	--
	    	--  |
	    	--  |
	    	--  |---
	    	--
			[MapPattern.RR]	= function(self, parent_map, events, seed, entityid, layer, maps)
				if parent_map.entry_direction == Direction.RIGHT then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.UP)
				elseif parent_map.entry_direction == Direction.UP then
					create_newmap(self, maps, parent_map, entityid, events, seed, layer, Direction.RIGHT)
				else
					assert(false, string.format("parent.entry_direction: %d, parent.pattern: %d", parent_map.entry_direction, parent_map.pattern))
				end
			end,
	    }
	    assert(t[map.pattern] ~= nil)
	    t[map.pattern](self, map, events, seed, entityid, layer, maps)
		--cc.ScriptDebugLog(string.format("maps: %d", table.size(maps)))
	end

	return true
end

--
-- void maps_locate()
--
function Scene:maps_locate()
	assert(self.entrymap ~= nil)
	assert(self.entrymap.locate == nil)
	
	local function map_locate(map)
		local t = {
			[Direction.LEFT] 	= {x = -1, y =  0},
			[Direction.RIGHT] 	= {x =  1, y =  0},
			[Direction.UP] 		= {x =  0, y =  1},
			[Direction.DOWN] 	= {x =  0, y = -1}
		}
		for direction, offset in pairs(t) do
			local neighbor = map.neighbor[direction]
			if neighbor ~= nil then
				neighbor.locate = {
					x = map.locate.x + offset.x * self.FLAG_map_width,
					y = map.locate.y + offset.y * self.FLAG_map_height
				}
				map_locate(neighbor)
			end
		end
	end
	
	self.entrymap.locate = { x = self.FLAG_entry_locate_x, y = self.FLAG_entry_locate_y }
	map_locate(self.entrymap)

	local function map_locate_min(map, min)
		local t = {
			Direction.LEFT, Direction.RIGHT, Direction.UP, Direction.DOWN
		}
		for _, direction in pairs(t) do
			local neighbor = map.neighbor[direction]
			if neighbor ~= nil then
				assert(neighbor.locate ~= nil)
				if neighbor.locate.x < min.x then min.x = neighbor.locate.x end
				if neighbor.locate.y < min.y then min.y = neighbor.locate.y end
				map_locate_min(neighbor, min)
			end
		end
	end
	
	local min = {
		x = 0, y = 0
	}
	map_locate_min(self.entrymap, min)
	cc.ScriptDebugLog(string.format("map locate min: (%d, %d)", min.x, min.y))

	-- relocate map
	if min.x < 0 or min.y < 0 then
		self.entrymap.locate = {
			x = math.abs(min.x), y = math.abs(min.y)
		}
		map_locate(self.entrymap)
	end
end

--
-- void maps_create_index()
--
function Scene:maps_create_index()	
	local function map_locate(self, map)
		assert(map.locate ~= nil)
		
		for yy = 0, self.FLAG_map_height - 1 do
			local y = yy + map.locate.y
			for xx = 0, self.FLAG_map_width - 1 do
				local x = xx + map.locate.x
				if self.maps[y] == nil then self.maps[y] = {} end
				assert(self.maps[y][x] == nil, string.format("coord: (%d,%d)", x, y))
				self.maps[y][x] = map
			end
		end
		
		local t = {
			Direction.LEFT, Direction.RIGHT, Direction.UP, Direction.DOWN
		}
		
		for _, direction in pairs(t) do
			local neighbor = map.neighbor[direction]
			if neighbor ~= nil then
				map_locate(self, neighbor)
			end
		end
	end
	self.maps = {}
	map_locate(self, self.entrymap)
end

--
--  bool generator()
--
function Scene:generator(entityid)
	self:maps_generator(entityid)
	self:maps_locate()
	self:maps_create_index()
	self:maps_dump()
end


