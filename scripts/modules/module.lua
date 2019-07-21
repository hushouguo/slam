--- 策划脚本入口
-- @author bw

require 'modules.extend'

Random       = require 'modules.random'

-- data
SPLBUFF      = require "modules.splecialbuff"

-- ext
EntityExt    = require "modules.entityext" -- TODO 整理require
BuffExt      = require "modules.buffext"

-- match
MatchUtil    = require 'modules.matchutil'

BuffScript   = require 'modules.buffscript'
CardScript   = require 'modules.cardscript'

-- copy
CopyUtil     = require	'modules.copyutil'

CopyScript   = require 'modules.copyscript'
EventScript  = require 'modules.eventscript'


ItemScript   = require 'modules.itemscript'
OptionScript = require 'modules.optionscript'