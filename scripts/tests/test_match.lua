
require('match/match')

--
------------------- unittest -------------------
--

require('tests/common')

local function unittest()
	init_common_interface()



    local buffid_base = 100
	cc.BuffAdd = function(entityid, buff_baseid, buff_layers)
		--cc.WriteLog("add buff, entity: " .. entityid .. ", baseid: " .. buff_baseid .. ", layers: " .. buff_layers)
		local buffid = buffid_base
		buffid_base = buffid_base + 1
		return buffid
	end
	cc.BuffRemove = function(entityid, buffid)
		--cc.WriteLog("remove buff, entity: " .. entityid .. ", buffid: " .. buffid)
	end
	cc.BuffUpdateLayers = function(entityid, buffid, buff_layers)
		--cc.WriteLog("update buff layers, entity: " .. entityid .. ", buffid: " .. buffid .. ", layers: " .. buff_layers)
	end
	
	lua_entry_match_prepare()
	lua_entry_add_member(1)
	lua_entry_add_opponent(2)
	lua_entry_add_opponent(3)
	lua_entry_match_start()

	local function Sleep(n)
		os.execute("sleep " .. n)
	end

	while true do
		Sleep(1)
		if table.size(members[1].stack_hold) > 0 then
			for cardid in pairs(members[1].stack_hold) do
				lua_entry_card_play(members[1].id, cardid)
				if g_match.isdone then os.exit(0) end
			end
			table.clear(members[1].stack_hold)
			lua_entry_round_end(members[1].id)
		end
		lua_entry_update(1000)
	end
end
unittest()
