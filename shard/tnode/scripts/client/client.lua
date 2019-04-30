require('scripts/tools/tools')

local sid = _G["cc.SERVICE"]

--local fd_client = cc.getsocket("client")
--assert(fd_client ~= -1)

local fd_client = cc.newclient("client" .. tostring(sid), "10.0.0.12", 12306)
assert(fd_client)

assert(cc.loadmsg("protocol/echo.proto") and cc.regmsg(1, "protocol.EchoRequest"))

function msgParser(fd, entityid, msgid, o)
	cc.log_trace("sid:" .. sid .. ", fd: " .. fd .. ", entityid: " .. entityid .. ", value_string: " .. o.value_string)
end

cc.newtimer(100, -1, nil, function(id, ctx)
	cc.response(fd_client, sid, 1, {value_string = "sid:" .. tostring(sid)})
end)
