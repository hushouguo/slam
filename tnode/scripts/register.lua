require('scripts/helper')
cc.log_trace("service: " .. _G["cc.service"])
--cc.regmsg(20, "NetData.PlayerRegisterRequest")
--cc.regmsg(60003, "NetData.PlayerRegisterRequest")
--cc.regmsg(60005, "NetData.Heartbeat")
cc.loadmsg("protocol/echo.proto")
cc.regmsg(1, "protocol.EchoRequest")
cc.regmsg(2, "protocol.EchoResponse")
local total = 0
local times = 10000
local o = {
	value_bool = true,
	value_string = "hello, world",
	value_float = 1.23,
	value_double = 0.25,
	value_sint32 = -100,
	value_uint32 = 100,
	value_sint64 = -10000,
	value_uint64 = 10000
}
local n = 0
local t1 = cc.timemsec()
local str = cc.message_encode(1, o)
--while (n < times) do
--	str = cc.message_encode(1, o)
--	n = n + 1
--end
local t2 = cc.timemsec();

local len = #str
print("len: " .. len)
cc.log_trace("len: " .. len .. ", encode: " .. str .. ", len: ")
local oo = cc.message_decode(1, str)
dump(oo)
cc.log_trace("times: " .. times .. ", cost milliseconds: " .. (t2 - t1))

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
