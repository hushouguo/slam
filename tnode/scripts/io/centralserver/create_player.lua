require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

local db
local database = "tnode"
local table = "player"

local init_player = {
	id = 0,
	name = '',
	maxhp = 100,
	curhp = 100,
	attack = 0,
	defence = 0,
	movespeed = 0,
	attackspeed = 0,
	coord = { x = 0, y = 0 },
	total_victory = 0,
	total_defeat = 0,
	total_match = 0,
	regtime = 0
}

function init(sid)
	db = cc.newdb("127.0.0.1", "root", "", 3306)
	assert(db)
	local rc = db:create_database(database) and db:select_database(database)
	assert(rc)
	rc = db:loadmsg("scripts/io/protocol/protocol.proto") and db:regtable(table, "protocol.Player")
	assert(rc)

	return init_protocol()
end

local function setup_player()
	init_player.name = "robot_" .. cc.random_string(6, true, false, false)
	init_player.attack = cc.random_between(10, 20)
	init_player.defence = cc.random_between(1, 10)
	init_player.movespeed = cc.random_between(1, 10)
	init_player.attackspeed = cc.random_between(1000, 2000)
	init_player.regtime = cc.timesec()
	--dump(init_player)
end

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_CREATE_REQ then
		setup_player()
		local objectid = db:create_object(table, init_player)
		assert(objectid)
--		cc.log_debug("create player: " .. tostring(objectid))
		cc.response(fd, entityid, protocol.PLAYER_CREATE_REP, { id = objectid })
	else
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

function destroy()
end
