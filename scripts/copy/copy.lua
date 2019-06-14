--
-- copy.lua
--

require('common')
require('tools/tools')
require('tools/astar')
require('tools/bit')
require('tools/noise')
require('copy/map')
require('copy/event')
require('copy/obstacle')
require('match/card')
require('match/entity')
require('match/monster')


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
    -- void enter_map()
    --
    enter_map = function(self, next_layer)
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

        -- generate Map & map.add_member
        local t_layouts, entry_coord = self.map:generator()
        self.map:add_member(self.t.member, entry_coord)

        -- EnterMap
        cc.EnterMap(self.t.member, self.map.id, t_layouts, entry_coord)
    end,

    --
    -- void exit_map()
    --
    exit_map = function(self)
        assert(self.t ~= nil)
        if self.map ~= nil then
            cc.ExitMap(self.t.member, self.map.id)
            self.map:destructor() -- EventDestroy & ObstacleDestroy
            cc.MapDestroy(self.map.id) -- MapDestroy
            self.map = nil
        end
    end,


    --
    -- void move_request(entityid, mapid, destx, desty)
    --
    move_request = function(self, entityid, mapid, destx, desty)
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
    end,
    
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
	local copy = {}
	self.__index = self -- copy.__index = function(key) return copy[key] end
	setmetatable(copy, self)
	copy:constructor(copy_baseid, t)
	return copy
end


--
------------------- lua APIs -------------------
--

function lua_entry_copy_enter(entityid, copy_baseid)
    local t = RecordUnserialize(entityid, copy_baseid)
    g_copy = Copy:new(copy_baseid, t)
    g_copy:enter_map(false)
end

function lua_entry_copy_exit(entityid)
end

function lua_entry_copy_move_request(entityid, mapid, destx, desty)
    assert(g_copy ~= nil)
    g_copy:move_request(entityid, mapid, destx, desty)
end

function lua_entry_copy_move(entityid, mapid, x, y)
end

function lua_entry_copy_event_retval(entityid, mapid, retval)
end

--
-- call me for every frame
function lua_entry_update(delta)
end
