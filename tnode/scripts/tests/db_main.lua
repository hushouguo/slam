require('scripts/tools/tools')
--dump(_G)

--for i = 1, 100, 1 do
--	cc.newservice("scripts/tests/db.lua")
--end

--local db
local database = "tnode"
local table = "test"
function serialize(sid)
	local db = cc.newdb("127.0.0.1", "root", "", 3306)
	assert(db)
	local rc = db:create_database(database) and db:select_database(database)
	assert(rc)
	rc = db:loadmsg("protocol/echo.proto") and db:regtable(table, "protocol.EchoRequest")
	assert(rc)
	local objectid = db:create_object(table, {value_string=tostring(sid), value_uint32=0})
	assert(objectid)
	local object = db:unserialize(table, objectid)
	assert(object)
	dump(object)
	cc.log_trace("value_string: " .. object.value_string)
	return objectid
end

function unserialize(objectid)
	local db = cc.newdb("127.0.0.1", "root", "", 3306)
	assert(db)
	local rc = db:create_database(database) and db:select_database(database)
	assert(rc)
	rc = db:loadmsg("protocol/echo.proto") and db:regtable(table, "protocol.EchoRequest")
	assert(rc)

	local object = db:unserialize(table, objectid)
	assert(objectid)
	cc.log_trace("unserialize value_string: " .. object.value_string)
end


function init(sid)
	local objectid = serialize(sid)
	unserialize(objectid)
	return true
end

function destroy()
end

