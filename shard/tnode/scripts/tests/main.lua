require('scripts/tools/tools')

--dump(_G)
--local sid = cc.newservice("scripts/tests/test.lua")

function init(sid)
	return true
end

function destroy()
end

function dispatch(entityid, msgid)
	return sid
end

