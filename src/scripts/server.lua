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

cc.loadmsg("protos")
cc.bindmsg(1, "slam.EchoRequest", function(fd, msgid, t)
	print(string.format("bind message from fd: %d, msgid: %d", fd, msgid))
	table.dump(t)
	cc.response(fd, 2, {milliseconds = 1})
end)
cc.bindmsg(2, "slam.EchoResponse", function(fd, msgid, t)
	print(string.format("bind message from fd: %d, msgid: %d", fd, msgid))
	table.dump(t)
end)
cc.newserver("127.0.0.1", 12306)

local timer1 = cc.newtimer(1000, true, '1000', function(timerid, ctx)
	print(string.format("os.time: %d, timerid: %d, ctx: %s", os.time(), timerid, tostring(ctx)))
end)

