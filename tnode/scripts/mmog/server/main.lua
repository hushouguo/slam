require('scripts/tools/tools')
require('scripts/mmog/protocol/protocol')

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

-- scene
local sid_scene_init = 0
local sid_scene_size = 4
local stable_scene = {}

function init(sid)
	--
	-- create listening
	fd_server = cc.newserver("server", "0.0.0.0", 12306)
	assert(fd_server)

	--
	-- create 4 services for createPlayer
	for i = sid_create_player_init, sid_create_player_size - 1, 1 do
		stable_create_player[i] = cc.newservice("scripts/mmog/create_player.lua")
		assert(stable_create_player[i])
	end

	--
	-- create 4 services for loginPlayer
	for i = sid_login_player_init, sid_login_player_size - 1, 1 do
		stable_login_player[i] = cc.newservice("scripts/mmog/login_player.lua")
		assert(stable_login_player[i])
	end

	--
	-- create services for scene
	for i = sid_scene_init, sid_scene_size - 1, 1 do
		stable_scene[i] = cc.newservice("scripts/mmog/scene.lua")
		assert(stable_scene[i])
	end
end

function destroy()
end

function dispatch(entityid, msgid)
	local sid = -1
	if msgid == protocol.PLAYER_CREATE then
		sid = stable_create_player[sid_create_player_init];
		sid_create_player_init = sid_create_player_init + 1
		sid_create_player_init = sid_create_player_init % sid_create_player_size
	else if msgid == protocol.PLAYER_LOGIN then
		sid = stable_login_player[sid_login_player_init];
		sid_login_player_init = sid_login_player_init + 1
		sid_login_player_init = sid_login_player_init % sid_login_player_size
	else if msgid == protocol.PLAYER_MATCH then

	else if msgid == protocol.PLAYER_MOVE then
		sid = entityid	-- entityid is sceneid, also is 
	else if msgid == protocol.PLAYER_ATTACK then
		sid = entityid
	else if msgid == protocol.PLAYER_DEATH then
		sid = entityid
	end
	return sid
end

