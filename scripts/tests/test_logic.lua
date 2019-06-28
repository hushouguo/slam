--
-- test_logic.lua
--

require('main')

--
------------------- unittest -------------------
--

--
-- n: 1 means sleep 1 second
-- n: 0.01 means sleep 10 milliseconds
--
local function Sleep(n)
	os.execute("sleep " .. n)
end

-- table load, func: void func(baseid, t)
local function table_load_linux(dir, func)
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

-- table load, func: void func(baseid, t)
local function table_load_windows(dir, func)
	local lfs = require 'lfs'
	local PATH = "D:\\SVN2\\LuaProj\\tables\\" .. dir 
	for file in lfs.dir(PATH) do
		local path = PATH .. "\\" .. file
		if file ~= "." and file ~= ".." then
			local chunk = loadfile(path)
			local baseid = string.split(file,".")[1]
			func(tonumber(baseid), chunk())
		end
	end
end

-- table load, func: void func(baseid, t)
local function table_load(dir, func)
    if os.name() == 'linux' then
        table_load_linux(dir, func)
    else
        table_load_windows(dir, func)
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
	stack_hold = nil,

	lock_event = nil,

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

		self.lock_event = nil
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
    local logfile = io.open("./logic.log", "w")
    
	cc.WriteLog = function(content) 
	    print(content) 
	    logfile:write(content .. "\n")
	end
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

	local exit_copy = false
    local members = {} -- {[entityid] = Player, ...}
	local entityid = nil
    
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
	    cc.WriteLog(string.format("CardDestroy: %d", cardid))
	end

	cc.CardSetField = function(entityid, cardid, name, value)
	    cc.WriteLog(string.format("CardSetField, entityid: %d, cardid: %d, set name: %s to value: %s", 
	            entityid, cardid, tostring(name), tostring(value)))
	end

	cc.BuffSetField = function(entityid, buffid, name, value)
	    cc.WriteLog(string.format("BuffSetField, entityid: %d, buffid: %d, set name: %s to value: %s", 
	            entityid, buffid, tostring(name), tostring(value)))
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
	    members[entityid].bag.cards[cardid] = cardid
	end
	cc.BagRemoveCard = function(entityid, cardid)
	    members[entityid].bag.cards[cardid] = nil
	end

	cc.BagAddItem = function(entityid, itemid)
	    members[entityid].bag.items[itemid] = itemid
	end
	cc.BagRemoveItem = function(entityid, itemid)
	    members[entityid].bag.items[itemid] = nil
	end

	cc.BagSetGold = function(entityid, gold_new, gold_old)
	    assert(members[entityid] ~= nil)
	    members[entityid].bag.gold = gold_new
	    cc.WriteLog(string.format("BagSetGold from: %d to %d", gold_old, gold_new))
	end

	--------------------------------- Copy -----------------------------------

    cc.ExitCopy = function(entityid)
        cc.WriteLog('ExitCopy')
        -- os.exit()
		exit_copy = true
    end
    
	cc.EnterMap = function(entityid, mapid, layouts, coord)
	    assert(members[entityid] ~= nil)
	    members[entityid].sceneid = mapid
	    members[entityid].coord = coord
	    cc.WriteLog('EnterMap: ' .. tostring(mapid))
	end

	cc.ExitMap = function(entityid, mapid)
		cc.WriteLog('ExitMap')
		members[entityid].lock_event = nil
	end

	cc.MoveTrigger = function(entityid, mapid, v)
	    assert(members[entityid] ~= nil)
	    --table.dump(v, "MoveTrigger")
	    --cc.WriteLog("MoveTrigger")
		for _, coord in pairs(v) do
			lua_entry_copy_move(entityid, members[entityid].sceneid, coord.x, coord.y)
		end
	end

	cc.EventMonsterTrigger = function(entityid, mapid, eventid, content)
		cc.WriteLog("EventMonsterTrigger: " .. tostring(eventid))
		table.clear(members[entityid].stack_hold)
    	lua_entry_match_start()
	end
	cc.EventShopTrigger = function(entityid, mapid, eventid, content)
		cc.WriteLog("EventShopTrigger: " .. tostring(eventid))
		if content ~= nil and content.cards ~= nil then
		    local card_baseid, price_gold = table.random(content.cards, table.size(content.cards))
    		lua_entry_copy_purchase_card(entityid, mapid, eventid, card_baseid)
		end
	end
	cc.EventOptionTrigger = function(entityid, mapid, eventid, content)
		cc.WriteLog("EventOptionTrigger: " .. tostring(eventid))
		cc.WriteLog(string.format("title: %s", content.title))
		cc.WriteLog(string.format("text: %s", content.text))
		for _, option in pairs(content.options) do
		    cc.WriteLog(string.format("[%s] : %s", _, option))    
		end
	end
	cc.EventDestroyCardTrigger = function(entityid, mapid, eventid, content)
		cc.WriteLog("EventDestroyCardTrigger: " .. tostring(eventid))
		local t_size = table.size(members[entityid].bag.cards)
		if t_size > 0 then
    		--local cardid, _ = table.random(members[entityid].bag.cards, t_size)
    		--lua_entry_copy_destroy_card(entityid, mapid, eventid, cardid)
		end
	end

	cc.AccomplishEvent = function(entityid, mapid, eventid)
		cc.WriteLog("AccomplishEvent: " .. tostring(eventid))		
		local event = g_copy.scene.events[eventid]
		if event ~= nil and event.base.category == EventCategory.MONSTER then
		    lua_entry_copy_event_reward(entityid, mapid, eventid, 1, 1)
		end
    	members[entityid].lock_event = nil
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

	cc.Damage = function(entityid, value, type) 
		cc.WriteLog(string.format("entityid: %d, Damage value: %d, type:%s, func: %s", entityid, value, tostring(type), Function()))
	end
	cc.Recovery = function(entityid, value, type) 
		cc.WriteLog(string.format("entityid: %d, Recovery value: %d, type:%s, func: %s", entityid, value, tostring(type), Function()))
	end
	cc.BuffTrigger = function(entityid, buffid)
	    cc.WriteLog(string.format("entityid: %d, buffid: %d, func: %s", entityid, buffid, Function()))
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

	entityid = cc.EntityNew(1001)
	lua_entry_copy_enter(entityid, table.random(tables_copy, table.size(tables_copy)))

    local function match_loop()
        while table.size(members[entityid].stack_hold) > 0 
                and g_copy.scene.match ~= nil and not g_copy.scene.match.isdone do
            local t_size = table.size(members[entityid].stack_hold)
            assert(t_size > 0)
            local cardid = table.random(members[entityid].stack_hold, t_size)
            assert(cardid ~= nil)
            if lua_entry_card_play_judge(entityid, cardid, nil) then
                lua_entry_card_play(entityid, cardid, nil)
            elseif lua_entry_card_discard_judge(entityid, cardid) then
                lua_entry_card_discard(entityid, cardid)
            else
                members[entityid].stack_hold[cardid] = nil -- destroy card
            end
        end
    	if g_copy.scene.match ~= nil and not g_copy.scene.match.isdone then
    		table.clear(members[entityid].stack_hold)
    		lua_entry_round_end(entityid)
    	end
    end

    local function copy_explore()
		--
		-- event looking_normal_event()
		--
		local function looking_normal_event()
        	for _, event in pairs(g_copy.scene.events) do
        	    if not event.accomplish 
        			and event.base.category ~= EventCategory.ENTRY 
        			and event.base.category ~= EventCategory.EXIT
        	    then
        	        local rc = lua_entry_copy_move_request(entityid, members[entityid].sceneid, event.coord.x, event.coord.y)
        	        if rc then
						cc.WriteLog(string.format("lock event: %d, %s", event.baseid, event.base.name.cn))
        	            return event
        	        end
        	    end
        	end
			return nil
		end

		--
		-- event looking_exit_event()
		--
		local function looking_exit_event()
        	for _, event in pairs(g_copy.scene.events) do
        	    if not event.accomplish and event.base.category == EventCategory.EXIT then
        	        local rc = lua_entry_copy_move_request(entityid, members[entityid].sceneid, event.coord.x, event.coord.y)
        	        if rc then
						cc.WriteLog(string.format("lock event: %d, %s", event.baseid, event.base.name.cn))
						return event
        	        end
        	    end
        	end
			return nil
		end

		local member = members[entityid]
        if member.lock_event == nil then
			member.lock_event = looking_normal_event() -- find MONSTER or SHOP event
			if member.lock_event == nil then
				member.lock_event = looking_exit_event() -- find EXIT event
			end

			assert(member.lock_event ~= nil)

			local src_coord = g_copy.members[entityid].coord
			local dest_coord = member.lock_event.coord
			cc.WriteLog(string.format("lock event: (%d, %d), src: (%d,%d)", dest_coord.x, dest_coord.y, src_coord.x, src_coord.y))
		else
    		cc.WriteLog(string.format("lock event: %d, %s, try to move_request: (%d,%d)", member.lock_event.baseid, member.lock_event.base.name.cn, member.lock_event.coord.x, member.lock_event.coord.y))

			local rc = lua_entry_copy_move_request(entityid, member.sceneid, member.lock_event.coord.x, member.lock_event.coord.y)
			assert(rc)
			member.lock_event = nil -- reset lock_event
    	end
    end
   
	local milliseconds = 10 -- milliseconds
	while not exit_copy do
		Sleep(milliseconds / 1000)
		lua_entry_update(milliseconds / 1000)

		if g_copy ~= nil and g_copy.scene ~= nil and not exit_copy then
			if g_copy.scene.match ~= nil then
			    match_loop()
			else
			    copy_explore()
			end
		end
	end

	cc.WriteLog("unittest exit")
	
	io.close(logfile)
end
unittest()

