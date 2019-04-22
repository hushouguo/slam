require('scripts/tools/tools')

--dump(_G)
local sid = cc.newservice("scripts/tests/test.lua")

function dispatch(entityid, msgid)
	return sid
end

