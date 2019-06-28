
--
------------------- Copy class -------------------
--

Copy = {
    baseid = nil,
    base = nil,
    script_func = nil,

    entity_master = nil,

    current_layer = function(self) return record.copy_layer(self.entity_master.record, self.baseid) end,
    current_seed = function(self) return record.seed(self.entity_master.record) end,
    
    seeds = nil, -- {[layer] = seed, ...}, dynamic generate seed for per layer    
    scene = nil, -- current scene
    members = nil, -- {[entityid] = entity, ...}

    enter_next_layer = nil, -- bool, indicate to enter next layer scene
        
	constructor = function(self, entityid, copy_baseid)
		self.baseid = copy_baseid
		self.base = cc.LookupTable("Copy", copy_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.enter_next_layer = false

        self.entity_master = Entity:new(self, entityid, Side.ALLIES)        
		
		self.seeds = {}
		local copy_layer = record.copy_layer(self.entity_master.record, self.baseid)
		if copy_layer == nil then 
		    copy_layer = 1
		    record.set_copy_layer(self.entity_master.record, self.baseid, copy_layer)
		end
		self.scene = self:create_scene(entityid, copy_layer)
		assert(self.scene ~= nil)
		self.members = {}
		self:add_member(self.entity_master)
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
        self.scene:add_member(entity)
    end,
    remove_member = function(self, entityid)
        local entity = self.members[entityid]
        -- assert(entity ~= nil)
        if entity == nil then return end
        
        -- assert(self.scene ~= nil)
		if self.scene ~= nil then
			self.scene:remove_member(entity)
		end
        entity:destructor()
        self.members[entityid] = nil
        cc.ExitCopy(entityid)
        if self.entity_master ~= nil and entityid == self.entity_master.id then self.entity_master = nil end
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
            local rc = self:enter_next_layer_scene(self.entity_master.id) -- try to enter map of next layer
			if not rc then
				self:remove_member(self.entity_master.id)
			end
        else
    		if self.scene:checkDone() then
    			cc.WriteLog(string.format("scene: %d, %d isDone", self.scene.id, self.scene.baseid))
    			self.scene:destructor()
    			cc.WriteLog(string.format("scene: %d, %d close", self.scene.id, self.scene.baseid))
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
        _, self.seeds[layer] = NewRandom(record.seed(self.entity_master.record))(1, layer)
    end
    assert(self.seeds[layer] ~= nil)
    cc.WriteLog(string.format("CreateScene, copy: %d, layer: %d, seed: %d", self.baseid, layer, self.seeds[layer]))

    -- copy.script_func(entityid, copy_baseid, copy_layers, randomseed)
    -- events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    local events = self.script_func(entityid, self.baseid, layer, self.seeds[layer])
    if events == nil then
        cc.WriteLog(string.format(">>>>>>>>>> copy: %d, layer: %d not exist", self.baseid, layer))
        return nil
    end
    assert(events.map_baseid ~= nil and events.map_events ~= nil)

    -- retrieve accomplish_events & init new scene
    local accomplish_events = record.copy_accomplish_events(self.entity_master.record, self.baseid, layer)
    local sceneid = cc.MapNew(events.map_baseid) -- MapNew
    return Scene:new(self, sceneid, events.map_baseid, self.seeds[layer], entityid, events.map_events, accomplish_events)
end


--
-- bool enter_next_layer_scene(entityid)
--
function Copy:enter_next_layer_scene(entityid)
    local entity = self.members[entityid]
    assert(entity ~= nil)
    
    local layer = record.copy_layer(self.entity_master.record, self.baseid)
    layer = layer + 1
    local scene = self:create_scene(entityid, layer)
    if scene == nil then
        cc.WriteLog(string.format(">>>>>>> enter next layer: %d scene failure, copy: %d", layer, self.baseid))
        return false
    end

    -- save progress of current copy
    record.set_copy_layer(self.entity_master.record, self.baseid, layer)

    -- remove all of members from self.scene
    for _, entity in pairs(self.members) do
        self.scene:remove_member(entity)
    end
    
    -- destroy self.scene
    self.scene:destructor()
    self.scene = scene

    -- add all of members to self.scene
    for _, entity in pairs(self.members) do
        self.scene:add_member(entity)
    end

    cc.WriteLog(string.format("\n^^^^^^^^^^^^^^^^^^^^^^^ entity: %d enter %d layer ^^^^^^^^^^^^^^^^^^^^^^^\n", entityid, layer))

	collectgarbage("collect")
	return true
end


--
-- void accomplish_event(event)
--
function Copy:accomplish_event(event)
    assert(self.scene ~= nil and self.entity_master.record ~= nil)
    record.set_copy_accomplish_events(self.entity_master.record, self.baseid, self:current_layer(), event.baseid)
    cc.AccomplishEvent(self.entity_master.id, self.scene.id, event.id)
end

--
-- bool move_request(entityid, sceneid, destx, desty)
--
function Copy:move_request(entityid, sceneid, destx, desty)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end
    
    local fullPath, event = self.scene:move_request(entityid, destx, desty)    
    if fullPath ~= nil then
        -- fullPath: { {x=?, y=?}, ... }
        if table.size(fullPath) == 0 then 
            cc.WriteLog(string.format(">>>>>> fullPath is empty, func: %s", Function()))
            return false 
        end -- unreachable destination
        cc.MoveTrigger(entityid, sceneid, fullPath)
    else
        if event == nil then 
            cc.WriteLog(string.format(">>>>>> event is nil, func: %s", Function()))
            return false 
        end -- scene:move_request failure
        
        assert(not event.accomplish and not event.reward)
        cc.WriteLog(string.format("eventTrigger: %d, category: %d", event.baseid, event.base.category))
        local eventDispatch = {
            [EventCategory.MONSTER] = function(entityid, sceneid, event, content)
                assert(content ~= nil)
                table.dump(content, "event.Monster")
                cc.EventMonsterTrigger(entityid, sceneid, event.id, content)
                end,

            [EventCategory.SHOP_BUY_CARD] = function(entityid, sceneid, event, content)
                assert(content ~= nil)
                table.dump(content, "event.BuyCard")
                -- accomplish event when trigger event
                self.scene:accomplish_event(event)
                cc.EventShopTrigger(entityid, sceneid, event.id, content)
                end,

            [EventCategory.STORY] = function(entityid, sceneid, event, content)
                assert(content ~= nil)
                table.dump(content, "event.Story")
                -- accomplish event when trigger event
                self.scene:accomplish_event(event)
                cc.EventOptionTrigger(entityid, sceneid, event.id, content)
                end,

            [EventCategory.SHOP_DESTROY_CARD] = function(entityid, sceneid, event, content)
                assert(content ~= nil)
                table.dump(content, "event.DestroyCard")
                -- accomplish event when trigger event
                self.scene:accomplish_event(event)
                cc.EventDestroyCardTrigger(entityid, sceneid, event.id, content)
                end,

            -- trigger event to exit map
            [EventCategory.EXIT] = function(entityid, sceneid, event, content)
                self.enter_next_layer = true
                cc.WriteLog(string.format("entity: %d enter next layer scene", entityid))
                end,
        }
		if eventDispatch[event.base.category] ~= nil then
        	eventDispatch[event.base.category](entityid, sceneid, event, self.scene:take_event_content(entityid, event))
        else
            cc.WriteLog(string.format(">>>>>> entityid: %d, event: %d,%d, category: %d unhandled", entityid, event.id, event.baseid, event.base.category))
		end
    end
    return true
end


--
-- bool move(entityid, sceneid, x, y)
-- 
function Copy:move(entityid, sceneid, x, y)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end
	
    return self.scene:move(entityid, x, y)
end

--
-- bool event_reward(entityid, sceneid, eventid, reward_index, entry_index)
--
function Copy:event_reward(entityid, sceneid, eventid, reward_index, entry_index)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end
	
	return self.scene:event_reward(entityid, eventid, reward_index, entry_index)
end

--
-- bool purchase_card(entityid, sceneid, eventid, card_baseid)
--
function Copy:purchase_card(entityid, sceneid, eventid, card_baseid)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:purchase_card(entityid, sceneid, eventid, card_baseid)
end

--
-- bool purchase_item(entityid, sceneid, eventid, item_baseid)
--
function Copy:purchase_item(entityid, sceneid, eventid, item_baseid)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:purchase_item(entityid, sceneid, eventid, item_baseid)
end


--
-- bool destroy_card(entityid, sceneid, eventid, cardid)
--
function Copy:destroy_card(entityid, sceneid, eventid, cardid)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:destroy_card(entityid, sceneid, eventid, cardid)
end


--
-- bool levelup_card(entityid, sceneid, eventid, cardid)
--
function Copy:levelup_card(entityid, sceneid, eventid, cardid)
    if self.scene == nil then
	    cc.WriteLog(string.format(">>>>>> self.scene is nil, entityid: %s, func: %s", tostring(entityid), Function()))
        return false 
    end

    if self.scene.id ~= sceneid then
	    cc.WriteLog(string.format(">>>>>> self.scene.id: %d, sceneid: %d, entityid: %s, func: %s", self.scene.id, sceneid, tostring(entityid), Function()))
        return false 
    end    
    
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>> self.scene.match is exist, entityid: %s, func: %s", tostring(entityid), Function()))
	    return false
	end

	return self.scene:levelup_card(entityid, sceneid, eventid, cardid)
end

