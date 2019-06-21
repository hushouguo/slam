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
require('copy/copy')


--
------------------- copy -------------------
--

function lua_entry_copy_enter(entityid, copy_baseid)
    g_copy = Copy:new(entityid, copy_baseid)
end

function lua_entry_copy_exit(entityid)
	assert(g_copy ~= nil)
	g_copy:remove_member(entityid)
end

function lua_entry_copy_move_request(entityid, sceneid, destx, desty)
    assert(g_copy ~= nil)
    return g_copy:move_request(entityid, sceneid, destx, desty)
end

function lua_entry_copy_move(entityid, sceneid, x, y)
	assert(g_copy ~= nil)
	g_copy:move(entityid, sceneid, x, y)
end

function lua_entry_copy_event_reward(entityid, sceneid, eventid, reward_index, entry_index)
	assert(g_copy ~= nil)
	return g_copy:event_reward(entityid, sceneid, eventid, reward_index, entry_index)
end

--
-- call me for every frame
function lua_entry_update(delta)
    if g_copy ~= nil then
        if g_copy:checkDone() then
            cc.WriteLog(string.format("g_copy: %d, %d closing soon", g_copy.id, g_copy.baseid))
            g_copy:destrcutor()
            g_copy = nil
        else
            g_copy:update(delta)
        end
    end
end


--
------------------- match -------------------
--

--
-- start match
function lua_entry_match_start()
    assert(g_copy ~= nil)
    g_copy.scene:start_match()
end

--
-- the play if can play a card
function lua_entry_card_play_judge(entityid, cardid, pick_entityid)
    assert(g_copy ~= nil)
    if g_copy.scene.match == nil or g_copy.scene.match.isdone then return false end -- match is over or done
	return g_copy.scene.match:card_play_judge(entityid, cardid, pick_entityid)
end

--
-- the player play a card
function lua_entry_card_play(entityid, cardid, pick_entityid)
    assert(g_copy ~= nil)
    if g_copy.scene.match == nil or g_copy.scene.match.isdone then return false end -- match is over or done
	g_copy.scene.match:card_play(entityid, cardid, pick_entityid)
end

--
-- the player if can discard a card
function lua_entry_card_discard_judge(entityid, cardid)
    assert(g_copy ~= nil)
    if g_copy.scene.match == nil or g_copy.scene.match.isdone then return false end -- match is over or done
	return g_copy.scene.match:card_discard_judge(entityid, cardid)
end

--
-- the player discard a card
function lua_entry_card_discard(entityid, cardid)
    assert(g_copy ~= nil)
    if g_copy.scene.match == nil or g_copy.scene.match.isdone then return false end -- match is over or done
	g_copy.scene.match:card_discard(entityid, cardid)
end

--
-- the player end this round
function lua_entry_round_end(entityid)
    assert(g_copy ~= nil)
    if g_copy.scene.match == nil or g_copy.scene.match.isdone then return false end -- match is over or done
	g_copy.scene.match:round_end(entityid)
end

--
-- abort match
function lua_entry_match_abort(entityid)
    assert(g_copy ~= nil)
    g_copy.scene:abort_match(entityid)
end

