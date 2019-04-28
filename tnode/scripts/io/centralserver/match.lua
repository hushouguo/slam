require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

function init(sid)
	return init_protocol()
end

function destroy()
end

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_MATCH_REQ then
	else
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

