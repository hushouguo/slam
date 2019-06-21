
--
------------------- Scene class -------------------
--

Scene = {
    copy = nil, -- reference to copy instance
    
    id = nil,
    baseid = nil,
    base = nil,
    
    seed = nil,
    random_func = nil,
    
    events_base = nil, -- copy.script_func() -> {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    accomplish_events = nil, -- {event_baseid, ...}
    
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

    --
    -- void resetblock(coord)
    --
    resetblock = function(self, coord)
        assert(self:valid_coord(coord))
        local t = self.tiles[coord.y][coord.x]
        assert(t ~= nil and t.objects ~= nil and t.block ~= nil)
        t.block = BlockCategory.NONE
        for _, object in pairs(t.objects) do
            local block = object.base.block
            if object.accomplish then block = BlockCategory.NONE end
            -- STATIC:2, DYNAMIC:1, NONE:0
            if block > t.block then t.block = block end
        end
    end,

    --
    -- members hashmap & [add|remove]_member methods
    --
    members = nil, -- {[entityid] = entity, ...}
    add_member = function(self, entity)
        assert(entity.copy.id == self.copy.id)
        entity.coord = self:tiles_entry_coord()
        assert(entity.coord ~= nil)
        assert(self:moveable(entity.coord))
        assert(self.members[entity.id] == nil)
    	self.members[entity.id] = entity
        cc.EnterMap(entity.id, self.id, self:tiles_layouts(), entity.coord)
        cc.WriteLog(string.format("entity: %d enterScene: %d, coord: (%d, %d)", entity.id, self.id, entity.coord.x, entity.coord.y))
        entity:enter_scene(self)
    end,
    remove_member = function(self, entity)
        assert(entity.copy.id == self.copy.id)
        assert(self.members[entity.id] ~= nil)
        self.members[entity.id] = nil
        entity.coord = nil
        if self.match ~= nil then
            self:abort_match(entity.id)
        end
        cc.ExitMap(entity.id, self.id)
        cc.WriteLog(string.format("entity: %d exitScene: %d", entity.id, self.id))
        entity:exit_scene(self)
    end,

    --
    -- trigger_event
    --
    trigger_event = nil,

    --
    -- void create_match() & void end_match
    --
    match = nil,
    start_match = function(self)
        assert(self.match == nil)
        assert(self.trigger_event ~= nil) -- the event must have been triggered before
        assert(self.trigger_event.base.category == EventCategory.MONSTER) -- MUST be monster event
        assert(self.trigger_event.accomplish == false) -- MUST not be accomplish
        
        self.match = Match:new(self.copy)
        self.match:prepare()
        for _, entity in pairs(self.members) do
            self.match:add_member(entity, Side.ALLIES)
        end
        assert(self.trigger_event.content ~= nil)
        assert(self.trigger_event.content.monster ~= nil)
        local monsters = self.trigger_event.content.monster.monsters
        assert(monsters ~= nil)
        for entity_baseid, number in pairs(monsters) do
            assert(number > 0)
            for i = 0, number do
                self.match:add_monster(entity_baseid, Side.ENEMY)
            end
        end
        self.match:start()
    end,
    end_match = function(self)
        assert(self.match ~= nil)
        assert(self.trigger_event ~= nil) -- the event be holded by trigger_event object
        assert(self.trigger_event.base.category == EventCategory.MONSTER)
        assert(self.trigger_event.accomplish == false) -- MUST not be accomplish
        
        assert(self.match.isdone)
        assert(self.match.side_victory ~= nil)
        
        local side_victory = self.match.side_victory
		self.match:destructor()
	    self.match = nil
        -- self.trigger_event = nil -- NOTE: don't destroy trigger_event, cause the entity get reward not yet!
        
        if side_victory == Side.ALLIES then
            self:accomplish_event(self.trigger_event)
        end
        
        if side_victory == Side.ENEMY then
            cc.WriteLog(string.format("defeated"))
            -- TODO: exit copy
            -- for entityid, _ in pairs(self.members) do
            --    self.copy:exit_copy(entityid)
            -- end
        end
    end,
    abort_match = function(self, entityid)
        assert(self.match ~= nil)
        assert(self.trigger_event ~= nil) -- the event be holded by trigger_event object
        assert(self.trigger_event.base.category == EventCategory.MONSTER)
        assert(self.trigger_event.accomplish == false) -- MUST not be accomplish
        self.match:abort(entityid)
    end,

    --
    -- tile obstacle baseid
    --
    tile_obstacle_baseid = nil,
    random_obstacle_baseid = function(self)
        local obstacles = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.TILE and obstacle_base.block == BlockCategory.NONE then
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
	FLAG_adjust_dest_obstacle = false,
	FLAG_adjust_dest_event = true,
    

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


    constructor = function(self, copy, id, baseid, seed, entityid, events_base, accomplish_events)
        self.copy = copy
        self.id = id
        self.baseid = baseid
        self.base = cc.LookupTable("Map", self.baseid)
        assert(self.base ~= nil)
        assert(self.base.width > 0 and self.base.height > 0)
        self.seed = seed
        self.random_func = NewRandom(self.seed)
        self.events_base = events_base
        assert(self.events_base ~= nil)
        self.accomplish_events = accomplish_events
        if self.accomplish_events == nil then self.accomplish_events = {} end
        self.tiles = {}
        self.events = {}
		self.obstacles = {}
		self.roads = {}
		self.members = {}
		self.match = nil
		self.trigger_event = nil

		-- search for available tile obstacle
		self.tile_obstacle_baseid = self:random_obstacle_baseid()
        assert(self.tile_obstacle_baseid ~= nil)
		cc.WriteLog('tile_obstacle_baseid: ' .. tostring(self.tile_obstacle_baseid))

		local rc = self:generator() -- init events & obstacles
		assert(rc)
		self:do_accomplish_events() -- accomplish event in last EnterScene

        -- init all of events to content
		self:init_events_content(entityid)
    end,

    destructor = function(self)        
        for eventid, _ in pairs(self.events) do
            cc.EventDestroy(eventid)
        end
        for obstacleid, _ in pairs(self.obstacles) do
            cc.ObstacleDestroy(obstacleid)
        end
        cc.MapDestroy(self.id) -- MapDestroy
    end
}

function Scene:new(copy, id, baseid, seed, entityid, events_base, accomplish_events)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, id, baseid, seed, entityid, events_base, accomplish_events)
	return object
end


--
-- bool checkDone()
--
function Scene:checkDone()
    return false
end

--
-- void update(delta)
--
function Scene:update(delta)
    if self.match ~= nil then 
		if self.match:checkDone() then
			cc.WriteLog(string.format("match closing soon, scene: %d, %d", self.id, self.baseid))    
			self:end_match()
		else
			self.match:update(delta) 
		end
	end
end


--
-----------------------------------------------------------------------------------------
--

--
-- void accomplish_event(event)
--
function Scene:accomplish_event(event)
    assert(event.accomplish == false)
    event.accomplish = true
    -- reset tiles[y][x].block
    for y = 0, event.base.height - 1 do
        local yy = y + event.coord.y
        for x = 0, event.base.width - 1 do
            local xx = x + event.coord.x
            self:resetblock({x = xx, y = yy})
        end
    end    
    cc.WriteLog(string.format("accomplish event: %d, %d", event.id, event.baseid))
    self.copy:accomplish_event(event) -- notify client & serialize
end


function Scene:init_events_content(entityid)
    --
    -- call all of events script_func, hold by event.content
    --
    local entity_baseid = cc.GetBaseid(entityid)
    local entity_base = cc.LookupTable("Entity", entity_baseid)
    assert(entity_base ~= nil)
    for _, event in pairs(self.events) do
        assert(event.content == nil)
        event.content = event.script_func(
                            entity_base.career, 
                            self.copy.baseid, 
                            self.copy:current_layer(), 
                            event.baseid, 
                            self.copy:current_seed()
                        )
        assert(event.content ~= nil)
    	local t = {
    		[EventCategory.MONSTER] = function(seed) 
    			--table.dump(event.content)
    			assert(event.content.monster ~= nil, string.format("event: %d, seed: %d", event.baseid, seed)) 
    		end,
    		[EventCategory.SHOP_BUY_CARD] = function(seed) 
    			--table.dump(event.content)
    			--assert(event.content.shop ~= nil, string.format("event: %d, seed: %d", event.baseid, seed)) 
    		end,
    		[EventCategory.STORY] = function(seed) 
    			--table.dump(event.content)
    			--assert(event.content.story ~= nil, string.format("event: %d, seed: %d", event.baseid, seed)) 
    		end
    	}
    	if t[event.base.category] ~= nil then
    		t[event.base.category](self.copy:current_seed())
    	end
    end
end

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
-- object create_event_object()
--
function Scene:create_event_object(event_baseid, coord)
    local eventid = cc.EventNew(event_baseid)
    assert(self.events[eventid] == nil)
    local object = Event:new(eventid, event_baseid, coord)
    self.events[eventid] = object
    return object
end

--
-- object create_obstacle_object()
--
function Scene:create_obstacle_object(obstacle_baseid, coord)
    local obstacleid = cc.ObstacleNew(obstacle_baseid)
    assert(self.obstacles[obstacleid] == nil)
    local object = Obstacle:new(obstacleid, obstacle_baseid, coord)
    self.obstacles[obstacleid] = object
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
            assert(self:valid_coord(coord))
            
			if self.tiles[yy][xx] == nil then
			    local objects = {}
			    if object.base.require_tile then 
			        -- auto fill tile & add tile object to self.obstacles
			        table.insert(objects, self:create_obstacle_object(self.tile_obstacle_baseid, coord))
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
end


--
-- void tiles_dump()
--
function Scene:tiles_dump()
    for y = 0, self.base.height - 1 do
        local s = ''
        for x = 0, self.base.width - 1 do
            local t = self.tiles[y][x]
            if t ~= nil then
            	local objects = self.tiles[y][x].objects
            	assert(objects ~= nil)

				local tilechar = ''
            	for _, object in pairs(objects) do
	                assert(object.objectCategory ~= GridObjectCategory.NONE)
					if object.objectCategory == GridObjectCategory.OBSTACLE then
						tilechar = tilechar .. object.base.tilechar
					else
						if object.base.category == EventCategory.ENTRY then
							tilechar = tilechar .. '>'
						elseif object.base.category == EventCategory.EXIT then
							tilechar = tilechar .. '<'
						else
							tilechar = tilechar .. '*'
						end
					end
				end
				s = s .. ' ' .. string.format("%4s", tilechar)
            else
                s = s .. ' ' .. ' ' 
            end 
        end 
        cc.WriteLog(s)
    end
end


--
-- void tiles_event_generator()
--
function Scene:tiles_event_generator()
    local function event_generator(self, events)
        assert(events ~= nil)
        for _, t in pairs(events) do
            local event_base = cc.LookupTable("Event", t.event_baseid)
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
-- void tiles_obstacle_STICK_generator()
--
function Scene:tiles_obstacle_STICK_generator()
    -- self.base.STICK_obstacles
    for _, t in pairs(self.base.STICK_obstacles) do
        local obstacle_baseid = t.obstacle_baseid
        assert(obstacle_baseid ~= nil)
        local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
        assert(obstacle_base.width > 0 and obstacle_base.height > 0)
        
        local coord = t.coord
        if coord == nil then
    		local t = self:tiles_available_grid({width = obstacle_base.width, height = obstacle_base.height})
    		local t_size = table.size(t)
    		if t_size > 0 then
            	local _, coord = table.random(t, t_size, self.random_func)
            end
        end

        if coord ~= nil then            
            self:tiles_fill(self:create_obstacle_object(obstacle_baseid, coord))
        else
            cc.WriteLog(string.format("not enough space to generate STICK obstacle: %d", obstacle_baseid))
        end
    end
end


--
-- void tiles_obstacle_BLOCK_generator(blocksize)
--
function Scene:tiles_obstacle_BLOCK_generator(blocksize)
    -- self.base.obstacles = { obstacle_baseid = number, ... }
    assert(blocksize > 0)
    for obstacle_baseid, number in pairs(self.base.obstacles) do
        local obstacle_base = cc.LookupTable("Obstacle", obstacle_baseid)
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
end

--
-- void tiles_obstacle_NOISE_generator()
--
function Scene:tiles_obstacle_NOISE_generator()
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
                    self:tiles_fill(self:create_obstacle_object(t[noise_value], {x = x, y = y}))
				else
					--cc.WriteLog('noise_value: ' .. tostring(noise_value))
                end
            end
        end
    end
end


--
-- void tiles_obstacle_padding()
--
function Scene:tiles_obstacle_padding()
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
               	self:tiles_fill(self:create_obstacle_object(obstacle_baseid, nil_coord))
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
                   	self:tiles_fill(self:create_obstacle_object(obstacle_baseid, {x = x, y = y}))
                end
            end
        end
    end

	local function tiles_obstacle_padding_tile(self)
		assert(self.tile_obstacle_baseid ~= nil)
        for y = 0, self.base.height - 1 do
            for x = 0, self.base.width - 1 do                
				if self.tiles[y][x] == nil then
				    self:tiles_fill(self:create_obstacle_object(self.tile_obstacle_baseid, {x = x, y = y}))
                end
            end
        end
	end

--    for _, road_coord in pairs(self.roads) do
--        tiles_obstacle_padding_road(self, road_coord)
--    end

	tiles_obstacle_padding_hole(self)
	tiles_obstacle_padding_tile(self)
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
					self:tiles_fill(self:create_obstacle_object(self.tile_obstacle_baseid, coord))
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
-- void Scene:do_accomplish_events()
--
function Scene:do_accomplish_events()
    assert(self.accomplish_events ~= nil)
    for _, event in pairs(self.events) do
        if self.accomplish_events[event.baseid] then
            self:accomplish_event(event)
        end
    end
end

--
--  bool generator()
--
function Scene:generator()    
    local size = self.FLAG_prior_obstacle_size
    local try_times = self.FLAG_generate_try_times
    while try_times > 0 do
        self:tiles_init()
        self:tiles_event_generator()
        self:tiles_obstacle_STICK_generator()
        self:tiles_obstacle_BLOCK_generator(size)
        if self:tiles_link_events() then
            self:tiles_obstacle_NOISE_generator()
            self:tiles_obstacle_padding()
			self:tiles_dump()
            return true
        end
        size = size + 1
        try_times = try_times - 1
		cc.WriteLog('try times: ' .. tostring(try_times) .. ', size: ' .. tostring(size))
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
    -- TODO:
end

--
-- bool import(string)
--
function Scene:import()
    -- TODO:
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

--
-----------------------------------------------------------------------------------------
--

--
-- fullPath, event Scene::move_request(entityid, destx, desty)
--
function Scene:move_request(entityid, destx, desty)
    -- return event or nil
    local function checkEventTrigger(self, src_coord, dest_coord)
        local t = self.tiles[dest_coord.y][dest_coord.x]
        assert(t ~= nil and t.objects ~= nil)

        local eventObject = nil
        for _, object in pairs(t.objects) do
            if object.objectCategory == GridObjectCategory.EVENT and object.accomplish == false then
                if eventObject ~= nil then
                    cc.WriteLog("multiple Events on the same coordinate")
                end
                eventObject = object
            end
        end
        
        if eventObject ~= nil
                -- check Manhattan Distance
                and math.abs(dest_coord.x - src_coord.x) <= self.FLAG_trigger_event_distance
                and math.abs(dest_coord.y - src_coord.y) <= self.FLAG_trigger_event_distance
        then
            return eventObject
        end

        return nil
    end

	-- check if exist event
	local function checkExistEvent(self, src_coord, dest_coord)
        local t = self.tiles[dest_coord.y][dest_coord.x]
        assert(t ~= nil and t.objects ~= nil)
		for _, object in pairs(t.objects) do
			if object.objectCategory == GridObjectCategory.EVENT then return true end
		end
		return false
	end

    -- return nearest coord
    local function findNearestRoad(self, src_coord, dest_coord)
        local t = self.tiles[dest_coord.y][dest_coord.x]
        assert(t ~= nil and t.block ~= nil)        
        if t.block == BlockCategory.NONE then return dest_coord end
	    local min_coord = nil
	    local min_distance = 0
	    for _, coord in pairs(self.roads) do
	        local distance = self:distance(coord, dest_coord)
	        if min_coord == nil or distance < min_distance then
	            min_coord = coord
	            min_distance = distance
	        end
	    end
	    return min_coord
    end
    
    assert(self.members[entityid] ~= nil)
    local src_coord = self.members[entityid].coord
    assert(src_coord ~= nil and self:moveable(src_coord))

    local fullPath = {}
    local dest_coord = {x = destx, y = desty}

    --
    -- check dest_coord is not illegal
    if not self:valid_coord(dest_coord) then
        cc.WriteLog(string.format(">>>>>>>>>>>>> illegal dest: (%d,%d)", destx, desty))
        return fullPath, nil
    end

    --
    -- check event trigger
    self.trigger_event = checkEventTrigger(self, src_coord, dest_coord)
    if self.trigger_event ~= nil then
        cc.WriteLog(string.format("trigger_event: %d,%d", self.trigger_event.id, self.trigger_event.baseid))
        return nil, self.trigger_event
    end

	--
	-- check block
	local t = self.tiles[dest_coord.y][dest_coord.x]
	assert(t ~= nil and t.block ~= nil)
	if t.block ~= BlockCategory.NONE then
		local existEvent = checkExistEvent(self, src_coord, dest_coord)
		if (self.FLAG_adjust_dest_event and existEvent)	or self.FLAG_adjust_dest_obstacle then
    		-- find nearest road point if dest_coord is block
    		dest_coord = findNearestRoad(self, src_coord, dest_coord)
    		if dest_coord == nil then
    			cc.WriteLog(string.format(">>>>>>>>>>>>> unreachable dest: (%d,%d)", destx, desty))
    			return fullPath, nil
    		end
        	-- cc.WriteLog(string.format("adjust old dest: (%d,%d) to new dest: (%d,%d)", destx, desty, dest_coord.x, dest_coord.y))
		end
	end

	-- cc.WriteLog(string.format("findPath from src: (%d,%d) to dest: (%d,%d)", src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))

    -- try to FindPath
    local roads, errorstring = FindPath(
    			src_coord,
    			dest_coord, 
    			1, 
    			math.max(self.base.width, self.base.height), 
	            function(coord) return self:moveable(coord) end
	            )

    if roads == nil then
        cc.WriteLog(string.format(">>>>>>>>>>>>> dest: (%d,%d) FindPath error: %s", dest_coord.x, dest_coord.y, errorstring))
        return fullPath, nil
    end

    -- retrieve roads point by desc order
    local road_size = table.size(roads)
    for _, coord in pairs(roads) do
        fullPath[road_size] = coord
        road_size = road_size - 1
    end

    return fullPath, nil
end


--
-- bool Scene::move(entityid, x, y)
--
function Scene:move(entityid, x, y)
    assert(self.members[entityid] ~= nil)
    local src_coord = self.members[entityid].coord
    assert(src_coord ~= nil and self:moveable(src_coord))
    local dest_coord = {x = x, y = y}
    
    --
    -- check dest_coord is not illegal
    if not self:valid_coord(dest_coord) then
        cc.WriteLog(string.format(">>>>>>>>>>>>> illegal dest: (%d,%d)", destx, desty))
        return false
    end

    --
    -- check dest_coord is moveable
    local t = self.tiles[y][x]
    assert(t ~= nil and t.block ~= nil)
    if t.block ~= BlockCategory.NONE then
        cc.WriteLog(string.format(">>>>>>>>>>>>> dest: (%d,%d) is block: %d", x, y, t.block))
        return false
    end

	--NOTE: check whether distance from src_coord to dest_coord exceeds the allowable range

	self.members[entityid].coord = dest_coord
	--cc.WriteLog(string.format("entityid: %d, move from (%d,%d) to (%d,%d)", entityid, src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))

	return true
end

--
-- bool event_reward(entityid, eventid, reward_index, entry_index)
--
function Scene:event_reward(entityid, eventid, reward_index, entry_index)
    local entity = self.members[entityid]
    assert(entity ~= nil)
    assert(self.trigger_event ~= nil) -- trigger_event must be hold
    assert(self.trigger_event.id == eventid)
    assert(self.trigger_event.accomplish) -- pass this event
    
    assert(not self.trigger_event.reward) -- still not reward
    self.trigger_event.reward = true
    
    cc.WriteLog(string.format("event: %d,%d reward: %d, entry_index: %d", self.trigger_event.id, self.trigger_event.baseid, reward_index, entry_index))
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.WriteLog(string.format("event: %d, %d is not MONSTER category", self.trigger_event.id, self.trigger_event.baseid))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    -- assert(self.trigger_event.content.monster ~= nil)
    if self.trigger_event.content.monster == nil then
        cc.WriteLog(string.format("event: %d, %d is not MONSTER content", self.trigger_event.id, self.trigger_event.baseid))
        return false
    end
    
    local rewards = self.trigger_event.content.monster.rewards
    if rewards == nil then
        cc.WriteLog(string.format("event: %d, %d not rewards", self.trigger_event.id, self.trigger_event.baseid))
        return false
    end
    if rewards[reward_index] == nil then -- entity cancel reward
        cc.WriteLog(string.format("event: %d, %d not reward_index: %d", self.trigger_event.id, self.trigger_event.baseid, reward_index))
        return false
    end
    
    local reward = rewards[reward_index]
    local function reward_cards(self, cards, entry_index)
        if cards[entry_index] == nil then
            cc.WriteLog(string.format("event: %d, %d cards not entry_index: %d", self.trigger_event.id, self.trigger_event.baseid, entry_index))
            return false
        end
        local card_baseid = cards[entry_index]
        local card_base = cc.LookupTable("Card", card_baseid)
        if card_base == nil then
            cc.WriteLog(string.format("event: %d, %d invalid card: %d", self.trigger_event.id, self.trigger_event.baseid, card_baseid))
            return false
        end
        cc.WriteLog(string.format("reward card: %d", card_baseid))
        entity.bag:add_card(card_baseid)
    end
    local function reward_items(self, items, entry_index)
        if items[entry_index] == nil then
            cc.WriteLog(string.format("event: %d, %d items not entry_index: %d", self.trigger_event.id, self.trigger_event.baseid, entry_index))
            return false
        end
        local item_baseid = entry_index
        local item_number = items[entry_index]
        cc.WriteLog(string.format("reward item: %d, number: %d", item_baseid, item_number))
        entity.bag:add_item(item_baseid, item_number)
    end
    if reward.cards ~= nil then
        return reward_cards(self, reward.cards, entry_index)
    elseif reward.items ~= nil then
        return reward_items(self, reward.items, entry_index)
    elseif reward.gold ~= nil then
        cc.WriteLog(string.format("reward gold: %d", reward.gold))
        entity.bag:add_gold(reward.gold)
    else
        cc.WriteLog(string.format("event: %d, %d unavailable reward", self.trigger_event.id, self.trigger_event.baseid))
        return false
    end
    return true
end

