--- Buff 表 脚本

--++++++++++++++++++++++++++++++++++++++
-- module(...,package.seeall)
local M  ={}



-- local CardExt   = require "modules.cardext"


--  BUFF.BUFFBreakPoint:
local ROUND_BEGIN_A  = BreakPoint.ROUND_BEGIN_A    --回合开始A
local ROUND_BEGIN_Z  = BreakPoint.ROUND_BEGIN_Z    --回合开始Z
local CARD_DEAL_A    = BreakPoint.CARD_DEAL_A      --抽卡前
local CARD_DEAL_Z    = BreakPoint.CARD_DEAL_Z      --抽卡后
local CARD_PLAY_A    = BreakPoint.CARD_PLAY_A      --出卡前
local CARD_PLAY_Z    = BreakPoint.CARD_PLAY_Z      --出卡后
local ROUND_END_A    = BreakPoint.ROUND_END_A      --回合结束前
local ROUND_END_Z    = BreakPoint.ROUND_END_Z      --回合结束后
local CARD_DISCARD_A = BreakPoint.CARD_DISCARD_A   --弃卡前
local CARD_DISCARD_Z = BreakPoint.CARD_DISCARD_Z   --弃卡后


-- setfenv(1,M)

--- 无
function M.none (...) end
--- 常驻Flag
function M.commonFlagBuff (...) end

--- 回合结束时减一层
function M.roundEndReduce (entity, card, buff, pick_entity, bp)

    if _check(buff) then return end

    if bp == ROUND_END_Z then
        -- MatchUtil.buffSelfReduce(buff)
        BuffExt.reduce(buff)
    end
end

--- 回合开始时减一层
function M.roundBeginReduce (entity, card, buff, pick_entity, bp)

    if _check(buff) then return end

    if bp == ROUND_BEGIN_A then
        -- MatchUtil.buffSelfReduce(buff)
        BuffExt.reduce(buff)
    end
end

-- 护甲
function M.armor(entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        -- print("=============回合开始销毁护甲")
        -- local is_keep = MatchUtil.queryFlagEntityKeepARM(entity)
        local is_keep = EntityExt.isKeepARM(entity)
        if not is_keep then
            entity:buff_remove(buff.id)
        end
    end
end

-- 多层护甲
-- 回合开始获得护甲，受到伤害层数-1
function M.multArmor(entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_A then
        -- MatchUtil.entityGainArmorBuffNotByCard(entity,buff.layers or 0)
        EntityExt.addARMBuff(entity,buff.layers or 0)
    end

    if bp == ENTITY_SUFFER_DAMAGE then
        -- MathcUtil.buffSelfReduce(buff)
        BuffExt.reduce(buff)
    end
end

-- 金属化
-- 回合结束获得护甲
function M.metallicize (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_END_Z then
        -- MatchUtil.entityGainArmorBuffNotByCard(entity,buff.layers or 0)
        EntityExt.addARMBuff(entity,buff.layers or 0)
    end
end

-- 再生
function M.rejuvenation (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_END_A then
        -- MatchUtil.entityTakeHeal(entity,buff.layers)
        -- MatchUtil.buffSelfReduce(buff)
        EntityExt.takeHeal(entity,buff.layers or 0)
    end
end

-- 中毒Debuff
function M.poison (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_A then
        -- MatchUtil.entityLoseHp(entity,buff.layers)
        EntityExt.loseHp(entity,buff.layers)
        -- MatchUtil.buffSelfReduce(buff)
        BuffExt.reduce(buff)
    end
end

-- 风怒 每使用3张攻击牌获得一张临时的【攻击牌】
-- 思路，原先使用临时计数器，现在使用BUFF层数
-- 初始3层，每使用一张攻击牌，层数减一，然后层数为0的时候触发攻击牌，然后buff层数增加到layer_value
function M.windfury (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    -- 出牌后 加牌
    if bp == CARD_PLAY_Z then
        if card == nil then return end

        if card.categroy == CardCategory.AGGRESSIVE then
            buff:layers_add(-1)
            if buff.layers == 0 then
                -- MatchUtil.entityHandAddNewCard(entity,{10150})
                EntityExt.handAddNewCard(entity,{10150})
                buff:layers_add(buff.base.layer_value)
            end
        end
    end
end


-- 每回合开始获得一张GM牌
function M.gm (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        -- MatchUtil.entityHandAddNewCard(entity,{10000})
        EntityExt.handAddNewCard(entity,{10000})
    end
        -- entity:stack_hold_newone(gm_cardbaseid)
end

-- -- 双发
-- doubleUse = function(entity, card, buff, pick_entity, breakpoint)
--     assert(buff ~= nil)
--     -- 出牌后
--     if breakpoint = CARD_PLAY_Z then
--         if card.base.category == CardCategory.AGGRESSIVE then

--         end
--     end
-- end


-- 残暴 回合开始后损失[层]点生命值,抽[层]张牌
brutality = function(entity,card,buff,pick_entity,bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        EntityExt.loseHp(entity,buff.layers)
        -- MatchUtil.entityLoseHp(entity,buff.layers)
        -- MatchUtil.entityDrawCard(entity,buff.layers)
        EntityExt.drawCard(entity,buff.layers)
    end
end

-- 额外能量,回合开始后额外获得layers能量
moreMpEachTurn = function(entity,card,buff,pick_entity,bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        -- MatchUtil.entityGainMp(entity,buff.layers)
        EntityExt.addMp(entity,buff.layers)
    end
end


return  M
