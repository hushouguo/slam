
--
------------------- Monster class -------------------
--

Monster = {
	entity = nil,
	round_interval = nil, -- seconds
	round_last_time = nil,
	
	constructor = function(self, entity)
		self.entity = entity
		self.round_interval = FLAG_monster_round_interval
		self.round_last_time = 0
		
		cc.WriteLog(string.format("entity: %d,%d,%s, create monster ai", 
		    self.entity.id, self.entity.baseid, self.entity.base.name.cn))
	end,

	destructor = function(self)
		cc.WriteLog(string.format("entity: %d,%d,%s, destroy monster ai", 
		    self.entity.id, self.entity.baseid, self.entity.base.name.cn))
	end
}

function Monster:new(entity)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity)
	return object
end

function Monster:update(delta)
	local match = self.entity.copy.scene.match
	if match == nil then 
--		cc.WriteLog(string.format(">>>>>>>>>> monster found match is nil"))
		return end -- match is over
	if match.isdone then 
--		cc.WriteLog(string.format(">>>>>>>>>> monster found match is done"))
		return end -- match is done
	if match.round_entityid ~= self.entity.id then 
--		cc.WriteLog(string.format(">>>>>>>>>> monster.id: %d, round_entityid: %d", self.entity.id, match.round_entityid))
		return end -- it's not my round
	if self.entity.death then 
	    match:round_end(self.entity.id)
	    return
	end -- monster is die
	
	local now = os.time()
	local interval = now - self.round_last_time
	if interval < self.round_interval then return end -- play card CD time: 1 second
	self.round_last_time = now
	
	if table.size(self.entity.pack.stack_hold) == 0 then 
		match:round_end(self.entity.id) 
	else
		local cardid = table.random(self.entity.pack.stack_hold, table.size(self.entity.pack.stack_hold), self.entity.random_func)
		assert(cardid ~= nil and self.entity.pack.stack_hold[cardid] ~= nil)
		match:card_play(self.entity.id, cardid)
		if self.entity.pack.stack_hold[cardid] ~= nil then
			match:card_discard(self.entity.id, cardid) -- MP not enough or this card can't be play
		end
	end
end

