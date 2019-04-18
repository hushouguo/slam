require('scripts/helper')
--dump(_G)
local sid = cc.newservice("scripts/test.lua")

function dispatch(entityid, msgid)
	return sid
end

