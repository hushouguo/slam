-- Card  扩展方法

-- 操作实例的一些功能封装

--- functional api
-- module(...,package.seeall)
local M = {}

------------------------------------------------
--       修改卡牌的静态字段内容
--      Modify Card
--      void Card:set_field(name, value)
--      void Card:reset_field(name)
------------------------------------------------

-- 修改卡牌【能耗】
-- value 传入nil 表示恢复初值
M.setCost = function (card, value)
    if card == nil then return end

    if value then
        card:reset_field("cost_mp")
    else
        card:set_field("cost_mp", value)
    end
end

-- 使卡牌变成【消耗】
M.setExhaust = function(card, is_exhaust)
    if card == nil then return end
    if is_exhaust == nil then
        card:reset_field("into_stackdiscard")
        card:reset_field("into_stackexhaust")
        return
    end

    if is_exhaust == false then
        card:set_field("into_stackdiscard", true)
        card:set_field("into_stackexhaust", false)
    else
        card:set_field("into_stackdiscard", false)
        card:set_field("into_stackexhaust", true)
    end
end

return M
