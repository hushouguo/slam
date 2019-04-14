cc.newservice("scripts/register.lua")
--cc.newservice("scripts/register.lua")
--cc.newservice("scripts/client.lua")
--cc.newservice("scripts/benchmark.lua")
--cc.newserivce("scripts/client.lua")

local fd = cc.newserver("0.0.0.0", 12306)
if fd ~= -1 then
	cc.log_trace("server on")
else
	cc.log_alarm("newserver error")
end




--local fd = cc.newclient("127.0.0.1", 12306)
--if fd ~= -1 then
--	cc.log_trace("client on")
--else
--	cc.log_alarm("newclient error")
--end

local n = 0
function dispatch(entityid, msgid)
	n = n + 1
	if ((n % 2) == 0) then
		return 1
	else
		return 2
	end
end

