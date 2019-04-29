require('scripts/tools/tools')
require('scripts/io/protocol/protocol')

--dump(_G)

-- create robot player
local sid_robot_init = 0
local sid_robot_size = 100
local stable_robot = {}

function init(sid)
	--
	-- create services for robotPlayer
	for i = sid_robot_init, sid_robot_size - 1 do
		stable_robot[i] = cc.newservice("scripts/io/client/robot.lua")
		assert(stable_robot[i])
	end

	return true
end

function destroy()
end

function dispatch(entityid, msgid)
	return entityid
end

