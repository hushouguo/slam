--
-- match.lua
--


local cc = _G
local runassingle = true
local runasmultiple = false
local runonclient = true
local runonserver = false

--
-- dump table
table.dump = function(t)  
    local dump_cache={}
    local function sub_dump(t,indent)
        if (dump_cache[tostring(t)]) then
            cc.WriteLog(indent.."*"..tostring(t))
        else
            dump_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        cc.WriteLog(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_dump(val,indent..string.rep(" ",string.len(pos)+8))
                        cc.WriteLog(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        cc.WriteLog(indent.."["..pos..'] => "'..val..'"')
                    else
                        cc.WriteLog(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                cc.WriteLog(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        cc.WriteLog(tostring(t).." {")
        sub_dump(t,"  ")
        cc.WriteLog("}")
    else
        sub_dump(t,"  ")
    end
--    cc.WriteLog()
end


--
-- clear table
table.clear = function(t)
	assert(type(t) == "table")
	for k in pairs (t) do t[k] = nil end
end

--
-- get size of table
table.size = function(t)
	assert(type(t) == "table")
	local size = 0
	for k, v in pairs (t) do 
		if v ~= nil then size = size + 1 end
	end
	return size
end


--
-- Entity object
local Entity = {	
	id = nil, -- entity.id
	baseid = nil, -- entity.baseid
	base = nil, -- {field_name=field_value}, related entity.xls
	host = nil, -- userdata(host entity)
	side = nil, -- 0, 1: allies, enemy
	
	stack_base = nil, -- {card_baseid, card_baseid, ...}	
	stack_discard = nil, -- {card_baseid, card_baseid, ...}
	stack_discard_shuffle = function(self)
		for k, v in pairs(self.stack_discard) do
			self.stack_base
		end
		assert(table.size(cards_base) > 0)	
	end,
	
	stack_exhaust = nil, -- {card_baseid, card_baseid, ...},
	stack_exhaust_shuffle = function(self)
	end,
	
	cards_hold = nil, -- {card_baseid, card_baseid, ...}
	card_deal = function(self)
	end,
	card_play = function(self, card_baseid)
	end,	
	card_discard = function(self, card_baseid)
	end,

	round_end = function(self)
	end,
	
	buff = nil, -- {buff_baseid, buff_baseid, ...}
	buff_add = function(self, buff_baseid)
	end,
	buff_remove = function(self, buff_baseid)
	end,
	buff_effect = function(self)
	end,
	
	equip = nil, -- {[slot]=equip_baseid, [slot]=equip_baseid, ...}
	puppet = nil,		

	--
	-- arg: entity_host is userdata, related with entity of host layer
	constructor = function(self, entity_host, side)
		self.id = entity_host:ID()
		self.baseid = entity_host:Baseid()
		self.base = cc.LookupTable("entity", self.baseid)
		self.host = entity_host
		self.side = side
		assert(side == 0 or side == 1)
		assert(self.base ~= nil)
		self.stack_base = entity_host:GetBaseCards()
		assert(self.stack_base ~= nil and table.size(self.stack_base) > 0)
		self.stack_discard = {}
		self.stack_exhaust = {}
		self.cards_hold = {}
		self.buff = {}
		self.equip = {}
		self.puppet = {}
		-- init cards cache
		for k, card_baseid in pairs(self.stack_base) do
			if cards[card_baseid] == nil then
				cards[card_baseid] = cc.LookupTable("card", card_baseid)
				assert(cards[card_baseid] ~= nil)
			end
		end
	end
}

function Entity:new(entity_host, side)
	local entity = {}
	self.__index = self -- Entity.__index = function(key) return Entity[key] end
	setmetatable(entity, self)
	entity:constructor(entity_host, side)
	return entity
end

--
-- key: card_baseid, value: card
local cards = {}

--
-- key: entityid, value: Entity
local entities = {}

--
-- Match
local Match = {
	start = false, -- indicate whether the match has started
	
	round_entityid = nil, -- entity currently play card
	next_round_entity = function(self) -- Queue
		for k, v in pairs(side_allies) do
			assert(v ~= nil)
			return k -- entityid
		end
		return nil		
	end,

	constructor = function(self)
		self.start = false
		self.round_entityid = nil
		self:next_round_entity()
	end
}

function Match:new()
	local match = {}
	self.__index = self -- Match.__index = function(key) return Match[key] end
	setmetatable(match, self)
	match:constructor()
	return match
end

--
-- current match instance
local match = nil

--
-- entity currently play card
local round_entityid = nil
local function entity_init_round()
	assert(match_start)
	for k, v in pairs(side_allies) do
		assert(v ~= nil)
		return k -- entityid
	end
	return nil
end


--
-- prepare match
function lua_entry_match_prepare()
	match = Match:new()
	table.clear(cards)
	table.clear(entities)
	cc.WriteLog("match prepare")
end

--
-- add member of allies to match
function lua_entry_add_member(entity_host)
	assert(not match and not match.start)
	assert(type(entity_host) == "userdata")
	local entity = Entity:new(entity_host, 0)
	entities[entity.id] = entity
	cc.WriteLog("add member: " .. entity.id)
	table.dump(entity)
end

--
-- add member of enemy to match
function lua_entry_add_opponent(entity_host)
	assert(not match and not match.start)
	assert(type(entity_host) == "userdata")
	local entity = Entity:new(entity_host, 1)
	entities[entity.id] = entity
	cc.WriteLog("add opponent: " .. entity.id)
	table.dump(entity)
end

--
-- start match
function lua_entry_match_start()
	assert(not match and not match.start)
	assert(table.size(entities) > 0)
	match.start = true
	round_entityid = entity_init_round()
	assert(round_entityid ~= nil)
	-- TODO: need new api to notify host, which entity is round
	cc.WriteLog("match start, entities: " .. table.size(entities) .. ", round: " .. round_entityid)
	local entity = entities[round_entityid]
	entity:card_deal()
end

--
-- the player play a card
function lua_entry_card_play(entityid, card_baseid)
	assert(match and match.start)
	assert(round_entityid == entityid)
	cc.WriteLog("play card, entityid: " .. entityid .. ", card: " .. card_baseid)
	local entity = entities[round_entityid]
	entity:card_play(card_baseid)
end

--
-- the player discard a card
function lua_entry_card_discard(entityid, baseid)
	assert(match and match.start)
	assert(round_entityid == entityid)
	cc.WriteLog("discard card, entityid: " .. entityid .. ", card: " .. card_baseid)
	local entity = entities[round_entityid]
	entity:card_discard(card_baseid)
end

--
-- the player end this round
function lua_entry_round_end(entityid)
	assert(match and match.start)
	assert(round_entityid == entityid)
	cc.WriteLog("round end, entityid: " .. entityid)
	local entity = entities[round_entityid]
	entity:round_end()
end

--
-- call me for every frame
function lua_entry_update(delta)
	--if not match_start then return end
end

