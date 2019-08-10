--++++++++++++++++++++++++++++++++++++++

-- 副本脚本

--++++++++++++++++++++++++++++++++++++++
-- module(...,package.seeall)
local M={}


local Random = Random or require('modules.random')


-- require('modules.extend')

-- local ScriptLookupTable = cc.ScriptLookupTable  -- require cc

--------------------------Copy Gen Config------------------------------------------

-- TODO: 做成配置文件,配置项如下

-- 1.EVENT content table
local event_monster  = {7001,7002,7003,7004,7005,7006,7007,7008,7009,7010,7012,7013} -- M
local event_elite    = {7101,7102,7103,7104,7105}                                    -- J
local event_boss     = {7201,7202,7203}
local event_shop     = {7301,7302}                                                   -- S
local event_destroy  = {7401}                                                        -- D
local event_stroy    = {7501,7502,7503}                                              -- E
local event_upgradeCard    = {7701}                                                  -- U
local event_upgradePuppet = {7702}                                                   -- P
local event_option    = {7801}                                                       -- O

-- 2.probability
local prob_data = {
    --          怪物    精英      商店      删卡   对话事件  休息      升级
    [0]    =  { M = .6, J = .08, S = .05, D = .1, E = .22, R = .12, U = 0.1,P = 0.1, O = 0.1  },  -- 不需要求和=1
    [4000] =  { M = .6, J = .08, S = .05, D = .1, E = .22, R = .12, U = 0.1,P = 0.1, O = 0.1  },
}

-- 3.distribution
local room_count_min = 2
local room_count_max = 14


local CreateCopyData = function(mapid)
    local t = {}
    t._mapid = mapid
    t._events = {}

    --- 添加事件,随机一个id和坐标
    -- 坐标是可选
    function t:addEvent(event_baseid,x,y)
        local tt = {}

        if x and y then
            tt.coord = {x = x,y=y}
        end
        tt.event_baseid = event_baseid
        self._events[#self._events+1] = tt or false
    end

    function t:get_data()
        return {
            map_baseid = self._mapid,
            map_events = self._events
        }
    end
    return t
end


-------------------------- Gen Process -----------------------------------

--- 生成房间事件总量
-- @param copy_id       number 副本ID
-- @param copy_layers   number 副本层数
-- @param random_func   func   随机函数1
-- return 返回一个符合标准正太分布的随机数
local function generateRoomCount(copy_id,copy_layers,random_func)
    local random_func  = random_func or math.random

    -- 获取房间数量期望
    local mu = 6 + random_func() + copy_layers * 0.1  -- 每层多一个随机点
    local mi = mu / 7                                 -- 标准差  99%的几率在 3个标准差之内

    local room_count = Random.randomNormal(mu,mi,random_func) -- TODO :这里有随机,传入序列化的随机数种子
    room_count = room_count - room_count % 1e-2
    room_count = math.clamp(room_count,room_count_min,room_count_max)

    -- print("[MG]step1:roomCount = " .. room_count)
    return room_count
end


--- 按照数量和期望生成房间类型
-- TODO : 这个应该和难度相关
-- @param copy_id number 副本ID
-- @param level   number 副本层数
-- @param room_mean  number  房间节点的期望数量
-- @param random_func  function  随机数发生函数
-- @return 一个包含事件类型的列表
local function generateRoomType(copy_id,level,room_mean,random_func)

    local copy_id = copy_id or 4000                 -- test copy 4000
    local prob = prob_data[copy_id] or prob_data[0] -- default prob
    print(table.serialize(prob))
    assert(prob ~= nil)

    local room_mean = room_mean or 5                -- 期望
    local type_counts = table.map(prob,function(t) return room_mean * t end)

    -- 构建权重表
    local weight = {}
    for k,v in pairs(type_counts) do
         weight[k] = 2^v
    end

    -- 从权重表选择东西填充进表格
    --[[
        填充是这么一个逻辑:
        每当有元素被挑选出来,那么元素的权重会减半,小于1的权重再减半会清零
    ]]--
    local picked_elements = {}

    for  i = 1,room_mean do
        local picked = Random.randomWeighted(weight,random_func)
        -- 权重减半
        if picked then
           weight[picked]  = weight[picked] >= 1 and weight[picked] * 0.5 or 0
        end
        picked_elements[i] = picked or false
    end

    return picked_elements
end

-- TODO 转成数据库查询
local pickFuncTable = {
    M = function() return Random.pickRandom(event_monster) end,
    J = function() return Random.pickRandom(event_elite) end,
    S = function() return Random.pickRandom(event_shop) end,
    D = function() return Random.pickRandom(event_destroy) end,
    E = function() return Random.pickRandom(event_stroy) end,
    R = function() return Random.pickRandom(event_stroy) end,  -- TODO pick rest
    U = function() return Random.pickRandom(event_upgradeCard) end,
    P = function() return Random.pickRandom(event_upgradePuppet) end,
    O = function() return Random.pickRandom(event_option) end,
}


--- 通用副本生成
-- return { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
local function generateDungeonLayer(entityid,copy_baseid,copy_layers,randomseed)

    do -- maxLayer 判断
        local copy_base = cc.ScriptLookupTable("Copy", copy_baseid)
        local max_layers = copy_base.max_layers
        -- local max_layers = 15
        if max_layers == -1 then
            -- TODO  无尽副本
        else
            if copy_layers > max_layers then return nil end
        end
    end

    local random_func = Random.newRandom(randomseed)
    local room_count = generateRoomCount(copy_baseid,copy_layers,random_func)
    local picked_elements = generateRoomType(copy_baseid,copy_layers,room_count,random_func)

    local cd = CreateCopyData(5000)
    for _,v in ipairs(picked_elements) do
        cd:addEvent(pickFuncTable[v]())
    end

    return cd:get_data()
end

--- 通用副本生成脚本
-- 这个脚本随机返回一个副本的内容,测试用例
-- @param entityid  玩家ID
-- @param copy_baseid    副本ID(这个参数无用,因为调用的时候是知道ID是多少的)
-- @param copy_layers    副本层数
-- @param randomseed     种子
M.common = function(entityid, copy_baseid, copy_layers, randomseed)
    return generateDungeonLayer(entity_career,copy_baseid,copy_layers,randomseed)
end


--- alpha版本手动配置地图
M.manual = function(entityid,copy_baseid,copy_layers,randomseed)
    -- config
    local maxLayer = 2
    local maps = {5000,5001}

    if copy_layers > maxLayer then return nil end

    local map_index = (copy_layers+1) % 2 + 1

    return {
        map_baseid = maps[map_index],
        map_events = {}
    }
end

return  M