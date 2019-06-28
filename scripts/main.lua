--
------------------- lua APIs -------------------
--

require('common')
require('tools/tools')
require('tools/astar')
require('tools/bit')
require('tools/noise')
require('tools/record')

require('match/bag')
require('match/buff')
require('match/card')
require('match/entity')
require('match/monster')
require('match/match')

require('copy/event')
require('copy/obstacle')
require('copy/scene')
require('copy/scene_creator')
require('copy/copy')

require("modules/module")

--
------------------- copy -------------------
--

function lua_entry_copy_enter(entityid, copy_baseid)
    g_copy = Copy:new(entityid, copy_baseid)
end

function lua_entry_copy_exit(entityid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	g_copy:remove_member(entityid)
end

function lua_entry_copy_move_request(entityid, sceneid, destx, desty)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
    return g_copy:move_request(entityid, sceneid, destx, desty)
end

function lua_entry_copy_move(entityid, sceneid, x, y)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	g_copy:move(entityid, sceneid, x, y)
end

function lua_entry_copy_event_reward(entityid, sceneid, eventid, reward_index, entry_index)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	return g_copy:event_reward(entityid, sceneid, eventid, reward_index, entry_index)
end

--
-- call me for every frame
function lua_entry_update(delta)
    if g_copy ~= nil then
        if g_copy:checkDone() then
            cc.WriteLog(string.format("g_copy: %d isDone", g_copy.baseid))
            g_copy:destructor()
            cc.WriteLog(string.format("g_copy: %d close", g_copy.baseid))
            g_copy = nil
        else
            g_copy:update(delta)
        end
    end
end

--
-- bool lua_entry_copy_purchase_card(entityid, mapid, eventid, card_baseid)
function lua_entry_copy_purchase_card(entityid, mapid, eventid, card_baseid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    return g_copy:purchase_card(entityid, mapid, eventid, card_baseid)
end

--
-- bool lua_entry_copy_purchase_item(entityid, mapid, eventid, item_baseid)
function lua_entry_copy_purchase_item(entityid, mapid, eventid, item_baseid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    return g_copy:purchase_item(entityid, mapid, eventid, item_baseid)
end

--
-- bool lua_entry_copy_destroy_card(entityid, mapid, eventid, cardid)
function lua_entry_copy_destroy_card(entityid, mapid, eventid, cardid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    return g_copy:destroy_card(entityid, mapid, eventid, cardid)
end

--
-- bool lua_entry_copy_levelup_card(entityid, mapid, eventid, cardid)
function lua_entry_copy_levelup_card(entityid, mapid, eventid, cardid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    return g_copy:levelup_card(entityid, mapid, eventid, cardid)
end


--
------------------- match -------------------
--

--
-- start match
function lua_entry_match_start()
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end
    
    return g_copy.scene:start_match()
end

--
-- the play if can play a card
function lua_entry_card_play_judge(entityid, cardid, pick_entityid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match.isdone then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return false
    end
    
	return g_copy.scene.match:card_play_judge(entityid, cardid, pick_entityid)
end

--
-- the player play a card
function lua_entry_card_play(entityid, cardid, pick_entityid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:card_play(entityid, cardid, pick_entityid)
end

--
-- the player if can discard a card
function lua_entry_card_discard_judge(entityid, cardid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match.isdone then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return false
    end
    
	return g_copy.scene.match:card_discard_judge(entityid, cardid)
end

--
-- the player discard a card
function lua_entry_card_discard(entityid, cardid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:card_discard(entityid, cardid)
end

--
-- the player end this round
function lua_entry_round_end(entityid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.WriteLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:round_end(entityid)
end

--
-- abort match
function lua_entry_match_abort(entityid)
    if g_copy == nil then 
        cc.WriteLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.WriteLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    g_copy.scene:abort_match(entityid)
end

