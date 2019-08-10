
--
------------------- Copy class -------------------
--

Copy = {
    baseid = nil,
    base = nil,
    script_func = nil,

    entity_master = nil,

	current_layer = function(self) return self.entity_master.copy_layer end,
	current_seed = function(self) return self.entity_master.copy_seed end,
	set_current_layer = function(self, layer) self.entity_master.copy_layer = layer end,
    
    seeds = nil, -- {[layer] = seed, ...}, dynamic generate seed for per layer    
    scene = nil, -- current scene
    members = nil, -- {[entityid] = entity, ...}

    enter_next_layer = nil, -- bool, indicate to enter next layer scene
        
	constructor = function(self, entityid, copy_baseid)
		self.baseid = copy_baseid
		self.base = cc.ScriptLookupTable("Copy", copy_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.enter_next_layer = false

        -- create entity master
        self.entity_master = Entity:new(self, entityid, Side.ALLIES, nil, Span.COPY)
        
		self.seeds = {}
		self.scene = self:create_scene(entityid, self:current_layer())
		assert(self.scene ~= nil)
		self.members = {}
		self:add_member(self.entity_master)

		collectgarbage("collect")
	end,

	destructor = function(self)
	    for entityid, _ in pairs(self.members) do
	        self:remove_member(entityid)
	    end
	    
        if self.scene ~= nil then
            self.scene:destructor()
            self.scene = nil
        end
	end,
    
    add_member = function(self, entity)
        assert(self.members[entity.id] == nil)
        self.members[entity.id] = entity
        assert(self.scene ~= nil)
        
        entity:enter_copy(self)
        
        self.scene:add_member(entity)
    end,
    remove_member = function(self, entityid)
        local entity = self.members[entityid]
        if entity == nil then return end

		if self.scene ~= nil then
			self.scene:remove_member(entity)
		end
		        
        cc.ScriptExitCopy(entityid)
        entity:exit_copy(self)
                
        entity:destructor()
        
        self.members[entity.id] = nil
        if self.entity_master ~= nil and entity.id == self.entity_master.id then 
            self.entity_master = nil 
        end
    end
}

function Copy:new(entityid, copy_baseid)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entityid, copy_baseid)
	return object
end


--
-- bool checkDone()
--
function Copy:checkDone()
    return table.size(self.members) == 0 or self.entity_master == nil or self.scene == nil
end

--
-- void update(delta)
--
function Copy:update(delta)
    if self.scene ~= nil then
        if self.enter_next_layer then
            self.enter_next_layer = false
            local rc = self:enter_scene(self.entity_master.id, 1) -- try to enter map of next layer
			if not rc then
				self:remove_member(self.entity_master.id)
			end
        else
    		if self.scene:checkDone() then
    			cc.ScriptDebugLog(string.format("scene: %d, %d isDone", self.scene.id, self.scene.baseid))
    			self.scene:destructor()
    			cc.ScriptDebugLog(string.format("scene: %d, %d close", self.scene.id, self.scene.baseid))
    			self.scene = nil
    		else
    			self.scene:update(delta) 
    		end
		end
	end
end
    

--
-- Scene create_scene(entityid, layer)
--
function Copy:create_scene(entityid, layer)
    -- init seed of layer
    if self.seeds[layer] == nil then
--        _, self.seeds[layer] = NewRandom(record.seed(self.entity_master.record))(1, layer)
        _, self.seeds[layer] = NewRandom(self:current_seed())(1, layer)
    end
    assert(self.seeds[layer] ~= nil)
    cc.ScriptDebugLog(string.format("CreateScene, copy: %d, layer: %d, seed: %d", self.baseid, layer, self.seeds[layer]))

    -- copy.script_func(entityid, copy_baseid, copy_layers, randomseed)
    -- events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    local events = self.script_func(entityid, self.baseid, layer, self.seeds[layer])
    if events == nil then
        cc.ScriptErrorLog(string.format(">>>>>>>>>> copy: %d, layer: %d not exist", self.baseid, layer))
        return nil
    end
    assert(events.map_baseid ~= nil and events.map_events ~= nil)

    -- init new scene
    return Scene:new(self, events.map_baseid, self.seeds[layer], entityid, events.map_events, {x=0,y=0})
end


--
-- bool enter_scene(entityid, layer_add)
--
function Copy:enter_scene(entityid, layer_add)
    local entity = self.members[entityid]
    assert(entity ~= nil)
    
	-- serialize 
--	if self.entity_master ~= nil and entity.id == self.entity_master.id then 
--		record.serialize(self.entity_master)
--	end
                
	local layer = self:current_layer()
    layer = layer + layer_add
    local scene = self:create_scene(entityid, layer)
    if scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>>> enter next layer: %d scene failure, copy: %d", layer, self.baseid))
        return false
    end

    -- remove all of members from self.scene
    for _, entity in pairs(self.members) do
        self.scene:remove_member(entity)
    end

    self:set_current_layer(layer)
    
    -- destroy self.scene
    self.scene:destructor()
    self.scene = scene

    -- add all of members to self.scene
    for _, entity in pairs(self.members) do
        self.scene:add_member(entity)
    end

    cc.ScriptDebugLog(string.format("\n^^^^^^^^^^^^^^^^^^^^^^^ entity: %d enter %d layer ^^^^^^^^^^^^^^^^^^^^^^^\n", entityid, layer))

	collectgarbage("collect")
	return true
end


--
-- bool move_request(entityid, destx, desty)
--
function Copy:move_request(entityid, destx, desty)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

    return self.scene:move_request(entityid, destx, desty)
end

--
-- bool move(entityid, x, y)
-- 
function Copy:move(entityid, x, y)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end
	
    return self.scene:move(entityid, x, y)
end

--
-- bool event_reward(entityid, eventid, reward_index, entry_index)
--
function Copy:event_reward(entityid, eventid, reward_index, entry_index)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end
	
	return self.scene:event_reward(entityid, eventid, reward_index, entry_index)
end

--
-- bool purchase_card(entityid, eventid, card_baseid)
--
function Copy:purchase_card(entityid, eventid, card_baseid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:purchase_card(entityid, sceneid, eventid, card_baseid)
end

--
-- bool purchase_item(entityid, eventid, item_baseid)
--
function Copy:purchase_item(entityid, eventid, item_baseid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:purchase_item(entityid, sceneid, eventid, item_baseid)
end


--
-- bool destroy_card(entityid, eventid, cardid)
--
function Copy:destroy_card(entityid, eventid, cardid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:destroy_card(entityid, sceneid, eventid, cardid)
end


--
-- bool levelup_card(entityid, eventid, cardid)
--
function Copy:levelup_card(entityid, eventid, cardid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:levelup_card(entityid, sceneid, eventid, cardid)
end

--
-- bool levelup_puppet(entityid, eventid, target_entityid)
--
function Copy:levelup_puppet(entityid, eventid, target_entityid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:levelup_puppet(entityid, sceneid, eventid, target_entityid)
end

--
-- bool choose_option(entityid, eventid, storyoptionid, option_index)
--
function Copy:choose_option(entityid, eventid, storyoptionid, option_index)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:choose_option(entityid, sceneid, eventid, storyoptionid, option_index)
end


--
-- bool trigger_linked_event(entityid)
--
function Copy:trigger_linked_event(entityid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:trigger_linked_event(entityid)
end

--
-- bool arrange_placeholder(entityid, target_entityid, placeholder)
--
function Copy:arrange_placeholder(entityid, target_entityid, placeholder)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end
    
	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:arrange_placeholder(entityid, target_entityid, placeholder)
end


--
-- bool destroy_puppet(entityid, target_entityid)
--
function Copy:destroy_puppet(entityid, target_entityid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end
    
	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:destroy_puppet(entityid, target_entityid)
end

--
-- bool use_item(entityid, itemid)
--
function Copy:use_item(entityid, itemid)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    return self.scene:use_item(entityid, itemid)
end

--
-- bool remove_equip(entityid, slot)
--
function Copy:remove_equip(entityid, slot)
    if self.scene == nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

	if self.scene.match ~= nil then
	    cc.ScriptErrorLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

    return self.scene:remove_equip(entityid, slot)
end

