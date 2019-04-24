require('scripts/tools/tools')

local db = cc.newdb("127.0.0.1", "root", "", 3306)
assert(db)
local database = "tnode"
local table = "player"
local rc = db:create_database(database)
assert(rc)
rc = db:select_database(database)
assert(rc)
rc = db:loadmsg("protocol/echo.proto") and db:regtable(table, "protocol.EchoRequest")
assert(rc)

cc.newtimer(200, -1, nil, function(id, ctx)
	test_db()
end)

local sid = _G["cc.SERVICE"]
local count = 0
local objects = {}

function test_db()
	if count < 100 then
		local objectid = db:create_object(table, {value_string=tostring(sid), value_uint32=0})
		assert(objectid)
		local object = db:unserialize(table, objectid)
		assert(objectid)
		objects[objectid] = nil
		count = count + 1
	else
		for k, v in pairs(objects) do
			local object = db:unserialize(table, k)
			assert(objectid)
			object.value_uint32 = object.value_uint32 + 1
			local rc = db:serialize(table, objectid, object)
			assert(rc)
		end
	end
end

