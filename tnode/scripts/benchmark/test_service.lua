local timer = cc.newtimer(1000, -1, nil, function(id, ctx)
	cc.log_trace("[" .. _G["cc.SERVICE"] .. "]: " .. id .. " tick")
end)

