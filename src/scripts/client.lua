require('scripts/tools')

local fd_client

function slam_lua_event_start()
	print("event start");
	local rc = cc.response(fd_client, 1, {
		value_int32 = -1,
		value_uint32 = 1,
		value_int64 = -1234567890,
		value_uint64 = 1234567890,
		value_sint32 = -987654321,
		value_sint64 = -98765432100,
		value_fixed32 = 1000000000,
		value_fixed64 = 100000000000,
		value_sfixed32 = -1000000000,
		value_sfixed64 = -100000000000,
		value_bool = true,
		value_string = 'value_string, 中文',
		value_bytes = '我们是中文',
		value_repeated_int32 = {
			10, 20, -30, 40, -50
		},
		value_entity = {
			id = 9223372036854775808,
			name = '姓名:hushouguo',
			gender = 1
		}
	})
	assert(rc)
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
end)
cc.bindmsg(2, "slam.EchoResponse", function(fd, msgid, t)
	print(string.format("bind message from fd: %d, msgid: %d", fd, msgid))
	table.dump(t)
end)
fd_client = cc.newclient("127.0.0.1", 12306)
print(string.format("fd_client: %d", fd_client))

