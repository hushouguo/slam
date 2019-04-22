cc.log_trace("cc.SERVICE: " .. _G["cc.SERVICE"])
cc.log_trace(string.format("[%4s] test `exitservice`", cc.exitservice() == nil and "OK" or "FAIL"))
