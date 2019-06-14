
--
------------------- Monster class -------------------
--

Monster = {
	entity = nil,
	round_interval = 1, -- 1 second
	round_last_time = 0,

	update = function(self, delta)
		if not g_match.isstart or g_match.isdone then return end
		if g_match.round_entityid ~= self.entity.id then return end
		
		local now = os.time()
		local interval = now - self.round_last_time
		if interval < self.round_interval then return end
		self.round_last_time = now
		
		if self.entity.stack_hold_size == 0 then 
			g_match:round_end(self.entity.id) 
		else
			local cardid = table.random(self.entity.stack_hold, self.entity.stack_hold_size, self.entity.random_func)
			assert(cardid ~= nil and self.entity.stack_hold[cardid] ~= nil)
			g_match:card_play(self.entity.id, cardid)
			if self.entity.stack_hold[cardid] ~= nil then
				g_match:card_discard(self.entity.id, cardid)
			end
		end
	end,
	
	constructor = function(self, entity)
		self.entity = entity
	end	
}

function Monster:new(entity)
	local monster = {}
	self.__index = self -- Monster.__index = function(key) return Monster[key] end
	setmetatable(monster, self)
	monster:constructor(entity)
	return monster
end
