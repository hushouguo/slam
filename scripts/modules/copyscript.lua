--++++++++++++++++++++++++++++++++++++++

-- 副本脚本

--++++++++++++++++++++++++++++++++++++++
module(...,package.seeall)

Random = Random or require('modules.random')
require('modules.extend')

local math = math

-- EVENT table

local event_monster  = {7001,7002,7003,7004,7005,7006,7007,7008,7009,7010,7012,7013}
local event_elite    = {7101,7102,7103,7104,7105}
local event_boss     = {7201,7202,7203}
local event_shop     = {7301,7302}
local event_destroy  = {7401}
local event_stroy    = {7501,7502,7503}

-- 生成房间事件总量
-- @param copy_id number 副本ID
-- @param level number 副本层数
-- return 返回一个符合标准正太分布的随机数
local function generateRoomCount(copy_id,level,random_func)
    local random_func  = random_func or math.random
    local max = 12
    local min = 2

    -- 获取房间数量期望
    local mu = 6 + random_func()
    local mi = mu / 7

    local room_count = Random.randomNormal(mu,mi,random_func) -- TODO :这里有随机,传入序列化的随机数种子
    room_count = room_count - room_count % 1e-2
    room_count = math.clamp(room_count,min,max)

    print("[MG]step1:roomCount = " .. room_count)   
    return room_count
end

-- 从表格随机一个
local function pickRandom(tbl,random_func)
    local random_func = random_func or math.random
    return tbl[random_func(#tbl)]
end

-- 按照数量和期望生成房间类型
-- TODO : 这个应该和难度相关
-- @param copy_id number 副本ID
-- @param level number 副本层数
-- @param room_count   房间节点的期望数量
-- return 一个包含事件类型的列表
local function generateRoomType(copy_id,level,room_count,random_func)
    -- 概率表(并不真实反映概率,权重)
    local prob = {
        M = .6,      -- 普通怪物 
        J = .08,     -- 精英
        S = .05,     -- 商店
        D = .1,      -- 删卡
        E = .22,     -- 选项事件
        R = .12,     -- 休息
    }
    local room_count = room_count or 5

    local type_counts = table.map(prob,function(t) return room_count * t end)

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

    for  i = 1,room_count do
        local picked = Random.randomWeighted(weight,random_func)
        -- 权重减半
        if picked then
           weight[picked]  = weight[picked] >= 1 and weight[picked] * 0.5 or 0 
        end
        picked_elements[i] = picked or false
    end

    return picked_elements
end

local pick_func = {
    M = function() return pickRandom(event_monster) end,
    J = function() return pickRandom(event_elite) end,
    S = function() return pickRandom(event_shop) end,
    D = function() return pickRandom(event_destroy) end,
    E = function() return pickRandom(event_stroy) end,   
    R = function() return pickRandom(event_stroy) end,  
}


local function generateRoomContent(rooms)
    local t = {}
    for _,v in ipairs(rooms) do
        t[#t+1] = {event_baseid = pick_func[v](),_COMMENT = v}
    end
    return t
end


-- 通用副本生成,测试
-- return { map_baseid: 10, map_events: {{event_baseid = ?, coord = {x = ?, y = ?}}, ...} }
local function generateDungeonLayer(role,copy_baseid,copy_layers,randomseed)

    local random_func = Random.newRandom(randomseed)

    local room_count = generateRoomCount(copy_baseid,copy_layers,random_func)
    local picked_elements = generateRoomType(copy_baseid,copy_layers,room_count,random_func)

    local events = generateRoomContent(picked_elements)
    return {
        map_baseid = 5000,
        map_events = events
    }
end

-- API

common = function(entity_career, copy_baseid, copy_layers, randomseed)    
    return generateDungeonLayer(entity_career,copy_baseid,copy_layers,randomseed)
end

--[[
    需要用到随机数的敌方
    怪物
    地图
    事件
    商人
    卡牌
    宝箱
    遗物
    道具
    怪物血量
    AI
    洗牌
    抽卡
    其他
]]
