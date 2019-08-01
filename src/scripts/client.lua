require('scripts/tools')

local fd_client

function slam_lua_event_start()
	print("event start");
	cc.response(fd_client, 1, {milliseconds = 101})
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
fd_client = cc.newclient("127.0.0.1", 12306)
print(string.format("fd_client: %d", fd_client))
