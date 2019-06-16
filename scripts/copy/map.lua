
--
------------------- Map class -------------------
--

Map = {
    id = nil,
    baseid = nil,
    base = nil,
    
    seed = nil,
    random_func = nil,
    events_base = nil, -- copy:script_func->{{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    
    tiles = nil, -- {[y][x] = {objects = {[1]=?, ...}, block = ?} , ... }
    events = nil, -- all of events, {[eventid] = event instance, ...}
    obstacles = nil, -- all of obstacles, {[obstacleid] = obstacle instance, ...}
    roads = nil, -- all of roads, {{x=?,y=?}, ...}

    --
    -- bool valid_coord(coord)
    --
    valid_coord = function(self, coord)
        return coord.x >= 0 and coord.y >= 0 and coord.x < self.base.width and coord.y < self.base.height
    end,

    --
    -- bool moveable(coord)
    --
    moveable = function(self, coord)
		return self:valid_coord(coord)
	    		and (self.tiles[coord.y][coord.x] == nil 
	    			or self.tiles[coord.y][coord.x].block == BlockCategory.NONE)    
    end,

    --
    -- bool passable(coord)
    -- 
    passable = function(self, coord)
		return self:valid_coord(coord)
				and (self.tiles[coord.y][coord.x] == nil 
					or self.tiles[coord.y][coord.x].block ~= BlockCategory.STATIC)    
    end,

    
    members = nil, -- {[entityid] = {x = ?, y = ?}, ...}
    add_member = function(self, entityid, coord)
    	assert(self:moveable(coord))
    	self.members[entityid] = coord
    end,


    tile_obstacle_baseid = nil,
    random_obstacle_baseid = function(self)
        local obstacles = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.TILE then
                obstacles[obstacle_baseid] = obstacle_baseid
            end
        end
        local t_size = table.size(obstacles)
        if t_size == 0 then return nil end
        local obstacle_baseid, _ = table.random(obstacles, t_size, self.random_func)
        return obstacle_baseid
    end,
    

	FLAG_generate_try_times = 10,    
    FLAG_prior_obstacle_size = 2,
    FLAG_event_distance = 1,
	FLAG_entry_branch = 1,
	FLAG_exit_branch = 1,
	FLAG_event_allow_corner = true,
	FLAG_event_allow_edge = true,
	FLAG_trigger_event_distance = 1,
    

    --
    -- unsigned int distance(from, to)
    --
    distance = function(self, from, to)
        assert(self:valid_coord(from) and self:valid_coord(to))
        return math.abs(to.x - from.x) + math.abs(to.y - from.y)
    end,

    --
    -- bool is_corner(coord)
    --
    is_corner = function(self, coord)
        return (coord.x == 0 and coord.y == 0)
            or (coord.x == 0 and coord.y == self.base.height - 1)
            or (coord.x == self.base.width - 1 and coord.y == 0)
            or (coord.x == self.base.width - 1 and coord.y == self.base.height - 1)
    end,
    
    --
    -- bool is_edge(coord)
    --
    is_edge = function(self, coord)
        return coord.x == 0 or coord.y == 0 
            or coord.x == self.base.width - 1 or coord.y == self.base.height - 1
    end,

   
    constructor = function(self, id, baseid, seed, events_base)
        self.id = id
        self.baseid = baseid
        self.base = cc.LookupTable("Map", self.baseid)
        assert(self.base ~= nil)
        assert(self.base.width > 0 and self.base.height > 0)
        self.seed = seed
        self.random_func = NewRandom(self.seed)
        self.events_base = events_base
        self.tiles = {}
        self.events = {}
		self.obstacles = {}
		self.roads = {}
		self.members = {}
		
		self.tile_obstacle_baseid = self:random_obstacle_baseid()
        assert(self.tile_obstacle_baseid ~= nil)
		print('tile_obstacle_baseid: ' .. tostring(self.tile_obstacle_baseid))
    end,

    destructor = function(self)
        for eventid, _ in pairs(self.events) do
            cc.EventDestroy(eventid)
        end
        for obstacleid, _ in pairs(self.obstacles) do
            cc.ObstacleDestroy(obstacleid)
        end
    end
}

function Map:new(id, baseid, seed, events_base)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(id, baseid, seed, events_base)
	return object
end



--
-----------------------------------------------------------------------------------------
--

--
-- reset self.tiles
--
function Map:tiles_init()
    table.clear(self.tiles)
	for y = 0, self.base.height - 1 do self.tiles[y] = {} end
	table.clear(self.events)
	table.clear(self.obstacles)
	table.clear(self.roads)
end

--
-- void tiles_fill(object)
--
function Map:tiles_fill(object)
    assert(object.coord ~= nil)
    assert(object.base.width > 0 and object.base.height > 0)
    for y = 0, object.base.height - 1 do
        local yy = y + object.coord.y
        assert(yy < self.base.height)
        for x = 0, object.base.width - 1 do
            local xx = x + object.coord.x
            assert(xx < self.base.width)
			if self.tiles[yy][xx] == nil then
	            self.tiles[yy][xx] = {
	            	objects = {object},
	            	block = object.base.block
	            }
            else
            	local t = self.tiles[yy][xx]
            	assert(t.objects ~= nil and t.block ~= nil)
            	table.insert(t.objects, object)
            	if object.base.block ~= BlockCategory.NONE and t.block ~= BlockCategory.STATIC then
            		t.block = object.base.block
            	end
            end
        end
    end
end


--
-- void tiles_dump()
--
function Map:tiles_dump()
	cc.WriteLog("--------------------------------")
    for y = 0, self.base.height - 1 do
        local s = ''
        for x = 0, self.base.width - 1 do
            local t = self.tiles[y][x]
            if t ~= nil then
            	local objects = self.tiles[y][x].objects
            	assert(objects ~= nil)

				local tilechar = nil
            	for _, object in pairs(objects) do
	                assert(object.objectCategory ~= GridObjectCategory.NONE)
					if object.objectCategory == GridObjectCategory.OBSTACLE then
						tilechar = object.base.tilechar
					else
						if object.base.category == EventCategory.ENTRY then
							tilechar = '>'
						elseif object.base.category == EventCategory.EXIT then
							tilechar = '>'
						else
							tilechar = '*'
						end
					end
				end
				s = s .. ' ' .. tostring(tilechar)
            else
                s = s .. ' ' .. ' ' 
            end 
        end 
        cc.WriteLog(s)
    end
	cc.WriteLog("--------------------------------")
end


--
-- void tiles_event_generator()
--
function Map:tiles_event_generator()
    local function event_generator(self, events)
        assert(events ~= nil)
        for _, t in pairs(events) do
            local event_base = cc.LookupTable("Event", t.event_baseid)
            assert(event_base.width > 0 and event_base.height > 0)
            if t.coord ~= nil then assert(self:valid_coord(t.coord)) end
            local eventid = cc.EventNew(t.event_baseid)
            assert(self.events[eventid] == nil)
            self.events[eventid] = Event:new(eventid, t.event_baseid, t.coord)
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
    
    -- level A: self.events_base = {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    event_generator(self, self.events_base)
    
    -- level B: self.base.STICK_events = {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    event_generator(self, self.base.STICK_events)

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
end

--
-- void tiles_obstacle_generator_noise()
--
function Map:tiles_obstacle_generator_noise()
    local t = {}
    for obstacle_baseid, _ in pairs(self.base.obstacles) do
        local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
        assert(obstacle_base.width > 0 and obstacle_base.height > 0)
        local noise_min = obstacle_base.noise_min
        local noise_max = obstacle_base.noise_max
        if noise_min > 0 and noise_max > 0 then
            for i = noise_min, noise_max do t[i] = obstacle_baseid end
        end
    end

    for y = 0, self.base.height - 1 do
        for x = 0, self.base.width - 1 do
            if self.tiles[y][x] == nil then
                local noise_value = SimplexNoise2DInteger(x, y)					
                if t[noise_value] ~= nil then
                    local obstacle_baseid = t[noise_value]
                    local obstacleid = cc.ObstacleNew(obstacle_baseid)
                    assert(self.obstacles[obstacleid] == nil)
                    self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, {x = x, y = y})
               		self:tiles_fill(self.obstacles[obstacleid])
				else
					--cc.WriteLog('noise_value: ' .. tostring(noise_value))
                end
            end
        end
    end
end


--
-- void tiles_obstacle_generator(size)
--
function Map:tiles_obstacle_generator(size)
    -- self.base.obstacles = { obstacle_baseid = number, ... }
    assert(size > 0)
    for obstacle_baseid, number in pairs(self.base.obstacles) do
        local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
        assert(obstacle_base.width > 0 and obstacle_base.height > 0)
        if obstacle_base.width >= size or obstacle_base.height >= size then
            while number > 0 do
			    local t = self:tiles_available_grid({width = obstacle_base.width, height = obstacle_base.height})
			    local t_size = table.size(t)
			    if t_size > 0 then
               		local i, coord = table.random(t, t_size, self.random_func)
                    local obstacleid = cc.ObstacleNew(obstacle_baseid)
                    assert(self.obstacles[obstacleid] == nil)
                    self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, coord)
               		self:tiles_fill(self.obstacles[obstacleid])
           		end
           		number = number - 1
       		end
        end
    end
end


--
-- void tiles_obstacle_padding()
--
function Map:tiles_obstacle_padding()
    local function tiles_obstacle_picksize(self, maxsize)
        local obstacles = {}
        for obstacle_baseid, number in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.width <= maxsize and obstacle_base.height <= maxsize 
				and obstacle_base.category ~= ObstacleCategory.TILE
                  and (obstacle_base.category == ObstacleCategory.TREE 
                          or obstacle_base.category == ObstacleCategory.GRASS
                          or obstacle_base.category == ObstacleCategory.STONE
                          or obstacle_base.category == ObstacleCategory.DECORATION)
            then
                obstacles[obstacle_baseid] = number
            end
        end
        return obstacles
    end
    
    local function tiles_obstacle_padding_road(self, road_coord)
        local obstacles = tiles_obstacle_picksize(self, 1)
        local t_size = table.size(obstacles)
        if t_size == 0 then return end
        
        local t = {
            {x = 1, y = 0}, {x = -1, y = 0}, {x = 0, y = 1}, {x = 0, y = -1},
            {x = 1, y = -1}, {x = -1, y = 1}, {x = 1, y = 1}, {x = -1, y = -1}
        }
        for _, coord in pairs(t) do
            local nil_coord = {
                x = road_coord.x + coord.x, 
                y = road_coord.y + coord.y
            }
            if self:valid_coord(nil_coord) and self.tiles[nil_coord.y][nil_coord.x] == nil then
               	local obstacle_baseid, _ = table.random(obstacles, t_size, self.random_func)
                local obstacleid = cc.ObstacleNew(obstacle_baseid)
                assert(self.obstacles[obstacleid] == nil)
                self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, nil_coord)
               	self:tiles_fill(self.obstacles[obstacleid])
            end
        end
    end

    local function tiles_obstacle_padding_hole(self)
        local obstacles = tiles_obstacle_picksize(self, 1)
        local t_size = table.size(obstacles)
        if t_size == 0 then return end
    
        for y = 0, self.base.height - 1 do
            for x = 0, self.base.width - 1 do                
				if self.tiles[y][x] == nil then
                   	local obstacle_baseid, _ = table.random(obstacles, t_size, self.random_func)
                    local obstacleid = cc.ObstacleNew(obstacle_baseid)
                    assert(self.obstacles[obstacleid] == nil)
                    self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, {x = x, y = y})
                   	self:tiles_fill(self.obstacles[obstacleid])
                end
            end
        end
    end

	local function tiles_obstacle_padding_tile(self)
		assert(self.tile_obstacle_baseid ~= nil)
        for y = 0, self.base.height - 1 do
            for x = 0, self.base.width - 1 do                
				if self.tiles[y][x] == nil then
                   	local obstacle_baseid = self.tile_obstacle_baseid
                    local obstacleid = cc.ObstacleNew(obstacle_baseid)
                    assert(self.obstacles[obstacleid] == nil)
                    self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, {x = x, y = y})
                   	self:tiles_fill(self.obstacles[obstacleid])
                end
            end
        end
	end

    for _, road_coord in pairs(self.roads) do
        tiles_obstacle_padding_road(self, road_coord)
    end

	tiles_obstacle_padding_hole(self)
	tiles_obstacle_padding_tile(self)
end


--
-- {coord, ...} tiles_available_grid(rect)
-- 
function Map:tiles_available_grid(rect)
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
function Map:tiles_event_find(eventCategory)
    for _, event in pairs(self.events) do
        if event.base.category == eventCategory then return event end
    end
    return nil
end


--
-- bool tiles_link_events()
--
function Map:tiles_link_events()
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
	    local min_coord = src_coord
	    local min_distance = self:distance(src_coord, dest_coord)
	    for _, coord in pairs(rc) do
	        local distance = self:distance(coord, dest_coord)
	        if distance < min_distance then
	            min_coord = coord
	            min_distance = distance
	        end
	    end
	    return min_coord
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
            for _, coord in pairs(road) do
                assert(self:valid_coord(coord))
                if self.tiles[coord.y][coord.x] == nil then
                    local obstacleid = cc.ObstacleNew(self.tile_obstacle_baseid)
                    self:tiles_fill(Obstacle:new(obstacleid, self.tile_obstacle_baseid, coord))
                end
            end
        else
            cc.WriteLog('link Event error: ' .. tostring(errorstring))
        end

        return road
	end

	local entry_event = self:tiles_event_find(EventCategory.ENTRY)
	assert(entry_event ~= nil and entry_event.coord ~= nil)

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
            from_coord = shortest_distance(self, from_coord, self.roads, event.coord)
            local road = link_event(self, from_coord, event.coord)
            if road == nil then return false end
            
            to[i] = nil
            from_coord = event.coord

            for _, road_coord in pairs(road) do
                table.insert(self.roads, road_coord)
            end
            
            break
        end
        to_size = table.size(to)
    end

    --
    -- link last event to EXIT
    return link_event(self, from_coord, exit_event.coord)
end


--
-- TODO: width > 1 or height > 1 ??
-- layouts, entryCoord generator()
-- layouts: {
--		{
--			events = {},
--			obstacles = {},
--			x = ?, y = ?
--		},
--		...
-- }
--
function Map:generator()
    local function tiles_layouts(self)
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
        
    	local entry_event = self:tiles_event_find(EventCategory.ENTRY)
    	assert(entry_event ~= nil and entry_event.coord ~= nil)
        
    	return t, entry_event.coord
    end
    
    local size = self.FLAG_prior_obstacle_size
    local try_times = self.FLAG_generate_try_times
    while try_times > 0 do
        self:tiles_init()
        self:tiles_event_generator()
          self:tiles_obstacle_generator(size)
        if self:tiles_link_events() then 
            self:tiles_obstacle_generator_noise()
            self:tiles_obstacle_padding()
			self:tiles_dump()
            return tiles_layouts(self)
        end
        size = size + 1
        try_times = try_times - 1
		cc.WriteLog('try times: ' .. tostring(try_times) .. ', size: ' .. tostring(size))
    end
    assert(false)
end


--
-----------------------------------------------------------------------------------------
--

--
-- fullPath, eventid Map::move_request(entityid, destx, desty)
--
function Map:move_request(entityid, destx, desty)
    assert(self.members[entityid] ~= nil)
    local coord = self.members[entityid]
    assert(entity.coord ~= nil and self:valid_coord(entity.coord))

    local fullPath = {}
    local dest_coord = {x = destx, y = desty}
    
    -- check dest coord is valid
    if not self:valid_coord(dest_coord) then
        cc.WriteLog("illegal dest: (" .. tostring(destx) .. "," .. tostring(desty) .. ")")
        return fullPath, nil
    end
    
    local object = self.tiles[desty][destx]
    assert(object ~= nil)
    assert(object.objectCategory ~= GridObjectCategory.NONE)
    if object.objectCategory == GridObjectCategory.OBSTACLE then
        -- check dest coord is not block
        if object.base.block == BlockCategory.STATIC then
            cc.WriteLog("dest: (" .. tostring(destx) .. "," .. tostring(desty) .. ") is block")
            return fullPath, nil
        end

        -- try to FindPath
        local road, errorstring = FindPath(
        			entity.coord, 
        			dest_coord, 
        			1, 
        			math.max(self.base.width, self.base.height), 
		            function(coord) return self:moveable(coord) end
		            )

        if road == nil then
            cc.WriteLog("dest: (" .. tostring(destx) .. "," .. tostring(desty) .. ") FindPath error: " .. errorstring)
            return fullPath, nil
        end

        -- return Path
        local road_size = table.size(road)
        for _, coord in pairs(road) do
            fullPath[road_size] = coord
            road_size = road_size - 1
        end

        return fullPath, nil
        
    else -- Event trigger
    
        -- check distance is enough
        if math.abs(destx - entity.coord.x) <= self.FLAG_trigger_event_distance
            and math.abs(desty - entity.coord.y) <= self.FLAG_trigger_event_distance
        then
            return nil, object.id
        end
        
        -- check dest coord is not block
        if object.base.block == BlockCategory.STATIC then
            cc.WriteLog("dest: (" .. tostring(destx) .. "," .. tostring(desty) .. ") is block")
            return fullPath, nil
        end

        -- try to FindPath ??
        cc.WriteLog("dest: (" .. tostring(destx) .. "," .. tostring(desty) .. ") event too far")
        return fullPath, nil            
    end
end


--
-- bool Map::move(entityid, x, y)
--
function Map:move(entityid, x, y)
    assert(self.members[entityid] ~= nil)
    local entity = self.members[entityid]
    assert(entity.coord ~= nil and self:valid_coord(entity.coord))

    local dest_coord = {x = x, y = y}
    if not self:valid_coord(dest_coord) then
        cc.WriteLog("illegal dest: (" .. tostring(x) .. "," .. tostring(y) .. ")")
        return false
    end
    
    local object = self.tiles[y][x]
    assert(object ~= nil)
    assert(object.objectCategory ~= GridObjectCategory.NONE)
	
	
end

