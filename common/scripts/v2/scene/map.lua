
--
------------------- Map class -------------------
--

Map = {
    scene = nil, -- reference to copy instance
    
    id = nil,
    baseid = nil,
    base = nil,
    
    seed = nil,
    random_func = nil,
    
    events_base = nil, -- copy.script_func() -> {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    
    tiles = nil, -- {[y][x] = {objects = {[1]=?, ...}, block = ?} , ... }
    events = nil, -- all of events, {[eventid] = event instance, ...}
    obstacles = nil, -- all of obstacles, {[obstacleid] = obstacle instance, ...}
    roads = nil, -- all of roads, {{x=?,y=?}, ...}
    
    pattern = nil, -- MapPattern
    neighbor = nil, -- { [direction] = map, ... }
    layer = nil,

    constructor = function(self, scene, baseid, seed, entityid, events_base, layer, link_direction)
        self.scene = scene
        self.id = cc.ScriptMapNew(baseid)
        self.baseid = baseid
        self.base = cc.ScriptLookupTable("Map", self.baseid)
        assert(self.base ~= nil)
        assert(self.base.width > 0 and self.base.height > 0)

        self.seed = seed
        self.random_func = NewRandom(self.seed)
        self.events_base = events_base
        assert(self.events_base ~= nil)
        self.tiles = {}
        self.events = {}
		self.obstacles = {}
		self.roads = {}

		self.layer = layer
		self.neighbor = {}

		-- search for available tile obstacle
		self:tile_obstacle_baseid_init()
        assert(self.tile_obstacle_baseid ~= nil and table.size(self.tile_obstacle_baseid) > 0)
        --table.dump(self.tile_obstacle_baseid, 'tile_obstacle_baseid')

		-- search for available wall obstacle
		self:wall_obstacle_baseid_init()
		assert(self.wall_obstacle_baseid ~= nil and table.size(self.wall_obstacle_baseid) > 0)
		--table.dump(self.wall_obstacle_baseid, 'wall_obstacle_baseid')

        -- MapPolicy
        self.FLAG_chessboard = false
        self.FLAG_wave_wall = false        
        local policies = {
            [MapPolicy.CHESSBOARD] = function(self) 
                self.FLAG_chessboard = true
                cc.ScriptDebugLog(string.format("    map: %d, policy: chessboard", self.baseid))
                end,
            [MapPolicy.WAVE_WALL] = function(self)
                self.FLAG_wave_wall = true
                cc.ScriptDebugLog(string.format("    map: %d, policy: wave wall", self.baseid))
                end,
        }        
        if self.base.policy == nil then self.base.policy = {} end
        for _, policy in pairs(self.base.policy) do
            if policies[policy] ~= nil then
               policies[policy](self) 
            else
                cc.ScriptErrorLog(string.format("map: %d, unsupport policy: %d", self.baseid, policy))
            end
        end

        -- MapPattern
        self.pattern = nil
        if self.base.pattern ~= nil and table.size(self.base.pattern) > 0 then 
            self.pattern = table.random(self.base.pattern, table.size(self.base.pattern), self.random_func)
        end
        if self.pattern == nil then self.pattern, _ = self.random_func(MapPattern.I, MapPattern.LRI) end
        assert(self.pattern ~= nil)
        cc.ScriptDebugLog(string.format("    map: %d, pattern: %d", self.baseid, self.pattern))

	    cc.ScriptDebugLog(string.format("++++++++ map: %d,%d,%s, layer: %d, constructor", self.id, self.baseid, self.base.name.cn, self.layer))

        -- generate map
		local rc = self:generator(link_direction) -- init events & obstacles
		assert(rc)
    end,

    destructor = function(self)
	    --cc.ScriptDebugLog(string.format("---------- map: %d,%d,%s, destructor", self.id, self.baseid, self.base.name.cn))
            
        for _, event in pairs(self.events) do event:destructor() end
        table.clear(self.events)
        
        for _, obstacle in pairs(self.obstacles) do obstacle:destructor() end
        table.clear(self.obstacles)
        
        cc.ScriptMapDestroy(self.id) -- ScriptMapDestroy
    end,
    

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
	    		and self.tiles[coord.y][coord.x] ~= nil 
	    		and self.tiles[coord.y][coord.x].block == BlockCategory.NONE
    end,
    
    --
    -- bool passable(coord)
    -- 
    passable = function(self, coord)
        local function passable_objects(self, coord)
            local t = self.tiles[coord.y][coord.x]
            assert(t ~= nil and t.objects ~= nil and t.block ~= nil)
            if t.block == BlockCategory.STATIC then return false end
            for _, object in pairs(t.objects) do
                if object.base.block == BlockCategory.STATIC -- static block
                        -- dynamic event block & this event will never accomplish
                        or (object.base.block == BlockCategory.DYNAMIC and object.base.endtype == EventEnd.NONE) 
                then
                    return false
                end
            end
            return true
        end
		return self:valid_coord(coord)
				and (self.tiles[coord.y][coord.x] == nil 
					or self.tiles[coord.y][coord.x].block == BlockCategory.NONE
					or passable_objects(self, coord))
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
            if object.accomplish and block == BlockCategory.DYNAMIC then block = BlockCategory.NONE end
            -- STATIC:2, DYNAMIC:1, NONE:0
            if block > t.block then t.block = block end
        end
        -- cc.ScriptErrorLog(string.format("resetblock: (%d,%d) to block: %d", coord.x, coord.y, t.block))
    end,

    --
    -- gate obstacle baseid: { obstacle_baseid, ... }
    --
    gate_obstacle_baseid = nil,
    gate_obstacle_baseid_init = function(self)
		self.gate_obstacle_baseid = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.ScriptLookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.GATE then 
                table.insert(self.gate_obstacle_baseid, obstacle_baseid)
            end
        end
    end,
    gate_obstacle_baseid_random = function(self)
        local t_size = table.size(self.gate_obstacle_baseid)
        if t_size == 0 then return nil end
        local _, obstacle_baseid = table.random(self.gate_obstacle_baseid, t_size, self.random_func)
        return obstacle_baseid
    end,
    gate_obstacle_baseid_retrieve = function(self)
        return self:gate_obstacle_baseid_random()
    end,

    
    --
    -- wall obstacle baseid: { obstacle_baseid, ... }
    --
    wall_obstacle_baseid = nil,
    wall_obstacle_baseid_init = function(self)
		self.wall_obstacle_baseid = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.ScriptLookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.WALL 
                    or obstacle_base.category == ObstacleCategory.HIGHWALL
                    or obstacle_base.category == ObstacleCategory.CORNER then
                table.insert(self.wall_obstacle_baseid, obstacle_baseid)
            end
        end
    end,
    wall_obstacle_baseid_random = function(self)
        local t_size = table.size(self.wall_obstacle_baseid)
        if t_size == 0 then return nil end
        local _, obstacle_baseid = table.random(self.wall_obstacle_baseid, t_size, self.random_func)
        return obstacle_baseid
    end,
    wall_obstacle_baseid_wave_wall = function(self, coord)
        local t_size = table.size(self.wall_obstacle_baseid)
        if t_size == 0 then return nil end
        local value = coord.x % t_size
        value = value + (coord.y % 2)
        value = (value % t_size) + 1
        assert(self.wall_obstacle_baseid[value] ~= nil, string.format("coord:(%d,%d), t_size:%d, value:%d", coord.x, coord.y, t_size, value))
        return self.wall_obstacle_baseid[value]
    end,
    wall_obstacle_baseid_retrieve = function(self, coord)
        if self.FLAG_wave_wall then
            return self:wall_obstacle_baseid_wave_wall(coord)
        else
            return self:wall_obstacle_baseid_random()
        end
    end,
    
    
    --
    -- tile obstacle baseid : { obstacle_baseid, ... }
    --
    tile_obstacle_baseid = nil,
    tile_obstacle_baseid_init = function(self)
		self.tile_obstacle_baseid = {}
        for obstacle_baseid, _ in pairs(self.base.obstacles) do
            local obstacle_base = cc.ScriptLookupTable("Obstacle", obstacle_baseid)
            assert(obstacle_base.width > 0 and obstacle_base.height > 0)
            if obstacle_base.category == ObstacleCategory.TILE and obstacle_base.block == BlockCategory.NONE then
                table.insert(self.tile_obstacle_baseid, obstacle_baseid)
            end
        end
    end,
    tile_obstacle_baseid_random = function(self)
        local t_size = table.size(self.tile_obstacle_baseid)
        if t_size == 0 then return nil end
        local _, obstacle_baseid = table.random(self.tile_obstacle_baseid, t_size, self.random_func)
        return obstacle_baseid
    end,
    tile_obstacle_baseid_chessboard = function(self, coord)
        local t_size = table.size(self.tile_obstacle_baseid)
        if t_size == 0 then return nil end
        local value = coord.x % t_size
        value = value + (coord.y % 2)
        value = (value % t_size) + 1
        assert(self.tile_obstacle_baseid[value] ~= nil, string.format("coord:(%d,%d), t_size:%d, value:%d", coord.x, coord.y, t_size, value))
        return self.tile_obstacle_baseid[value]
    end,
    tile_obstacle_baseid_retrieve = function(self, coord)
        if self.FLAG_chessboard then
            return self:tile_obstacle_baseid_chessboard(coord)
        else
            return self:tile_obstacle_baseid_random()
        end
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

    -- MapPolicy
    FLAG_chessboard = false,
    FLAG_wave_wall = false,

    -- 
	FLAG_road_width = 3,
	FLAG_walk_wall_width = 1,
	FLAG_area_width = 5,
	FLAG_area_height = 5,
	FLAG_pattern_width = 15,
	FLAG_pattern_height = 15,
    
    --
    -- unsigned int distance(from, to)
    --
    distance = function(self, from, to)
        assert(self:valid_coord(from) and self:valid_coord(to))
        return ManhattanDistance(from, to)
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
    end
}

function Map:new(scene, baseid, seed, entityid, events_base, layer, link_direction)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(scene, baseid, seed, entityid, events_base, layer, link_direction)
	return object
end


--
-----------------------------------------------------------------------------------------
--


--
-- bool Scene::move_request(entityid, destx, desty)
--
function Map:move_request(entityid, destx, desty)
    local function getUncompleteEvent(self, dest_coord)
        local t = self.tiles[dest_coord.y][dest_coord.x]
        assert(t ~= nil and t.objects ~= nil)
        local eventObject = nil
        for _, object in pairs(t.objects) do
            if object.objectCategory == GridObjectCategory.EVENT and object.accomplish == false then
                return object
            end
        end
    end

    local function getEventCoords(self, event)
        local coords = {}
        for y = 0, event.base.height - 1 do
            for x = 0, event.base.width - 1 do
                table.insert(coords, {x = x + event.coord.x, y = y + event.coord.y})
            end
        end
        return coords
    end
    
    -- return event or nil
    local function checkEventTrigger(self, src_coord, dest_coord)
        local eventObject = getUncompleteEvent(self, dest_coord)
        if eventObject == nil then return nil end -- no event on coordinate

        local coords = getEventCoords(self, eventObject)
        for _, coord in pairs(coords) do
            -- check Manhattan Distance
			if ManhattanDistance(src_coord, coord) <= self.FLAG_trigger_event_distance then
                return eventObject
            end
        end

        return nil
    end

    --
	-- event checkExistEvent(dest_coord)
	local function checkExistEvent(self, dest_coord)
        local t = self.tiles[dest_coord.y][dest_coord.x]
        assert(t ~= nil and t.objects ~= nil)
		for _, object in pairs(t.objects) do
			if object.objectCategory == GridObjectCategory.EVENT then return object end
		end
		return nil
	end

    --
    -- looking around for object
    -- coord explore_around_object(src_coord, object)
    --
    local function explore_around_object(self, src_coord, object)
        if self:moveable(object.coord) then return object.coord end

        local function explore_object_coordinate(self, src_coord, dest_coord, coords)
        	local adjust = {
        		{  x =  1,  y = 0 },  {  x =  0,  y =  1 },
        		{  x =  0,  y = -1 }, {  x = -1,  y = 0 }
        	}

        	for _, adjust_coord in pairs(adjust) do
        	    local dest = {
        	        x = dest_coord.x + adjust_coord.x, y = dest_coord.y + adjust_coord.y
        	    }
        	    if self:valid_coord(dest) and self:moveable(dest) then
                    local roads, errorstring = FindPath(
                        			src_coord,
                        			dest, 
                        			1, 
                        			math.max(self.base.width, self.base.height), 
                    	            function(coord) return self:moveable(coord) end
                	            )
                    if roads ~= nil then table.insert(coords, dest) end
        	    end
        	end
        end
        
        local coords = {}
        
        explore_object_coordinate(self, src_coord, object.coord, coords)
        if object.base.width > 1 then
            explore_object_coordinate(self, src_coord, {x = object.coord.x + object.base.width - 1, y = object.coord.y}, coords)
        end
        if object.base.height > 1 then
            explore_object_coordinate(self, src_coord, {x = object.coord.x, y = object.coord.y + object.base.height - 1}, coords)
        end
        explore_object_coordinate(self, src_coord, {x = object.coord.x + object.base.width - 1, y = object.coord.y + object.base.height - 1}, coords)

        if table.size(coords) == 0 then return nil end

        local min_coord = nil
        local min_distance = nil
        for _, coord in pairs(coords) do
            local distance = self:distance(src_coord, coord)
            if min_distance == nil or distance < min_distance then
                min_coord = coord
                min_distance = distance
            end 
        end 

        return min_coord
    end
        
    if self.members[entityid] == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    local src_coord = self.members[entityid].coord
    if not self:valid_coord(src_coord) or not self:moveable(src_coord) then
        cc.ScriptErrorLog(string.format(">>>>>>> illegal src: (%d,%d), func: %s", src_coord.x, src_coord.y, Function()))
        return false
    end

    local dest_coord = {x = destx, y = desty}
    if not self:valid_coord(dest_coord) then
        cc.ScriptErrorLog(string.format(">>>>>>> illegal dest: (%d,%d), func: %s", destx, desty, Function()))
        return false
    end

    --
    -- check event trigger
    self.trigger_event = checkEventTrigger(self, src_coord, dest_coord)
    if self.trigger_event ~= nil then
        cc.ScriptDebugLog(string.format("------------------ trigger_event: %d,%d,%s, coord:(%d,%d)", self.trigger_event.id, self.trigger_event.baseid, self.trigger_event.base.name.cn, self.trigger_event.coord.x, self.trigger_event.coord.y))
        return self:dispatch_event(entityid, self.trigger_event, false)
    end

	--
	-- adjust dest_coord
	local t = self.tiles[dest_coord.y][dest_coord.x]
	assert(t ~= nil and t.block ~= nil)
	if t.block ~= BlockCategory.NONE then
		local existEvent = checkExistEvent(self, dest_coord)		
--		if (self.FLAG_adjust_dest_event and existEvent ~= nil) or self.FLAG_adjust_dest_obstacle then
        if self.FLAG_adjust_dest_event and existEvent ~= nil then
    		dest_coord = explore_around_object(self, src_coord, existEvent)
    		if dest_coord == nil then
    			cc.ScriptErrorLog(string.format(">>>>>>> unreachable dest: (%d,%d)", destx, desty))
    			return false
    		end
        	cc.ScriptDebugLog(string.format("adjust old dest: (%d,%d) to new dest: (%d,%d)", destx, desty, dest_coord.x, dest_coord.y))
		end
	end

--	cc.ScriptDebugLog(string.format("findPath from src: (%d,%d) to dest: (%d,%d)", src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))

	if src_coord.x == dest_coord.x and src_coord.y == dest_coord.y then
		cc.ScriptErrorLog(string.format(">>>>>>>>> src_coord is the same with dest_coord"))
		return true
	end

    -- try to FindPath
    local roads, errorstring = FindPath(
        			src_coord,
        			dest_coord, 
        			1, 
        			math.max(self.base.width, self.base.height), 
    	            function(coord) return self:moveable(coord) end
	            )

    if roads == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> dest: (%d,%d) FindPath error: %s, src: (%d,%d), func: %s", dest_coord.x, dest_coord.y, errorstring, src_coord.x, src_coord.y, Function()))
        return false
    end

    local fullPath = {}
    
    local road_size = table.size(roads)
    for _, coord in pairs(roads) do
        fullPath[road_size] = coord
        road_size = road_size - 1
    end

    -- fullPath: { {x=?, y=?}, ... }
    if table.size(fullPath) == 0 then 
        cc.ScriptErrorLog(string.format(">>>>>> fullPath is empty, func: %s", Function()))
        return false 
    end

--    table.dump(fullPath, 'fullPath')
    cc.ScriptTriggerMove(entityid, fullPath)
    
    return true
end


--
-- bool Map::move(entityid, x, y)
--
function Map:move(entityid, x, y)
    if self.members[entityid] == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    local src_coord = self.members[entityid].coord
    assert(src_coord ~= nil and self:moveable(src_coord))
    local dest_coord = {x = x, y = y}
    
    --
    -- check dest_coord is not illegal
    if not self:valid_coord(dest_coord) then
        cc.ScriptErrorLog(string.format(">>>>>>> illegal dest: (%d,%d), func: %s", destx, desty, Function()))
        return false
    end

    --
    -- check dest_coord is moveable
    local t = self.tiles[y][x]
    assert(t ~= nil and t.block ~= nil)
    if t.block ~= BlockCategory.NONE then
        cc.ScriptErrorLog(string.format(">>>>>>> dest: (%d,%d) is block: %d, func: %s", x, y, t.block, Function()))
        return false
    end

	--NOTE: check whether distance from src_coord to dest_coord exceeds the allowable range

	self.members[entityid].coord = dest_coord
	-- cc.ScriptErrorLog(string.format("entityid: %d, move from (%d,%d) to (%d,%d)", entityid, src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))

	return true
end

