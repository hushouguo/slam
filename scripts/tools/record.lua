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
                    
                    shop_discount = {
                        card_quality = ?,
                        card_level = ?,
                        card_category = ?,
                        card_baseid = {
                            [card_baseid] = ?, ...
                        }
                    },
                    
                    accomplish_events = {
                        [layer] = {event_baseid, ...}, ...
                    }
                }
            }
        }
]]
record = {}


--
-- table create(entityid)
--
record.create = function(entityid)
    local createtime = os.time()
    return {
        member = entityid,
        createtime = createtime,
        seed = createtime * 100 + entityid,
        copies = {}
    }
end

--
-- table unserialize(entityid)
--
record.unserialize = function(entityid)
    local json = require('tools/json')
    local jsonstr = cc.EntityUnserialize(entityid)
	cc.WriteLog('Unserialize: ' .. tostring(jsonstr))
    if jsonstr ~= nil then
        local record_table = json.decode(jsonstr)
        assert(record_table ~= nil, tostring(jsonstr))
        assert(record_table.member == entityid, tostring(jsonstr))
        assert(record_table.seed ~= nil, tostring(jsonstr))
        return record_table
    end
    local record_table = record.create(entityid)
	record.serialize(entityid, record_table)
    return record_table
end

--
-- void serialize(entityid, record_table)
--
record.serialize = function(entityid, record_table)
    local json = require('tools/json')
	local jsonstr = json.encode(record_table)
	cc.WriteLog('Serialize: ' .. tostring(jsonstr))
    cc.EntitySerialize(entityid, jsonstr)
end


-----------------------------------------------------------------------

--
-- entityid record.member(record_table)
--
record.member = function(record_table)
	assert(record_table ~= nil and record_table.member ~= nil)
	return record_table.member
end

--
-- seed record.seed(record_table)
-- 
record.seed = function(record_table)
	assert(record_table ~= nil and record_table.seed ~= nil)
	return record_table.seed
end


--
-- layer record.copy_layer(record_table, copy_baseid)
--
record.copy_layer = function(record_table, copy_baseid)
	assert(record_table ~= nil and record_table.copies ~= nil)
	local copy_baseid_str = tostring(copy_baseid) 
	if record_table.copies[copy_baseid_str] == nil then
	    return nil
	end
	return record_table.copies[copy_baseid_str].layer
end

--
-- void record.set_copy_layer(record_table, copy_baseid, layer)
--
record.set_copy_layer = function(record_table, copy_baseid, layer)
	assert(record_table ~= nil and record_table.copies ~= nil)
	local copy_baseid_str = tostring(copy_baseid)
	if record_table.copies[copy_baseid_str] == nil then
	    record_table.copies[copy_baseid_str] = {}
	end
	record_table.copies[copy_baseid_str].layer = layer
	record.serialize(record.member(record_table), record_table)
end

--
-- table record.copy_accomplish_events(record_table, copy_baseid, layer)
--
record.copy_accomplish_events = function(record_table, copy_baseid, layer)
	assert(record_table ~= nil and record_table.copies ~= nil)
	local copy_baseid_str = tostring(copy_baseid)
	if record_table.copies[copy_baseid_str] == nil then
	    return nil -- no copy_baseid
	end
	if record_table.copies[copy_baseid_str].accomplish_events == nil then
	    return nil -- no accomplish_events
	end
	local layer_str = tostring(layer)
	if record_table.copies[copy_baseid_str].accomplish_events[layer_str] == nil then
	    return nil -- no accomplish_events[layer]
	end
	return table.dup(record_table.copies[copy_baseid_str].accomplish_events[layer_str])
end

--
-- void record.set_copy_accomplish_events(record_table, copy_baseid, layer, event_baseid)
--
record.set_copy_accomplish_events = function(record_table, copy_baseid, layer, event_baseid)
	assert(record_table ~= nil and record_table.copies ~= nil)
	local copy_baseid_str = tostring(copy_baseid)
	if record_table.copies[copy_baseid_str] == nil then
	    record_table.copies[copy_baseid_str] = {} -- copy_baseid
	end
	if record_table.copies[copy_baseid_str].accomplish_events == nil then
	    record_table.copies[copy_baseid_str].accomplish_events = {} -- accomplish_events
	end
	local layer_str = tostring(layer)
	if record_table.copies[copy_baseid_str].accomplish_events[layer_str] == nil then
	    record_table.copies[copy_baseid_str].accomplish_events[layer_str] = {} -- accomplish_events[layer]
	end
	local accomplish_events = record_table.copies[copy_baseid_str].accomplish_events[layer_str]
	for _, baseid in pairs(accomplish_events) do
		if baseid == event_baseid then return end
	end
	table.insert(accomplish_events, event_baseid)
	record.serialize(record.member(record_table), record_table)
end


--
-- table record.copy_shop_discount(record_table, copy_baseid)
--
record.copy_shop_discount = function(record_table, copy_baseid)
	assert(record_table ~= nil and record_table.copies ~= nil)
	local copy_baseid_str = tostring(copy_baseid)
	if record_table.copies[copy_baseid_str] == nil then
	    return nil -- no copy_baseid
	end
	if record_table.copies[copy_baseid_str].shop_discount == nil then
	    return nil -- no shop_discount
	end
	return table.dup(record_table.copies[copy_baseid_str].shop_discount)
end

--
-- void record.set_copy_shop_discount(record_table, copy_baseid, table_discount)
--
record.set_copy_shop_discount = function(record_table, copy_baseid, table_discount)
    assert(record_table ~= nil and record_table.copies ~= nil)
    local copy_baseid_str = tostring(copy_baseid)
	if record_table.copies[copy_baseid_str] == nil then
	    record_table.copies[copy_baseid_str] = {}
	end
	record_table.copies[copy_baseid_str].shop_discount = table.dup(table_discount)
	record.serialize(record.member(record_table), record_table)
end

