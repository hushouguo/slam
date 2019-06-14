
-- table load, func: void func(baseid, t)
function table_load(dir, func)
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

local Member = {
	stack_hold = {},

	id = nil,
	baseid = nil,
	career = 1,
	cards = nil,

	constructor = function(self, id, baseid, cards)
		assert(cards ~= nil)
		self.id = id
		self.baseid = baseid
		self.cards = cards
		self.stack_hold = {}
	end
}

function Member:new(id, baseid, cards)
	local member = {}
	self.__index = self
	setmetatable(member, self)
	member:constructor(id, baseid, cards)
	return member
end

function init_common_interface()
	cc.WriteLog = function(content) print(content) end
	cc.Milliseconds = function() return os.time() end


	local tables_copy = {}
	table_load("Copy", function(baseid, t) tables_copy[baseid] = t end)

	local tables_map = {}
	table_load("Map", function(baseid, t) tables_map[baseid] = t end)

	local tables_obstacle = {}
	table_load("Obstacle", function(baseid, t) tables_obstacle[baseid] = t end)

	local tables_event = {}
	table_load("Event", function(baseid, t) tables_event[baseid] = t end)

	local tables_entity = {}
	table_load("Entity", function(baseid, t) tables_entity[baseid] = t end)

	local tables_card = {}
	table_load("Card", function(baseid, t) tables_card[baseid] = t end)

	local tables_buff = {}
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

	cc.GetBaseid = function(entityid) 
		assert(members[entityid] ~= nil)
		return members[entityid].baseid
	end

	cc.GetBuildCards = function(entityid) 
		assert(members[entityid] ~= nil)
		return members[entityid].cards
	end

	local records = {}
	cc.EntitySerialize = function(entityid, data)
		records[entityid] = data
	end
	cc.EntityUnserialize = function(entityid)
		return records[entityid]
	end

	cc.members = {
		[1] = Member:new(1, 1001, tables_entity[1001].init_cards),
		[2] = Member:new(2, 1002, tables_entity[1002].init_cards),
		[3] = Member:new(3, 1002, tables_entity[1002].init_cards),
	}

	local cardid_base = 10000
	cc.CardNew = function(entityid, card_baseid)
		assert(members[entityid] ~= nil)
		local cardid = cardid_base
		assert(members[entityid].cards[cardid] == nil)
		cardid_base = cardid_base + 1
		return cardid
	end
	cc.CardDestroy = function(entityid, cardid)
		assert(members[entityid] ~= nil)
		assert(members[entityid].cards[cardid] ~= nil)
		members[entityid].cards[cardid] = nil
	end

	local obstacleid_base = 100
	cc.ObstacleNew = function(obstacle_baseid)
		local obstacleid = obstacleid_base
		obstacleid_base = obstacleid_base + 1
		return obstacleid
	end
	cc.ObstacleDestroy = function(obstacleid)
	end

	local eventid_base = 1000
	cc.EventNew = function(event_baseid)
		local eventid = eventid_base
		eventid_base = eventid_base + 1
		return eventid
	end
	cc.EventDestroy = function(eventid)
	end

	local mapid_base = 10000
	cc.MapNew = function(map_baseid)
		local mapid = mapid_base
		mapid_base = mapid_base + 1
		return mapid
	end
	cc.MapDestroy = function(mapid)
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
		--cc.WriteLog("entity: " .. entityid .. " victory: " .. (victory and "true" or "false"))
	end
	
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
end
