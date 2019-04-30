require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

local fd_robotclient

function init(sid)
	fd_robotclient = cc.newclient("127.0.0.1", 12306)
	assert(fd_robotclient)
	local rc = init_protocol()
	assert(rc)
	return cc.response(fd_robotclient, sid, protocol.PLAYER_CREATE_REQ, {})
end

function destroy()
end

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_CREATE_REP
		then
		cc.response(fd, entityid, protocol.PLAYER_LOGIN_REQ, {id = o.id})
	elseif msgid == protocol.PLAYER_LOGIN_REP
		then
		--dump(o)
		cc.log_trace("id: " .. tostring(o.id) .. " login OK")
		cc.response(fd, entityid, protocol.PLAYER_MATCH_REQ, {id = o.id})
	elseif msgid == protocol.PLAYER_MATCH_REP
		then
		cc.log_trace("received match response")
	else
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

