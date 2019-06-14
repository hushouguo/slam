
require('copy/copy')

--
------------------- unittest -------------------
--

require('tests/common')

local function unittest()
	init_common_interface()

    local enter_mapid = nil
    
	cc.EnterMap = function(entityid, mapid, layouts, coord)
		print('EnterMap: entityid: ' .. tostring(entityid) .. ', mapid: ' .. tostring(mapid))
		--table.dump(layouts, 'layouts')
		table.dump(coord, 'entry coord')
		enter_mapid = mapid
	end

	cc.ExitMap = function(entityid, mapid)
		print('ExitMap')
	end

	cc.MoveTrigger = function(entityid, mapid, v)
	    table.dump(v, "MoveTrigger")
	end

	cc.EventTrigger = function(entityid, mapid, eventid)
	    print("EventTrigger")
	end

	cc.EventBroken = function(entityid, mapid, eventid)
	end
	
	lua_entry_copy_enter(1, 4000)

	--os.exit()

	local function Sleep(n)
		os.execute("sleep " .. n)
	end
    
	while true do
		Sleep(1)
		lua_entry_update(1000)		
		lua_entry_copy_move_request(1, enter_mapid, math.random_between(0, 14), math.random_between(0, 14))
	end
end
unittest()
