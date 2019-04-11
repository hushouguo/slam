require("scripts/helper")

cc.log_trace("login service start")
cc.log_debug("this is debug log")
cc.log_error("this is error log")
cc.log_alarm("this is alarm log")
cc.log_trace("timestamp: " .. cc.timestamp())
cc.log_trace("{a=1, [2]=2, name='hushouguo'}")
local o = {a=1, [2]=2, name='hushouguo'}
local jsonstr = cc.json_encode(o);
cc.log_trace(jsonstr)
local oo = cc.json_decode(jsonstr);
dump(oo)
cc.log_trace(oo.a)
cc.log_trace(oo.name)
cc.log_trace(oo["2"])

cc.log_trace("hash_string('hushouguo'): " .. cc.hash_string('hushouguo'))
cc.log_trace("random: " .. cc.random())
cc.log_trace("random_between(1, 100): " .. cc.random_between(1, 100))
cc.log_trace("base64")
local rawstring = "hushouguo";
local encodedstring = cc.base64_encode(rawstring)
cc.log_trace("raw: " .. rawstring .. ", encodestring: " .. encodedstring .. ", decode: " .. cc.base64_decode(encodedstring))
local digest = cc.md5(rawstring)
dump(digest)
cc.log_trace("msleep")
cc.msleep(1000)
cc.log_trace("wakeup")
cc.log_trace("second: " .. cc.timesec() .. ", millisecond: " .. cc.timemsec())

function test_message_parser()
	local o = {
		people = {
			{
				name = "hushouguo",
				id = 1,
				email = "kilimajaro@gmail.com",
				phones = {
					{
						number = "086-87654321",
						type = 2
					},
					{
						number = "086-12345678",
						type = 1
					}
				}
			}
		}
	}
	local s = cc.message_encode(1, o)
	print("s: " .. s)
	local o = cc.message_decode(1, s)
	dump(o)
end

--cc.regmsg(1, "tnode.AddressBook")
--test_message_parser()

cc.log_trace("prepare register timer")
local t1 = cc.newtimer(1000, -1, "this is a timer 1000", function(id, ctx)
	cc.log_trace("timer tick, id: " .. id .. ", ctx: " .. ctx)
end)
local t2 = cc.newtimer(2000, 2, "this is a timer 2000", function(id, ctx)
	cc.log_trace("timer tick, id: " .. id .. ", ctx: " .. ctx)
end)
local t3 = cc.newtimer(3000, 5, "this is a timer 3000", function(id, ctx)
	cc.log_trace("timer tick, id: " .. id .. ", ctx: " .. ctx)
end)
cc.log_trace("t1: " .. t1 .. ", t2: " .. t2 .. ", t3: " .. t3)

