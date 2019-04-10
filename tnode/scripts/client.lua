require('scripts/helper')

cc.log_trace("service: " .. _G["cc.service"])

cc.regmsg(1, "protocol.EchoRequest")
cc.regmsg(2, "protocol.EchoResponse")

function msgParser(fd, entityid, msgid, o)
	cc.log_trace("service: " .. _G["cc.service"] .. ", fd: " .. fd .. ", entityid: " .. entityid .. ", msgid: " .. msgid)
	dump(o)
end
