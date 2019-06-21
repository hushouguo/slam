--
-- test_logic.lua
--

require('main')

--
------------------- unittest -------------------
--

local function Sleep(n)
	os.execute("sleep " .. n)
end

-- table load, func: void func(baseid, t)
local function table_load(dir, func)
	local PATH = "../tables/" .. dir
	local function GetFileName(file)
		len = string.len(PATH)
		return string.sub(file, len + 2)
	end
	local function GetFileNameSuffix(file)
		local idx = file:match(".+()%.%w+$")
		return idx and file:sub(1, idx-1) or file
	end
	local files = io.popen('find ' .. PATH)
	for file in files:lines() do
		if string.find(file, "%.lua$") then
			local chunk = loadfile(file)
			file = GetFileName(file)
			local baseid = GetFileNameSuffix(file)
			func(tonumber(baseid), chunk())
		end
	end
end

Player = {
	id = nil,
	baseid = nil,
	base = nil,

	record = nil,
	bag = nil,
	sceneid = nil,
	coord = nil,
	
	career = nil,	
	stack_hold = {},

	constructor = function(self, id, baseid)
		self.id = id
		self.baseid = baseid
		self.base = cc.LookupTable("Entity", baseid)
		assert(self.base ~= nil)
		
		self.record = nil
		self.bag = {
		    cards = {}, -- {[cardid] = card, ...}
		    items = {}, -- {[itemid] = item, ...}
		    gold = 0
		}
		self.sceneid = nil
		self.coord = nil

		self.career = 1
		self.stack_hold = {}
	end
}

function Player:new(id, baseid)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(id, baseid)
	return object
end

local function unittest()
	cc.WriteLog = function(content) print(content) end
	cc.Milliseconds = function() return os.time() end

	local tables_copy = {}
	local tables_map = {}
	local tables_obstacle = {}
	local tables_event = {}
	local tables_entity = {}
	local tables_card = {}
	local tables_buff = {}
	
	table_load("Copy", function(baseid, t) tables_copy[baseid] = t end)
	table_load("Map", function(baseid, t) tables_map[baseid] = t end)
	table_load("Obstacle", function(baseid, t) tables_obstacle[baseid] = t end)
	table_load("Event", function(baseid, t) tables_event[baseid] = t end)
	table_load("Entity", function(baseid, t) tables_entity[baseid] = t end)
	table_load("Card", function(baseid, t) tables_card[baseid] = t end)
	table_load("Buff", function(baseid, t) tables_buff[baseid] = t end)

	cc.LookupTable = function(table_name, baseid)
		local tables = {
			Copy = tables_copy,
			Map = tables_map,
			Obstacle = tables_obstacle,
			Event = tables_event,
			Entity = tables_entity,
			Card = tables_card,
			Buff = tables_buff,
		}
		assert(tables[table_name] ~= nil)
		assert(tables[table_name][baseid] ~= nil, 'baseid: ' .. tostring(baseid))
		return tables[table_name][baseid]
	end

    local members = {} -- {[entityid] = Player, ...}
    
    local identifier = {
        entity = 100,
        map = 200,
        obstacle = 300,
        event = 400,
        card = 500,
        item = 600,
        buff = 700
    }
    
	cc.EntityNew = function(entity_baseid)
	    identifier.entity = identifier.entity + 1
		assert(members[identifier.entity] == nil)
		members[identifier.entity] = Player:new(identifier.entity, entity_baseid)
	    return identifier.entity
	end
	cc.EntityDestroy = function(entityid)
	end

	cc.MapNew = function(map_baseid)
	    identifier.map = identifier.map + 1
	    return identifier.map
	end
	cc.MapDestroy = function(mapid)
	end

	cc.ObstacleNew = function(obstacle_baseid)
	    identifier.obstacle = identifier.obstacle + 1
	    return identifier.obstacle
	end
	cc.ObstacleDestroy = function(obstacleid)
	end

	cc.EventNew = function(event_baseid)
	    identifier.event = identifier.event + 1
	    return identifier.event
	end
	cc.EventDestroy = function(eventid)
	end
	
	cc.CardNew = function(entityid, card_baseid)
		assert(entityid ~= nil and card_baseid ~= nil)
	    identifier.card = identifier.card + 1
	    return identifier.card
	end
	cc.CardDestroy = function(entityid, cardid)
	end

	cc.GetBaseid = function(entityid) 
		assert(members[entityid] ~= nil)
		return members[entityid].baseid
	end
	
	cc.EntitySerialize = function(entityid, string)
	    assert(members[entityid] ~= nil)
		members[entityid].record = string
	end
	cc.EntityUnserialize = function(entityid)
	    assert(members[entityid] ~= nil)
		return members[entityid].record
	end

	cc.BagAddCard = function(entityid, cardid)
	end
	cc.BagRemoveCard = function(entityid, cardid)
	end

	cc.BagAddItem = function(entityid, itemid)
	end
	cc.BagRemoveItem = function(entityid, itemid)
	end

	cc.BagSetGold = function(entityid, gold_new, gold_old)
	    assert(members[entityid] ~= nil)
	    members[entityid].bag.gold = gold_new
	end

	--------------------------------- Copy -----------------------------------

    cc.ExitCopy = function(entityid)
        print('ExitCopy')
        os.exit()
    end
    
	cc.EnterMap = function(entityid, mapid, layouts, coord)
	    assert(members[entityid] ~= nil)
	    members[entityid].sceneid = mapid
	    members[entityid].coord = coord
	end

	cc.ExitMap = function(entityid, mapid)
		-- print('ExitMap')
	end

	cc.MoveTrigger = function(entityid, mapid, v)
	    assert(members[entityid] ~= nil)
	    --table.dump(v, "MoveTrigger")
	    --print("MoveTrigger")
		for _, coord in pairs(v) do
			lua_entry_copy_move(entityid, members[entityid].sceneid, coord.x, coord.y)
		end
	end

	cc.EventMonsterTrigger = function(entityid, mapid, eventid, content)
		print("EventMonsterTrigger: " .. tostring(eventid))
    	lua_entry_match_start()
	end
	cc.EventShopTrigger = function(entityid, mapid, eventid, content)
		print("EventShopTrigger: " .. tostring(eventid))
	end
	cc.EventOptionTrigger = function(entityid, mapid, eventid, content)
		print("EventOptionTrigger: " .. tostring(eventid))
	end
	cc.EventDestroyCardTrigger = function(entityid, mapid, eventid, content)
		print("EventDestroyCardTrigger: " .. tostring(eventid))
	end

	cc.AccomplishEvent = function(entityid, mapid, eventid)
		print("AccomplishEvent: " .. tostring(eventid))
	end

	cc.EventAccomplish = function(entityid, mapid, eventid)
		lua_entry_copy_event_reward(entityid, mapid, eventid, 1, 1)
	end
	
	--------------------------------- Match -----------------------------------

	cc.StackHoldAdd = function(entityid, cardid)
		assert(members[entityid] ~= nil)
		members[entityid].stack_hold[cardid] = cardid
		--cc.WriteLog("entity: " .. entityid .. " HoldAdd cardid: " .. cardid)
	end
	cc.StackHoldRemove = function(entityid, cardid)
		assert(members[entityid] ~= nil)
		members[entityid].stack_hold[cardid] = nil
		--cc.WriteLog("entity: " .. entityid .. " HoldRemove cardid: " .. cardid)
	end
	
	cc.StackDealAdd = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackDealAdd: " .. cardid)
	end
	cc.StackDealRemove = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackDealRemove: " .. cardid)
	end
	cc.StackDiscardAdd = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackDiscardAdd: " .. cardid)
	end
	cc.StackDiscardRemove = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackDiscardRemove: " .. cardid)
	end
	cc.StackExhaustAdd = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackExhaustAdd: " .. cardid)
	end
	cc.StackExhaustRemove = function(entityid, cardid) 
		--cc.WriteLog("entity: " .. entityid .. " StackExhaustRemove: " .. cardid)
	end

	cc.SetCurHP = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetCurHP: " .. value)
	end
	cc.SetCurMP = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetCurMP: " .. value)
	end
	cc.SetMaxHP = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetMaxHP: " .. value)
	end
	cc.SetMaxMP = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetMaxMP: " .. value)
	end	
	cc.SetStrength = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetStrength: " .. value)
	end
	cc.SetArmor = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetArmor: " .. value)
	end
	cc.SetShield = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " SetShield: " .. value)
	end

	cc.Damage = function(entityid, value) 
		--cc.WriteLog("entity: " .. entityid .. " Damage: " .. value)
	end
	cc.Die = function(entityid) 
		--cc.WriteLog("entity: " .. entityid .. " Die")
	end
	cc.Relive = function(entityid) end
	cc.MatchEnd = function(entityid, victory) 
		cc.WriteLog("entity: " .. entityid .. " victory: " .. (victory and "true" or "false"))
	end
	
	cc.BuffAdd = function(entityid, buff_baseid, buff_layers)
		identifier.buff = identifier.buff + 1
		return identifier.buff
	end
	cc.BuffRemove = function(entityid, buffid)
		--cc.WriteLog("remove buff, entity: " .. entityid .. ", buffid: " .. buffid)
	end
	cc.BuffUpdateLayers = function(entityid, buffid, buff_layers)
		--cc.WriteLog("update buff layers, entity: " .. entityid .. ", buffid: " .. buffid .. ", layers: " .. buff_layers)
	end

    local entity_baseid = 1001
	local entityid = cc.EntityNew(entity_baseid)
	members[entityid] = Player:new(entityid, entity_baseid)
	
	lua_entry_copy_enter(entityid, table.random(tables_copy, table.size(tables_copy)))

	while true do
		Sleep(1)
		lua_entry_update(1000)

		if g_copy.scene.match ~= nil then
    		if table.size(members[entityid].stack_hold) > 0 then
    			for cardid in pairs(members[entityid].stack_hold) do
    				lua_entry_card_play(members[entityid].id, cardid)
    				if g_copy.scene.match == nil or g_copy.scene.match.isdone then break end
    			end
    			if g_copy.scene.match ~= nil and not g_copy.scene.match.isdone then
        			table.clear(members[entityid].stack_hold)
        			lua_entry_round_end(members[entityid].id)
    			end
    		end
		else
    		local dest_coord = {
    			x = math.random_between(0, 14), y = math.random_between(0, 14)
    		}

    		local events = {}
    		for _, event in pairs(g_copy.scene.events) do
        		if event.accomplish == false 
        		    and event.base.category ~= EventCategory.ENTRY 
        		    and event.base.category ~= EventCategory.EXIT
        		    and event.base.category == EventCategory.MONSTER
        		then
        		    events[event.id] = event
        		end
    		end

    		local t_size = table.size(events)
    		if t_size > 0 then
            	local _, event = table.random(events, t_size)
            	assert(event.coord ~= nil)
            	dest_coord.x = event.coord.x
            	dest_coord.y = event.coord.y
            end        			
    		lua_entry_copy_move_request(entityid, members[entityid].sceneid, dest_coord.x, dest_coord.y)
		end
	end
end
unittest()

