
--
------------------- Match class -------------------
--

Match = {
    copy = nil, -- reference to copy instance

    side_victory = nil, -- indicate which side win the match
	isdone = nil, -- indicate whether the match has done
	
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

    isstart = nil, -- bool

    
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
		self.isstart = false
	end,

	destructor = function(self)
    	cc.WriteLog(string.format("比赛结束, 胜利方: %s", (self.side_victory == Side.ALLIES and "allies" or "enemy")))
    	
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

        -- destroy all of monsters	
	    for entityid, _ in pairs(self.monsters) do
	        cc.WriteLog(string.format("destroy monster: %d from match", entityid))
	        cc.EntityDestroy(entityid)
	    end
	    
	    -- TODO: destroy card, item, buff etc ... 
	end,
	

	--
	-- round list
	round_total = nil, -- 
	round_entityid = nil,
	round_side = nil, -- Side.ALLIES,
	round_list = nil, -- {[side] = {[entityid] = round_total, ...}, ...}
	init_round_list = function(self)
		self.round_total = 1
		self.round_entityid = nil
		self.round_side = Side.ALLIES
		self.round_list = {[Side.ALLIES] = {}, [Side.ENEMY] = {}}
		for _, entity in pairs(self.entities) do
		    self.round_list[entity.side][entity.id] = 0
		end
	end,		
	next_round_entity = function(self)
	    local function select_round_entity(self)
    	    local t = self.round_list[self.round_side]
    	    for entityid, round_total in pairs(t) do
    	        local entity = self.entities[entityid] -- filter death entity
                if entity ~= nil and round_total < self.round_total then
                    return entityid
                end
    	    end
    	    return nil
	    end

	    local select_entityid = select_round_entity(self)
	    if select_entityid ~= nil then
	        self.round_entityid = select_entityid
	        self.round_list[self.round_side][self.round_entityid] = self.round_total
	        return self.round_entityid
	    end
	    
		self.round_total = self.round_total + 1
		self.round_side = (self.round_side == Side.ALLIES and Side.ENEMY or Side.ALLIES)		

		select_entityid = select_round_entity(self)
		assert(select_entityid ~= nil)
	    self.round_entityid = select_entityid
	    self.round_list[self.round_side][self.round_entityid] = self.round_total
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
		self.entities[entity.id] = entity
		cc.WriteLog(string.format("add member: %d, %d, side: %d to match", entity.id, entity.baseid, side))
	end,

	--
	-- void add_monster(entity_baseid, side)
	--
	add_monster = function(self, entity_baseid, side) 
	    assert(not self.isdone)
	    local entityid = cc.EntityNew(entity_baseid)
        assert(self.entities[entityid] == nil and self.monsters[entityid] == nil)
        local entity = Entity:new(self.copy, entityid, side)
		self.entities[entityid] = entity
		self.monsters[entityid] = entity
		cc.WriteLog(string.format("add monster: %d, %d, side: %d to match", entity.id, entity.baseid, side))
	end,

	--
	-- {entityid = Side, ...} start()
	--
	start = function(self)
	    assert(not self.isdone)
	    assert(not self.isstart)
	    local allies_size = self:stat_entity_size(Side.ALLIES)
	    local enemy_size = self:stat_entity_size(Side.ENEMY)
	    assert(allies_size > 0 and enemy_size > 0)
		self:init_round_list()
	    --self.isstart = true
    	local res = {}
    	for _, entity in pairs(self.entities) do
    	    res[entity.id] = entity.side
    	    entity:enter_match(self) -- reset information of match
    	end
		cc.WriteLog(string.format("match start, allies size: %d, enemy size: %d", allies_size, enemy_size))
    	return res
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
        self.entities[entityid] = nil
        assert(self.dead_entities[entityid] == nil)
        self.dead_entities[entityid] = entity

        local alliesSize = self:stat_entity_size(Side.ALLIES)
        local enemySize = self:stat_entity_size(Side.ENEMY)

        if alliesSize == 0 or enemySize == 0 then
    		self:done(alliesSize == 0 and Side.ENEMY or Side.ALLIES)
        else
            if self.round_entityid == entityid then
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
    if not self.isstart then
        self.isstart = true
		self:next_round_entity()
		assert(self.round_entityid ~= nil)
		cc.WriteLog(string.format("比赛开始, round entityid: %d", self.round_entityid))
		self.entities[self.round_entityid]:round_begin()        
    end
    for _, entity in pairs(self.entities) do entity:update(delta) end
end

