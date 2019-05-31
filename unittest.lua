
require('match')

--
------------------- unittest -------------------
--

local function unittest()
	cc.WriteLog = function(content) print(content) end
	cc.Milliseconds = function() return os.time() end
	
	function entity_load(baseid)
		local chunk = loadfile("../tables/Entity/" .. tostring(baseid) .. ".lua")
		return chunk()
	end

	function card_load(baseid)
		local chunk = loadfile("../tables/Card/" .. tostring(baseid) .. ".lua")
		--print("card_load: " .. baseid)
		return chunk()
	end

	function buff_load(baseid)
		local chunk = loadfile("../tables/Buff/" .. tostring(baseid) .. ".lua")
		return chunk()
	end

	local tables_entity = {}
	tables_entity[1001] = entity_load(1001)
	tables_entity[1002] = entity_load(1002)
	
	local tables_card = {}
	tables_card[10000] = card_load(10000)
	tables_card[10011] = card_load(10011)
	tables_card[10021] = card_load(10021)
	tables_card[10031] = card_load(10031)
	tables_card[10041] = card_load(10041)
	tables_card[10051] = card_load(10051)
	tables_card[10010] = card_load(10010)
	tables_card[10020] = card_load(10020)
	tables_card[10030] = card_load(10030)
	tables_card[10040] = card_load(10040)
	tables_card[10050] = card_load(10050)
	tables_card[10060] = card_load(10060)
	tables_card[10070] = card_load(10070)
	tables_card[10080] = card_load(10080)
	--tables_card[10090] = card_load(10090)
	tables_card[19998] = card_load(19998)
	tables_card[19999] = card_load(19999)
	
	local tables_buff = {}
	tables_buff[20000] = buff_load(20000)
	tables_buff[20001] = buff_load(20001)
	tables_buff[20002] = buff_load(20002)
	tables_buff[20003] = buff_load(20003)
	tables_buff[20004] = buff_load(20004)

	--table.dump(tables_entity, "tables_entity")
	--table.dump(tables_card, "tables_card")
	--table.dump(tables_buff, "tables_buff")
	
	cc.LookupTable = function(table_name, baseid)
		local tables = {
			Entity = tables_entity,
			Card = tables_card,
			Buff = tables_buff
		}
		--print("table_name: " .. table_name .. ", baseid: " .. tostring(baseid))
		assert(tables[table_name] ~= nil)
		assert(tables[table_name][baseid] ~= nil)
		return tables[table_name][baseid]
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

	--table.dump(tables_entity[1001].init_cards, "entity:1001")

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
	
	local members = {
		[1] = Member:new(1, 1001, tables_entity[1001].init_cards),
		[2] = Member:new(2, 1002, tables_entity[1002].init_cards),
		[3] = Member:new(3, 1002, tables_entity[1002].init_cards),
	}

	cc.GetBaseid = function(entityid) 
		assert(members[entityid] ~= nil)
		return members[entityid].baseid
	end
	cc.GetBuildCards = function(entityid) 
		assert(members[entityid] ~= nil)
		return members[entityid].cards
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

    local buffid_base = 100
	cc.BuffAdd = function(entityid, buff_baseid, buff_layers)
		--cc.WriteLog("add buff, entity: " .. entityid .. ", baseid: " .. buff_baseid .. ", layers: " .. buff_layers)
		local buffid = buffid_base
		buffid_base = buffid_base + 1
		return buffid
	end
	cc.BuffRemove = function(entityid, buffid)
		--cc.WriteLog("remove buff, entity: " .. entityid .. ", buffid: " .. buffid)
	end
	cc.BuffUpdateLayers = function(entityid, buffid, buff_layers)
		--cc.WriteLog("update buff layers, entity: " .. entityid .. ", buffid: " .. buffid .. ", layers: " .. buff_layers)
	end
	
	
	lua_entry_match_prepare()
	lua_entry_add_member(1)
	lua_entry_add_opponent(2)
	lua_entry_add_opponent(3)
	lua_entry_match_start()

	local function Sleep(n)
		os.execute("sleep " .. n)
	end

	while true do
		Sleep(1)
		if table.size(members[1].stack_hold) > 0 then
			for cardid in pairs(members[1].stack_hold) do
				lua_entry_card_play(members[1].id, cardid)
				if g_match.isdone then os.exit(0) end
			end
			table.clear(members[1].stack_hold)
			lua_entry_round_end(members[1].id)
		end
		lua_entry_update(1000)
	end
end
unittest()
