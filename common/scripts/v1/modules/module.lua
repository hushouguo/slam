--- 策划脚本入口
-- @author bw

require 'modules.extend'

Random       = require 'modules.random'

-- data
Stats      = require "modules.stats"       -- 战斗中的BUFF属性

-- ext
EntityExt    = require "modules.entityext"
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


Actions = require 'modules.actions'