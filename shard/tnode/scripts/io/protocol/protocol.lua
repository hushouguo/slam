--
-- protocol.MSGID

protocol = {
	PLAYER_CREATE_REQ	=	1,
	PLAYER_CREATE_REP	=	2,
	PLAYER_LOGIN_REQ	=	3,
	PLAYER_LOGIN_REP	=	4,
	PLAYER_MATCH_REQ	=	5,
	PLAYER_MATCH_REP	=	6,
	PLAYER_MOVE_REQ		=	7,
	PLAYER_MOVE_REP		=	8,
	PLAYER_ATTACK_REQ	=	9,
	PLAYER_ATTACK_REP	=	10,
	PLAYER_DEATH_REQ	=	11,
	PLAYER_DEATH_REP	=	12,
}

function init_protocol()
	return
	cc.loadmsg("scripts/io/protocol/protocol.proto")
	and cc.regmsg(protocol.PLAYER_CREATE_REQ, "protocol.PlayerCreateRequest")
	and cc.regmsg(protocol.PLAYER_CREATE_REP, "protocol.PlayerCreateResponse")
	and cc.regmsg(protocol.PLAYER_LOGIN_REQ, "protocol.PlayerLoginRequest")
	and cc.regmsg(protocol.PLAYER_LOGIN_REP, "protocol.PlayerLoginResponse")
	and cc.regmsg(protocol.PLAYER_MATCH_REQ, "protocol.PlayerMatchRequest")
	and cc.regmsg(protocol.PLAYER_MATCH_REP, "protocol.PlayerMatchResponse")
	and cc.regmsg(protocol.PLAYER_MOVE_REQ, "protocol.PlayerMoveRequest")
	and cc.regmsg(protocol.PLAYER_MOVE_REP, "protocol.PlayerMoveResponse")
	and cc.regmsg(protocol.PLAYER_ATTACK_REQ, "protocol.PlayerAttackRequest")
	and cc.regmsg(protocol.PLAYER_ATTACK_REP, "protocol.PlayerAttackResponse")
	--and cc.regmsg(protocol.PLAYER_DEATH_REQ, "protocol.PlayerDeathRequest")
	and cc.regmsg(protocol.PLAYER_DEATH_REP, "protocol.PlayerDeathResponse")
end
