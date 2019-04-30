require('scripts/tools/tools')
--dump(_G)

local fd_server = cc.newserver("server", "0.0.0.0", 12306)
assert(fd_server)

local stable = {}
for i = 1, 10, 1 do
	stable[i] = cc.newservice("scripts/server/server.lua")
end

function dispatch(entityid, msgid)
	local i = cc.random_between(1, 10)
	return stable[i]
end

