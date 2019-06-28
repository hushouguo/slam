--++++++++++++++++++++++++++++++++++++++

-- 副本脚本

--++++++++++++++++++++++++++++++++++++++
module(...,package.seeall)

local Random = Random or require('modules.random')

require('modules.extend')

-- local LookupTable = cc.LookupTable  -- require cc

-- EVENT table
-- TODO: 做成配置文件
local event_monster  = {7001,7002,7003,7004,7005,7006,7007,7008,7009,7010,7012,7013}
local event_elite    = {7101,7102,7103,7104,7105}
local event_boss     = {7201,7202,7203}
local event_shop     = {7301,7302}
local event_destroy  = {7401}
local event_stroy    = {7501,7502,7503}

local prob_data = {
    --          怪物    精英      商店      删卡   对话事件  休息
    [0]    =  { M = .6, J = .08, S = .05, D = .1, E = .22, R = .12   },  -- 不要求和=1
    [4000] =  { M = .6, J = .08, S = .05, D = .1, E = .22, R = .12   },
}

--- 生成房间事件总量 
-- @param copy_id       number 副本ID
-- @param copy_layers   number 副本层数
-- @param random_func   func   随机函数1
-- return 返回一个符合标准正太分布的随机数
local function generateRoomCount(copy_id,copy_layers,random_func)
    local random_func  = random_func or math.random
    local max = 14
    local min = 2

    -- 获取房间数量期望
    local mu = 6 + random_func() + copy_layers * 0.1  -- 每层多一个随机点    
    local mi = mu / 7                                 -- 标准差  99%的几率在 3个标准差之内

    local room_count = Random.randomNormal(mu,mi,random_func) -- TODO :这里有随机,传入序列化的随机数种子
    room_count = room_count - room_count % 1e-2
    room_count = math.clamp(room_count,min,max)

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
}


local function generateRoomContent(rooms)
    local t = {}
    for _,v in ipairs(rooms) do
        t[#t+1] = {event_baseid = pickFuncTable[v](),_COMMENT = v}
    end
    return t
end


--- 通用副本生成
-- return { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
local function generateDungeonLayer(entity_career,copy_baseid,copy_layers,randomseed)

    do -- maxLayer 判断
        local copy_base = cc.LookupTable("Copy", copy_baseid)
        local max_layers = copy_base.max_layers
        if max_layers == -1 then
            -- TODO  无尽副本
        else
            if copy_layers > max_layers then return nil end 
        end   
        
    end

    local random_func = Random.newRandom(randomseed)
    local room_count = generateRoomCount(copy_baseid,copy_layers,random_func)
    local picked_elements = generateRoomType(copy_baseid,copy_layers,room_count,random_func)
    local map_events = generateRoomContent(picked_elements)
    local map_baseid = 5000

    return {
        map_baseid = map_baseid,
        map_events = map_events
    }
end

--- 通用副本生成脚本
-- 这个脚本随机返回一个副本的内容,一般作为测试用例返回
-- @param entity_career  职业
-- @param copy_baseid    副本ID
-- @param copy_layers    副本层数
-- @param randomseed     种子
common = function(entity_career, copy_baseid, copy_layers, randomseed)
    return generateDungeonLayer(entity_career,copy_baseid,copy_layers,randomseed)
end
