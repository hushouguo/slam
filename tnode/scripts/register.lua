require('scripts/helper')
cc.log_trace("service: " .. _G["cc.service"])
--cc.regmsg(20, "NetData.PlayerRegisterRequest")
--cc.regmsg(60003, "NetData.PlayerRegisterRequest")
--cc.regmsg(60005, "NetData.Heartbeat")
cc.loadmsg("protocol/echo.proto")
cc.regmsg(1, "protocol.EchoRequest")
cc.regmsg(2, "protocol.EchoResponse")
local total = 0
function msgParser(fd, entityid, msgid, o)
	cc.log_trace("service: " .. _G["cc.service"] .. ", fd: " .. fd .. ", entityid: " .. entityid .. ", msgid: " .. msgid)
	if (msgid == 1) then
		cc.log_trace("o: ")
		dump(o)
		cc.response(fd, entityid, 2, {
			value_bool = o.value_bool,
			value_string = o.value_string,
			value_float = o.value_float,
			value_double = o.value_double,
			value_sint32 = o.value_sint32,
			value_uint32 = o.value_uint32,
			value_sint64 = o.value_sint64,
			value_uint64 = o.value_uint64
		})
		total = total + 1
		if (total >= 3) then
			cc.closesocket(fd)
			cc.exitservice()
		end
	end
end
