require('scripts/tools/tools')

local t1 = cc.newtimer(1000, -1, nil, function(id, ctx)
	cc.log_trace("t1(1000) tick")
	test_timer()
end)

local t2 = cc.newtimer(100, 10, nil, function(id, ctx)
	cc.log_trace("t2(100) tick")
end)

local t3 = cc.newtimer(20000, -1, nil, function(id, ctx)
	cc.log_trace("t3 tick")
end)

function test_timer()
	cc.set_timer_interval(t3, 100)
	cc.remove_timer(t1)
end

