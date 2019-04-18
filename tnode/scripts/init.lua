require('scripts/helper')
--dump(_G)
--local sid = cc.newservice("scripts/test.lua")

local log = cc.newlog()
assert(log)
log:level(0)
log:autosplit_day(true)
log:autosplit_hour(false)
log:destination("./.logs")
log:tofile(0, "lua")
log:tostdout(0, true)

log:debug("debug");
log:error("error");

function dispatch(entityid, msgid)
	return sid
end

