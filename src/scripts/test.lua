require('scripts/tools')

function slam_lua_event_start()
	print("event start");
end

function slam_lua_event_stop()
	print("event stop");
end

function slam_lua_event_establish_connection(fd)
	print(string.format("establish connection: %d", fd))
end

function slam_lua_event_lost_connection(fd)
	print(string.format("lost connection: %d", fd))
end

function slam_lua_event_message(fd, msgid, t)
	print(string.format("event message from fd: %d, msgid: %d", fd, msgid))
	table.dump(t)
end

--cc.loadmsg("protos")
--cc.regmsg(1, "slam.EchoRequest")
--cc.regmsg(2, "slam.EchoResponse")
--cc.newserver("127.0.0.1", 12306)
--

local count = 0
local timer1 = cc.newtimer(1000, true, '1000', function(timerid, ctx)
	print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
end)
local timer2 = cc.newtimer(2000, true, '2000', function(timerid, ctx)
	print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
	count = count + 1
	if count == 3 then
		cc.removetimer(timer1)
	end
end)

--[[
local count = 0
local timer1 = cc.newtimer(100, true, '1000', function(timerid, ctx)
	print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
end)

local timer2 = cc.newtimer(2000, true, '2000', function(timerid, ctx)
	print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
	count = count + 1
	if count == 3 then 
		cc.removetimer(timer1)
		local timer3 = cc.newtimer(3000, true, '3000', function(timerid, ctx) 
			print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
		end)
	end
end)
--]]

--[[cc.loadmsg("protos")
cc.bindmsg(1, "slam.EchoRequest", function(fd, msgid, t)
	print(string.format("bind message from fd: %d, msgid: %d", fd, msgid))
	table.dump(t)
end)
]]--


local db = cc.newdb("127.0.0.1", "root", "", 3306)
assert(db ~= nil)

table.dump(db)

--[[
local t = db:query("SHOW DATABASES")
assert(t ~= nil)
table.dump(t)

local rc = db:execute("USE `slam`")
assert(rc)
local t = db:query("select * from user")
assert(t ~= nil)
table.dump(t)
--]]

local rc = db:execute("USE `test`")
assert(rc)
--local t = db:insert("test1", {id=2, name='dany'})
local t = db:query("select * from test1")
assert(t)
table.dump(t)
--local t = db:insert("test1", {id=3, name='tom'})
assert(t)
local t = db:update("test1", {name = 'spanish'}, 'id=3')
assert(t)


