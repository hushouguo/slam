--
------------------- lua APIs -------------------
--

require('common')
require('tools/tools')
require('tools/astar')
require('tools/bit')
require('tools/noise')

require('match/buff')
require('match/card')
require('match/entity')
require('match/monster')
require('match/match')

require('copy/event')
require('copy/obstacle')
require('copy/map')
require('copy/copy')


--
------------------- copy -------------------
--

function lua_entry_copy_enter(entityid, copy_baseid)
    local t = RecordUnserialize(entityid, copy_baseid)
    g_copy = Copy:new(copy_baseid, t)
    g_copy:enter_map(false)
end

function lua_entry_copy_exit(entityid)
	assert(g_copy ~= nil)
	g_copy:exit_map(entityid)
end

function lua_entry_copy_move_request(entityid, mapid, destx, desty)
    assert(g_copy ~= nil)
    g_copy:move_request(entityid, mapid, destx, desty)
end

function lua_entry_copy_move(entityid, mapid, x, y)
	assert(g_copy ~= nil)
	g_copy:move(entityid, mapid, x, y)
end

function lua_entry_copy_event_retval(entityid, mapid, retval)
end

--
-- call me for every frame
function lua_entry_update(delta)
	assert(g_copy ~= nil)
	g_copy:update(delta)
end


--
------------------- match -------------------
--


--
-- prepare match
function lua_entry_match_prepare()
	g_match = Match:new()
	g_match:prepare()
end

--
-- add member of allies to match
function lua_entry_add_member(entityid)
	assert(g_match)
	g_match:add_member(entityid, Side.ALLIES)
end

--
-- add member of enemy to match
function lua_entry_add_opponent(entityid)
	assert(g_match)
	g_match:add_member(entityid, Side.ENEMY)
end

--
-- start match
function lua_entry_match_start()
	assert(g_match)
	g_match:start()
end

--
-- the play if can play a card
function lua_entry_card_play_judge(entityid, cardid, pick_entityid)
	assert(g_match)
	return g_match:card_play_judge(entityid, cardid, pick_entityid)
end

--
-- the player play a card
function lua_entry_card_play(entityid, cardid, pick_entityid)
	assert(g_match)
	g_match:card_play(entityid, cardid, pick_entityid)
end

--
-- the player if can discard a card
function lua_entry_card_discard_judge(entityid, cardid)
	assert(g_match)
	return g_match:card_discard_judge(entityid, cardid)
end

--
-- the player discard a card
function lua_entry_card_discard(entityid, cardid)
	assert(g_match)
	g_match:card_discard(entityid, cardid)
end

--
-- the player end this round
function lua_entry_round_end(entityid)
	assert(g_match)
	g_match:round_end(entityid)
end

--
-- call me for every frame
function lua_entry_update(delta)
	if g_match ~= nil then g_match:update(delta) end
end



