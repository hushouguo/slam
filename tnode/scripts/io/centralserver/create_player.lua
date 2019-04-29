require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

local db
local database = "tnode"
local table = "player"

function init(sid)
	db = cc.newdb("127.0.0.1", "root", "", 3306)
	assert(db)
	local rc = db:create_database(database) and db:select_database(database)
	assert(rc)
	rc = db:loadmsg("scripts/io/protocol/protocol.proto") and db:regtable(table, "protocol.Player")
	assert(rc)
	return init_protocol()
end

function destroy()
end

local init_player = {
	id = 0,
	name = cc.random_string(6, false, true, false),
	maxhp = 100,
	curhp = 100,
	attack = cc.random_between(10, 20),
	defence = cc.random_between(1, 10),
	movespeed = cc.random_between(1, 10),
	attackspeed = cc.random_between(1000, 2000),
	coord = { x = 0, y = 0 },
	total_victory = 0,
	total_defeat = 0,
	total_match = 0,
	regtime = cc.timesec()
}

cc.domsg(entityid, msgid, function(fd, entityid, msgid, o) end)

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_CREATE_REQ then
		local objectid = db:create_object(table, init_player)
		assert(objectid)
		cc.log_debug("create player: " .. tostring(objectid))
		cc.response(fd, entityid, protocol.PLAYER_CREATE_REP, { id = objectid })
	else
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

