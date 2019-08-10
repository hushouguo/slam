
--
------------------- Match class -------------------
--

Match = {
    copy = nil, -- reference to copy instance

    side_victory = nil, -- indicate which side win the match
	isdone = nil, -- indicate whether the match has done
	
	entities = nil, -- {[entityid = entity, ...}
	living_entity_size = function(self, side)
		local size = 0
		for _, entity in pairs(self.entities) do
			if entity.side == side and not entity.death then size = size + 1 end
		end
		return size
	end,

    isstart = nil, -- bool
    
	constructor = function(self, copy)
	    self.copy = copy
	    cc.WriteLog(string.format("++++++++++ match constructor"))
	    
        self.entities = {}
        
    	self.round_total = 0
    	self.round_entityid = nil
		self.round_iterator = nil
		self.round_list = {}
		self.round_list_size = nil
        
		self.side_victory = nil
		self.isdone = false
		self.isstart = false
	end,

	destructor = function(self)
	    cc.WriteLog(string.format("---------- match destructor"))	
    	cc.WriteLog(string.format("比赛结束, 胜利方: %s", (self.side_victory == Side.ALLIES and "allies" or "enemy")))
    	
    	for _, entity in pairs(self.entities) do
    		if entity.side == Side.ALLIES then -- only member
    			entity:exit_match(self)
    	    else
    	        cc.WriteLog(string.format("destroy monster: %d from match", entity.id))
    	        entity:exit_match(self)
    	        entity:destructor()
    		end
    	end
    	table.clear(self.entities)
    	
    	cc.MatchEnd(self.copy.entity_master.id, self.side_victory == self.copy.entity_master.side)
	end,
	

	--
	-- round list
	round_total = nil,
	round_entityid = nil,
	round_iterator = nil,
	round_list = nil, -- { entityid, ... }
	round_list_size = nil,
	init_round_list = function(self)
		self.round_total = 0
		self.round_entityid = nil
		self.round_iterator = 0
		self.round_list_size = table.size(self.round_list)
		assert(self.round_list_size >= 2)
	end,
	next_round_entity = function(self)
    	self.round_total = self.round_total + 1
	    while true do
    		self.round_iterator = self.round_iterator + 1
    		if self.round_iterator > self.round_list_size then self.round_iterator = 1 end
    		self.round_entityid = self.round_list[self.round_iterator]
    		assert(self.round_entityid ~= nil)
    		if not self.entities[self.round_entityid].death then break end
		end
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
	-- void add_member(entity)
	--
	add_member = function(self, entity)
	    assert(not self.isdone)
        assert(self.entities[entity.id] == nil)
		self.entities[entity.id] = entity
		cc.WriteLog(string.format("add member: %d, %d, %s, side: %d to match", entity.id, entity.baseid, entity.base.name.cn, entity.side))
		table.insert(self.round_list, entity.id) -- order by add
	end,

	--
	-- void add_monster(entity_baseid, side)
	--
	add_monster = function(self, entity_baseid, side) 
	    assert(not self.isdone)
        local entity = Entity:new(self.copy, cc.EntityNew(entity_baseid), side, nil, Span.COPY)
        assert(self.entities[entity.id] == nil)
		self.entities[entity.id] = entity
		cc.WriteLog(string.format("add monster: %d, %d, side: %d to match", entity.id, entity.baseid, side))
		table.insert(self.round_list, entity.id) -- order by add
	end,
	
	--
	-- {entityid = Side, ...} prepare()
	--
	prepare = function(self)
	    assert(not self.isdone)
	    assert(not self.isstart)
	    assert(self:living_entity_size(Side.ALLIES) > 0 and self:living_entity_size(Side.ENEMY) > 0)
	    self:init_round_list()
    	local res = {}
    	for _, entity in pairs(self.entities) do
    	    res[entity.id] = entity.side
    	end
		cc.WriteLog(string.format("match prepare, allies size: %d, enemy size: %d", 
		    self:living_entity_size(Side.ALLIES), self:living_entity_size(Side.ENEMY))
		    )
    	return res
	end,

	--
	-- void start()
	--
	start = function(self)
	    assert(not self.isdone)
	    assert(not self.isstart)
	    assert(self:living_entity_size(Side.ALLIES) > 0 and self:living_entity_size(Side.ENEMY) > 0)
        self.isstart = true

	    -- call enter_match to switch span
    	for _, entity in pairs(self.entities) do
    	    entity:enter_match(self) -- reset information of match
    	end
    			    
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		cc.WriteLog(string.format("比赛开始, round entityid: %d", self.round_entityid))
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
		if entityid ~= self.round_entityid then return end
		if self.entities[entityid] ~= nil then -- perhaps entity is die
		    self.entities[entityid]:round_end()
		end
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

        local allies_living_size = self:living_entity_size(Side.ALLIES)
        local enemy_living_size = self:living_entity_size(Side.ENEMY)

        if allies_living_size == 0 or enemy_living_size == 0 then
    		self:done(allies_living_size == 0 and Side.ENEMY or Side.ALLIES)
        else
            if self.round_entityid == entityid then -- entity:enter_match, perhaps call here, self.round_entityid is nil
                self:round_end(entityid)
            end
        end
    end,
    
	--
	-- void done(side_victory)
	-- 
	done = function(self, side_victory)
		if not self.isdone then
    		assert(self.side_victory == nil)
    		self.isdone = true
    		self.side_victory = side_victory
        	cc.WriteLog(string.format("match done, victory: %s", (self.side_victory == Side.ALLIES and "allies" or "enemy")))
        else
            cc.WriteLog(string.format(">>>>>>> match already is done, func: %s", Function()))
		end
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

