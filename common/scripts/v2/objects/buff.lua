
--
------------------- Buff class -------------------
--

Buff = {
	entity = nil, -- related with entity object
	
	id = nil, -- buff.id
	baseid = nil, -- buff.baseid
	base = nil, -- {field_name = field_value, ..., __base = {}}, related buff.xls
	script_func = nil,

	layers = nil, -- buff layers
	
	constructor = function(self, entity, buff_baseid, layers)
		self.entity = entity
		
		self.id = cc.ScriptBuffAdd(self.entity.id, buff_baseid, layers)
		self.baseid = buff_baseid
		self:init_field()
		
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")

		self.layers = 0
		self:layers_add(layers)
		
		cc.ScriptDebugLog(string.format("entity: %d,%s, add buff: %d,%s layer: %d", 
		    self.entity.id, self.entity.base.name.cn, self.baseid, self.base.name.cn, self.layers))
	end,

	destructor = function(self)
		cc.ScriptDebugLog(string.format("entity: %d,%s, remove buff: %d,%s layer: %d", 
		    self.entity.id, self.entity.base.name.cn, self.baseid, self.base.name.cn, self.layers))

        cc.ScriptBuffRemove(self.entity.id, self.id)
	end,

	
	layers_add = function(self, value)
	    self.layers = self.layers + value
	    if not self.base.enable_negative_layer and self.layers < 0 then
    	    self.layers = 0
        end
		if self.layers > self.base.max_layers then
			self.layers = self.base.max_layers
		end		
		cc.ScriptBuffUpdateLayers(self.entity.id, self.id, self.layers)
		cc.ScriptDebugLog(string.format("entity: %d,%s, update buff: %d,%s layer: %d", 
		    self.entity.id, self.entity.base.name.cn, self.baseid, self.base.name.cn, self.layers))
	end,
}

function Buff:new(entity, buff_baseid, layers)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity, buff_baseid, layers)
	return object
end

--
-- void init_field()
--
function Buff:init_field()
    assert(self.baseid ~= nil)
    self.base = {}
    self.base.__base = cc.ScriptLookupTable("Buff", self.baseid)
    self:reset_allfields()
    -- table.dump(self.base, string.format("Buff: %d", self.baseid))
end

--
-- bool check_field(name)
--
function Buff:check_field(name)
    local reserve_fields = {
        'baseid'
    }
    for _, reserve_field in pairs(reserve_fields) do
        if reserve_field == name then
            cc.ScriptErrorLog(string.format(">>>>>>> Buff.baseid: %d set_field, name: %s is a reserve field", self.baseid, tostring(name)))
            return false
        end
    end
    return true
end

--
-- void set_field(name, value)
--
function Buff:set_field(name, value)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if type(value) ~= type(self.base[name]) then
        cc.ScriptErrorLog(string.format(">>>>>>> Buff.baseid: %d set_field, name: %s, value: %s, type(value): %s, type(self.base[name]): %s", 
            self.baseid, tostring(name), tostring(value), type(value), type(self.base[name])))
    end
    if not self:check_field(name) then return end
    self.base[name] = table.dup(value)
    cc.ScriptDebugLog(string.format("Buff: %d, set_field: %s to value: %s, old: %s", self.baseid, tostring(name), tostring(value), tostring(self.base.__base[name])))
    cc.ScriptBuffSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_field(name)
--
function Buff:reset_field(name)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if not self:check_field(name) then return end
    self.base[name] = table.dup(self.base.__base[name])
    cc.ScriptDebugLog(string.format("Buff: %d, reset_field: %s to value: %s", self.baseid, tostring(name), tostring(self.base[name])))
    cc.ScriptBuffSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_allfields()
--
function Buff:reset_allfields()
    local __base = self.base.__base
    assert(__base ~= nil)
    self.base = table.dup(__base)
	assert(table.equal(self.base, __base))
    self.base.__base = __base
    -- cc.ScriptDebugLog(string.format("Buff: %d, reset_allfields", self.baseid))
    -- TODO: notify client ??
end


