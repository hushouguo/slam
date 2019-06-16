
--
------------------- Copy class -------------------
--

local Copy = {
    baseid = nil,
    base = nil,
    script_func = nil,
    
    t = nil, -- {member = entityid, seed = ?, layer = ?, createtime = ?}
    seeds = nil, -- {[layer] = seed}
    map = nil,

    
	--
	-- void update()
	--
	update = function(self, delta)
	end,
    
	constructor = function(self, copy_baseid, t)
		self.baseid = copy_baseid
		self.base = cc.LookupTable("Copy", copy_baseid)
		assert(self.base ~= nil)
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		self.t = t
		self.seeds = {}
		self.map = nil
	end
}

function Copy:new(copy_baseid, t)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy_baseid, t)
	return object
end


--
-- void enter_map(bool next_layer)
--
function Copy:enter_map(next_layer)
    assert(self.t ~= nil)
    local t = self.t.copies[self.baseid]
    assert(t ~= nil)
    assert(t.layer > 0)

    -- exit current map if exist
    self:exit_map()

    if next_layer then t.layer = t.layer + 1 end

    -- init seed of layer
    if self.seeds[t.layer] == nil then
        local seed = 0
        _, seed = NewRandom(self.t.seed)(1, t.layer)
        self.seeds[t.layer] = seed
    end
    assert(self.seeds[t.layer] ~= nil)

    -- retrieve entity_base
    local entity_baseid = cc.GetBaseid(self.t.member)
    local entity_base = cc.LookupTable("Entity", entity_baseid)
    assert(entity_base ~= nil)
    
    -- events function copy.script_func(entity_career, copy_baseid, copy_layers, randomseed)
    local events = self.script_func(entity_base.career, self.baseid, t.layer, self.seeds[t.layer])
    assert(events ~= nil) -- no next layer                                
    
    -- init Map, events: { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
    local mapid = cc.MapNew(events.map_baseid)
    assert(mapid ~= nil)
    self.map = Map:new(mapid, events.map_baseid, self.seeds[t.layer], events.map_events)

    -- generate Map
    local t_layouts, entry_coord = self.map:generator()

    -- mem.add_member
    self.map:add_member(self.t.member, entry_coord)

    -- EnterMap
    cc.EnterMap(self.t.member, self.map.id, t_layouts, entry_coord)
end


--
-- void exit_map()
--
function Copy:exit_map()
    assert(self.t ~= nil)
    if self.map ~= nil then
        cc.ExitMap(self.t.member, self.map.id)
        self.map:destructor() -- EventDestroy & ObstacleDestroy
        cc.MapDestroy(self.map.id) -- MapDestroy
        self.map = nil
    end
end


--
-- void move_request(entityid, mapid, destx, desty)
--
function Copy:move_request(entityid, mapid, destx, desty)
    assert(self.map ~= nil)
    assert(entityid == self.t.member)
    assert(self.map.id == mapid)
    local fullPath, eventid = self.map:move_request(entityid, destx, desty)
    if fullPath ~= nil then
        cc.MoveTrigger(entityid, mapid, fullPath)
    else
        assert(eventid ~= nil)
        cc.EventTrigger(entityid, mapid, eventid)
    end
end


--
-- void move(entityid, mapid, x, y)
-- 
function Copy:move(entityid, mapid, x, y)
    assert(self.map ~= nil)
    assert(entityid == self.t.member)
    assert(self.map.id == mapid)
    self.map:move(entityid, x, y)
end

