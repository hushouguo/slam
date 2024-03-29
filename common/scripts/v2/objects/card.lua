
--
------------------- Card class -------------------
--

Card = {
    entity = nil, -- reference to entity instance
    
	id = nil, -- card.id
	baseid = nil, -- card.baseid
	base = nil, -- {field_name = field_value, ..., __base = {}}, related card.xls
	script_func = nil,

	constructor = function(self, entity, card_baseid)
	    self.entity = entity
		self.id = cc.ScriptCardNew(self.entity.id, card_baseid)
		self.baseid = card_baseid
		self:init_field()

		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		
		cc.ScriptDebugLog(string.format("entity: %d,%s, create card: %d, %d, %s", 
		    self.entity.id, self.entity.base.name.cn, self.id, self.baseid, self.base.name.cn))
	end,
	
	destructor = function(self)
		cc.ScriptDebugLog(string.format("entity: %d,%s, destroy card: %d, %d, %s", 
		    self.entity.id, self.entity.base.name.cn, self.id, self.baseid, self.base.name.cn))
		    
        cc.ScriptCardDestroy(self.entity.id, self.id)
	end
}

function Card:new(entity, card_baseid)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity, card_baseid)
	return object
end

--
-- void init_field()
--
function Card:init_field()
    assert(self.baseid ~= nil)
    self.base = {}
    self.base.__base = cc.ScriptLookupTable("Card", self.baseid)
    self:reset_allfields()
    -- table.dump(self.base, string.format("Card: %d", self.baseid))
end

--
-- bool check_field(name)
--
function Card:check_field(name)
    local reserve_fields = {
        'baseid'
    }
    for _, reserve_field in pairs(reserve_fields) do
        if reserve_field == name then
            cc.ScriptErrorLog(string.format(">>>>>>> Card.baseid: %d set_field, name: %s is a reserved field", self.baseid, tostring(name)))
            return false
        end
    end
    return true
end

--
-- void set_field(name, value)
--
function Card:set_field(name, value)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if type(value) ~= type(self.base[name]) then
        cc.ScriptErrorLog(string.format(">>>>>>> Card.baseid: %d set_field, name: %s, value: %s, type(value): %s, type(self.base[name]): %s", 
            self.baseid, tostring(name), tostring(value), type(value), type(self.base[name])))
    end
    if not self:check_field(name) then return end
    self.base[name] = table.dup(value)
    cc.ScriptDebugLog(string.format("Card: %d, set_field: %s to value: %s, old: %s", self.baseid, tostring(name), tostring(value), tostring(self.base.__base[name])))
    cc.ScriptCardSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_field(name)
--
function Card:reset_field(name)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if not self:check_field(name) then return end
    self.base[name] = table.dup(self.base.__base[name])
    cc.ScriptDebugLog(string.format("Card: %d, reset_field: %s to value: %s", self.baseid, tostring(name), tostring(self.base[name])))
    cc.ScriptCardSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_allfields()
--
function Card:reset_allfields()
    local __base = self.base.__base
    assert(__base ~= nil)
    self.base = table.dup(__base)
	assert(table.equal(self.base, __base))
    self.base.__base = __base
    -- cc.ScriptDebugLog(string.format("Card: %d, reset_allfields", self.baseid))
    -- TODO: notify client ??
end

