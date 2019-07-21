--
-- record.lua
--

-- record format:
--[[    {
            id = ?,
            createtime = ?,
            seed = ?,
            
            [tostring(copy_baseid)] = {
                current_layer = ?, -- current layer of copy

                [tostring(layer)] = { -- by layer
                    events_accomplish = {
                        event_baseid, ...
                    }
                },

                pack = {
                	-- base property
                    hp = ?,
                    mp = ?,
                    maxhp = ?,
                    maxmp = ?,
                    strength = ?,
                    armor = ?,
                	shield = ?,
                	weakness = ?, -- [0 ~ 1]

                    -- match parameters
                    round_cards = ?,
                    max_hold_cards = ?,
                                        
                    -- money
                	gold = ?, -- gold

                	-- cards
                	cards = { 
                	    card_baseid, ... 
                	},

                	-- items
                	items = {
                	    [tostring(item_baseid)] = item_number, ...
                	},

                    -- equips
--TODO:                    equips = nil, -- {[EquipPlaceholder.COPY_OUTSIDE]->{[slot]->item, ...}, [EquipPlaceholder.COPY_INSIDE]->{}}

                	-- puppets
                	puppets = {
                	    entity_baseid, ...
                	},
                	
                	placeholders = {
                	    entity_baseid, ...
                	},
                	
                    -- buffs
                    buffs = {
                        [tostring(buff_baseid)] = buff_layer, ...
                    },                
                }, -- end pack
            },
            ...
        }
]]
record = {}

--
-- void record.serialize(entity)
--
record.serialize = function(entity)
    if entity.record == nil then
        entity.record = {
            id = entity.id,
            createtime = entity.createtime,
            seed = entity.seed
        }
    end

    local copy_baseid_str = tostring(entity.copy.baseid)
    if entity.record[copy_baseid_str] == nil then entity.record[copy_baseid_str] = {} end
    entity.record[copy_baseid_str].current_layer = entity.copy:current_layer()
    
    -- TODO: events_accomplish
    
    if entity.record[copy_baseid_str].pack == nil then entity.record[copy_baseid_str].pack = {} end
    local pack = entity.record[copy_baseid_str].pack
        
    pack.hp = entity.pack.hp
    pack.mp = entity.pack.mp
    pack.maxhp = entity.pack.maxhp
    pack.maxmp = entity.pack.maxmp
    pack.strength = entity.pack.strength
    pack.armor = entity.pack.armor
    pack.shield = entity.pack.shield
    pack.weakness = entity.pack.weakness

    pack.round_cards = entity.pack.round_cards
    pack.max_hold_cards = entity.pack.max_hold_cards

    pack.gold = entity.pack.gold 

    pack.cards = {}
    for _, card in pairs(entity.pack.cards) do table.insert(pack.cards, card.baseid) end

    -- TODO: 道具的属性可能会发生变化
    pack.items = {}
    for _, item in pairs(entity.pack.items) do pack.items[tostring(item.baseid)] = item.number end

    -- TODO: 道具的属性可能会发生变化
    pack.equips = {}
    for _, item in pairs(entity.pack.equips) do pack.equips[tostring(item.baseid)] = item.number end

    -- TODO: 宠物的属性可能会发生变化
    pack.puppets = {}
    for _, target in pairs(entity.pack.puppets) do table.insert(pack.puppets, target.baseid) end

    -- TODO: 宠物的属性可能会发生变化
    pack.placeholders = {}
    for placeholder, target in pairs(entity.pack.placeholders) do pack.placeholders[placeholder] = target.baseid end

    pack.buffs = {}
    for _, buff in pairs(entity.pack.buffs) do pack.buffs[tostring(buff.baseid)] = buff.layer end
    
    local json = require('tools/json')
	local jsonstr = json.encode(entity.record)
	cc.WriteLog('Serialize: ' .. tostring(jsonstr))
    cc.EntitySerialize(entity.id, jsonstr)
end

--
-- table record.unserialize(entity)
--
record.unserialize = function(entity)
    local json = require('tools/json')
    local jsonstr = cc.EntityUnserialize(entity.id)
	cc.WriteLog('Unserialize: ' .. tostring(jsonstr))
	if jsonstr == nil then return nil end
    local t = json.decode(jsonstr)
    assert(t ~= nil, tostring(jsonstr))
    assert(t.id == entity.id, tostring(jsonstr))
    return t
end

