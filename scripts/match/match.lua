--
-- match.lua
--

require('common')
require('tools/tools')
require('match/buff')
require('match/card')
require('match/entity')
require('match/monster')

--
------------------- Match class -------------------
--

local Match = {
	isstart = false, -- indicate whether the match has started
	isdone = false, -- indicate whether the match has done
	
	--
	-- entity instances
	entities = {}, -- key: entityid, value: Entity instance
	entity_size = function(self, side)
		local size = 0
		for _, entity in pairs(self.entities) do
			if entity.side == side then size = size + 1 end
		end
		return size
	end,
	entity_side = function(self, side)
		local t = {}
		for _, entity in pairs(self.entities) do
			if entity.side == side then t[entity.id] = entity end
		end
		return t
	end,
	dead_entities = {}, -- key: entityid, value: Entity instance

	--
	-- round list
	round_total = 0, -- 
	round_entityid = nil,
	round_side = Side.ALLIES,
	init_round_list = function(self)
		self.round_total = 0
		self.round_entityid = nil
		self.round_side = Side.ALLIES
		for _, entity in pairs(self.entities) do
			entity.round_total = 0
		end
	end,		
	next_round_entity = function(self)
		self.round_total = self.round_total + 1
		local round_entity = nil
		local round_total = 0
		for _, entity in pairs(self.entities) do
			if entity.side == self.round_side and 
				(round_entity == nil or entity.round_total < round_total) 
			then
				round_entity = entity
				round_total = entity.round_total
			end
		end
		assert(round_entity ~= nil and not round_entity.death)
		round_entity.round_total = round_entity.round_total + 1
		self.round_entityid = round_entity.id
		self.round_side = (self.round_side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
		Debug(nil, nil, nil, "*************************************** 回合: " .. self.round_total .. ", entity: " .. self.round_entityid .. " ******************************")
		return self.round_entityid
	end,
	remove_round_entity = function(self, entityid)
	    assert(entityid ~= nil)
		assert(self.entities[entityid] == nil)
		assert(self.dead_entities[entityid] ~= nil)
	    Debug(nil, nil, nil, "remove round entityid: " .. tostring(entityid))
	end,

	--
	-- bool card_play_judge(entityid, cardid, pick_entityid)
	--
	card_play_judge = function(self, entityid, cardid, pick_entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end
		
		return self.entities[entityid]:card_play_judge(cardid, pick_entityid)
	end,
	
	--
	-- bool card_play(entityid, cardid, pick_entityid)
	--
	card_play = function(self, entityid, cardid, pick_entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_play(cardid, pick_entityid)
	end,

	--
	-- bool card_discard_judge(entityid, cardid)
	--
	card_discard_judge = function(self, entityid, cardid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_discard_judge(cardid)
	end,
	
	--
	-- bool card_discard(entityid, cardid)
	--
	card_discard = function(self, entityid, cardid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return false
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return false
		end

		return self.entities[entityid]:card_discard(cardid, false)
	end,

	--
	-- void abort()
	-- 
	abort = function(self, entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		self:done(self.entities[entityid].side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
	end,	

	--
	-- void update()
	--
	update = function(self, delta)
		if not self.isstart or self.isdone then
			--Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		
		for _, entity in pairs(self.entities) do
			entity:update(delta)
		end
	end,

	--
	-- void round_end(entityid)
	--
	round_end = function(self, entityid)
		if not self.isstart or self.isdone then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end

		if entityid ~= self.round_entityid then
			Error(nil, nil, nil, "self.round_entityid" .. tostring(self.round_entityid))
			return
		end

		self.entities[entityid]:round_end()
				
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()			
	end,

	--
	-- void prepare()
	--
	prepare = function(self)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		Debug(nil, nil, nil, "prepare")
	end,

	--
	-- void add_member(entityid, side)
	--
	add_member = function(self, entityid, side)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end

		if self.entities[entityid] ~= nil then
			Error(nil, nil, nil, "entityid: " .. tostring(entityid) .. " already exit")
			return
		end
		
		self.entities[entityid] = Entity:new(entityid, side)
		--table.dump(entity)
		Debug(nil, nil, nil, "add member: " .. tostring(entityid) .. ", side: " .. tostring(side))
	end,

	--
	-- void start()
	--
	start = function(self)
		if self.isstart then
			Error(nil, nil, nil, "self.isstart: " .. tostring(self.isstart) .. ", self.isdone: " .. tostring(self.isdone))
			return
		end
		
		if self:entity_size(Side.ALLIES) < 1 or self:entity_size(Side.ENEMY) < 1 then
			Error(nil, nil, nil, "ALLIES size: " .. self:entity_size(Side.ALLIES) .. ", ENEMY size: " .. self:entity_size(Side.ENEMY))
			return
		end

		self.isstart = true
		self.isdone = false
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()
	end,

    --
    -- void die_entity(entityid)
    --
    die_entity = function(self, entityid)
        assert(self.entities[entityid] ~= nil)
        local entity = self.entities[entityid]
        assert(entity.death)
        self.entities[entityid] = nil
        assert(self.dead_entities[entityid] == nil)
        self.dead_entities[entityid] = entity
        self:check_done()
        if not self.isdone then
            self:remove_round_entity(entityid)
        end
    end,
    
	--
	-- bool check_done()
	--
	check_done = function(self)
		local sides = {
			[Side.ALLIES] = 0,
			[Side.ENEMY] = 0
		}
		
		for _, entity in pairs(self.entities) do
			assert(sides[entity.side] ~= nil)
			if not entity.death then sides[entity.side] = sides[entity.side] + 1 end
		end
		
		assert(sides[Side.ALLIES] > 0 or sides[Side.ENEMY] > 0)
		if sides[Side.ALLIES] > 0 and sides[Side.ENEMY] > 0 then return false end

		self:done(sides[Side.ALLIES] > 0 and Side.ALLIES or Side.ENEMY)
		return true
	end,

	--
	-- void done(side_victory)
	-- 
	done = function(self, side_victory)
		assert(self.isstart and not self.isdone)
		-- notify allies members
		for _, entity in pairs(self.entities) do
			if entity.side == Side.ALLIES then
				cc.MatchEnd(entity.id, side_victory == entity.side)
			end
		end
		self.isdone = true
		self.isstart = false
		Debug(nil, nil, nil, "比赛结束, 胜利方: " .. (side_victory == Side.ALLIES and "allies" or "enemy"))
	end,	
    
	constructor = function(self)
		self.isstart = false
		self.isdone = false
		table.clear(self.entities)
		self:init_round_list()
	end
}

function Match:new()	
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor()
	return object
end


