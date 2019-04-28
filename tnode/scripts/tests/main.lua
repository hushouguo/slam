require('scripts/tools/tools')

local value1 = 0x1234abcd5678ef20
local value2 = 0x1234abcd5678ef00
local value = value1 - value2
cc.log_trace(string.format("%u", value))
cc.log_trace(string.format("%x", value))
--print(bit.tohex(value))
--print(bit.tohex(bit.lshift(value, 16)))
--print(bit.tohex(bit.rshift(value, 16)))

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

