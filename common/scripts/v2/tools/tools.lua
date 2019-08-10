
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
            cc.ScriptDebugLog(indent .. "*" .. tostring(t))
        else
            dump_cache[tostring(t)] = true
            if type(t) == "table" then
                for pos, val in pairs(t) do
                    if type(val) == "table" then
                        cc.ScriptDebugLog(indent .. "[" .. tostring(pos) .. "] => " .. tostring(t) .. " {")
                        sub_dump(val, indent .. string.rep(" ", string.len(pos) + 8))
                        cc.ScriptDebugLog(indent .. string.rep(" ", string.len(pos) + 6) .. "}")
                    elseif type(val) == "string" then
                        cc.ScriptDebugLog(indent .. "[" .. tostring(pos) .. '] => "' .. val .. '"')
                    else
                        cc.ScriptDebugLog(indent .. "[" .. tostring(pos) .. "] => " .. tostring(val))
                    end
                end
            else
                cc.ScriptDebugLog(indent .. tostring(t))
            end
        end
    end
	if prefix ~= nil then
		cc.ScriptDebugLog(tostring(t) .. ", " ..  tostring(prefix))
	end
    if type(t) == "table" then
		cc.ScriptDebugLog(tostring(t) .. " {")
        sub_dump(t, "  ")
        cc.ScriptDebugLog("}")
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
	if type(t) == 'table' then
		local t_dup = {}
		for name, value in pairs(t) do
			t_dup[name] = table.dup(value)
		end
		return t_dup
	else
		return t
	end
end

--
-- bool table.equal(t1, t2)
--
table.equal = function(t1, t2)
	if type(t1) ~= type(t2) then return false end
	if type(t1) ~= 'table' then
		return t1 == t2
	end
	for name, _ in pairs(t1) do
		if not table.equal(t1[name], t2[name]) then return false end
	end
	return true
end

--
-- key, value table.pop_front(t)
--
table.pop_front = function(t)
	if type(t) ~= 'table' then return nil end
	for key, value in pairs(t) do 
		t[key] = nil
		return key, value 
	end
	return nil
end

function File() return debug.getinfo(2,'S').source end
function Line() return debug.getinfo(2, 'l').currentline end
function Function() 
	local func = debug.getinfo(2, 'n').name
	if func == nil then
		func = debug.getinfo(2,'S').source .. ':' .. debug.getinfo(2, 'l').currentline
	end
	return func
end

os.name = function()
    return package.config:sub(1,1) == '/' and 'linux' or 'windows'
end

function Seed(entityid)
--	return 156231035801
	return os.time() * 100 + entityid
end

function SplitString(str, reps)
    local resultStrList = {}
    string.gsub(str,'[^'..reps..']+', function ( w )
        table.insert(resultStrList, w)
    end)
    return resultStrList
end

--
-- calculate distance by Manhattan algorithm
--
function ManhattanDistance(src_coord, dest_coord)
	return math.abs(dest_coord.x - src_coord.x) + math.abs(dest_coord.y - src_coord.y)
end

--
-- calculate distance by Chebyshev algorithm
--
function ChebyshevDistance(src_coord, dest_coord)
	return math.max(math.abs(dest_coord.x - src_coord.x), math.abs(dest_coord.y - src_coord.y))
end

