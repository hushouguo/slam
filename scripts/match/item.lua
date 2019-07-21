
--
------------------- Item class -------------------
--

Item = {
    entity = nil, -- reference to entity instance
    
	id = nil, -- item.id
	baseid = nil, -- item.baseid
	base = nil, -- {field_name = field_value, ..., __base = {}}, related item.xls
	script_func = nil,

	number = nil,
	use_times = nil,

	constructor = function(self, entity, item_baseid, item_number)
    	local base = cc.LookupTable("Item", item_baseid)
		if item_number < 0 then item_number = 0 end
		if item_number > base.max_overlap then
			item_number = base.max_overlap > 0 and base.max_overlap or 1
		end
	
	    self.entity = entity
		self.id = cc.ItemNew(self.entity.id, item_baseid, item_number)
		self.baseid = item_baseid
		self:init_field()

        self.number = item_number
        self.use_times = 0
        
		self.script_func = self.base.script_func -- loadstring(self.base.script_func)
		assert(self.script_func ~= nil and type(self.script_func) == "function")
		
		cc.WriteLog(string.format("entity: %d,%s, create item: %d, %d, %s, number: %d", 
		    self.entity.id, self.entity.base.name.cn, self.id, self.baseid, self.base.name.cn, self.number))
	end,
	
	destructor = function(self)
		cc.WriteLog(string.format("entity: %d,%s, destroy item: %d, %d, %s, number: %d", 
		    self.entity.id, self.entity.base.name.cn, self.id, self.baseid, self.base.name.cn, self.number))

        cc.ItemDestroy(self.entity.id, self.id)
	end
}

function Item:set_number(item_number)
    cc.WriteLog(string.format("entity: %d,%s, set item:%d,%d,%s number from %d to %d",
        self.entity.id, self.entity.base.name.cn, self.id, self.baseid, self.base.name.cn, 
        self.number, item_number
    ))
    self.number = item_number
    if self.number < 0 then self.number = 0 end
    if self.number > self.base.max_overlap then 
        self.number = self.base.max_overlap > 0 and self.base.max_overlap or 1
    end
end

function Item:new(entity, item_baseid, item_number)
	local object = {}
	self.__index = self
	setmetatable(object, self)
	object:constructor(entity, item_baseid, item_number)
	return object
end

--
-- void init_field()
--
function Item:init_field()
    assert(self.baseid ~= nil)
    self.base = {}
    self.base.__base = cc.LookupTable("Item", self.baseid)
    self:reset_allfields()
    -- table.dump(self.base, string.format("Item: %d", self.baseid))
end

--
-- bool check_field(name)
--
function Item:check_field(name)
    local reserve_fields = {
        'baseid'
    }
    for _, reserve_field in pairs(reserve_fields) do
        if reserve_field == name then
            cc.WriteLog(string.format(">>>>>>> Item.baseid: %d set_field, name: %s is a reserved field", self.baseid, tostring(name)))
            return false
        end
    end
    return true
end

--
-- void set_field(name, value)
--
function Item:set_field(name, value)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if type(value) ~= type(self.base[name]) then
        cc.WriteLog(string.format(">>>>>>> Item.baseid: %d set_field, name: %s, value: %s, type(value): %s, type(self.base[name]): %s",
            self.baseid, tostring(name), tostring(value), type(value), type(self.base[name])))
    end
    if not self:check_field(name) then return end
    self.base[name] = table.dup(value)
    cc.WriteLog(string.format("Item: %d, set_field: %s to value: %s, old: %s", self.baseid, tostring(name), tostring(value), tostring(self.base.__base[name])))
    cc.ItemSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_field(name)
--
function Item:reset_field(name)
    assert(self.base[name] ~= nil)
    assert(self.base.__base ~= nil and self.base.__base[name] ~= nil)
    if not self:check_field(name) then return end
    self.base[name] = table.dup(self.base.__base[name])
    cc.WriteLog(string.format("Item: %d, reset_field: %s to value: %s", self.baseid, tostring(name), tostring(self.base[name])))
    cc.ItemSetField(self.entity.id, self.id, name, tostring(self.base[name]))
end

--
-- void reset_allfields()
--
function Item:reset_allfields()
    local __base = self.base.__base
    assert(__base ~= nil)
    self.base = table.dup(__base)
	assert(table.equal(self.base, __base))
    self.base.__base = __base
    -- cc.WriteLog(string.format("Item: %d, reset_allfields", self.baseid))
    -- TODO: notify client ??
end

