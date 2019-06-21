
--
------------------- Match class -------------------
--

Match = {
    copy = nil, -- reference to copy instance

    side_victory = nil, -- indicate which side win the match    
	isdone = false, -- indicate whether the match has done
	
	entities = nil, -- {[entityid = entity, ...}
	monsters = nil, -- {[entityid = entity, ...}, will be destroy when the match is over
	stat_entity_size = function(self, side)
		local size = 0
		for _, entity in pairs(self.entities) do
			if entity.side == side then size = size + 1 end
		end
		return size
	end,
	dead_entities = nil, -- {[entityid = entity, ...}

	--
	-- round list
	round_total = nil, -- 
	round_entityid = nil,
	round_side = nil, -- Side.ALLIES,
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
		cc.WriteLog(string.format("*************************************** 回合: %d, entity: %d ******************************", self.round_total, self.round_entityid))
		return self.round_entityid
	end,


	--
	-- void abort()
	-- 
	abort = function(self, entityid)
	    assert(not self.isdone)
		self:done(self.entities[entityid].side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
	end,	

	--
	-- void prepare()
	--
	prepare = function(self)
	    assert(not self.isdone)
	    cc.WriteLog(string.format("match prepare"))
	end,

	--
	-- void add_member(entity, side)
	--
	add_member = function(self, entity, side)
	    assert(not self.isdone)
        assert(self.entities[entity.id] == nil)
		entity:enter_match(self) -- reset information of match
		self.entities[entity.id] = entity
		cc.WriteLog(string.format("add member: %d, side: %d to match", entity.id, side))
	end,

	--
	-- void add_monster(entity_baseid, side)
	--
	add_monster = function(self, entity_baseid, side) 
	    assert(not self.isdone)
	    local entityid = cc.EntityNew(entity_baseid)
        assert(self.entities[entityid] == nil and self.monsters[entityid] == nil)
        local entity = Entity:new(self.copy, entityid, side)
		entity:enter_match(self) -- reset information of match
		self.entities[entityid] = entity
		self.monsters[entityid] = entity
		cc.WriteLog(string.format("add monster: %d, side: %d to match", entityid, side))
	end,

	--
	-- void start()
	--
	start = function(self)
	    assert(not self.isdone)
	    local allies_size = self:stat_entity_size(Side.ALLIES)
	    local enemy_size = self:stat_entity_size(Side.ENEMY)
	    assert(allies_size > 0 and enemy_size > 0)
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		cc.WriteLog(string.format("比赛开始, allies size: %d, enemy size: %d", allies_size, enemy_size))
		self.entities[self.round_entityid]:round_begin()
	end,

	--
	-- bool card_play_judge(entityid, cardid, pick_entityid)
	--
	card_play_judge = function(self, entityid, cardid, pick_entityid)
	    assert(not self.isdone)
        assert(entityid == self.round_entityid)		
		return self.entities[entityid]:card_play_judge(cardid, pick_entityid)
	end,
	
	--
	-- bool card_play(entityid, cardid, pick_entityid)
	--
	card_play = function(self, entityid, cardid, pick_entityid)
	    assert(not self.isdone)
        assert(entityid == self.round_entityid)		
		return self.entities[entityid]:card_play(cardid, pick_entityid)
	end,

	--
	-- bool card_discard_judge(entityid, cardid)
	--
	card_discard_judge = function(self, entityid, cardid)
	    assert(not self.isdone)
        assert(entityid == self.round_entityid)		
		return self.entities[entityid]:card_discard_judge(cardid)
	end,
	
	--
	-- bool card_discard(entityid, cardid)
	--
	card_discard = function(self, entityid, cardid)
	    assert(not self.isdone)
        assert(entityid == self.round_entityid)		
		return self.entities[entityid]:card_discard(cardid, false)
	end,

	--
	-- void round_end(entityid)
	--
	round_end = function(self, entityid)
	    assert(not self.isdone)
        assert(entityid == self.round_entityid)		
		self.entities[entityid]:round_end()
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		self.entities[self.round_entityid]:round_begin()
	end,

	--
	-- void abort(entityid)
	--
	abort = function(self, entityid)
	    assert(not self.isdone)
	    local entity = self.entities[entityid]
	    assert(entity ~= nil)
	    self:done(entity.side == Side.ALLIES and Side.ENEMY or Side.ALLIES)
	end,

    --
    -- void die_entity(entityid)
    --
    die_entity = function(self, entityid)
	    assert(not self.isdone)
        assert(self.entities[entityid] ~= nil)
        local entity = self.entities[entityid]
        assert(entity.death)
        self.entities[entityid] = nil
        assert(self.dead_entities[entityid] == nil)
        self.dead_entities[entityid] = entity
        
		local sides = {
			[Side.ALLIES] = 0,
			[Side.ENEMY] = 0
		}
		
		for _, entity in pairs(self.entities) do
		    assert(not entity.death)
			assert(sides[entity.side] ~= nil)
			sides[entity.side] = sides[entity.side] + 1
		end
		
		assert(sides[Side.ALLIES] > 0 or sides[Side.ENEMY] > 0)
		if sides[Side.ALLIES] > 0 and sides[Side.ENEMY] > 0 then return false end

		self:done(sides[Side.ALLIES] > 0 and Side.ALLIES or Side.ENEMY)
    end,
    
	--
	-- void done(side_victory)
	-- 
	done = function(self, side_victory)
		assert(not self.isdone)
		assert(self.side_victory == nil)
		self.isdone = true
		self.side_victory = side_victory
		cc.WriteLog(string.format("比赛结束, 胜利方: %s", (side_victory == Side.ALLIES and "allies" or "enemy")))
		-- notify all of living entity
		for _, entity in pairs(self.entities) do
			if entity.side == Side.ALLIES then -- only member
				cc.MatchEnd(entity.id, side_victory == entity.side)
				entity:exit_match(self)
			end
		end
		-- notify all of death entity
		for _, entity in pairs(self.dead_entities) do
			if entity.side == Side.ALLIES then -- only member
				cc.MatchEnd(entity.id, side_victory == entity.side)
				entity:exit_match(self)
			end
		end
		-- NOTE: wait for Scene:update to destroy this
	end,	
    
	constructor = function(self, copy)
	    self.copy = copy
	    
        self.entities = {}
        self.monsters = {}
        self.dead_entities = {}
        
    	self.round_total = 0
    	self.round_entityid = nil
    	self.round_side = Side.ALLIES
        
		self.side_victory = nil
		self.isdone = false
		
		self:init_round_list()
	end,

	destructor = function(self)
	    for entityid, _ in pairs(self.monsters) do
	        cc.WriteLog(string.format("destroy monster: %d from match", entityid))
	        cc.EntityDestroy(entityid)
	    end
	    -- TODO: destroy card, item, buff etc ... 
	end
}

function Match:new(copy)	
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(copy)
	return object
end

--
-- bool checkDone()
--
function Match:checkDone()
    return self.isdone
end

--
-- void update(delta)
--
function Match:update(delta)
    assert(not self.isdone)
    for _, entity in pairs(self.entities) do entity:update(delta) end
end

