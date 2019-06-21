--
-- record.lua
--

-- record format:
--[[    {
            member = ?,
            createtime = ?,
            seed = ?,
            copies = {
                [tostring(copy_baseid)] = {
                    layer = ?,
                    [tostring(map_baseid)] = {
                        accomplish_events = {event_baseid = true/false, ...}
                    }
                }
            }
        }
]]
record = {}

--
-- entityid record.member(r)
--
record.member = function(r)
	assert(r ~= nil and r.member ~= nil)
	return r.member
end

--
-- seed record.seed(r)
-- 
record.seed = function(r)
	assert(r ~= nil and r.seed ~= nil)
	return r.seed
end

--
-- layer record.layer(r, copy_baseid)
--
record.layer = function(r, copy_baseid)
	assert(r ~= nil and r.copies ~= nil)
	local r_copy = r.copies[tostring(copy_baseid)]
	assert(r_copy ~= nil and r_copy.layer ~= nil)
	return r_copy.layer
end

--
-- void record.set_layer(r, copy_baseid, layer)
--
record.set_layer = function(r, copy_baseid, layer)
	assert(r ~= nil and r.copies ~= nil)
	local r_copy = r.copies[tostring(copy_baseid)]
	assert(r_copy ~= nil and r_copy.layer ~= nil)
	r_copy.layer = layer
	record.serialize(r)
end

--
-- table record.accomplish_events(r, copy_baseid, map_baseid)
--
record.accomplish_events = function(r, copy_baseid, map_baseid)
	assert(r ~= nil and r.copies ~= nil)
	local r_copy = r.copies[tostring(copy_baseid)]
	assert(r_copy ~= nil)
	local r_map = r_copy[tostring(map_baseid)]
	if r_map == nil then return nil end
	assert(r_map.accomplish_events ~= nil)
	return table.dup(r_map.accomplish_events)
end

--
-- void record.set_accomplish_events(r, copy_baseid, map_baseid, event_baseid)
--
record.set_accomplish_events = function(r, copy_baseid, map_baseid, event_baseid)
	assert(r ~= nil and r.copies ~= nil)
	local r_copy = r.copies[tostring(copy_baseid)]
	assert(r_copy ~= nil)
	if r_copy[tostring(map_baseid)] == nil then
		r_copy[tostring(map_baseid)] = {
			accomplish_events = {}
		}
	end
	local r_map = r_copy[tostring(map_baseid)]
	assert(r_map ~= nil and r_map.accomplish_events ~= nil)
	for _, baseid in pairs(r_map.accomplish_events) do
		if baseid == event_baseid then return end
	end
	table.insert(r_map.accomplish_events, event_baseid)
	record.serialize(r)
end


--
-- table create(entityid, copy_baseid)
--
record.create = function(entityid, copy_baseid)
    local createtime = os.time()
    return {
        member = entityid, 
        createtime = createtime,
        seed = createtime * 100 + entityid,
        copies = {
            [tostring(copy_baseid)] = {
				layer = 1
--				[tostring(map_baseid)] = {
--					accomplish_events = {}
--				}
			}
        }
    }
end

--
-- void serialize(entityid, record_table)
--
record.serialize = function(r)
    local json = require('tools/json')
	local jsonstr = json.encode(r)
	print('Serialize: ' .. tostring(jsonstr))
    cc.EntitySerialize(record.member(r), jsonstr)
end

--
-- table unserialize(entityid, copy_baseid)
--
record.unserialize = function(entityid, copy_baseid)
    local json = require('tools/json')
    local jsonstr = cc.EntityUnserialize(entityid)
	print('Unserialize: ' .. tostring(jsonstr))
    if jsonstr ~= nil then
        local r = json.decode(jsonstr)
        assert(r ~= nil, tostring(jsonstr))
        assert(r.member ~= nil, tostring(jsonstr))
        assert(r.seed ~= nil, tostring(jsonstr))
        if r.copies == nil then r.copies = {} end
        if r.copies[tostring(copy_baseid)] == nil then 
			r.copies[tostring(copy_baseid)] = {
				layer = 1
			}
		end
        return r
    end
    local r = record.create(entityid, copy_baseid)
	record.serialize(r)
    return r
end

