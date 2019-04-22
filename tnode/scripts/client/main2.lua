require('scripts/helper')
--dump(_G)

local fd_client = cc.newclient("client", "0.0.0.0", 12306)
assert(fd_client)

for i = 1, 10, 1 do
	cc.newservice("scripts/client/client2.lua")
end

function dispatch(entityid, msgid)
	return entityid
end

