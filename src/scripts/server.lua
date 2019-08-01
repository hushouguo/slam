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
cc.regmsg(1, "slam.EchoRequest")
cc.regmsg(2, "slam.EchoResponse")
cc.newserver("127.0.0.1", 12306)

