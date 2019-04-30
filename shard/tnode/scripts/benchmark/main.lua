require('scripts/tools/tools')
--dump(_G)

local stable = {
}

for i = 1, 100, 1 do
	stable[i] = cc.newservice("scripts/benchmark/test_service.lua")
end

function dispatch(entityid, msgid)
	return sid
end

