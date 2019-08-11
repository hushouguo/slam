
--
------------------- Scene class -------------------
--

Scene = {
    copy = nil, -- reference to copy instance
        
    seed = nil,
    random_func = nil,

    members = nil, -- {[entityid] = entity, ...}
    trigger_event = nil,
    match = nil,

    entrymap = nil,
    maps = nil,


	FLAG_map_width = 15,
	FLAG_map_height = 15,	
	FLAG_entry_locate_x = 0,
	FLAG_entry_locate_y = 0,

    
    constructor = function(self, copy, seed, entityid)
        self.copy = copy
        self.seed = seed
        self.random_func = NewRandom(self.seed)

		self.members = {}
		self.trigger_event = nil
		self.match = nil
		
        self.entrymap = nil
        self.maps = nil

        -- generate scene
		local rc = self:generator(entityid) -- init events & obstacles
		assert(rc)
    end,

    destructor = function(self)
        for _, entity in pairs(self.members) do
            self:remove_member(entity)
        end
    end,

	--
	-- map locate_coord(coord)
	locate_coord = function(self, coord)
		return self.maps[coord.y] == nil and nil or self.maps[coord.y][coord.x]
	end,

    --
    -- bool valid_coord(coord)
    valid_coord = function(self, coord)
    	return self:locate_coord(coord) ~= nil
    end,

    --
    -- bool moveable(coord)
    moveable = function(self, coord)
    	local map = self:locate_coord(coord)
    	return map == nil and false or map:moveable(coord)
    end,
    
    --
    -- bool passable(coord)
    passable = function(self, coord)
    	local map = self:locate_coord(coord)
    	return map == nil and false or map:passable(coord)
    end,

    --
    -- members hashmap & [add|remove]_member methods
    add_member = function(self, entity)
        assert(entity.copy.id == self.copy.id)
        entity.coord = self:tiles_entry_coord()
        assert(entity.coord ~= nil)
        assert(self:moveable(entity.coord))
        assert(self.members[entity.id] == nil)
    	self.members[entity.id] = entity
        cc.ScriptEnterMap(entity.id, self.id, self:tiles_layouts(), entity.coord)
        cc.ScriptDebugLog(string.format("entity: %d enterScene: %d, coord: (%d, %d)", 
			entity.id, self.id, entity.coord.x, entity.coord.y)
			)
        entity:enter_scene(self)
    end,
    remove_member = function(self, entity)
        assert(entity.copy.id == self.copy.id)
        if self.members[entity.id] ~= nil then
            self.members[entity.id] = nil
            entity.coord = nil
            if self.match ~= nil then
                self:abort_match(entity.id)
            end
            cc.ScriptExitMap(entity.id, self.id)
            cc.ScriptDebugLog(string.format("entity: %d exitScene: %d", entity.id, self.id))
            entity:exit_scene(self)
        end
    end,

}

function Scene:new(copy, baseid, seed, entityid, events_base, coord_base)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy, baseid, seed, entityid, events_base, coord_base)
	return object
end

function Scene:checkDone()
    return table.size(self.members) == 0
end

function Scene:update(delta)
    if self.match ~= nil then 
		if self.match:checkDone() then
			cc.ScriptDebugLog(string.format("match isDone"))    
			self:end_match()
			cc.ScriptDebugLog(string.format("match close"))    
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
    cc.ScriptDebugLog(string.format("------------------ accomplish event: %d, %d", event.id, event.baseid))    
    for _, entity in pairs(self.members) do
        entity:accomplish_event(event)
    end
    self:tiles_dump()
	collectgarbage("collect")
end

--
-- void Scene:do_accomplish_events(accomplish_events)
--
function Scene:do_accomplish_events(events_accomplish)
	if events_accomplish == nil then return end -- no any accomplish events
    local function is_accomplish_event(event)
        for _, event_baseid in pairs(events_accomplish) do
            if event.baseid == event_baseid then return true end
        end
        return false
    end
    for _, event in pairs(self.events) do
        if is_accomplish_event(event) then
			event.trigger = true -- indicate this event already trigger at least once
			event.touch = true -- indicate this event already be touched
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
    if event.content == nil then
        event.content = event.script_func(
                            entityid, 
                            self.copy.baseid, 
                            self.copy:current_layer(), 
                            event.baseid, 
                            self.copy:current_seed()
                        )
        assert(event.content ~= nil)
    end
	table.dump(event.content, 'event.content')
	local t = {
		[EventCategory.ENTRY] = function(self, event)
		    -- TODO: entry event trigger
		end,
	
		[EventCategory.MONSTER] = function(self, event) 
		    -- trigger: { entity_baseid = number, ... }
		    assert(event.content.monster ~= nil)
		    return event.content.monster
		end,

		[EventCategory.REWARD] = function(self, event)
		    -- trigger: {
		    --              {
		    --                  cards = { card_baseid, ...}, 
		    --                  items = { {item_baseid = number}, ...}, 
		    --                  gold = ?, 
		    --                  puppets = { entity_baseid, ... }
		    --              }, ...
		    -- }
		    assert(event.content.reward ~= nil)
		    return event.content.reward
		end,
		
		[EventCategory.SHOP_BUY_CARD] = function(self, event)
            -- event.content.shop: { cards = { {card_baseid = ?, ...} }, items = { {item_baseid = ?, ...} }
            -- trigger: { 
            --      cards = { 
            --          { card_baseid = price_gold },
            --          ...
            --      }, 
            --      items = { 
            --          { item_baseid = price_gold },
            --          ...
            --      }
            -- }
            assert(event.content.shop ~= nil)
            local list = {}
            if event.content.shop.cards ~= nil then
                list.cards = {}
                for _, t in pairs(event.content.shop.cards) do
                    local card_base = cc.ScriptLookupTable("Card", t.card_baseid)
                    local price_gold = card_base.price_gold
					if self.members[entityid].pack:card_find(t.card_baseid) ~= nil then
						price_gold = -1
					end
-- TODO: sold out card
--                    local rc = record.copy_events_sold_cards(self.members[entityid].record, self.copy.baseid, self.copy:current_layer(), event.baseid, t.card_baseid)
--                    if rc then
--                        price_gold = -1
--                        cc.ScriptErrorLog(string.format("entityid: %d, event: %d, card: %d, sold out", entityid, event.baseid, t.card_baseid))
--                    else
                        -- TODO: discount
--                    end
                    table.insert(list.cards, {[t.card_baseid] = price_gold})
                end
            end
            if event.content.shop.items ~= nil then
                list.items = {}
                for _, t in pairs(event.content.shop.items) do
                    local item_base = cc.ScriptLookupTable("Item", t.item_baseid)
                    local price_gold = item_base.price_gold
-- TODO: sold out item
--                    local rc = record.copy_events_sold_items(self.members[entityid].record, self.copy.baseid, self.copy:current_layer(), event.baseid, t.item_baseid)
--                    if rc then
--                        price_gold = -1
--                        cc.ScriptErrorLog(string.format("entityid: %d, event: %d, item: %d, sold out", entityid, event.baseid, t.item_baseid))
--                    else
                        -- TODO: discount
--                    end       
                    table.insert(list.items, {[t.item_baseid] = price_gold})
                end
            end
            return list
		end,
		
		[EventCategory.SHOP_DESTROY_CARD] = function(self, event)
            -- trigger: { price_gold = ? }
            assert(event.content.destroy_card ~= nil)
		    return event.content.destroy_card
		end,

		[EventCategory.SHOP_LEVELUP_CARD] = function(self, event)
            -- trigger: { price_gold = ? }
            assert(event.content.levelup_card ~= nil)
		    return event.content.levelup_card
		end,

		[EventCategory.SHOP_LEVELUP_PUPPET] = function(self, event)
            -- trigger: { price_gold = ? }
            assert(event.content.levelup_puppet ~= nil)
		    return event.content.levelup_puppet
		end,

		[EventCategory.STORY] = function(self, event)
            -- trigger: storyid
            assert(event.content.story ~= nil)
		    return event.content.story
		end,
		
		[EventCategory.STORY_OPTION] = function(self, event)
		    -- trigger: storyoptionid
			assert(event.content.storyoption ~= nil)
			return event.content.storyoption
		end,

		[EventCategory.EXIT] = function(self, event)
		    return event.content
		end
	}
	assert(t[event.base.category] ~= nil)
	return t[event.base.category](self, event)
end


--
-----------------------------------------------------------------------------------------
--

--
-- bool dispatch_event(entityid, event, is_update)
--
function Scene:dispatch_event(entityid, event, is_update)
    assert(not event.accomplish)
    cc.ScriptDebugLog(string.format("eventTrigger: %d, category: %d, is_update: %s, func: %s", event.baseid, event.base.category, tostring(is_update), Function()))
    local eventDispatch = {
    	[EventCategory.ENTRY] = function(entityid, event, content, is_update)
            -- assert(content ~= nil)
            -- table.dump(content, "event.Entry")
    	    end,

        [EventCategory.MONSTER] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.Monster")
            cc.ScriptTriggerEventMonster(entityid, event.id, content, is_update)
            end,
            
        [EventCategory.REWARD] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.Reward")
            cc.ScriptTriggerEventReward(entityid, event.id, content, is_update)
            end,

        [EventCategory.SHOP_BUY_CARD] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.BuyCard")
            cc.ScriptTriggerEventShop(entityid, event.id, content, is_update)
            end,

        [EventCategory.SHOP_DESTROY_CARD] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.DestroyCard")
            cc.ScriptTriggerEventDestroyCard(entityid, event.id, content, is_update)
            end,

    	[EventCategory.SHOP_LEVELUP_CARD] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.LevelupCard")
            cc.ScriptTriggerEventLevelupCard(entityid, event.id, content, is_update)
    		end,

        [EventCategory.SHOP_LEVELUP_PUPPET] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.LevelupPuppet")
            cc.ScriptTriggerEventLevelupPuppet(entityid, event.id, content, is_update)
            end,
        
        [EventCategory.STORY] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.Story")
            cc.ScriptTriggerEventStory(entityid, event.id, content, is_update)
            end,

    	[EventCategory.STORY_OPTION] = function(entityid, event, content, is_update)
            assert(content ~= nil)
            table.dump(content, "event.StoryOption")
            cc.ScriptTriggerEventOption(entityid, event.id, content, is_update)
    		end,

        -- trigger event to exit map
        [EventCategory.EXIT] = function(entityid, event, content, is_update)
            self.copy.enter_next_layer = true
            cc.ScriptErrorLog(string.format("entity: %d enter next layer scene", entityid))
            end,
    }
    
    if eventDispatch[event.base.category] == nil then
        cc.ScriptErrorLog(string.format(">>>>>> entityid: %d, event: %d,%d, category: %d unhandled, func: %s", entityid, event.id, event.baseid, event.base.category, Function()))
        return false
    end
    
    event.trigger = true -- indicate this event already trigger at least once
    if event.base.category == EventCategory.SHOP_BUY_CARD then -- Shop can be update data
        eventDispatch[event.base.category](entityid, event, self:take_event_content(entityid, event), is_update)
	elseif event.base.category == EventCategory.SHOP_BUY_CARD then -- Reward can be update data
        eventDispatch[event.base.category](entityid, event, self:take_event_content(entityid, event), is_update)
    else
		-- for other events, force to not update
        eventDispatch[event.base.category](entityid, event, self:take_event_content(entityid, event), false) 
    end

    if not is_update then -- 
        if event.base.endtype == EventEnd.TRIGGER then
            self:accomplish_event(event) -- accomplish event when trigger event
        end
    end
    
    return true
end

--
-- bool Scene::move_request(entityid, destx, desty)
--
function Scene:move_request(entityid, destx, desty)
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
-- bool Scene::move(entityid, x, y)
--
function Scene:move(entityid, x, y)
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

--
-- {entityid = Side, ...} prepare_match()
--
function Scene:prepare_match()
    assert(self.match == nil)
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return nil
    end
    
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not MONSTER category, func: %s", Function()))
        return nil
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return nil
    end

    self.match = Match:new(self.copy)

    -- add member
    for _, entity in pairs(self.members) do
        for placeholder, target in pairs(entity.pack.placeholders) do
            self.match:add_member(target)
        end
    end

    -- add opponent
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.monster ~= nil)
    for entity_baseid, number in pairs(self.trigger_event.content.monster) do
        while number > 0 do
            number = number - 1
            self.match:add_monster(entity_baseid, Side.ENEMY)
        end
    end
    return self.match:prepare()
end

--
-- void start_match()
--
function Scene:start_match()
    assert(self.match ~= nil)

    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return
    end
    
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not MONSTER category, func: %s", Function()))
        return
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return
    end

    assert(not self.match.isstart)
    assert(not self.match.isdone)
    
    self.match:start()
end    

--
-- void end_match
--
function Scene:end_match()
    assert(self.match ~= nil)
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return
    end
    
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not MONSTER category, func: %s", Function()))
        return
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return
    end
    
    assert(self.match.isdone)
    assert(self.match.side_victory ~= nil)
    
    local side_victory = self.match.side_victory
    
	self.match:destructor()
    self.match = nil
    
--    if side_victory == Side.ALLIES and self.trigger_event.base.endtype == EventEnd.REWARD then
--        self:accomplish_event(self.trigger_event)
--    end

    self.trigger_event.touch = true
    
    if side_victory == Side.ENEMY then
        cc.ScriptErrorLog(string.format("allies is defeated"))
        for _, entity in pairs(self.members) do
            cc.ScriptDebugLog(string.format("    entity: %d,%d,%s lost match, reset copy data", entity.id, entity.baseid, entity.base.name.cn))
-- TODO:            record.delete(entity) -- remove record of entity
            self:remove_member(entity)
        end
    end
end

--
-- void abort_match(entityid)
--
function Scene:abort_match(entityid)
    assert(self.match ~= nil)    
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return
    end
    
    if self.trigger_event.base.category ~= EventCategory.MONSTER then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not MONSTER category, func: %s", Function()))
        return
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return
    end
    
    self.match:abort(entityid)
end


--
-- bool event_reward(entityid, eventid, reward_index, entry_index)
--
function Scene:event_reward(entityid, eventid, reward_index, entry_index)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.REWARD then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not REWARD category, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end    
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.reward ~= nil)

    self.trigger_event.touch = true
    
    local rc = entity:event_reward(self.trigger_event.content.reward, reward_index, entry_index)
	if rc then
		-- remove reward_index from content.reward
		local t = {}
		assert(self.trigger_event.content.reward[reward_index] ~= nil)
		self.trigger_event.content.reward[reward_index] = nil
		for idx, reward in pairs(self.trigger_event.content.reward) do table.insert(t, reward) end
		self.trigger_event.content.reward = t

		if table.size(t) > 0 then
			-- update event
			self:dispatch_event(entityid, self.trigger_event, true)
		end
	end

    return rc
end

--
-- bool purchase_card(entityid, sceneid, eventid, card_baseid)
--
function Scene:purchase_card(entityid, sceneid, eventid, card_baseid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.SHOP_BUY_CARD then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not SHOP_BUY_CARD category, func: %s", Function()))
        return false
    end

    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end

-- TODO: record sold out cards
--    if record.copy_events_sold_cards(entity.record, self.copy.baseid, self.copy:current_layer(), self.trigger_event.baseid, card_baseid) then
--        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s, card: %d already sold, func: %s", tostring(eventid), card_baseid, Function()))
--        return false
--    end
              
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.shop ~= nil)

    self.trigger_event.touch = true
    
    local rc = entity:purchase_card(card_baseid, self.trigger_event.content.shop)
    if rc then
        self:dispatch_event(entityid, self.trigger_event, true)
    end
	return rc
end

--
-- bool purchase_item(entityid, sceneid, eventid, item_baseid)
--
function Scene:purchase_item(entityid, sceneid, eventid, item_baseid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.SHOP_BUY_CARD then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not SHOP_BUY_CARD category, func: %s", Function()))
        return false
    end

    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end

-- TODO: record sold out items
--    if record.copy_events_sold_items(entity.record, self.copy.baseid, self.copy:current_layer(), self.trigger_event.baseid, item_baseid) then
--        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s, item: %d already sold, func: %s", tostring(eventid), item_baseid, Function()))
--        return false
--    end
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.shop ~= nil)

    self.trigger_event.touch = true

    local rc = entity:purchase_item(card_baseid, self.trigger_event.content.shop)
    if rc then
        self:dispatch_event(entityid, self.trigger_event, true)
    end 
	return rc
end


--
-- bool destroy_card(entityid, sceneid, eventid, cardid)
--
function Scene:destroy_card(entityid, sceneid, eventid, cardid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.SHOP_DESTROY_CARD then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not SHOP_DESTROY_CARD category, func: %s", Function()))
        return false
    end

    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.destroy_card ~= nil)

    self.trigger_event.touch = true
    
    return entity:destroy_card(cardid, self.trigger_event.content.destroy_card)
end


--
-- bool levelup_card(entityid, sceneid, eventid, cardid)
--
function Scene:levelup_card(entityid, sceneid, eventid, cardid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.SHOP_LEVELUP_CARD then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not SHOP_LEVELUP_CARD category, func: %s", Function()))
        return false
    end

    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.levelup_card ~= nil)

    self.trigger_event.touch = true
    
    return entity:levelup_card(cardid, self.trigger_event.content.levelup_card)
end


--
-- bool levelup_puppet(entityid, sceneid, eventid, target_entityid)
--
function Scene:levelup_puppet(entityid, sceneid, eventid, target_entityid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.SHOP_LEVELUP_PUPPET then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not SHOP_LEVELUP_PUPPET category, func: %s", Function()))
        return false
    end

    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.levelup_puppet ~= nil)

    self.trigger_event.touch = true
    
    return entity:levelup_puppet(target_entityid, self.trigger_event.content.levelup_puppet)
end


--
-- bool choose_option(entityid, sceneid, eventid, storyoptionid, option_index)
--
function Scene:choose_option(entityid, sceneid, eventid, storyoptionid, option_index)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end
    
    if self.trigger_event.id ~= eventid then
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.id: %s, eventid: %s, func: %s", tostring(self.trigger_event.id), tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.accomplish then -- already accomplish this event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %s already accomplish, func: %s", tostring(eventid), Function()))
        return false
    end
    
    if self.trigger_event.base.category ~= EventCategory.STORY_OPTION then
        cc.ScriptErrorLog(string.format(">>>>>>> event is not STORY_OPTION category, func: %s", Function()))
        return false
    end

    assert(self.trigger_event.content ~= nil)
    assert(self.trigger_event.content.storyoption ~= nil)

    local storyoption_base = cc.ScriptLookupTable("StoryOption", storyoptionid)
    if storyoption_base == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> storyoptionid: %s not found, func: %s", tostring(storyoptionid), Function()))
        return false
    end

    if storyoption_base.options == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> storyoptionid: %s not found options, func: %s", tostring(storyoptionid), Function()))
        return false
    end

    local option = storyoption_base.options[option_index]
    
    if option == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> storyoptionid: %s options not found index: %s, func: %s", tostring(storyoptionid), tostring(option_index), Function()))
        return false
    end

    if option.script_func == nil or type(option.script_func) ~= 'function' then
        cc.ScriptErrorLog(string.format(">>>>>>> storyoptionid: %s script_func not exist or not function, func: %s", tostring(storyoptionid), Function()))
        return false
    end

    cc.ScriptDebugLog(string.format("entityid: %d, event: %d,%d,%s, optionid: %d, option:%d,%s", entityid,
        self.trigger_event.id, self.trigger_event.baseid, self.trigger_event.base.name.cn,
        storyoptionid, option_index, option.caption.cn
    ))

    self.trigger_event.touch = true
    
    option.script_func(
                entityid, 
                self.copy.baseid, 
                self.copy:current_layer(),
                self.trigger_event.baseid,
                self.copy:current_seed()
            )

    return true
end


--
-- bool trigger_linked_event(entityid)
--
function Scene:trigger_linked_event(entityid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end
    
    if self.trigger_event == nil then -- trigger_event must be hold
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event is nil, func: %s", Function()))
        return false
    end

    if not self.trigger_event.accomplish and self.trigger_event.touch then
        if self.trigger_event.base.endtype == EventEnd.REWARD then    
            self:accomplish_event(self.trigger_event)    -- accomplish event when receive reward
        end
    end
        
    if not self.trigger_event.accomplish then -- not accomplish and not touch event
        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event: %d not accomplish, func: %s", self.trigger_event.baseid, Function()))
        return false
    end
    
    assert(self.trigger_event.content ~= nil)
    
    if self.trigger_event.content.trigger_event_baseid == nil then
--        cc.ScriptErrorLog(string.format(">>>>>>> self.trigger_event.content.trigger_event_baseid is nil, func: %s", Function()))
        return false
    end

    -- NOTE: put it to self.events but don't put it to scene
    local linked_event = self:create_event_object(self.trigger_event.content.trigger_event_baseid, self.trigger_event.coord)
    cc.ScriptDebugLog(string.format("entityid: %d, event:%d,%d,%s, trigger_linked_event: %d,%d,%s", entityid,
        self.trigger_event.id, self.trigger_event.baseid, self.trigger_event.base.name.cn,
        linked_event.id, linked_event.baseid, linked_event.base.name.cn
    ))
    self.trigger_event = linked_event
    
    return self:dispatch_event(entityid, self.trigger_event, false)
end

--
-- bool arrange_placeholder(entityid, target_entityid, placeholder)
--
function Scene:arrange_placeholder(entityid, target_entityid, placeholder)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end

    return entity:arrange_placeholder(target_entityid, placeholder)
end

--
-- bool destroy_puppet(entityid, target_entityid)
--
function Scene:destroy_puppet(entityid, target_entityid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end

    return entity:destroy_puppet(target_entityid)
end

--
-- bool use_item(entityid, itemid)
--
function Scene:use_item(entityid, itemid)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end

    return entity:use_item(itemid)
end

--
-- bool remove_equip(entityid, slot)
--
function Scene:remove_equip(entityid, slot)
    local entity = self.members[entityid]
    if entity == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> not found entity: %s, func: %s", tostring(entityid), Function()))
        return false
    end

    return entity:remove_equip(slot)
end

