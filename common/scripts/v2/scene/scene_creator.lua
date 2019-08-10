
--
------------------- Scene creator -------------------
--

--
-- reset self.tiles
--
function Scene:tiles_init()
    table.clear(self.tiles)
	for y = 0, self.base.height - 1 do self.tiles[y] = {} end
	table.clear(self.events)
	table.clear(self.obstacles)
	table.clear(self.roads)
end

--
-- object create_event_object(event_baseid, coord)
--
function Scene:create_event_object(event_baseid, coord)
    local object = Event:new(self, event_baseid, coord)
    assert(self.events[object.id] == nil)
    self.events[object.id] = object
    return object
end

--
-- object create_obstacle_object(obstacle_baseid, coord)
--
function Scene:create_obstacle_object(obstacle_baseid, coord)
    local object = Obstacle:new(self, obstacle_baseid, coord)
    assert(self.obstacles[object.id] == nil)
    self.obstacles[object.id] = object
    return object
end

--
-- void tiles_fill(object)
--
function Scene:tiles_fill(object)
    assert(object.coord ~= nil)
    assert(object.base.width > 0 and object.base.height > 0)

    --
    -- try to setup self.events or self.obstacles
    assert(object.objectCategory ~= GridObjectCategory.NONE)
    if object.objectCategory == GridObjectCategory.EVENT then
        if self.events[object.id] == nil then self.events[object.id] = object end
        assert(self.events[object.id].id == object.id)
    else
        if self.obstacles[object.id] == nil then self.obstacles[object.id] = object end
        assert(self.obstacles[object.id].id == object.id)
    end

    --
    -- fill tile
    for y = 0, object.base.height - 1 do
        local yy = y + object.coord.y
        for x = 0, object.base.width - 1 do
            local xx = x + object.coord.x
            local coord = {x = xx, y = yy}
            assert(self:valid_coord(coord), string.format("baseid: %d, coord:(%d,%d)", object.baseid, coord.x, coord.y))
            
			if self.tiles[yy][xx] == nil then
			    local objects = {}
			    if object.base.require_tile then 
			        -- auto fill tile & add tile object to self.obstacles
			        table.insert(objects, self:create_obstacle_object(self:tile_obstacle_baseid_retrieve(coord), coord))
			    end
			    table.insert(objects, object)
			    
	            self.tiles[yy][xx] = {
	            	objects = objects,
	            	block = BlockCategory.NONE
	            }
            else
            	local t = self.tiles[yy][xx]
            	assert(t.objects ~= nil and t.block ~= nil)
            	table.insert(t.objects, object) -- add extra object
            end
            
           	self:resetblock(coord)
        end
    end

--	cc.ScriptDebugLog(string.format("tiles_fill object: %d,%s on coord: (%d,%d)", object.baseid, object.base.name.cn, object.coord.x, object.coord.y))
end


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
    assert(self.base.width >= FLAG_pattern_width)
    assert(self.base.height >= FLAG_pattern_height)
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
    		else
    			cc.ScriptErrorLog(string.format("illegal char: %s", cr))
    		end
    	end
    end
end

--
-- void tiles_event_generator()
--
function Scene:tiles_event_generator()
    local function event_generator(self, events)
        assert(events ~= nil)
        for _, t in pairs(events) do
            local event_base = cc.ScriptLookupTable("Event", t.event_baseid)
            assert(event_base.width > 0 and event_base.height > 0)
            if t.coord ~= nil then assert(self:valid_coord(t.coord)) end
            self:create_event_object(t.event_baseid, t.coord)
        end
    end

    local function event_distance(self, coord, FLAG_event_distance)
        for _, event in pairs(self.events) do
            if event.coord ~= nil then
                if (math.abs(event.coord.x - coord.x) - 1) < FLAG_event_distance
                    or (math.abs(event.coord.y - coord.y) - 1) < FLAG_event_distance
                then
                    return false
                end
            end
        end
        return true
    end

    -- level A: self.base.STICK_events = {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    event_generator(self, self.base.STICK_events)
	cc.ScriptDebugLog(string.format("STICK_events: %d", table.size(self.events)))
    
    -- level B: self.events_base = {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    event_generator(self, self.events_base)    
	cc.ScriptDebugLog(string.format("events_base: %d", table.size(self.events)))

    for _, event in pairs(self.events) do
        if event.coord ~= nil then
            self:tiles_fill(event)
        end
    end
    
    -- random coord for no speciafy coord event
    for eventid, event in pairs(self.events) do
        local FLAG_event_distance = self.FLAG_event_distance
        if FLAG_event_distance < 0 then FLAG_event_distance = 0 end        
        while event.coord == nil do
		    local t = self:tiles_available_grid({width = event.base.width, height = event.base.height})
		    local t_size = table.size(t)
		    while t_size > 0 do
           		local i, coord = table.random(t, t_size, self.random_func)
           		if event_distance(self, coord, FLAG_event_distance) then 
               		event.coord = coord
               		self:tiles_fill(event)
           		    break 
           		end
                t[i] = nil
                t_size = table.size(t)
		    end
			if event.coord == nil then
				FLAG_event_distance = FLAG_event_distance - 1
				assert(FLAG_event_distance >= 0)
			end
        end
    end

	for _, event in pairs(self.events) do
		cc.ScriptDebugLog(string.format("    event: %d,%d,%s on coord: (%d,%d)", event.id, event.baseid, event.base.name.cn, event.coord.x, event.coord.y))
	end
end

--
-- void tiles_obstacle_STICK_generator()
--
function Scene:tiles_obstacle_STICK_generator()
    -- self.base.STICK_obstacles
    for _, t in pairs(self.base.STICK_obstacles) do
        local obstacle_baseid = t.obstacle_baseid
        assert(obstacle_baseid ~= nil)
        local obstacle_base = cc.ScriptLookupTable("Obstacle", obstacle_baseid)
        assert(obstacle_base.width > 0 and obstacle_base.height > 0)
        
        local coord = t.coord
        if coord == nil then
    		local t = self:tiles_available_grid({width = obstacle_base.width, height = obstacle_base.height})
    		local t_size = table.size(t)
    		if t_size > 0 then
            	local _, coord_random = table.random(t, t_size, self.random_func)
				coord = coord_random
            end
        end

        if coord ~= nil then            
            self:tiles_fill(self:create_obstacle_object(obstacle_baseid, coord))
        else
            cc.ScriptDebugLog(string.format("not enough space to generate STICK obstacle: %d", obstacle_baseid))
        end
    end
	cc.ScriptDebugLog(string.format("STICK_obstacles: %d", table.size(self.obstacles)))
end


--
-- void tiles_obstacle_BLOCK_generator(blocksize)
--
function Scene:tiles_obstacle_BLOCK_generator(blocksize)
    -- self.base.obstacles = { obstacle_baseid = number, ... }
    assert(blocksize > 0)
    for obstacle_baseid, number in pairs(self.base.obstacles) do
        local obstacle_base = cc.ScriptLookupTable("Obstacle", obstacle_baseid)
        assert(obstacle_base.width > 0 and obstacle_base.height > 0)
        if obstacle_base.width >= blocksize or obstacle_base.height >= blocksize then
            while number > 0 do
			    local t = self:tiles_available_grid({width = obstacle_base.width, height = obstacle_base.height})
			    local t_size = table.size(t)
			    if t_size > 0 then
               		local _, coord = table.random(t, t_size, self.random_func)
               		self:tiles_fill(self:create_obstacle_object(obstacle_baseid, coord))
           		end
           		number = number - 1
       		end
        end
    end
	cc.ScriptDebugLog(string.format("BLOCK_obstacles: %d", table.size(self.obstacles)))
end

--
-- {coord, ...} tiles_available_grid(rect)
-- 
function Scene:tiles_available_grid(rect)
	local function tiles_check_grid(self, coord, rect)
		assert(rect.width > 0 and rect.height > 0)
		for y = 0, rect.height - 1 do
		    local yy = y + coord.y
		    if yy >= self.base.height then return false end -- overflow

			for x = 0, rect.width -1 do
			    local xx = x + coord.x
			    if xx >= self.base.width then return false end -- overflow
			    
			    if self.tiles[yy][xx] ~= nil then return false end
			end
		end
		return true
	end

	local t = {} -- {coord, ...}
	for y = 0, self.base.height - 1 do
		for x = 0, self.base.width - 1 do
		    local coord = {x = x, y = y}
			if tiles_check_grid(self, coord, rect) then
			    table.insert(t, coord)
			end
		end
	end
	return t
end


--
-- event tiles_event_find(eventCategory)
--
function Scene:tiles_event_find(eventCategory)
    for _, event in pairs(self.events) do
        if event.base.category == eventCategory then return event end
    end
    return nil
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
--  bool generator()
--
function Scene:generator()    
    local size = self.FLAG_prior_obstacle_size
    local try_times = self.FLAG_generate_try_times
    while try_times > 0 do
        self:tiles_init()
        self:tiles_pattern_generator()
        --self:tiles_event_generator()
        --self:tiles_obstacle_STICK_generator()
        --self:tiles_obstacle_BLOCK_generator(size)
        --if self:tiles_link_events() then
            --self:tiles_obstacle_padding_tile()
			self:tiles_dump()
            return true
        --end
        --size = size + 1
        --try_times = try_times - 1
		--cc.ScriptDebugLog('try times: ' .. tostring(try_times) .. ', size: ' .. tostring(size))
    end
    return false
end

--
--------------------- export & import ---------------------
--

--
-- string export()
--
function Scene:export()
    -- TODO: export layout setting
end

--
-- bool import(string)
--
function Scene:import()
    -- TODO: import layout setting
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

