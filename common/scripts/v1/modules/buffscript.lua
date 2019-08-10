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
        BuffExt.reduce(buff)
    end
end

--- 回合开始时减一层
function M.roundBeginReduce (entity, card, buff, pick_entity, bp)

    if _check(buff) then return end

    if bp == ROUND_BEGIN_A then
        BuffExt.reduce(buff)
    end
end


-- 护盾
function M.armor(entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
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
        EntityExt.addARMBuff(entity,buff.layers or 0)
    end

    if bp == ENTITY_SUFFER_DAMAGE then
        BuffExt.reduce(buff)
    end
end

-- 金属化
-- 回合结束获得护甲
function M.metallicize (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_END_Z then
        EntityExt.addARMBuff(entity,buff.layers or 0)
    end
end

-- 再生
function M.rejuvenation (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_END_A then
        EntityExt.takeHeal(entity,buff.layers or 0)
    end
end

-- 中毒Debuff
function M.poison (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_A then
        EntityExt.loseHp(entity,buff.layers)
        BuffExt.reduce(buff)
    end
end


--- 狂暴
-- 风怒 每使用2张【攻击类型】获得一张临时的【普通攻击*】
-- 初始2层，每使用一张攻击牌，层数减一，然后层数为0的时候触发攻击牌，然后buff层数增加到layer_value
function M.kuangbao (entity, card, buff, pick_entity, bp)

    if _check(buff,entity) then return end

    local init_layers = buff.base.params[1] or 2

    -- 出牌后 加牌
    if bp == CARD_PLAY_Z then
        if card == nil then return end
        if card.categroy == CardCategory.AGGRESSIVE then
            buff:layers_add(-1)
            if buff.layers == 0 then
                -- MatchUtil.entityHandAddNewCard(entity,{10150})
                EntityExt.handAddNewCard(entity,10010)
                buff:layers_add(init_layers)
            end
        end
    end
end


--- 【回合开始后】手牌获得{params[1]}牌
function M.handAddNewCardOnRoundBegin (entity, card, buff, pick_entity, bp)
    if _check(buff,entity) then return end

    local card_baseid = buff.base.params[1]
    if bp == ROUND_BEGIN_Z then
        EntityExt.handAddNewCard(entity,card_baseid)
    end
end


-- 残暴 回合开始后损失[层]点生命值,抽[层]张牌
M.brutality = function(entity,card,buff,pick_entity,bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        EntityExt.loseHp(entity,buff.layers)
        EntityExt.drawCard(entity,buff.layers)
    end
end

-- 【回合开始后】额外获得{layer}点能量
M.gainMpOnRoundBegin = function(entity,card,buff,pick_entity,bp)

    if _check(buff,entity) then return end

    if bp == ROUND_BEGIN_Z then
        EntityExt.addMp(entity,buff.layers)
    end
end

-- 【#32兴奋剂】
M.xingfenji = function (entity,card,buff,pick_entity,bp)
    if _check(buff,entity) then return end

    if bp == ROUND_END_A then
        entity:buff_add(Stats.STR,buff.layers)  -- 损失三层攻
        entity:buff_remove(buff.id)             -- 移除xianji
    end
end


-- 【回合开始后】对【随机目标】造成{[1]}【物理伤害】
-- 参数组定义：{damage}
M.damageRandomEnemyOnRoundBegin = function(entity,card,buff,pick_entity,bp)
    if _check(buff,entity) then return end

    local damage = buff.base.params[1]

    if bp == ROUND_BEGIN_Z then
        Actions.damageRandomEnemy(damage)
    end
end


-- 【回合开始后】获得{[2]}层{[1]}BUFF
-- 参数组定义:{buff_baseid,amount}
M.gainBuffOnRoundBegin = function(entity,card,buff,pick_entity,bp)
    if _check(buff,entity) then return end

    local baseid = buff.base.params[1]
    local amount = buff.layers * buff.base.params[2]

    if bp == ROUND_BEGIN_Z then
        Actions.addBuff(baseid,amount)
    end
end

-- 【回合结束后】获得{[2]}层{[1]}BUFF
-- 参数组定义:{buff_baseid,amount}
M.gainBuffOnRoundEnd = function(entity,card,buff,pick_entity,bp)
    if _check(buff,entity) then return end

    local baseid = buff.base.params[1]
    local amount = buff.layers * buff.base.params[2]

    if bp == ROUND_END_A then
        Actions.addBuff(baseid,amount)
    end
end


return  M
