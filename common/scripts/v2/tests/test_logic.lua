--
-- test_logic.lua
--

require('main')

--
------------------- unittest -------------------
--

--
-- n: 1 means sleep 1 second
-- n: 0.01 means sleep 10 ScriptMilliseconds
--
local function Sleep(n)
	os.execute("sleep " .. n)
end

-- table load, func: void func(baseid, t)
local function table_load_linux(dir, func)
	local PATH = "../../tables/" .. dir
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
			assert(chunk ~= nil, 'file: ' .. tostring(file))
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
	pack = nil,
	sceneid = nil,
	coord = nil,
	
	career = nil,	
	stack_hold = nil,
	death = nil,

	constructor = function(self, id, baseid)
		self.id = id
		self.baseid = baseid
		self.base = cc.ScriptLookupTable("Entity", baseid)
		assert(self.base ~= nil)
		
		self.record = nil
		self.pack = {
		    cards = {}, -- {[cardid] = card, ...}
		    items = {}, -- {[itemid] = item, ...}
			equips = {},-- {[slot] = item, ...}
		    gold = 0,
		    puppets = {}, -- {[entityid] = entity, ...}
			placeholders = {}, -- {[1] = entity}
			buffs = {} -- {[buffid] = buff}
		}
		self.sceneid = nil
		self.coord = nil

		self.career = 1
		self.stack_hold = {}
		self.death = false
	end
}

function Player:new(id, baseid)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(id, baseid)
	return object
end

local exit_copy = false
local entities = {} -- {[entityid] = Player, ...}
local entityid_master = nil -- entity_master.id
local receive_touch_event = false

local function unittest()
    local logfile = io.open("./.logic.log", "w")
    
	cc.ScriptDebugLog = function(content) 
		if os.name() == 'linux' then
	    	console.resetcolor()
		end
	    print(content) 
	    logfile:write(content .. "\n")
	end
	cc.ScriptErrorLog = function(content)
		if os.name() == 'linux' then
			console.setcolor(CONSOLE_COLOR.LRED)
			print(content)
			console.resetcolor()
		else
			print(content)
		end
	    logfile:write(content .. "\n")
	end
	cc.ScriptMilliseconds = function() return os.time() end

	local tables_copy = {}
	local tables_map = {}
	local tables_obstacle = {}
	local tables_event = {}
	local tables_entity = {}
	local tables_card = {}
	local tables_item = {}
	local tables_buff = {}
	local tables_storyoption = {}
	
	table_load("Copy", function(baseid, t) tables_copy[baseid] = t end)
	table_load("Map", function(baseid, t) tables_map[baseid] = t end)
	table_load("Obstacle", function(baseid, t) tables_obstacle[baseid] = t end)
	table_load("Event", function(baseid, t) tables_event[baseid] = t end)
	table_load("Entity", function(baseid, t) tables_entity[baseid] = t end)
	table_load("Card", function(baseid, t) tables_card[baseid] = t end)
	table_load("Item", function(baseid, t) tables_item[baseid] = t end)
	table_load("Buff", function(baseid, t) tables_buff[baseid] = t end)
	table_load("StoryOption", function(baseid, t) tables_storyoption[baseid] = t end)

	cc.ScriptLookupTable = function(table_name, baseid)
		local tables = {
			Copy = tables_copy,
			Map = tables_map,
			Obstacle = tables_obstacle,
			Event = tables_event,
			Entity = tables_entity,
			Card = tables_card,
			Item = tables_item,
			Buff = tables_buff,
			StoryOption = tables_storyoption,
		}
		assert(tables[table_name] ~= nil)
		assert(tables[table_name][baseid] ~= nil, string.format("table_name: %s, baseid: %s", table_name, tostring(baseid)))
		return tables[table_name][baseid]
	end
    
    local identifier = {
        entity = 100,
        map = 200,
        obstacle = 300,
        event = 400,
        card = 500,
        item = 600,
        buff = 700
    }

	cc.ScriptGetEntityBaseid = function(entityid) 
		assert(entities[entityid] ~= nil)
		return entities[entityid].baseid
	end

    ----------------------------------------------------------
    
	cc.ScriptMapNew = function(map_baseid)
	    identifier.map = identifier.map + 1
	    return identifier.map
	end
	cc.ScriptMapDestroy = function(mapid)
	end

	cc.ScriptEntityNew = function(entity_baseid)
	    identifier.entity = identifier.entity + 1
		assert(entities[identifier.entity] == nil)
		entities[identifier.entity] = Player:new(identifier.entity, entity_baseid)
	    return identifier.entity
	end
	cc.ScriptEntityDestroy = function(entityid)
	    entities[entityid] = nil
	end
    
	cc.ScriptObstacleNew = function(obstacle_baseid)
	    identifier.obstacle = identifier.obstacle + 1
	    return identifier.obstacle
	end
	cc.ScriptObstacleDestroy = function(obstacleid)
	end

	cc.ScriptEventNew = function(event_baseid)
	    identifier.event = identifier.event + 1
	    return identifier.event
	end
	cc.ScriptEventDestroy = function(eventid)
	end
	
	cc.ScriptCardNew = function(entityid, card_baseid)
		assert(entityid ~= nil and card_baseid ~= nil)
	    identifier.card = identifier.card + 1
	    return identifier.card
	end
	cc.ScriptCardDestroy = function(entityid, cardid)
	end

	cc.ScriptItemNew = function(entityid, item_baseid)
		assert(entityid ~= nil and item_baseid ~= nil)
	    identifier.item = identifier.item + 1
	    return identifier.item
	end
	cc.ScriptItemDestroy = function(entityid, itemid)
	end

    ----------------------------------------------------------------

    cc.ScriptSetCurHP = function(entityid, value) end
	cc.ScriptSetCurMP = function(entityid, value) end
	cc.ScriptSetMaxHP = function(entityid, value) end
	cc.ScriptSetMaxMP = function(entityid, value) end
	cc.ScriptSetStrength = function(entityid, value) end
	cc.ScriptSetArmor = function(entityid, value) end
	cc.SetShield = function(entityid, value) end

    ----------------------------------------------------------------

	cc.ScriptBuffAdd = function(entityid, buff_baseid, buff_layers)
		identifier.buff = identifier.buff + 1
		return identifier.buff
	end
	cc.ScriptBuffRemove = function(entityid, buffid) end
	cc.ScriptBuffUpdateLayers = function(entityid, buffid, buff_layers) end

    ----------------------------------------------------------------

    cc.ScriptEquipBarAdd = function(entityid, slot, itemid)
        assert(entities[entityid].pack.equips[slot] == nil)
        entities[entityid].pack.equips[slot] = itemid
    end    
    cc.ScriptEquipBarRemove = function(entityid, slot)
        assert(entities[entityid].pack.equips[slot] ~= nil)
        entities[entityid].pack.equips[slot] = nil
    end

    ----------------------------------------------------------------

	cc.ScriptBagAddCard = function(entityid, cardid)
	    assert(entities[entityid].pack.cards[cardid] == nil)
	    entities[entityid].pack.cards[cardid] = cardid
	end
	cc.ScriptBagRemoveCard = function(entityid, cardid)
	    assert(entities[entityid].pack.cards[cardid] ~= nil)
	    entities[entityid].pack.cards[cardid] = nil
	end
	cc.ScriptBagAddItem = function(entityid, itemid)
	    assert(entities[entityid].pack.items[itemid] == nil)
	    entities[entityid].pack.items[itemid] = itemid
	end
	cc.ScriptBagRemoveItem = function(entityid, itemid)
	    assert(entities[entityid].pack.items[itemid] ~= nil)
	    entities[entityid].pack.items[itemid] = nil
	end
    cc.ScriptBagSetItemNumber = function(entityid, itemid, item_number)
    end
	cc.ScriptBagSetGold = function(entityid, gold_new, gold_old)
	    entities[entityid].pack.gold = gold_new
	end
    cc.ScriptBagAddPuppet	= function(entityid, target_entityid)
        assert(entities[entityid].pack.puppets[target_entityid] == nil)
        entities[entityid].pack.puppets[target_entityid] = target_entityid
    end
    cc.ScriptBagRemovePuppet = function(entityid, target_entityid)
        assert(entities[entityid].pack.puppets[target_entityid] ~= nil)
        entities[entityid].pack.puppets[target_entityid] = nil
    end
    cc.ScriptBagArrangePlaceholder = function(entityid, target_entityid, placeholder)
        if target_entityid == 0 then
            assert(entities[entityid].pack.placeholders[placeholder] ~= nil)
            entities[entityid].pack.placeholders[placeholder] = nil
        else
            assert(entities[entityid].pack.placeholders[placeholder] == nil)
		    entities[entityid].pack.placeholders[placeholder] = target_entityid
		end
    end
    cc.ScriptBagClearCard = function(entityid)
        table.clear(entities[entityid].pack.cards)
    end
    cc.ScriptBagClearItem = function(entityid)
        table.clear(entities[entityid].pack.items)
    end
    cc.ScriptBagClearPuppet = function(entityid)
        table.clear(entities[entityid].pack.puppets)
        table.clear(entities[entityid].pack.placeholders)
    end
    cc.ScriptBagClearEquip = function(entityid)
        table.clear(entities[entityid].pack.equips)
    end

    ----------------------------------------------------------------
    
	cc.ScriptCardSetField = function(entityid, cardid, name, value) end
	cc.ScriptBuffSetField = function(entityid, buffid, name, value) end
    cc.ScriptItemSetField = function(entityid, itemid, name, value) end
    
    ----------------------------------------------------------------
	
	cc.ScriptEntitySerialize = function(entityid, string)
	    assert(entities[entityid] ~= nil)
		entities[entityid].record = string
        local recordfile = io.open("./.record." .. tostring(entityid), "w")
        recordfile:write(string)
        io.close(recordfile)		
	end
	cc.ScriptEntityUnserialize = function(entityid)
	    assert(entities[entityid] ~= nil)
		if entities[entityid].record == nil then
		    local recordfile = io.open("./.record." .. tostring(entityid), "r")
		    if recordfile ~= nil then
		        entities[entityid].record = recordfile:read("*all")
		        recordfile:close()
		    end
		end
		return entities[entityid].record
	end

    ----------------------------------------------------------------

	cc.ScriptEnterMap = function(entityid, mapid, layouts, coord)
	    entities[entityid].sceneid = mapid
	    entities[entityid].coord = coord
	end
	cc.ScriptExitMap = function(entityid, mapid)
	end
    cc.ScriptExitCopy = function(entityid)
        -- os.exit()
		exit_copy = true
        cc.ScriptDebugLog('ExitCopy')
    end
    
    ----------------------------------------------------------------

	cc.ScriptTriggerMove = function(entityid, v)
		for _, coord in pairs(v) do
			script_entry_scene_move(entityid, coord.x, coord.y)
			-- cc.ScriptDebugLog(string.format("entityid: %d move to (%d,%d)", entityid, coord.x, coord.y))
		end
	end
	cc.ScriptTriggerEventMonster = function(entityid, mapid, eventid, content)		
    	script_entry_match_prepare()
    	script_entry_match_start()
    	receive_touch_event = true
	end
	cc.ScriptTriggerEventReward = function(entityid, eventid, content, is_update)
	    assert(table.size(content) > 0)
	    local reward_index = table.random(content, table.size(content))
	    assert(content[reward_index] ~= nil)	    
	    local reward = content[reward_index]
	    local entry_index = nil
	    if reward.cards ~= nil then
	        assert(table.size(reward.cards) > 0)
	        -- _ = card_baseid
	        entry_index, _ = table.random(reward.cards, table.size(reward.cards))
	    elseif reward.items ~= nil then
	        assert(table.size(reward.items) > 0)
	        -- item_baseid = item_number
	        entry_index, _ = table.random(reward.items, table.size(reward.items))
	    elseif reward.gold ~= nil then
	        entry_index = 0 -- ineffective
	    elseif reward.puppets ~= nil then
	        assert(table.size(reward.puppets) > 0)
	        -- _ = entity_baseid
	        entry_index, _ = table.random(reward.puppets, table.size(reward.puppets))
	    else
	        cc.ScriptErrorLog(string.format(">>>> no reward"))	        
	    end
	    if entry_index ~= nil then
	        script_entry_event_reward(entityid, eventid, reward_index, entry_index)
	    end
		receive_touch_event = true
	end
	cc.ScriptTriggerEventShop = function(entityid, eventid, content, is_update)
		if content ~= nil and content.cards ~= nil and table.size(content.cards) > 0 then
		    -- cards: { { [card_baseid] = price_gold }, ...}
		    local _, t = table.random(content.cards, table.size(content.cards))
		    if table.size(t) > 0 then		    
    		    local card_baseid, price_gold = table.random(t, table.size(t))
        		script_entry_event_purchase_card(entityid, eventid, card_baseid)
    		end
		end
		
		receive_touch_event = true
	end
	cc.ScriptTriggerEventOption = function(entityid, eventid, content)
		if content ~= 0 and content ~= nil then
        	script_entry_event_choose_option(entityid, eventid, content, 1)
		end
		
		receive_touch_event = true
	end
	cc.ScriptTriggerEventStory = function(entityid, eventid, content)
		receive_touch_event = true
	end
	cc.ScriptTriggerEventDestroyCard = function(entityid, eventid, content)
		local t_size = table.size(entities[entityid].pack.cards)
		if t_size > 0 then
    		local cardid, _ = table.random(entities[entityid].pack.cards, t_size)
    		script_entry_event_destroy_card(entityid, eventid, cardid)
		end
		
		receive_touch_event = true
	end
	cc.ScriptTriggerEventLevelupCard = function(entityid, eventid, content)
		local t_size = table.size(entities[entityid].pack.cards)
		if t_size > 0 then
    		local cardid, _ = table.random(entities[entityid].pack.cards, t_size)
    		script_entry_event_levelup_card(entityid, eventid, cardid)
		end

		receive_touch_event = true
	end
	cc.ScriptTriggerEventLevelupPuppet = function(entityid, eventid, content)
		receive_touch_event = true
	end
	cc.ScriptTriggerEventAccomplish = function(entityid, eventid)
	end

    ----------------------------------------------------------------
	
	cc.ScriptMatchEntityRoundBegin = function(entityid, rounds) end
	cc.ScriptMatchEntityCardDeal = function(entityid, cardid) end
    cc.ScriptMatchEntityCardPlay = function(entityid, cardid, targets) end-- targets: {target_entityid, ...}
    cc.ScriptMatchEntityCardDiscard = function(entityid, cardid) end
    cc.ScriptMatchEntityRoundEnd = function(entityid) end

    ----------------------------------------------------------------
    
    cc.ScriptMatchEventDamage = function(entityid, value, type) end
    cc.ScriptMatchEventRecovery = function(entityid, value, type) end
    cc.ScriptMatchEventDie = function(entityid) 
	    entities[entityid].death = true
    end

    ----------------------------------------------------------------
    
	cc.ScriptMatchStackDealAdd = function(entityid, cardid) end
	cc.ScriptMatchStackDealRemove = function(entityid, cardid) end
	cc.ScriptMatchStackDiscardAdd = function(entityid, cardid) end
	cc.ScriptMatchStackDiscardRemove = function(entityid, cardid) end
	cc.ScriptMatchStackExhaustAdd = function(entityid, cardid) end
	cc.ScriptMatchStackExhaustRemove = function(entityid, cardid) end
	cc.ScriptMatchStackHoldAdd = function(entityid, cardid)
	    assert(entities[entityid].stack_hold[cardid] == nil)
		entities[entityid].stack_hold[cardid] = cardid
	end
	cc.ScriptMatchStackHoldRemove = function(entityid, cardid)
--	    assert(entities[entityid].stack_hold[cardid] ~= nil, string.format("entityid: %d, cardid: %d", entityid, cardid))
		entities[entityid].stack_hold[cardid] = nil
	end	

    ----------------------------------------------------------------
	
	cc.ScriptMatchEnd = function(entityid, victory) 
		receive_touch_event = true
		-- reset match.info
		entities[entityid].death = false
	end
	
    ----------------------------------------------------------------

	entityid_master = cc.ScriptEntityNew(1001)
	script_entry_copy_enter(entityid_master, table.random(tables_copy, table.size(tables_copy)))

    local function match_loop()
        -- use item & equip
        for itemid, item in pairs(g_copy.entity_master.pack.items) do
            local rc = false
            if item.base.category == ItemCategory.USABLE then
                rc = script_entry_item_use(g_copy.entity_master.id, item.id)
            end
            if item.base.category == ItemCategory.EQUIPABLE and table.size(g_copy.entity_master.pack.equips) < g_copy.entity_master.base.max_equip_slots then
                rc = script_entry_item_use(g_copy.entity_master.id, item.id)
            end
            if not rc then break end
        end
                
        -- card play
        while table.size(entities[entityid_master].stack_hold) > 0 
                and g_copy.scene.match ~= nil and not g_copy.scene.match.isdone do
            local cardid = table.random(entities[entityid_master].stack_hold, table.size(entities[entityid_master].stack_hold))
			assert(cardid ~= nil)
            if script_entry_match_card_play_judge(entityid_master, cardid, nil) then
                script_entry_match_card_play(entityid_master, cardid, nil)
            elseif script_entry_match_card_discard_judge(entityid_master, cardid) then
                script_entry_match_card_discard(entityid_master, cardid)
            else
                entities[entityid_master].stack_hold[cardid] = nil -- destroy card
            end
        end
    	if g_copy.scene.match ~= nil and not g_copy.scene.match.isdone then
    		script_entry_match_round_end(entityid_master)
    	end
    end

    local function copy_explore()
		--
		-- event lookup_normal_event()
		--
		local function lookup_normal_event()
        	for _, event in pairs(g_copy.scene.events) do
        	    if not event.trigger 
        			and event.base.category ~= EventCategory.ENTRY 
        			and event.base.category ~= EventCategory.EXIT
        	    then
        	        local rc = script_entry_scene_move_request(entityid_master, event.coord.x, event.coord.y)
        	        if rc then
						cc.ScriptErrorLog(string.format("lock normal event: %d, %s, coord: (%d,%d)", event.baseid, event.base.name.cn, event.coord.x, event.coord.y))
        	            return event
        	        end
        	    end
        	end
			return nil
		end

		--
		-- event lookup_exit_event()
		--
		local function lookup_exit_event()
        	for _, event in pairs(g_copy.scene.events) do
        	    if not event.trigger and event.base.category == EventCategory.EXIT then
        	        local rc = script_entry_scene_move_request(entityid_master, event.coord.x, event.coord.y)
        	        if rc then
						cc.ScriptErrorLog(string.format("lock exit event: %d, %s", event.baseid, event.base.name.cn))
						return event
        	        end
        	    end
        	end
			return nil
		end

		local entity = entities[entityid_master]
		local lock_event = lookup_normal_event() -- find MONSTER or SHOP event
		if lock_event == nil then
		    lock_event = lookup_exit_event() -- find EXIT event
		end
		if lock_event ~= nil then
			local rc = script_entry_scene_move_request(entity.id, lock_event.coord.x, lock_event.coord.y)
			assert(rc)
		end
    end

    local function arrange_placeholder(placeholder)
        if g_copy.entity_master.pack.placeholders[placeholder] ~= nil then return end
        if table.size(g_copy.entity_master.pack.puppets) == 0 then return end
        local target_entityid, _ = table.random(g_copy.entity_master.pack.puppets, table.size(g_copy.entity_master.pack.puppets))
        script_entry_puppet_arrange_placeholder(entityid_master, target_entityid, placeholder)
    end
   
	local ScriptMilliseconds = 10 -- ScriptMilliseconds
	while not exit_copy do
		Sleep(ScriptMilliseconds / 1000)
		script_entry_update(ScriptMilliseconds / 1000)

		if g_copy ~= nil and g_copy.scene ~= nil and not exit_copy then
			if g_copy.scene.match ~= nil then
			    match_loop()
			else
				if receive_touch_event then
					receive_touch_event = false
    				script_entry_event_trigger_linked(entityid_master)
				else
				    arrange_placeholder(Placeholder.FRONT)
				    arrange_placeholder(Placeholder.MIDDLE)
				    arrange_placeholder(Placeholder.BACK)
			    	copy_explore()
				end
			end
		end
	end

	cc.ScriptDebugLog("unittest exit")
	
	io.close(logfile)
end
unittest()

