--- 卡牌 脚本模块
-- 在这里实现特殊需求的卡牌

--++++++++++++++++++++++++++++++++++++++

-- module(...,package.seeall)
local M = {}



-- 卡牌生效的BP
local CARD_PLAY_A    =  BreakPoint.CARD_PLAY_A      --出卡前 end
local CARD_PLAY_Z    =  BreakPoint.CARD_PLAY_Z      --出卡后 end
local CARD_DISCARD_A =  BreakPoint.CARD_DISCARD_A   --弃卡前 end
local CARD_DISCARD_Z =  BreakPoint.CARD_DISCARD_Z   --弃卡后 end
-- setfenv(1,M)

--- 卡牌通用脚本,基础的卡牌执行逻辑
-- @param player 当前打出卡牌或拥有BUFF的实体对象, entity肯定不是nil
-- @param card   当前打出的卡牌对象, 如果不是打牌的BreakPoint, card 是 nil
-- @param buff   当前处理的buff对象，如果不是处理buff的BreakPoint, buff 是 nil
-- @param pick_entity   打牌时玩家自主选择的目标对象， pick_entity也可能是nil
-- @param bp 是BreakPoint的枚举
M.common = function (player, card, buff, pick_entity, bp)
    if _check(player,card,bp) then return end
        -- cc.EntityCardPlay(player.id, card.id, target_ids) -- notify client to show
    if bp == CARD_PLAY_Z then
        MatchUtil.useCard(player, card, pick_entity)
    end
end


-- 多重打击
M.multiStrike = function (player, card, buff, pick_entity, bp)
    if _check(player,card,bp) then return end
    local DrawEnemy = function(enemies)
        return enemies[table.random(enemies,#enemies)]
    end

    if bp == CARD_PLAY_Z then
        local enemies = MatchUtil.getAllEnemy()
        local attack  = MatchUtil.calcCardAttack(player,card.base.damage_value)

        local enemies_id = MatchUtil.getEntitiesId(enemies)
        cc.EntityCardPlay(player.id, card.id, enemies_id)

        for _ = 1,3 do -- 抽三次
            local target = DrawEnemy(enemies)
            if target then
                EntityExt.takeDamage(target,attack)
            end
        end
    end
end

-- 巩固
M.entrench = function (player,card,buff,pick_entity,bp)

    if _check(player,card,bp) then return end

    if bp == CARD_PLAY_Z then
        cc.EntityCardPlay(player.id, card.id, {player.id})
        EntityExt.scaleARMBuff(player,2)
    end
end


-- 全身撞击
M.bodyslam = function(player,card,buff,pick_entity,bp)

    if _check(player,card,bp) then return end

    if bp == CARD_PLAY_Z then
        if pick_entity == nil then return end
        cc.EntityCardPlay(player.id, card.id, pick_entity.id)
        local arm_amount = MatchUtil.queryEntityArmor(player)
        -- print("===========================================全身撞击")
        local attack = MatchUtil.calcCardAttack(player,arm_amount)
        if attack > 0 then
            EntityExt.takeDamage(pick_entity,attack,CardDamageType.PHYSICAL)
        end
    end
end

-- 诅咒(TODO:有疑问,卡牌没有回合结束的回调,所以暂时无法处理)
M.curse = function(player,card,buff,pick_entity,bp)
    -- if bp == BreakPoint.ROUND_END_A then
    --     -- local n = entity.stack_hold_size
    --     -- entity:hp_add(-n)
    -- end
end

-- 流血,弃牌造成伤害
M.blood = function (player,card,buff,pick_entity,bp)

    if _check(player,card,bp) then return end

    local card_base = card.base
    -- cc.WriteLog("bbbbgl" .. card.id)
    -- print(table.serialize(card))
    -- assert(card_base~=nil)
    if bp == CARD_DISCARD_A then
        EntityExt.takeDamage(player,card_base.damage_value,card_base.damage_type) -- 扣血按表里填的弄
    end
end


--

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

return  M