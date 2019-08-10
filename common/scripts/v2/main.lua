--
------------------- lua APIs -------------------
--

require('common/common')

require('tools/console')
require('tools/tools')
require('tools/astar')
require('tools/bit')
require('tools/noise')
require('tools/record')
require('tools/god')

require('objects/item')
require('objects/pack')
require('objects/buff')
require('objects/card')
require('objects/entity')
require('objects/monster')
require('objects/event')
require('objects/obstacle')

require('match/match')

require('scene/map')
require('scene/map_pattern')
require('scene/map_generator')
require('scene/scene')

require('copy/copy')

require("modules/module")


--
------------------- copy -------------------
--

function script_entry_copy_enter(entityid, copy_baseid)
    g_copy = Copy:new(entityid, copy_baseid)
end

function script_entry_copy_exit(entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	g_copy:remove_member(entityid)
end

--
-- call me for every frame
function script_entry_update(delta)
    if g_copy ~= nil then
        if g_copy:checkDone() then
            cc.ScriptDebugLog(string.format("g_copy: %d isDone", g_copy.baseid))
            g_copy:destructor()
            cc.ScriptDebugLog(string.format("g_copy: %d close", g_copy.baseid))
            g_copy = nil
        else
            g_copy:update(delta)
        end
    end
end

--
-- God Command
function script_entry_god_command(args)
    return God.dispatch(args)
end

--
------------------- scene.move -------------------
--

function script_entry_scene_move_request(entityid, destx, desty)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
    return g_copy:move_request(entityid, destx, desty)
end

function script_entry_scene_move(entityid, destx, desty)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	g_copy:move(entityid, destx, desty)
end

--
------------------- event trigger -------------------
--

--
-- bool script_entry_event_trigger_linked(entityid)
function script_entry_event_trigger_linked(entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:trigger_linked_event(entityid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_purchase_card(entityid, eventid, card_baseid)
function script_entry_event_purchase_card(entityid, eventid, card_baseid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:purchase_card(entityid, eventid, card_baseid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_purchase_item(entityid, eventid, item_baseid)
function script_entry_event_purchase_item(entityid, eventid, item_baseid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:purchase_item(entityid, eventid, item_baseid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_destroy_card(entityid, eventid, cardid)
function script_entry_event_destroy_card(entityid, eventid, cardid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:destroy_card(entityid, eventid, cardid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_levelup_card(entityid, eventid, cardid)
function script_entry_event_levelup_card(entityid, eventid, cardid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:levelup_card(entityid, eventid, cardid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_levelup_puppet(entityid, eventid, cardid)
function script_entry_event_levelup_puppet(entityid,  eventid, cardid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:levelup_card(entityid, eventid, cardid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_event_reward(entityid, eventid, reward_index, entry_index)
function script_entry_event_reward(entityid, eventid, reward_index, entry_index)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end
	return g_copy:event_reward(entityid, eventid, reward_index, entry_index)
end

--
-- bool script_entry_event_choose_option(entityid, eventid, storyoptionid, option_index)
function script_entry_event_choose_option(entityid, eventid, storyoptionid, option_index)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:choose_option(entityid, eventid, storyoptionid, option_index)
	assert(rc ~= nil)
	return rc
end


--
------------------- puppet -------------------
--

--
-- bool script_entry_puppet_arrange_placeholder(entityid, target_entityid, placeholder)
function script_entry_puppet_arrange_placeholder(entityid, target_entityid, placeholder)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:arrange_placeholder(entityid, target_entityid, placeholder)
	assert(rc ~= nil)
	return rc
end


--
-- bool script_entry_puppet_destroy(entityid, target_entityid)
function script_entry_puppet_destroy(entityid, target_entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:destroy_puppet(entityid, target_entityid)
	assert(rc ~= nil)
	return rc
end


--
-- bool script_entry_item_use(entityid, itemid)
function script_entry_item_use(entityid, itemid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:use_item(entityid, itemid)
	assert(rc ~= nil)
	return rc
end

--
-- bool script_entry_equip_undress(entityid, slot)
function script_entry_equip_undress(entityid, slot)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    local rc = g_copy:remove_equip(entityid, slot)
	assert(rc ~= nil)
	return rc
end


--
------------------- match -------------------
--

--
-- prepare match
function script_entry_match_prepare()
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end
    
    return g_copy.scene:prepare_match()
end

--
-- start match
function script_entry_match_start()
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end
    
    g_copy.scene:start_match()
end

--
-- the play if can play a card
function script_entry_match_card_play_judge(entityid, cardid, pick_entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match.isdone then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return false
    end
    
	return g_copy.scene.match:card_play_judge(entityid, cardid, pick_entityid)
end

--
-- the player play a card
function script_entry_match_card_play(entityid, cardid, pick_entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:card_play(entityid, cardid, pick_entityid)
end

--
-- the player if can discard a card
function script_entry_match_card_discard_judge(entityid, cardid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return false
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return false
    end

    if g_copy.scene.match.isdone then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return false
    end
    
	return g_copy.scene.match:card_discard_judge(entityid, cardid)
end

--
-- the player discard a card
function script_entry_match_card_discard(entityid, cardid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:card_discard(entityid, cardid)
end

--
-- the player end this round
function script_entry_match_round_end(entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is nil, func: %s", Function()))
        return 
    end

    if g_copy.scene.match.isdone then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene.match is done, func: %s", Function()))
        return 
    end
    
	g_copy.scene.match:round_end(entityid)
end

--
-- abort match
function script_entry_match_abort(entityid)
    if g_copy == nil then 
        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, entityid: %d, func: %s", entityid, Function()))
        return 
    end

    if g_copy.scene == nil then
        cc.ScriptErrorLog(string.format(">>>>>> g_copy.scene is nil, func: %s", Function()))
        return 
    end

    g_copy.scene:abort_match(entityid)
end


