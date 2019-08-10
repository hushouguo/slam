
--
------------------- Scene generator -------------------
--

--
-- void tiles_dump()
--
function Scene:tiles_dump()
	local function exist_entity(self, coord)
		for _, entity in pairs(self.members) do
			if entity.coord.x == coord.x and entity.coord.y == coord.y then return true end
		end
		return false
	end
    for y = 0, self.base.height - 1 do
        local s = ''
        for x = 0, self.base.width - 1 do
            local t = self.tiles[y][x]
			local rc = exist_entity(self, {x=x, y=y})
            if t ~= nil then
            	local objects = self.tiles[y][x].objects
            	assert(objects ~= nil)

				local tilechar = ''
            	for _, object in pairs(objects) do
	                assert(object.objectCategory ~= GridObjectCategory.NONE)
					if object.objectCategory == GridObjectCategory.OBSTACLE then
						tilechar = object.base.tilechar .. tilechar
					else
						if object.base.category == EventCategory.ENTRY then
							tilechar = '>' .. tilechar
						elseif object.base.category == EventCategory.EXIT then
							tilechar = '<' .. tilechar
						elseif object.accomplish then
						    tilechar = '_' .. tilechar
						else
							tilechar = '*' .. tilechar
						end
					end
				end
				if rc then tilechar = '+' .. tilechar end
				s = s .. ' ' .. string.format("%4s", tilechar)
            else
                s = s .. ' ' .. (rc and '   +' or '    ')
            end 
        end 
        cc.ScriptDebugLog(s)
    end
	-- table.dump(self.roads, 'self.roads')
	-- for _, coord in pairs(self.roads) do
	--	cc.ScriptDebugLog(string.format("    coord: (%d, %d), block: %d", coord.x, coord.y, self.tiles[coord.y][coord.x].block))
	-- end
end


--
-- void tiles_pattern_generator()
--
function Scene:tiles_pattern_generator()
    assert(self.pattern ~= nil)
    assert(self.base.width >= self.FLAG_pattern_width)
    assert(self.base.height >= self.FLAG_pattern_height)
    local _, template_pattern = self:tiles_pattern_retrieve(self.pattern)
    for y, t in pairs(template_pattern) do
    	for x, cr in pairs(t) do
    		local coord = {
    			x = x, y = y
    		}
    		if cr == '.' then    		
    			self:tiles_fill(self:create_obstacle_object(self:tile_obstacle_baseid_retrieve(coord), coord))
    		elseif cr == 'I' then
    			self:tiles_fill(self:create_obstacle_object(self:wall_obstacle_baseid_retrieve(coord), coord))
			elseif cr == '*' then
				-- block point
    		else
    			cc.ScriptErrorLog(string.format("illegal char: %s", cr))
    		end
    	end
    end
end

--
-- bool tiles_link_events()
--
function Scene:tiles_link_events()
	local function sort_events(self, from)
		local t = {}
		for _, event in pairs(self.events) do
		    if event.base.category ~= EventCategory.ENTRY and event.base.category ~= EventCategory.EXIT then
                local distance = self:distance(from.coord, event.coord)
				while t[distance] ~= nil do distance = distance + 1 end
    			t[distance] = event
		    end
		end
		return t
	end

	local function shortest_distance(self, src_coord, rc, dest_coord)
	    local min_coord = nil
	    local min_distance = nil
	    if self:passable(src_coord) then 
	        min_coord = src_coord 
    	    min_distance = self:distance(src_coord, dest_coord)
        end
	    for _, coord in pairs(rc) do
	        assert(self:passable(coord))
	        local distance = self:distance(coord, dest_coord)
	        if min_coord == nil or distance < min_distance then
	            min_coord = coord
	            min_distance = distance
	        end
	    end
	    return table.dup(min_coord)
	end

	local function link_event(self, src_coord, dest_coord)
	    assert(src_coord ~= nil and dest_coord ~= nil)
        local road, errorstring = FindPath(
        		src_coord, 
        		dest_coord, 
        		1, 
        		math.max(self.base.width, self.base.height), 
            	function(coord)	return self:passable(coord) end
            	)

        if road ~= nil then
--			table.dump(road, string.format("link_event from (%d,%d) to (%d,%d)", src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))
            for _, coord in pairs(road) do
                assert(self:valid_coord(coord))
                if self.tiles[coord.y][coord.x] == nil then
					self:tiles_fill(self:create_obstacle_object(self:tile_obstacle_baseid_retrieve(coord), coord))
				else
					local o = self.tiles[coord.y][coord.x]
					if o.block == BlockCategory.STATIC then
						cc.ScriptDebugLog(string.format(">>>> link_event found coord: (%d,%d) exist object, block: %d", coord.x, coord.y, o.block))
						cc.ScriptDebugLog(string.format("         src_coord: (%d,%d), dest_coord: (%d,%d)", src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))
						cc.ScriptDebugLog(string.format("          coord: (%d,%d), moveable:%s, passable:%s", coord.x, coord.y, tostring(self:moveable(coord)), tostring(self:passable(coord))))
						for _, object in pairs(o.objects) do
							cc.ScriptDebugLog(string.format("         object: %d,%s", object.baseid, object.base.name.cn))
						end
					end
                end
            end
        else
            cc.ScriptDebugLog('link Event error: ' .. tostring(errorstring))
        end

        return road
	end

	local entry_event = self:tiles_event_find(EventCategory.ENTRY)
	assert(entry_event ~= nil and entry_event.coord ~= nil)
	assert(self:valid_coord(entry_event.coord)) -- is valid coord for entry
	assert(self:moveable(entry_event.coord)) -- is moveable coord for entry

	local exit_event = self:tiles_event_find(EventCategory.EXIT)
	assert(exit_event ~= nil and exit_event.coord ~= nil)
	
	local from = entry_event
    local to = sort_events(self, from) -- except for ENTRY and EXIT event

    --local roads = {}
    local from_coord = from.coord

    --
    -- link events start from ENTRY
    local to_size = table.size(to)
    while to_size > 0 do
        for i, event in pairs(to) do            
            local dest_coord = event.coord
            from_coord = shortest_distance(self, from_coord, self.roads, dest_coord)
            cc.ScriptDebugLog(string.format("link event from (%d,%d) to (%d,%d)", from_coord.x, from_coord.y, dest_coord.x, dest_coord.y))
            local road = link_event(self, from_coord, dest_coord)
            if road == nil then 
				cc.ScriptDebugLog(string.format(">>>>> event: %d,%d,%s unreachable", event.id, event.baseid, event.base.name.cn))
				return false 
			end
            
            to[i] = nil
            from_coord = event.coord

            for _, road_coord in pairs(road) do
				assert(self:passable(road_coord))
                table.insert(self.roads, table.dup(road_coord)) -- NOTE: duplicate road_coord avoid reference
            end
            
            break
        end
        to_size = table.size(to)
    end

	local dest_coord = exit_event.coord
	from_coord = shortest_distance(self, from_coord, self.roads, dest_coord)
    cc.ScriptDebugLog(string.format("link exit event from (%d,%d) to (%d,%d)", from_coord.x, from_coord.y, dest_coord.x, dest_coord.y))

    --
    -- link last event to EXIT
    return link_event(self, from_coord, exit_event.coord)
end

--
-- void dump()
--
function Scene:dump()
	local function dump_map(map)
		cc.ScriptDebugLog(string.format("map: %d, layer: %d", map.baseid, map.layer))
		map:tiles_dump()
		local t = {
			Direction.LEFT, Direction.RIGHT, Direction.UP, Direction.DOWN
		}
		for _, direction in pairs(t) do
			if map.neighbor[direction] ~= nil then
				cc.ScriptDebugLog(string.format("----- direction: %d", direction))
				dump_map(map.neighbor[direction])
			end
		end
	end
	cc.ScriptDebugLog("Scene dump")
	dump_map(self.entrymap)
end

--
--  bool generator()
--
function Scene:generator(entityid)
	local function seed_layer(layer)
		local _, seed = NewRandom(self.seed)(1, layer)
		return seed
	end

	local layer = 1
	local seed = seed_layer(layer)
	local maps = {}

	-- copy.script_func(entityid, copy_baseid, copy_layers, randomseed)
	-- events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
	local events = self.copy.script_func(entityid, self.copy.baseid, layer, seed)
	if events == nil then
	    cc.ScriptErrorLog(string.format(">>>>>>>>>> copy: %d, layer: %d not exist", self.copy.baseid, layer))
	    return false
	end
	self.entrymap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.NONE)
	assert(self.entrymap ~= nil)
	table.insert(maps, self.entrymap)

	while table.size(maps) > 0 do
		local _, map = table.pop_front(maps)
		assert(map ~= nil)

		layer = map.layer + 1
		local seed = seed_layer(layer)
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
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
				end
			end,
	    	--
	    	--  ----
	    	--     |
	    	--     |
	    	--
			[MapPattern.L] 	= function(self, parent_map, events, seed, entityid, layer, maps)
				if true then -- LEFT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.RIGHT)
					parent_map.neighbor[Direction.LEFT] = newmap
					table.insert(maps, newmap)
				end
			end,
	    	--
	    	--  ----
	    	--  |   
	    	--  |  
	    	--
			[MapPattern.R] 	= function(self, parent_map, events, seed, entityid, layer, maps)
				if true then -- RIGHT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.LEFT)
					parent_map.neighbor[Direction.RIGHT] = newmap
					table.insert(maps, newmap)
				end
			end,
	    	--
	    	--  -------
	    	--     |
	    	--     |
	    	--
			[MapPattern.LR] = function(self, parent_map, events, seed, entityid, layer, maps)
				if true then -- LEFT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.RIGHT)
					parent_map.neighbor[Direction.LEFT] = newmap
					table.insert(maps, newmap)
				end
				if true then -- RIGHT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.LEFT)
					parent_map.neighbor[Direction.RIGHT] = newmap
					table.insert(maps, newmap)
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
				if true then -- LEFT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.RIGHT)
					parent_map.neighbor[Direction.LEFT] = newmap
					table.insert(maps, newmap)
				end
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
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
				if true then -- RIGHT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.LEFT)
					parent_map.neighbor[Direction.RIGHT] = newmap
					table.insert(maps, newmap)
				end
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
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
				if true then -- LEFT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.RIGHT)
					parent_map.neighbor[Direction.LEFT] = newmap
					table.insert(maps, newmap)
				end
				if true then -- RIGHT
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.LEFT)
					parent_map.neighbor[Direction.RIGHT] = newmap
					table.insert(maps, newmap)
				end
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
				end
			end,
			-----------------------------
	    	--
	    	--     |
	    	--     |
	    	--  ----
	    	--
			[MapPattern.RL]	= function(self, parent_map, events, seed, entityid, layer, maps)
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
				end
			end,
	    	--
	    	--  |
	    	--  |
	    	--  |---
	    	--
			[MapPattern.RR]	= function(self, parent_map, events, seed, entityid, layer, maps)
				if true then -- UP
					local newmap = Map:new(self, events.map_baseid, seed, entityid, events.map_events, layer, Direction.DOWN)
					parent_map.neighbor[Direction.UP] = newmap
					table.insert(maps, newmap)
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
-- t_layouts Scene:tiles_layouts()
--      t_layouts: {
--		    {
--			    events = {},
--			    obstacles = {},
--			    x = ?, y = ?
--		    },
--		    ...
--      }
--
function Scene:tiles_layouts()
    local t = {}
    for _, event in pairs(self.events) do
    	table.insert(t, {
    		events = {event.id}, obstacles = {}, x = event.coord.x, y = event.coord.y
    	})
    end
    for _, obstacle in pairs(self.obstacles) do
    	table.insert(t, {
    		events = {}, obstacles = {obstacle.id}, x = obstacle.coord.x, y = obstacle.coord.y
    	})
    end
    return t
end

--
-- entryCoord Scene:tiles_entry_coord()
--
function Scene:tiles_entry_coord()
    return self:tiles_event_find(EventCategory.ENTRY).coord
end

