
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
    
    tiles = nil, -- {[y][x] = {objects = {[1]=?, ...}, block = ?} , ... }
    events = nil, -- all of events, {[eventid] = event instance, ...}
    obstacles = nil, -- all of obstacles, {[obstacleid] = obstacle instance, ...}
    roads = nil, -- all of roads, {{x=?,y=?}, ...}


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
		self:do_accomplish_events(accomplish_events) -- accomplish event in last EnterScene

        -- init all of events to content
		-- self:init_events_content(entityid)
    end,

    destructor = function(self)
        for _, entity in pairs(self.members) do
            self:remove_member(entity)
        end
        
        for eventid, _ in pairs(self.events) do
            cc.EventDestroy(eventid)
        end
        for obstacleid, _ in pairs(self.obstacles) do
            cc.ObstacleDestroy(obstacleid)
        end
        cc.MapDestroy(self.id) -- MapDestroy
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
        -- cc.WriteLog(string.format("resetblock: (%d,%d) to block: %d", coord.x, coord.y, t.block))
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
        -- assert(self.members[entity.id] ~= nil)
        if self.members[entity.id] ~= nil then
            self.members[entity.id] = nil
            entity.coord = nil
            if self.match ~= nil then
                self:abort_match(entity.id)
            end
            cc.ExitMap(entity.id, self.id)
            cc.WriteLog(string.format("entity: %d exitScene: %d", entity.id, self.id))
            entity:exit_scene(self)
        end
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
            while number > 0 do
                number = number - 1
                self.match:add_monster(entity_baseid, Side.ENEMY)
            end
        end
        return self.match:start()
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
            cc.WriteLog(string.format("allies is defeated"))
            for _, entity in pairs(self.members) do
                self:remove_member(entity)
            end
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
    return table.size(self.members) == 0
end

--
-- void update(delta)
--
function Scene:update(delta)
    if self.match ~= nil then 
		if self.match:checkDone() then
			cc.WriteLog(string.format("match isDone, scene: %d, %d", self.id, self.baseid))    
			self:end_match()
			cc.WriteLog(string.format("match close, scene: %d, %d", self.id, self.baseid))    
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

--
-- void Scene:do_accomplish_events(accomplish_events)
--
function Scene:do_accomplish_events(accomplish_events)
	if accomplish_events == nil then return end
    local function is_accomplish_event(event)
        for _, event_baseid in pairs(accomplish_events) do
            if event.baseid == event_baseid then return true end
        end
        return false
    end
    for _, event in pairs(self.events) do
        if is_accomplish_event(event) then
            self:accomplish_event(event)
        end
    end
end



--
-- table take_event_content(entityid, event)
--
function Scene:take_event_content(entityid, event)
    assert(event.script_func ~= nil and type(event.script_func) == "function")
    assert(not event.accomplish)
    assert(not event.reward)
    if event.content == nil then
        event.content = event.script_func(
                            entityid, 
                            self.baseid, 
                            self.copy:current_layer(), 
                            event.baseid, 
                            self.copy:current_seed()
                        )
        assert(event.content ~= nil)
    end
	table.dump(event.content, 'event.content')
	local t = {
		[EventCategory.MONSTER] = function(self, event) 
		    assert(event.content.monster ~= nil)
		    return event.content.monster
		end,
		
		[EventCategory.SHOP_BUY_CARD] = function(self, event)
            -- event.content.shop: { cards = {card_baseid, ...}, items = {item_baseid, ...} 
            -- trigger: { cards = {card_baseid = price_gold, ...}, items = {item_baseid = price_gold, ...} 
            assert(event.content.shop ~= nil)
            local list = {}
            if event.content.shop.cards ~= nil then
                list.cards = {}
                for _, t in pairs(event.content.shop.cards) do
                    local card_base = cc.LookupTable("Card", t.card_baseid)
                    list.cards[t.card_baseid] = card_base.price_gold
                    -- TODO: discount
                end
            end
            if event.content.shop.items ~= nil then
                list.items = {}
                for _, t in pairs(event.content.shop.items) do
                    --local card_base = cc.LookupTable("Card", t.card_baseid)
                    --list.cards[t.card_baseid] = card_base.price_gold
                    -- TODO: discount
                end
            end
            return list
		end,
		
		[EventCategory.STORY] = function(self, event)
    		-- event.content.story: { title = ?, text = ?, options = { [1] = ?, [2] = ?, ... } }
            -- trigger: { title = ?, text = ?, options = { [1] = ?, [2] = ?, ... } }
            assert(event.content.story ~= nil)
		    return event.content.story
		end,
		
		[EventCategory.SHOP_DESTROY_CARD] = function(self, event)
            -- event.content.destroy_card: { gold = ? }
            -- trigger: { gold = ? }
            assert(event.content.destroy_card ~= nil)
		    return event.content.destroy_card
		end,

		[EventCategory.EXIT] = function(self, event)
		    return event.content
		end
	}
	assert(t[event.base.category] ~= nil)
	return t[event.base.category](self, event)
end

--[[
function Scene:init_events_content(entityid)
    --
    -- call all of events script_func, hold by event.content
    --
    for _, event in pairs(self.events) do
        assert(event.content == nil)
        event.content = event.script_func(
                            entityid, 
                            self.copy.baseid, 
                            self.copy:current_layer(), 
                            event.baseid, 
                            self.copy:current_seed()
                        )
        assert(event.content ~= nil)
    end
end
--]]



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

    --
    -- looking around
    --
    local function findNearestRoad(self, src_coord, dest_coord)
        if self:moveable(dest_coord) then return dest_coord end
        
        local coords = {}
    	local adjust = {
    		{  x =  1,  y = 0 },  {  x =  0,  y =  1 },
    		{  x =  1,  y = 1 },  {  x =  0,  y = -1 },
    		{  x = -1,  y = 0 },  {  x = -1,  y = -1 },
    		{  x = -1,  y = 1 },  {  x =  1,  y = -1 }
    	}
    	
    	for _, adjust_coord in pairs(adjust) do
    	    local dest = {
    	        x = dest_coord.x + adjust_coord.x, y = dest_coord.y + adjust_coord.y
    	    }
    	    if self:valid_coord(dest) and self:moveable(dest) then
                -- try to FindPath
                local roads, errorstring = FindPath(
                    			src_coord,
                    			dest, 
                    			1, 
                    			math.max(self.base.width, self.base.height), 
                	            function(coord) return self:moveable(coord) end
            	            )
                if roads ~= nil then return dest end
    	    end
    	end

    	return nil
    end
    
    if self.members[entityid] == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return nil, nil
    end
    
    local src_coord = self.members[entityid].coord
    assert(src_coord ~= nil and self:moveable(src_coord))

    local fullPath = {}
    local dest_coord = {x = destx, y = desty}

    --
    -- check dest_coord is not illegal
    if not self:valid_coord(dest_coord) then
        cc.WriteLog(string.format(">>>>>>> illegal dest: (%d,%d), func: %s", destx, desty, Function()))
        return fullPath, nil
    end

    --
    -- check event trigger
    self.trigger_event = checkEventTrigger(self, src_coord, dest_coord)
    if self.trigger_event ~= nil then
        cc.WriteLog(string.format("trigger_event: %d,%d, coord:(%d,%d)", self.trigger_event.id, self.trigger_event.baseid, self.trigger_event.coord.x, self.trigger_event.coord.y))
        return nil, self.trigger_event
    end

	--
	-- check block
	local t = self.tiles[dest_coord.y][dest_coord.x]
	assert(t ~= nil and t.block ~= nil)
	if t.block ~= BlockCategory.NONE then
		local existEvent = checkExistEvent(self, src_coord, dest_coord)
		if (self.FLAG_adjust_dest_event and existEvent)	or self.FLAG_adjust_dest_obstacle then
    		dest_coord = findNearestRoad(self, src_coord, dest_coord)
    		if dest_coord == nil then
    			cc.WriteLog(string.format(">>>>>>> unreachable dest: (%d,%d)", destx, desty))
    			return fullPath, nil
    		end
        	cc.WriteLog(string.format("adjust old dest: (%d,%d) to new dest: (%d,%d)", destx, desty, dest_coord.x, dest_coord.y))
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
        cc.WriteLog(string.format(">>>>>>> dest: (%d,%d) FindPath error: %s, func: %s", dest_coord.x, dest_coord.y, errorstring, Function()))
        return fullPath, nil
    end

    -- table.dump(roads, "roads")
    
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
    if self.members[entityid] == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    local src_coord = self.members[entityid].coord
    assert(src_coord ~= nil and self:moveable(src_coord))
    local dest_coord = {x = x, y = y}
    
    --
    -- check dest_coord is not illegal
    if not self:valid_coord(dest_coord) then
        cc.WriteLog(string.format(">>>>>>> illegal dest: (%d,%d), func: %s", destx, desty, Function()))
        return false
    end

    --
    -- check dest_coord is moveable
    local t = self.tiles[y][x]
    assert(t ~= nil and t.block ~= nil)
    if t.block ~= BlockCategory.NONE then
        cc.WriteLog(string.format(">>>>>>> dest: (%d,%d) is block: %d, func: %s", x, y, t.block, Function()))
        return false
    end

	--NOTE: check whether distance from src_coord to dest_coord exceeds the allowable range

	self.members[entityid].coord = dest_coord
	-- cc.WriteLog(string.format("entityid: %d, move from (%d,%d) to (%d,%d)", entityid, src_coord.x, src_coord.y, dest_coord.x, dest_coord.y))

	return true
end

--
-- bool event_reward(entityid, eventid, reward_index, entry_index)
--
function Scene:event_reward(entityid, eventid, reward_index, entry_index)
    local entity = self.members[entityid]
    if entity == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.WriteLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.WriteLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if not self.trigger_event.accomplish then -- already passed this event
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s not accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.reward then -- still not reward
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s already reward, func: %s", tostring(eventid), Function()))
        return false
    end
            
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.WriteLog(string.format(">>>>>>> event is not MONSTER category, func: %s", Function()))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.monster ~= nil)

    self.trigger_event.reward = true
    
    return entity:event_reward(self.trigger_event.content.monster, reward_index, entry_index)
end

--
-- bool purchase_card(entityid, sceneid, eventid, card_baseid)
--
function Scene:purchase_card(entityid, sceneid, eventid, card_baseid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.WriteLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.WriteLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if not self.trigger_event.accomplish then -- already passed this event
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s not accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.reward then -- still not reward
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s already reward, func: %s", tostring(eventid), Function()))
        return false
    end
            
    if self.trigger_event.base.category ~= EventCategory.SHOP_BUY_CARD then
        cc.WriteLog(string.format(">>>>>>> event is not SHOP_BUY_CARD category, func: %s", Function()))
        return false
    end

    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.shop ~= nil)

    self.trigger_event.reward = true

    return entity:purchase_card(card_baseid, self.trigger_event.content.shop)
end

--
-- bool purchase_item(entityid, sceneid, eventid, item_baseid)
--
function Scene:purchase_item(entityid, sceneid, eventid, item_baseid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.WriteLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.WriteLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if not self.trigger_event.accomplish then -- already passed this event
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s not accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.reward then -- still not reward
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s already reward, func: %s", tostring(eventid), Function()))
        return false
    end
            
    if self.trigger_event.base.category ~= EventCategory.SHOP_BUY_CARD then
        cc.WriteLog(string.format(">>>>>>> event is not SHOP_BUY_CARD category, func: %s", Function()))
        return false
    end

    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.shop ~= nil)

    self.trigger_event.reward = true

    return entity:purchase_item(item_baseid, self.trigger_event.content.shop)
end


--
-- bool destroy_card(entityid, sceneid, eventid, cardid)
--
function Scene:destroy_card(entityid, sceneid, eventid, cardid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.WriteLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.WriteLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if not self.trigger_event.accomplish then -- already passed this event
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s not accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.reward then -- still not reward
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s already reward, func: %s", tostring(eventid), Function()))
        return false
    end
            
    if self.trigger_event.base.category ~= EventCategory.SHOP_DESTROY_CARD then
        cc.WriteLog(string.format(">>>>>>> event is not SHOP_DESTROY_CARD category, func: %s", Function()))
        return false
    end

    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.destroy_card ~= nil)

    self.trigger_event.reward = true
    
    return entity:destroy_card(cardid, self.trigger_event.content.destroy_card)
end


--
-- bool levelup_card(entityid, sceneid, eventid, cardid)
--
function Scene:levelup_card(entityid, sceneid, eventid, cardid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.WriteLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.WriteLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.WriteLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if not self.trigger_event.accomplish then -- already passed this event
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s not accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.reward then -- still not reward
        cc.WriteLog(string.format(">>>>>>> self.trigger_event: %s already reward, func: %s", tostring(eventid), Function()))
        return false
    end

    if self.trigger_event.base.category ~= EventCategory.SHOP_LEVELUP_CARD then
        cc.WriteLog(string.format(">>>>>>> event is not SHOP_LEVELUP_CARD category, func: %s", Function()))
        return false
    end

    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.levelup_card ~= nil)
    
    self.trigger_event.reward = true

    return entity:levelup_card(cardid, self.trigger_event.content.levelup_card)
end

