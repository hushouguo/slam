require('scripts/helper')

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

local lite_player = {
	value_string = "lite player"
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
--local id = db:create_object("player", 100, lite_player)
--cc.log_trace("id : " .. id)
--local objectid = db:create_object("player", lite_player)
local objectid = 10 
--cc.log_trace("objectid: " .. objectid)
local object = db:unserialize("player", objectid)
cc.log_trace("unserialize object: 10")
dump(object)

-- test modify old field
object.value_string = object.value_string .. ", update"
if (object.value_uint32 ~= nil) then
	object.value_uint32 = object.value_uint32 + 1
else
	object.value_uint32 = 1
end
--object.value_not_exist = "not exist"
rc = db:serialize("player", objectid, object);

-- unserialize again
local oo = db:unserialize("player", objectid)
cc.log_trace("unserialize object: 10 again")
dump(object)

db:flush("player", objectid);

