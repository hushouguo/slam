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

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_LOGIN_REQ then
		local object = db:unserialize(table, o.id)
		assert(object)
		if object.id == 0
			then -- first login
			object.id = o.id
		end
		assert(object.id == o.id)
		cc.log_debug("login player: " .. tostring(o.id))
		cc.response(fd, entityid, protocol.PLAYER_LOGIN_REP, { id = object.id, player = object })
	else
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

