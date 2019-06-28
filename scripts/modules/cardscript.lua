--- 卡牌 脚本模块
-- 在这里实现特殊需求的卡牌

--++++++++++++++++++++++++++++++++++++++

module(...,package.seeall)


--[[

文档

卡牌有如下BreakPoint

    BreakPoint.CARD_PLAY_A      --出卡前 end
    BreakPoint.CARD_PLAY_Z      --出卡后 end
    BreakPoint.CARD_DISCARD_A   --弃卡前 end
    BreakPoint.CARD_DISCARD_Z   --弃卡后 end


--]]



--- 卡牌通用脚本,基础的卡牌执行逻辑
-- @param entity 当前打出卡牌或拥有BUFF的实体对象, entity肯定不是nil
-- @param card   当前打出的卡牌对象, 如果不是打牌的BreakPoint, card 是 nil
-- @param buff   当前处理的buff对象，如果不是处理buff的BreakPoint, buff 是 nil
-- @param pick_entity   打牌时玩家自主选择的目标对象， pick_entity也可能是nil
-- @param breakpoint 是BreakPoint的枚举
common = function (entity, card, buff, pick_entity, breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then
        MatchUtil.useCard(entity, card, pick_entity)
    end
end


-- 多重打击
multiStrike = function (entity, card, buff, pick_entity, breakpoint)

    local DrawEnemy = function(enemies)
        return enemies[table.random(enemies,#enemies)]
    end

    if breakpoint == BreakPoint.CARD_PLAY_Z then

        local enemies = MatchUtil.getAllEnemy()
        local attack = MatchUtil.calcCardAttack(entity,card.base.damage_value)
        -- local damage = (card.base.damage_value + entity.strength) * entity.weakness -- (攻击+力量)*虚弱

        for i = 1,3 do -- 抽三次
            local target = DrawEnemy(enemies)
            if target then
                MatchUtil.entityTakeDamage(target,attack)
            end
        end
    end
end

-- 巩固
entrench = function (entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_A then
        local arm_buff =  MatchUtil.queryEntityBuff(entity,3004)
        if arm_buff then
            arm_buff:layers_add(arm_buff.layers)
            -- MatchUtil.addARM(entity,ac)
        end
    end
end


-- 全身撞击
bodyslam = function(entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_A then
        if pick_entity == nil then return end
        local arm_amount = MatchUtil.queryEntityBuffLayers(entity,3004)

        local attack = MatchUtil.calcCardAttack(entity,arm_amount)
        if attack > 0 then
            MatchUtil.entityTakeDamage(pick_entity,attack,CardDamageType.PHYSICAL)
        end
    end
end

-- 诅咒(TODO:有疑问,卡牌没有回合结束的回调,所以暂时无法处理)
curse = function(entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.ROUND_END_A then
        -- local n = entity.stack_hold_size
        -- entity:hp_add(-n)
    end
end

-- -- 冲锋
-- rush = function(entity,card,buff,pick_entity,breakpoint)
--     local tempcard = 10150      -- 普通攻击*
--     if breakpoint == BreakPoint.CARD_PLAY_Z then
--         MatchUtil.useCard(entity, card, pick_entity)
--         entity:stack_hold_newone(tempcard)
--     end
-- end

-- -- 嗜血打击
-- bloodStrike = function(entity,card,buff,pick_entity,breakpoint)
--     local tempcard = 10220   -- 流血
--     if breakpoint == BreakPoint.CARD_PLAY_A then
--         MatchUtil.useCard(entity, card, pick_entity)
--         entity:stack_hold_newone(tempcard)
--     end
-- end

