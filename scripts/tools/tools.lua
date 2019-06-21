
--
-- 线性同余器
--
function NewRandom(seed)
    return function(low, up)  -- range: [low,up]
		assert(low ~= nil and up ~= nil)
		assert(type(low) == "number" and type(up) == "number")
		if low == up then return low, seed end
        seed = (seed * 9301 + 49279) % 233280
		local min = low < up and low or up
		local max = low < up and up or low
		return math.floor(seed / 233280 * (max - min + 1) + min), seed
    end 
end

--
-- random, range: [min, max], return random_value, seed
--
math.random_between = function(min, max)
    if math.random_func == nil then
        math.random_func = NewRandom(os.time())
    end
    --local value, math.random_seed = math.random_func(min, max)
    --return value
    return math.random_func(min, max)
end

--
-- dump table
--
table.dump = function(t, prefix)  
    local dump_cache = {}
    local function sub_dump(t, indent)
        if dump_cache[tostring(t)] then
            print(indent .. "*" .. tostring(t))
        else
            dump_cache[tostring(t)] = true
            if type(t) == "table" then
                for pos, val in pairs(t) do
                    if type(val) == "table" then
                        print(indent .. "[" .. pos .. "] => " .. tostring(t) .. " {")
                        sub_dump(val, indent .. string.rep(" ", string.len(pos) + 8))
                        print(indent .. string.rep(" ", string.len(pos) + 6) .. "}")
                    elseif type(val) == "string" then
                        print(indent .. "[" .. pos .. '] => "' .. val .. '"')
                    else
                        print(indent .. "[" .. pos .. "] => " .. tostring(val))
                    end
                end
            else
                print(indent .. tostring(t))
            end
        end
    end
    if type(t) == "table" then
		if prefix ~= nil then
			print(tostring(t) .. ", " ..  tostring(prefix) .. " {")
		else
			print(tostring(t) .. " {")
		end
        sub_dump(t, "  ")
        print("}")
    else
        sub_dump(t, "  ")
    end
end


--
-- clear table
--
table.clear = function(t)
	assert(type(t) == "table")
	for k in pairs (t) do t[k] = nil end
end

--
-- get size of table
--
table.size = function(t)
	assert(type(t) == "table")
	local size = 0
	for _, v in pairs (t) do 
		if v ~= nil then size = size + 1 end
	end
	return size
end

--
-- random retrieve a KEY from table, return KEY, VALUE
--
table.random = function(t, table_size, random_func)
	if table_size > 0 then
	    local size = 0
	    if random_func ~= nil then
	        size, _ = random_func(0, table_size - 1)
	    else
		    size = math.random_between(0, table_size - 1)
	    end
		for KEY, VALUE in pairs(t) do
			if size == 0 then return KEY, VALUE end
			size = size - 1
		end
	end
	return nil, nil
end

--
-- table table.dup(t)
--
table.dup = function(t)
	assert(type(t) == "table")
	local tt = {}
	for KEY, VALUE in pairs(t) do tt[KEY] = VALUE end
	return tt
end

function File() return debug.getinfo(2,'S').source end
function Line() return debug.getinfo(2, 'l').currentline end

function Debug(entity, card, buff, debugstring)
	if not g_enable_debug then return end
	local entity_string = ""
	if entity ~= nil then entity_string = "[" .. entity.base.name.cn .. ":" .. entity.id .. ":" .. entity.baseid .. "] " end
	local card_string = ""
	if card ~= nil then card_string = "[" .. card.base.name.cn .. ":" .. card.id .. ":" .. card.baseid .. "] " end
	local buff_string = ""
	if buff ~= nil then buff_string = "[" .. buff.base.name.cn .. ":" .. buff.id .. ":" .. buff.baseid .. "] " end
	cc.WriteLog(entity_string .. card_string .. buff_string .. (debugstring ~= nil and debugstring or ""))
end

function Error(entity, card, buff, errorstring)
	local entity_string = ""
	if entity ~= nil then entity_string = "[" .. entity.base.name.cn .. ":" .. entity.id .. ":" .. entity.baseid .. "] " end
	local card_string = ""
	if card ~= nil then card_string = "[" .. card.base.name.cn .. ":" .. card.id .. ":" .. card.baseid .. "] " end
	local buff_string = ""
	if buff ~= nil then buff_string = "[" .. buff.base.name.cn .. ":" .. buff.id .. ":" .. buff.baseid .. "] " end
	cc.WriteLog(">>>>>>>>>>>> Error: " .. File() .. ":" .. Line() .. " " .. entity_string .. card_string .. buff_string .. (errorstring ~= nil and errorstring or ""))
end


