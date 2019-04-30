require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

function init(sid)
	return init_protocol()
end

function destroy()
end

local MATCH_SIZE = 8
local members = 0
local mtable = {}
local sceneid = 1
local function match()
	if members < MATCH_SIZE
		then
		return
	end

	local i = 0 
	local res = {
		matchid = sceneid,
		member = {
		}
	}
	
	for id, fd in pairs(mtable) do
		res.member[i] = {
			id = id
		}
		i = i + 1
		members = members - 1
		if i == MATCH_SIZE
			then
			break
		end
	end

	for id, fd in pairs(mtable) do
		mtable[id] = nil
		cc.response(fd, sceneid, protocol.PLAYER_MATCH_REP, res)
	end
	
	sceneid = sceneid + 1

	cc.log_trace("sceneid: " .. tostring(sceneid) .. ", members: " .. tostring(members))
end

function msgParser(fd, entityid, msgid, o)
	if msgid == protocol.PLAYER_MATCH_REQ 
		then
		--dump(o)
		mtable[o.id] = fd
		members = members + 1
		cc.log_trace("id: " .. tostring(o.id) .. " match: " .. tostring(fd) .. ", members: " .. tostring(members))
		if members >= MATCH_SIZE
			then
			match()
		end
	else
		cc.closesocket(fd)
		cc.log_error("unhandled message: " .. tostring(msgid))
	end
end

