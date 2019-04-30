require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

--dump(_G)

local fd_server = nil

-- create player
local sid_create_player_init = 0
local sid_create_player_size = 4 
local stable_create_player = {}

-- login player
local sid_login_player_init = 0
local sid_login_player_size = 4
local stable_login_player = {}

-- match
local sid_match_player = nil

-- scene
local sid_scene_init = 0
local sid_scene_size = 8
local stable_scene = {}

function init(sid)
	--
	-- create listening
	fd_server = cc.newserver("0.0.0.0", 12306)
	assert(fd_server)

	--
	-- create 4 services for createPlayer
	for i = sid_create_player_init, sid_create_player_size - 1, 1 do
		stable_create_player[i] = cc.newservice("scripts/io/centralserver/create_player.lua")
		assert(stable_create_player[i])
	end

	--
	-- create 4 services for loginPlayer
	for i = sid_login_player_init, sid_login_player_size - 1, 1 do
		stable_login_player[i] = cc.newservice("scripts/io/centralserver/login_player.lua")
		assert(stable_login_player[i])
	end

	--
	-- create service for match
	sid_match_player = cc.newservice("scripts/io/centralserver/match.lua")

	--
	-- create 8 services for scene
	for i = sid_scene_init, sid_scene_size - 1, 1 do
		stable_scene[i] = cc.newservice("scripts/io/centralserver/scene.lua")
		assert(stable_scene[i])
	end
	
	return true
end

function destroy()
end

function dispatch(entityid, msgid)
	local sid = -1
	if msgid == protocol.PLAYER_CREATE_REQ
		then
		sid = stable_create_player[sid_create_player_init]
		sid_create_player_init = sid_create_player_init + 1
		sid_create_player_init = sid_create_player_init % sid_create_player_size
	elseif msgid == protocol.PLAYER_LOGIN_REQ 
		then
		sid = stable_login_player[sid_login_player_init]
		sid_login_player_init = sid_login_player_init + 1
		sid_login_player_init = sid_login_player_init % sid_login_player_size
	elseif msgid == protocol.PLAYER_MATCH_REQ 
		then
		sid = sid_match_player
	else
		sid = entityid	-- entityid is sceneid
--	else
--		cc.log_error("unhandled message: " .. tostring(msgid))
	end
	return sid
end