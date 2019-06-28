--- Buff 表 脚本

--++++++++++++++++++++++++++++++++++++++
module(...,package.seeall)


--[[

文档

BUFFBreakPoint:
    BreakPoint.ROUND_BEGIN_A    --回合开始A
    BreakPoint.ROUND_BEGIN_Z    --回合开始Z
    BreakPoint.CARD_DEAL_A      --抽卡前
    BreakPoint.CARD_DEAL_Z      --抽卡后
    BreakPoint.CARD_PLAY_A      --出卡前
    BreakPoint.CARD_PLAY_Z      --出卡后
    BreakPoint.ROUND_END_A      --回合结束前
    BreakPoint.ROUND_END_Z      --回合结束后
    BreakPoint.CARD_DISCARD_A   --弃卡前
    BreakPoint.CARD_DISCARD_Z   --弃卡后
--]]

--- 无
common = function(entity, card, buff, pick_entity, bp) end


-- BUFF层数自减
-- @param buff buff实例
local _buffSelfReduce = function(buff)
    -- check
    if buff == nil then return end
    if buff.entity == nil then 
        print("buffSelfReduce 错误,buffentity为空"..table.serialize(buff))
        return
    end
    buff:layers_add(-1)
    if buff.layers == 0 then        
        buff.entity:buff_remove(buff.id)        
    end
end

--- 回合开始时减一层
-- 比如易伤,
commonTurnBase = function(entity, card, buff, pick_entity, bp)
    assert(buff ~= nil)
    if bp == BreakPoint.ROUND_BEGIN_A then
        _buffSelfReduce(buff)
    end
end

--- 常驻Flag
commonFlagBuff = function(entity, card, buff, pick_entity, bp) end

-- 护甲
armor = function(entity, card, buff, pick_entity, bp)
    assert(buff ~= nil)
    if bp == BreakPoint.ROUND_BEGIN_Z then
        print("=============回合开始销毁护甲")
        local is_keep = MatchUtil.queryFlagEntityKeepARM(who)
        if not is_keep then
            entity:buff_remove(buff.id)
        end
    end
end

-- 多层护甲
multArmor = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)

    if breakpoint == BreakPoint.ROUND_END_A then
        local amount = buff.layers or 0
        MatchUtil.entityGainArmorBuffNotByCard(entity,amount)
    end
end

-- 再生
rejuvenation = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff~=nil)
    if breakpoint == BreakPoint.ROUND_END_A then
        MatchUtil.entityTakeHeal(entity,buff.layers)
        _buffSelfReduce(buff)
    end
end

-- 中毒Debuff
poison = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)                                                                                                                                                                                     
    if breakpoint == BreakPoint.ROUND_BEGIN_A then        
        local layers = buff.layers
        if layers > 0 then
            MatchUtil.entityLoseHp(entity,buff.layers)
            _buffSelfReduce(buff)
        end
    end    
end

-- 风怒
-- ctx 使用范例 
-- ctx已经被干掉了
windfury = function(entity, card, buff, pick_entity, breakpoint)
    -- assert(buff ~= nil)


    if entity == nil then return end

    entity._temp = entity._temp or {use_attack_card_count = 0}
    
    -- export = export or {}
    -- export.ctx = export.ctx or {}
    -- local ctx = export.ctx

    local temp = 10150           -- 临时普通攻击

    -- 回合开始 时设置计数器
    if breakpoint == BreakPoint.ROUND_BEGIN_A then
        entity._temp.use_attack_card_count = 0
    end
    -- 出牌后 加牌
    if breakpoint == BreakPoint.CARD_PLAY_Z then        
        entity._temp.use_attack_card_count = (entity._temp.use_attack_card_count or 0) + 1
        if entity._temp.use_attack_card_count >= 3 then
            entity:stack_hold_newone(temp)
            entity._temp.use_attack_card_count = 0
            print("触发风怒,获得一张【普通攻击】!!")
        end
    end
end

-- 余震
aftershock = function(entity, card, buff, pick_entity, bp)
    assert(buff ~= nil)
    export.ctx = export.ctx or {}
    local ctx = export.ctx

    -- 回合开始时,设置计数器
    if bp == BreakPoint.ROUND_BEGIN_A then
        ctx.buff20010_counter =  0
    end
    -- 回合结束前,对所有目标造成伤害
    if bp == BreakPoint.ROUND_END_A then
        local entities = GetAllEnemy();
        for _,e in ipairs(entities) do
            MatchUtil.entityTakeDamage(e,ctx.buff20010_counter)
        end
    end
    -- 出牌后 增加计数器
    if bp == BreakPoint.CARD_PLAY_Z then
        if card or card.base.category == CardCategory.AGGRESSIVE then
            ctx.buff20010_counter = ctx.buff20010_counter + 1
        end
    end
end

-- 每回合开始获得一张GM牌
gm = function(entity, card, buff, pick_entity, bp)
    assert(buff~=nil)
    local gm_cardbaseid = 10000
    if bp == BreakPoint.ROUND_BEGIN_Z then
        MatchUtil.entityHandAddNewCard(entity,{10000})
        -- entity:stack_hold_newone(gm_cardbaseid) 
    end
end

-- -- 双发
-- doubleUse = function(entity, card, buff, pick_entity, breakpoint)
--     assert(buff ~= nil)
--     -- 出牌后
--     if breakpoint = BreakPoint.CARD_PLAY_Z then
--         if card.base.category == CardCategory.AGGRESSIVE then

--         end 
--     end
-- end


-- 残暴 回合开始损失一点生命值,抽一张牌
brutality = function(entity,card,buff,pick_entity,bp)
    assert(buff~=nil)
    if bp == BreakPoint.ROUND_BEGIN_Z then
        MatchUtil.entityLoseHp(entity,1)
        MatchUtil.entityDrawCard(entity,1)
    end
end


