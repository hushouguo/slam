
--
------------------- Copy class -------------------
--

Copy = {
    baseid = nil,
    base = nil,
    script_func = nil,

    record = nil,
    current_layer = function(self) return record.layer(self.record, self.baseid) end,
    current_seed = function(self) return record.seed(self.record) end,
    
    seeds = nil, -- {[layer] = seed, ...}, dynamic generate seed for per layer    
    scene = nil, -- current scene
    members = nil, -- {[entityid] = entity, ...}
    add_member = function(self, entityid)
        local entity = Entity:new(self, entityid, Side.ALLIES)
        assert(self.members[entityid] == nil)
        self.members[entityid] = entity
        assert(self.scene ~= nil)
        self.scene:add_member(entity)
    end,
    remove_member = function(self, entityid)
        local entity = self.members[entityid]
        assert(entity ~= nil)
        assert(self.scene ~= nil)
        self.scene:remove_member(entity)
        entity:destructor()
        self.members[entityid] = nil
        cc.ExitCopy(entityid)
    end,
        
	constructor = function(self, entityid, copy_baseid)
		self.baseid = copy_baseid
		self.base = cc.LookupTable("Copy", copy_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.record = record.unserialize(entityid, copy_baseid)
		assert(self.record ~= nil)
		self.seeds = {}
		self.scene = self:create_scene(entityid, record.layer(self.record, self.baseid))
		assert(self.scene ~= nil)
		self.members = {}
		self:add_member(entityid)
	end,

	destructor = function(self)
	    for entityid, _ in pairs(self.members) do
	        self:remove_member(entityid)
	    end
	    
        if self.scene ~= nil then
            self.scene:destructor()
            self.scene = nil
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
    return table.size(self.members) == 0 or self.scene == nil
end

--
-- void update(delta)
--
function Copy:update(delta)
    if self.scene ~= nil then 
		if self.scene:checkDone() then
			cc.WriteLog(string.format("scene: %d, %d closing soon", self.scene.id, self.scene.baseid))
			self.scene:destructor()
			self.scene = nil
		else
			self.scene:update(delta) 
		end
	end
end
    

--
-- Scene create_scene(entityid, layer)
--
function Copy:create_scene(entityid, layer)
    -- init seed of layer
    if self.seeds[layer] == nil then
        _, self.seeds[layer] = NewRandom(record.seed(self.record))(1, layer)
    end
    assert(self.seeds[layer] ~= nil)
    cc.WriteLog(string.format("copy: %d, layer: %d, seed: %d", self.baseid, layer, self.seeds[layer]))

    -- retrieve entity_base
    local entity_baseid = cc.GetBaseid(entityid)
    local entity_base = cc.LookupTable("Entity", entity_baseid)
    assert(entity_base ~= nil)
    
    -- copy.script_func(entity_career, copy_baseid, copy_layers, randomseed)
    -- events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    local events = self.script_func(entity_base.career, self.baseid, layer, self.seeds[layer])
    if events == nil then
        cc.WriteLog(string.format(">>>>>>>>>> copy: %d, layer: %d not exist", self.baseid, layer))
        return nil
    end
    assert(events.map_baseid ~= nil and events.map_events ~= nil)

    -- retrieve accomplish_events & init new scene
    local accomplish_events = record.accomplish_events(self.record, self.baseid, events.map_baseid)
    local sceneid = cc.MapNew(events.map_baseid) -- MapNew
    return Scene:new(self, sceneid, events.map_baseid, self.seeds[layer], entityid, events.map_events, accomplish_events)
end


--
-- void enter_next_layer_scene(entityid)
--
function Copy:enter_next_layer_scene(entityid)
    local entity = self.members[entityid]
    assert(entity ~= nil)
    
    local layer = record.layer(self.record, self.baseid)
    layer = layer + 1
    local scene = self:create_scene(entityid, layer)
    if scene == nil then
        cc.WriteLog(string.format("enter next layer: %d scene failure, copy: %d", layer, self.baseid))
        return
    end

    -- save progress of current copy
    record.set_layer(self.record, self.baseid, layer)

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
end


--
-- void accomplish_event(event)
--
function Copy:accomplish_event(event)
    assert(self.scene ~= nil and self.record ~= nil)
    record.set_accomplish_events(self.record, self.baseid, self.scene.baseid, event.baseid)
    cc.EventAccomplish(record.member(self.record), self.scene.id, event.id)
end

--
-- bool move_request(entityid, sceneid, destx, desty)
--
function Copy:move_request(entityid, sceneid, destx, desty)
    assert(entityid == record.member(self.record))
    assert(self.scene ~= nil)
    assert(self.scene.id == sceneid)
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>>>>>>> forbid move_requeset when in the match"))
	    return false
	end
    
    local fullPath, event = self.scene:move_request(entityid, destx, desty)
    if fullPath ~= nil then
        -- fullPath: { {x=?, y=?}, ... }
        if table.size(fullPath) == 0 then return false end
        cc.MoveTrigger(entityid, sceneid, fullPath)
    else
        assert(event ~= nil)
        assert(event.accomplish == false)
        cc.WriteLog(string.format("eventTrigger: %d", event.baseid))
        local eventDispatch = {
            [EventCategory.MONSTER] = function(entityid, sceneid, event)
                -- { 
                --      monsters = {entity_baseid = number, ...}, 
                --      rewards = {cards = {card_baseid = number, ...}, items = {item_baseid = number, ...}, gold = ?} 
                -- }
                assert(event.content ~= nil)
                assert(event.content.monster ~= nil)
                table.dump(event.content.monster, "event.monster")
                cc.EventMonsterTrigger(entityid, sceneid, event.id, event.content.monster)
                end,

            [EventCategory.SHOP_BUY_CARD] = function(entityid, sceneid, event)
                -- { cards = {card_baseid, ...}, items = {item_baseid, ...} 
                assert(event.content ~= nil)
                assert(event.content.shop ~= nil)
                table.dump(event.content.shop, "event.shop")
                cc.EventShopTrigger(entityid, sceneid, event.id, event.content.shop)
                end,

            [EventCategory.STORY] = function(entityid, sceneid, event)
                -- { title = ?, text = ?, options = { [1] = ?, [2] = ?, ... } }
                assert(event.content ~= nil)
                assert(event.content.story ~= nil)
                table.dump(event.content.story, "event.story")
                cc.EventOptionTrigger(entityid, sceneid, event.id, event.content.story)
                end,

            [EventCategory.SHOP_DESTROY_CARD] = function(entityid, sceneid, event)
                -- TODO: 
                -- cc.EventDestroyCardTrigger(entityid, sceneid, event.id, content)
                end,

            -- trigger event to exit map
            [EventCategory.EXIT] = function(entityid, sceneid, event) 
                self:enter_next_layer_scene(entityid) -- try to enter map of next layer
                end,
        }        
        -- assert(eventDispatch[event.base.category] ~= nil)
		if eventDispatch[event.base.category] ~= nil then
        	eventDispatch[event.base.category](entityid, sceneid, event)
		end
    end
    return true
end


--
-- void move(entityid, sceneid, x, y)
-- 
function Copy:move(entityid, sceneid, x, y)
    assert(entityid == record.member(self.record))
    assert(self.scene ~= nil)
    assert(self.scene.id == sceneid)
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>>>>>>> forbid move when in the match"))
	    return
	end
    self.scene:move(entityid, x, y)
end

--
-- bool event_reward(entityid, sceneid, eventid, reward_index, entry_index)
--
function Copy:event_reward(entityid, sceneid, eventid, reward_index, entry_index)
    assert(entityid == record.member(self.record))
    assert(self.scene ~= nil)
    assert(self.scene.id == sceneid)
	if self.scene.match ~= nil then
	    cc.WriteLog(string.format(">>>>>>>>>>> forbid event reward when match is not over yet"))
	    return false
	end
	return self.scene:event_reward(entityid, eventid, reward_index, entry_index)
end

