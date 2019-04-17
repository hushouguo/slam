require('scripts/helper')
cc.newservice("scripts/register.lua")
--cc.newservice("scripts/register.lua")
--cc.newservice("scripts/client.lua")
--cc.newservice("scripts/benchmark.lua")
--cc.newserivce("scripts/client.lua")

local fd = cc.newserver("0.0.0.0", 12306)
if fd ~= -1 then
	cc.log_trace("server on")
else
	cc.log_alarm("newserver error")
end

local newplayer = {
	value_bool = true,
	value_string = "hello, world",
	value_float = 1.23,
	value_double = 0.25,
	value_sint32 = -100,
	value_uint32 = 100,
	value_sint64 = -10000,
	value_uint64 = 10000
}

local db = cc.newdb("127.0.0.1", "root", "", 3306)
assert(db)
local rc = db:select_database("tnode")
if rc == false then
	rc = db:create_database("tnode")
	assert(rc)
	rc = db:select_database("tnode")
	assert(rc)
end
rc = db:loadmsg("protocol/echo.proto")
assert(rc)
rc = db:regtable("player", "protocol.EchoRequest")
assert(rc)
local objectid = db:create_object("player", newplayer)
--local objectid = 8
cc.log_trace("objectid: " .. objectid)
local object = db:unserialize("player", objectid)
dump(object)


--local fd = cc.newclient("127.0.0.1", 12306)
--if fd ~= -1 then
--	cc.log_trace("client on")
--else
--	cc.log_alarm("newclient error")
--end

local n = 0
function dispatch(entityid, msgid)
	n = n + 1
	if ((n % 2) == 0) then
		return 1
	else
		return 2
	end
end

