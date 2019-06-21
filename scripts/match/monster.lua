
--
------------------- Monster class -------------------
--

Monster = {
	entity = nil,
	round_interval = nil, -- 1 second
	round_last_time = nil,
	
	constructor = function(self, entity)
		self.entity = entity
		self.round_interval = 1
		self.round_last_time = 0
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
	if match == nil then return end -- match is over
	if match.isdone then return end -- match is done
	if match.round_entityid ~= self.entity.id then return end -- it's not my round
	
	local now = os.time()
	local interval = now - self.round_last_time
	if interval < self.round_interval then return end -- play card CD time: 1 second
	self.round_last_time = now
	
	if self.entity.stack_hold_size == 0 then 
		match:round_end(self.entity.id) 
	else
		local cardid = table.random(self.entity.stack_hold, self.entity.stack_hold_size, self.entity.random_func)
		assert(cardid ~= nil and self.entity.stack_hold[cardid] ~= nil)
		match:card_play(self.entity.id, cardid)
		if self.entity.stack_hold[cardid] ~= nil then
			match:card_discard(self.entity.id, cardid) -- MP not enough or this card can't be play
		end
	end
end

