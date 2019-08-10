--- 战斗辅助功能

--++++++++++++++++++++++++++++++++++++++ ]]

-- module(..., package.seeall)
local M = {}


require "modules.extend" -- TODO 整理require

-- local EntityExt = require "modules.entityext" -- TODO 整理require
-- local BuffExt = require "modules.buffext" -- TODO 整理require
-- local CardExt = require "modules.cardext" -- TODO 整理require


-- setfenv(1,M)

-- -- 特殊BUFF,战斗结算相关BUFF
-- local Stats = {
--     STR     = 3001, -- 强壮(增加卡牌造成的伤害)
--     DEX     = 3002, -- 灵巧(从卡牌获得的护甲增加)
--     INT     = 3003, -- 领悟(第三职业主属性)
--     ARM     = 3004, -- 护甲
--     WEAK    = 3005, -- 弱小(卡牌造成的伤害减少)
--     BROKEN  = 3006, -- 破损(从卡牌获得的护甲减少)
--     ED      = 3007, -- 易伤(受到的伤害增加)
--     THORN   = 3011, -- 荆棘

--     KEEPAMR = 3010, -- 保留护甲
--     UNREAL  = 3012, -- 无实体

--     NODRAW  = 3019 -- 无法再抽牌
-- }

local Stats = require("modules.stats")

local _isDebug = true

local old_print = print
local print = function (log)
    if _isDebug then
        cc.WriteLog([[<color="red"><b>]] .. log .. [[</color></b>]] )
        -- old_print("<color=maroon><b>" .. log .. "</b></color>")
    end
end


-------------------------- Util ----------------------------------
--
--          主要是一些战斗中的数据查询操作
--
-------------------------- Util ----------------------------------

--- 从对象数组转成ID数组
M.getEntitiesId = function(entities)
    local r = {}
    for i,v in ipairs(entities) do
        r[i] = v.id or false
    end
    return r
end

--- 从ID数组获取对象数组
M.getEntities = function (entities_id)
    local r = {}
    for i,v in ipairs(entities_id) do
        r[i] = g_copy.scene.match.entities[v] or false
    end
    return r
end


-- 获取战斗某方的角色列表
local function _getSideMembers(side)
    local r = {}
    for _, v in pairs(g_copy.scene.match.entities) do
        if v.side == side then -- Side.ENEMY
            r[#r + 1] = v or false
        end
    end
    return r
end

--- 筛选功能
-- @param func 一个返回 bool 的函数,接受一个entity作为参数
-- @return 返回entity 实例数组
local function filterEntities(func)
    local r = {}
    for _, e in pairs(g_copy.scene.match.entities) do
        if func(e) then
            r[#r + 1] = e or false
        end
    end
    return r
end

--- 获取战斗的所有敌人
-- @return entity实例数组{entity,entity...}
M.getAllEnemy = function()
    return filterEntities(
        function (e)return e.side == Side.ENEMY end
    )
end

--- 获取战斗的所有友方单位
-- @return entity实例数组{entity,entity...}
M.getAllAllies = function()
    return filterEntities(
        function (e) return e.side == Side.ALLIES end
    )
end

--- 获取随机敌人
-- @param random_func 随机函数
-- @return entity实例数组
M.getRandomEnemy = function(random_func)
    local enemies = M.getAllEnemy()
    return Random.pickRandom(enemies,random_func or math.random)
end


--- 计算卡牌的攻击力
-- (攻击+力量)*(1-弱小系数)
-- @param player entity
-- @param card_attack  卡牌面板攻击值
-- @return 计算后的真实攻击力
M.calcCardAttack = function(player, card_attack)
    if player == nil or card_attack < 0 then return end
    -- buff 影响
    local str_buff = EntityExt.getBuffLayers(player, Stats.STR) -- 力量
    local weak_value = EntityExt.getEDValue(player) -- 弱小
    -- 属性影响
    local str_attr = player.pack.strength

    -- 攻击力计算公式
    local attack = math.max(0,
        math.floor((card_attack + str_buff + str_attr) * math.clamp01(1 - weak_value)))

    print("计算卡牌攻击力=(攻击+力量)*(1-弱小系数):"..string.pyformat(
            "卡牌攻击计算公式(${base_attack}+${str})*(1-${weak_value})=${attack}",
            {base_attack = card_attack, str = str_buff, weak_value = weak_value, attack = attack}))

    return attack
end

------------------ CardSettle -------------------
--
--    出卡逻辑
--
-------------------------------------------------
do


    -- 1.结算一张卡的伤害
    -- @param player        出牌人
    -- @param target_ids    结算目标ID表
    -- @param card_base     牌面静态数据
    local cardSettleDamage = function(player, target_ids, card_base)

        local card_attack = card_base.damage_value
        local damage_type = card_base.damage_type
        local count       = card_base.settle_times

        local attack = M.calcCardAttack(player, card_attack)

        for _ = 1, count do
            for _, id in pairs(target_ids) do
                local target = g_copy.scene.match.entities[id]
                local dmg = EntityExt.takeDamage(target, attack, damage_type)

                do --TODO 【回调：玩家造成伤害】
                    -- 攻守兼备(造成伤害时获得护甲)BUFF
                    if dmg and dmg > 0 then
                        local gsjb = EntityExt.getBuffLayers(player,Stats.GongShouJianBei)
                        if gsjb > 0 then
                            EntityExt.addARMBuff(player,math.floor(gsjb*dmg))
                        end
                    end

                    -- 荆棘BUFF
                    local thron_buff, thorn_value = EntityExt.getBuff(target, Stats.THORN)
                    if thorn_value > 0 then
                        EntityExt.loseHp(player, thorn_value)
                        BuffExt.reduce(thron_buff)
                    end
                end
            end
        end
    end

    -- 2.结算卡牌对目标的效果
    local cardSettleTargetEffect = function(player, target_ids, card)
        local card_base = card.base
        for _, tid in pairs(target_ids) do
            local target = g_copy.scene.match.entities[tid]
            -- assert(target ~= nil)
            if target == nil then
                return
            end -- perhaps target is die

            target:hp_add(card_base.effect_hp) -- 治疗目标
            target:mp_add(card_base.effect_mp) -- 恢复能量
            target:strength_add(card_base.effect_strength) -- 增加力量 TODO:属性有效期
            target:armor_add(card_base.effect_armor) -- 增加护甲 TODO:属性有效期
            target:shield_add(card_base.effect_shield) -- 增加护盾 TODO:属性有效期

            if card_base.effect_buff ~= nil then -- 增加BUFF
                for buff_baseid, buff_layers in pairs(card_base.effect_buff) do
                    if(buff_baseid == Stats.ARM) then
                        EntityExt.addARMBuffByCard(target, buff_layers) -- 特殊BUFF
                    else
                        target:buff_add(buff_baseid, buff_layers)
                        -- entityScaleArmorBuff(target,0.25)
                    end
                end -- end for
            end
        end -- end for
    end

    -- 3.对自己结算 -- resume_mp效果已经执行过了,不要在这里执行第二次
    local cardSettleSelfEffect = function(player, card)

        if player.death then return end -- perhaps entity is death

        local card_base = card.base
        if EntityExt.canDraw(who) then
            -- 抽牌
            if card_base.draw_stackdeal > 0 then
                player:card_draw_from_stackdeal(card_base.draw_stackdeal)
            end
            if card_base.draw_stackdiscard > 0 then
                player:card_draw_from_stackdiscard(card_base.draw_stackdiscard)
            end
            if card_base.draw_stackexhaust > 0 then
                player:card_draw_from_stackexhaust(card_base.draw_stackexhaust)
            end
        end
        -- 洗牌
        if card_base.shuffle_stackdiscard then
            player:shuffle_stackdiscard()
        end
        if card_base.shuffle_stackexhaust then
            player:shuffle_stackexhaust()
        end
    end

    --- 使用卡牌,改过后的CardSettle
    -- @param player  使用卡的人
    -- @param card   使用的卡
    -- @param pick_entity 选择的对象
    function M.useCard(player, card, pick_entity)
        -- 获取目标ID集合
        local target_ids = CardGetTargets(player, card, pick_entity ~= nil and pick_entity.id or nil)

        cc.EntityCardPlay(player.id, card.id, target_ids)

        local card_base = card.base
        local double_buff = 3015

        -- TODO【回调：玩家出卡】
        -- 判断双发BUFF
        local useTime = 1
        local hasDoubleUse = player:buff_find(double_buff)
        local needDouble = card_base.category == CardCategory.AGGRESSIVE and hasDoubleUse
        if needDouble then
            useTime = useTime + (hasDoubleUse.layers or 0)
        end

        for _ = 1, useTime do
            -- 卡牌结算伤害
            if card_base.damage_type ~= CardDamageType.NONE
                and card_base.damage_value > 0
                and card_base.settle_times > 0 then
                cardSettleDamage(player, target_ids, card_base)
            end

            do -- 【出血】buff攻击牌使目标流血
                if card_base.category == CardCategory.AGGRESSIVE then
                    local chuxue = EntityExt.getBuffLayers(player,Stats.CHUXUE)
                    if chuxue > 0 then
                        for _, tid in pairs(target_ids) do
                            local target = g_copy.scene.match.entities[tid]
                            target:buff_add(3027,chuxue)   -- magic id
                        end
                    end
                end
            end

            -- 处理目标效果(添加BUFF等效果)
            cardSettleTargetEffect(player, target_ids, card)

            -- 处理自身效果(抽牌等项目)
            cardSettleSelfEffect(player, card)
        end

        if needDouble then
            player:buff_remove(double_buff)
        end
    end
end -- end do

return M