require('scripts/tools/tools')

local db = cc.newdb("127.0.0.1", "root", "", 3306)
assert(db)
local database = cc.timesec()
local table = "player"
local rc = db:create_database(tostring(database))
assert(rc)
rc = db:select_database(tostring(database))
assert(rc)
rc = db:loadmsg("protocol/echo.proto") and db:regtable(table, "protocol.EchoRequest")
assert(rc)

local objectid = 1;
local object = db:unserialize(table, objectid)
cc.log_trace(string.format("[%4s] test `db:unserialize` inexist", object == nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:delete_object` inexist", db:delete_object(table, objectid) == false and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:flush_object` inexist", db:flush_object(table, objectid) == false and "OK" or "FAIL"))
object = {value_string = "this is a object"}
cc.log_trace(string.format("[%4s] test `db:serialize` inexist", not db:serialize(table, objectid, object) and "OK" or "FAIL"))

objectid = db:create_object(table, object)
cc.log_trace(string.format("[%4s] test `db:create_object`", objectid == 1 and "OK" or "FAIL"))
