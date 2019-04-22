--
-- logger
cc.log_debug(string.format("[%4s] test `log_debug",	"OK"))
cc.log_trace(string.format("[%4s] test `log_trace",	"OK"))
cc.log_alarm(string.format("[%4s] test `log_alarm",	"OK"))
cc.log_error(string.format("[%4s] test `log_error",	"OK"))

local log = cc.newlog()
cc.log_trace(string.format("[%4s] test `newlog`", log ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:level`", log:level(0) == 0 and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:autosplit_day`", log:autosplit_day(true) and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:autosplit_hour`", log:autosplit_hour(false) == false and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:destination`", log:destination("./.logs") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:tofile`", log:tofile(0, "lua") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `log:tostdout`", log:tostdout(0, true) and "OK" or "FAIL"))
log:debug(string.format("[%4s] test `log:debug",	"OK"))
log:trace(string.format("[%4s] test `log:trace",	"OK"))
log:alarm(string.format("[%4s] test `log:alarm",	"OK"))
log:error(string.format("[%4s] test `log:error",	"OK"))


--
-- random
cc.log_trace(string.format("[%4s] test `random`", cc.random() ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `random_between`", cc.random_between(1, 1) == 1 and "OK" or "FAIL"))

--
-- hash_string & md5
cc.log_trace(string.format("[%4s] test `hash_string`", cc.hash_string("cc.hash_string") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `md5`", cc.md5("cc.md5") ~= nil and "OK" or "FAIL"))

--
-- base64 encode & decode
cc.log_trace(string.format("[%4s] test `base64_encode`", cc.base64_encode("cc.base64_encode") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `base64_decode`", cc.base64_decode("cc.base64_decode") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `base64`", cc.base64_decode(cc.base64_encode("cc.base64")) == "cc.base64" and "OK" or "FAIL"))

--
-- time
cc.log_trace(string.format("[%4s] test `timesec`", cc.timesec() ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `timemsec`", cc.timemsec() ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `timestamp`", cc.timestamp() ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `timestamp`", cc.timestamp(cc.timesec()) ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `timestamp`", cc.timestamp("%Y/%02m/%02d %02H:%02M:%02S") ~= nil and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `timestamp`", cc.timestamp(cc.timesec(), "%Y/%02m/%02d %02H:%02M:%02S") ~= nil and "OK" or "FAIL"))

--
-- msleep
local t1 = cc.timemsec()
cc.msleep(10)
local t2 = cc.timemsec()
cc.log_trace(string.format("[%4s] test `msleep`", (t2 - t1) >= 10 and "OK" or "FAIL"))

--
-- timer
cc.newtimer(10, 1, 1, function(id, ctx)
	cc.log_trace(string.format("[%4s] test `newtimer`", ctx == 1 and "OK" or "FAIL"))
end)

--
-- json encode & decode
local o = { id = 1, name = 'hushouguo' }
local json_encode_string = cc.json_encode(o)
cc.log_trace(string.format("[%4s] test `json_encode`", json_encode_string ~= nil and "OK" or "FAIL"))
local table = cc.json_decode(json_encode_string)
cc.log_trace(string.format("[%4s] test `json_decode`", (table ~= nil and table.id == 1) and "OK" or "FAIL"))

--
-- xml decode
local table = cc.xml_decode("conf/conf.xml")
cc.log_trace(string.format("[%4s] test `xml_decode`", (table ~= nil and table.log.level == 0) and "OK" or "FAIL"))

--
-- service
cc.log_trace(string.format("[%4s] test `newservice`", cc.newservice("scripts/tests/newservice.lua") ~= -1 and "OK" or "FAIL"))

--
-- network
cc.log_trace(string.format("[%4s] test `loadmsg`", cc.loadmsg("protocol/echo.proto") and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `regmsg`", cc.regmsg(1, "protocol.EchoRequest") and "OK" or "FAIL"))

local fd_server = cc.newserver("server", "0.0.0.0", 12306)
local fd_client = cc.newclient("client", "0.0.0.0", 12306)
local playerid = 1000
cc.log_trace(string.format("[%4s] test `newserver`", fd_server ~= -1 and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `newclient`", fd_client ~= -1 and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `response`", cc.response(fd_client, playerid, 1, {value_string = "test_response"}) == nil and "OK" or "FAIL"))
function msgParser(fd, entityid, msgid, o)
	cc.log_trace(string.format("[%4s] test `msgParser`", (msgid == 1 and entityid == playerid) and "OK" or "FAIL"))
	cc.log_trace(string.format("[%4s] test `closesocket`", cc.closesocket(fd) == nil and "OK" or "FAIL"))
end

--
-- message encode & decode
local o = { value_string = "this is a lua table" }
local message_encode_string = cc.message_encode(1, o)
cc.log_trace(string.format("[%4s] test `message_encode`", message_encode_string ~= -1 and "OK" or "FAIL"))
local table = cc.message_decode(1, message_encode_string)
cc.log_trace(string.format("[%4s] test `message_decode`", (table ~= nil and table.value_string == "this is a lua table") and "OK" or "FAIL"))

--
-- db
local db = cc.newdb("127.0.0.1", "root", "", 3306)
cc.log_trace(string.format("[%4s] test `newdb`", db ~= nil and "OK" or "FAIL"))
local database = cc.timesec()
local table = "player"
cc.log_trace(string.format("[%4s] test `db:create_database`", db:create_database(tostring(database)) and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:select_database`", db:select_database(tostring(database)) and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:loadmsg`", db:loadmsg("protocol/echo.proto") and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:regtable`", db:regtable(table, "protocol.EchoRequest") and "OK" or "FAIL"))
local objectid = db:create_object(table, {value_string = "this is a object"})
cc.log_trace(string.format("[%4s] test `db:create_object`", objectid ~= nil and "OK" or "FAIL"))
local object = db:unserialize(table, objectid)
object.value_uint32 = 100
cc.log_trace(string.format("[%4s] test `db:serialize`", db:serialize(table, objectid, object) and "OK" or "FAIL"))
object = db:unserialize(table, objectid)
cc.log_trace(string.format("[%4s] test `db:unserialize`", (object ~= nil and object.value_string == "this is a object" and object.value_uint32 == 100) and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:flush_object`", db:flush_object(table, objectid) and "OK" or "FAIL"))
cc.log_trace(string.format("[%4s] test `db:delete_object`", db:delete_object(table, objectid) and "OK" or "FAIL"))

