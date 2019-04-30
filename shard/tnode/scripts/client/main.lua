require('scripts/tools/tools')
--dump(_G)

--local fd_client = cc.newclient("client", "0.0.0.0", 12306)
--assert(fd_client)

for i = 1, 100, 1 do
	cc.newservice("scripts/client/client.lua")
end

function dispatch(entityid, msgid)
	return entityid
end

