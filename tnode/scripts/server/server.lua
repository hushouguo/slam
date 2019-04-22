require('scripts/helper')

local sid = _G["cc.SERVICE"]
local fd_server = cc.getsocket("server")
assert(fd_server ~= -1)
assert(cc.loadmsg("protocol/echo.proto") and cc.regmsg(1, "protocol.EchoRequest"))

function msgParser(fd, entityid, msgid, o)
	cc.log_trace("sid:" .. sid .. ", fd: " .. fd .. ", entityid: " .. entityid .. ", value_string: " .. o.value_string)
end
