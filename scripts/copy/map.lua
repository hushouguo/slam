
--
------------------- Map class -------------------
--

Map = {
    id = nil,
    baseid = nil,
    base = nil,
    
    seed = nil,
    randomfunc = nil,
    events_base = nil, -- {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    
    tiles = nil, -- {[y][x] = object , ... }
    events = nil, -- all of events, {[eventid] = event instance, ...}
    obstacles = nil, -- all of obstacles, {[obstacleid] = obstacle instance, ...}
    roads = nil, -- all of roads, {{x=?,y=?}, ...}


    --
    ---------------- entity move & move_request
    --
    
    members = nil, -- {[entityid] = entity, ...}
    add_member = function(self, entityid, coord)
        assert(self:valid_coord(coord))
        local object = self.tiles[coord.y][coord.x]
        assert(object ~= nil)
        assert(object.base.block ~= BlockCategory.STATIC)
        
        if self.members[entityid] == nil then
            self.members[entityid] = Entity:new(entityid, Side.ALLIES)            
        end
        
        local entity = self.members[entityid]
        assert(entity ~= nil)
        entity.coord = coord
    end,

    --
    -- fullPath, eventid function move_request(entityid, destx, desty)
    --
    move_request = function(self, entityid, destx, desty)
        assert(self.members[entityid] ~= nil)
        local entity = self.members[entityid]
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
            local road, errorstring = FindPath(entity.coord, dest_coord, 1, math.max(self.base.width, self.base.height), 
                function(coord)
    	            return self:valid_coord(coord)
    	                and (self.tiles[coord.y][coord.x] == nil or self.tiles[coord.y][coord.x].base.block == BlockCategory.NONE)
                end)

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
    end,


    --
    ---------------- generate Map
    --

    tile_obstacle_baseid = nil,
    random_obstacle_baseid = function(self)
        local obstacles = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base ~= nil)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.TILE then
                obstacles[obstacle_baseid] = obstacle_baseid
            end
        end
        local t_size = table.size(obstacles)
        if t_size == 0 then return nil end
        local obstacle_baseid, _ = table.random(obstacles, t_size, self.randomfunc)
        return obstacle_baseid
    end,
    
    FLAG_prior_obstacle_size = 2,
    FLAG_event_distance = 1,
	FLAG_entry_branch = 1,
	FLAG_exit_branch = 1,
	FLAG_event_allow_corner = true,
	FLAG_event_allow_edge = true,
	FLAG_trigger_event_distance = 1,
    
    --
    -- bool valid_coord(coord)
    --
    valid_coord = function(self, coord)
        return coord.x >= 0 and coord.y >= 0 and coord.x < self.base.width and coord.y < self.base.height
    end,

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

    --
    -- reset self.tiles
    --
    tiles_init = function(self)
        table.clear(self.tiles)
		for y = 0, self.base.height - 1 do self.tiles[y] = {} end
		table.clear(self.events)
		table.clear(self.obstacles)
		table.clear(self.roads)
    end,
    
    --
    -- void tiles_fill(object)
    --
    tiles_fill = function(self, object)
        assert(object.coord ~= nil)
        assert(object.base.width > 0 and object.base.height > 0)
        for y = 0, object.base.height - 1 do
            local yy = y + object.coord.y
            assert(yy < self.base.height)
            for x = 0, object.base.width - 1 do
                local xx = x + object.coord.x
                assert(xx < self.base.width)
				assert(self.tiles[yy][xx] == nil)
                self.tiles[yy][xx] = object
            end
        end
    end,

    --
    -- void tiles_dump()
    --
    tiles_dump = function(self)
		cc.WriteLog("--------------------------------")
        for y = 0, self.base.height - 1 do
            local s = ''
            for x = 0, self.base.width - 1 do
                local object = self.tiles[y][x]
                if object ~= nil then
                    assert(object.objectCategory ~= GridObjectCategory.NONE)
					if object.objectCategory == GridObjectCategory.OBSTACLE then
						s = s .. ' ' .. object.base.tilechar
					else
						if object.base.category == EventCategory.ENTRY then
							s = s .. ' ' .. '>'
						elseif object.base.category == EventCategory.EXIT then
							s = s .. ' ' .. '>'
						else
							s = s .. ' ' .. '*'
						end
					end
                else
                    s = s .. ' ' .. ' ' 
                end 
            end 
            cc.WriteLog(s)
        end
		cc.WriteLog("--------------------------------")
    end,

    --
    -- void tiles_event_generator()
    --
    tiles_event_generator = function(self)
        local function event_generator(self, events)
            assert(events ~= nil)
            for _, t in pairs(events) do
                local event_base = cc.LookupTable("Event", t.event_baseid)
                assert(event_base ~= nil)
                assert(event_base.width > 0 and event_base.height > 0)
                if t.coord ~= nil then assert(self:valid_coord(t.coord)) end
                local eventid = cc.EventNew(t.event_baseid)
                assert(eventid ~= nil)
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

        --assert(table.size(self.events) > 0)

        for _, event in pairs(self.events) do
            if event.coord ~= nil then
                self:tiles_fill(event)
            end
        end
        
        -- random coord for no speciafy coord event
        for eventid, event in pairs(self.events) do
            local FLAG_event_distance = self.base.FLAG_event_distance
            if FLAG_event_distance < 0 then FLAG_event_distance = 0 end
            while event.coord == nil do
			    local t = self:tiles_available_grid({width = event.base.width, height = event.base.height})
			    local t_size = table.size(t)
			    while t_size > 0 do
               		local i, coord = table.random(t, t_size, self.randomfunc)
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
    end,

    --
    -- void tiles_obstacle_generator_noise()
    --
    tiles_obstacle_generator_noise = function(self)
        local t = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base ~= nil)
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
                        assert(obstacleid ~= nil)
                        assert(self.obstacles[obstacleid] == nil)
                        self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, {x = x, y = y})
                   		self:tiles_fill(self.obstacles[obstacleid])
					else
						--cc.WriteLog('noise_value: ' .. tostring(noise_value))
                    end
                end
            end
        end
    end,
    
    --
    -- void tiles_obstacle_generator(size)
    --
    tiles_obstacle_generator = function(self, size)
        -- self.base.obstacles = { obstacle_baseid = number, ... }
        assert(size > 0)
        for obstacle_baseid, number in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base ~= nil)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.width >= size or obstacle_base.height >= size then
                while number > 0 do
    			    local t = self:tiles_available_grid({width = obstacle_base.width, height = obstacle_base.height})
    			    local t_size = table.size(t)
    			    if t_size > 0 then
                   		local i, coord = table.random(t, t_size, self.randomfunc)
                        local obstacleid = cc.ObstacleNew(obstacle_baseid)
                        assert(obstacleid ~= nil)
                        assert(self.obstacles[obstacleid] == nil)
                        self.obstacles[obstacleid] = Obstacle:new(obstacleid, obstacle_baseid, coord)
                   		self:tiles_fill(self.obstacles[obstacleid])
               		end
               		number = number - 1
           		end
            end
        end
    end,

    --
    -- void tiles_obstacle_padding()
    --
    tiles_obstacle_padding = function(self)
        local function tiles_obstacle_picksize(self, maxsize)
            local obstacles = {}
            for obstacle_baseid, number in pairs(self.base.obstacles) do
                local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
                assert(obstacle_base ~= nil)
                assert(obstacle_base.width > 0 and obstacle_base.height > 0)
                if obstacle_base.width <= maxsize and obstacle_base.height <= maxsize 
					and obstacle_base.category ~= ObstacleCategory.TILE
--                    and (obstacle_base.category == ObstacleCategory.TREE 
--                            or obstacle_base.category == ObstacleCategory.GRASS
--                            or obstacle_base.category == ObstacleCategory.STONE
--                            or obstacle_base.category == ObstacleCategory.DECORATION)
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
                   	local obstacle_baseid, _ = table.random(obstacles, t_size, self.randomfunc)
                    local obstacleid = cc.ObstacleNew(obstacle_baseid)
                    assert(obstacleid ~= nil)
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
                       	local obstacle_baseid, _ = table.random(obstacles, t_size, self.randomfunc)
                        local obstacleid = cc.ObstacleNew(obstacle_baseid)
                        assert(obstacleid ~= nil)
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
                        assert(obstacleid ~= nil)
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
    end,

    --
    -- {coord, ...} tiles_available_grid(rect)
    -- 
    tiles_available_grid = function(self, rect)
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
    end,

    --
    -- event tiles_event_find(eventCategory)
    --
    tiles_event_find = function(self, eventCategory)
        for _, event in pairs(self.events) do
            if event.base.category == eventCategory then return event end
        end
        return nil
    end,

    --
    -- bool tiles_link_events()
    --
    tiles_link_events = function(self)
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
            local road, errorstring = FindPath(src_coord, dest_coord, 1, math.max(self.base.width, self.base.height), 
                function(coord)
    	            return self:valid_coord(coord)
    	                and (self.tiles[coord.y][coord.x] == nil or self.tiles[coord.y][coord.x].base.block ~= BlockCategory.STATIC)
                end)

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
    end,

    --
    -- layouts, entryCoord generator()
    --
    generator = function(self)
        local function tiles_layouts(self)
        	local t = {}
        	for y = 0, self.base.height - 1 do
        		for x = 0, self.base.width - 1 do
        		    if self.tiles[y][x] ~= nil then
        		        local object = self.tiles[y][x]
        		        if object.objectCategory == GridObjectCategory.OBSTACLE then
        		            table.insert(t, {eventid = nil, obstacleid = object.id, x = x, y = y})
        		        elseif object.objectCategory == GridObjectCategory.EVENT then
        		            table.insert(t, {eventid = object.id, obstacleid = nil, x = x, y = y})
        		        else
        		            assert(false)
        		        end
        		    end
        		end
        	end
            
        	local entry_event = self:tiles_event_find(EventCategory.ENTRY)
        	assert(entry_event ~= nil and entry_event.coord ~= nil)
            
        	return t, entry_event.coord
        end
        
        local size = 2 -- NOTE: FLAG_prior_obstacle_size
        local try_times = 10
        while try_times > 0 do
            self:tiles_init()
            self:tiles_event_generator()
--            self:tiles_obstacle_generator(size)
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
    end,
   
    constructor = function(self, id, baseid, seed, events_base)
        self.id = id
        self.baseid = baseid
        self.base = cc.LookupTable("Map", self.baseid)
        assert(self.base ~= nil)
        assert(self.base.width > 0 and self.base.height > 0)
        self.seed = seed
        self.randomfunc = NewRandom(self.seed)
        self.events_base = events_base
        self.tiles = {}
        self.events = {}
		self.obstacles = {}
		self.roads = {}
		self.members = {}
		
		self.tile_obstacle_baseid = self:random_obstacle_baseid()
        assert(self.tile_obstacle_baseid ~= nil)
		cc.WriteLog('tile_obstacle_baseid: ' .. tostring(self.tile_obstacle_baseid))
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
	local map = {}
	self.__index = self -- map.__index = function(key) return map[key] end
	setmetatable(map, self)
	map:constructor(id, baseid, seed, events_base)
	return map
end

