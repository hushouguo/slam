--- Buff 表 脚本

--++++++++++++++++++++++++++++++++++++++
module(...,package.seeall)

-- Buff特性 在某个breakpoint层数-1
local selfReduce = function(buff,breakpoint,reduce_point)
    if breakpoint == reduce_point then
        buff:layers_modify(-1)
    end
end


-- 常规BUFF
common = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)
    if breakpoint == BreakPoint.ROUND_BEGIN_A then
        BuffSettle(entity, buff)
    end
end


-- 易伤
vulnerability = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)
    -- 回合开始时-1层
    selfReduce(buff,breakpoint,BreakPoint.ROUND_BEGIN_A)
end

-- 护甲
armor = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)
    if breakpoint == BreakPoint.ROUND_BEGIN_A then
        if not entity:buff_find(20005) then                 -- 20005 保留护甲
            entity:armor_modify(-entity.armor)              -- 回合开始失效
        end
    end
end

-- 中毒
poison = function(entity, card, buff, pick_entity, breakpoint)
        assert(buff ~= nil)
        -- 回合结束-1层
        selfReduce(buff,breakpoint,BreakPoint.ROUND_END)
        -- 回合开始时扣血
        if breakpoint == BreakPoint.ROUND_BEGIN_A then
            entity:hp_modify(-buff.layers)
        end
    end

-- 风怒
windfury = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)
    export = export or {}
    export.ctx = export.ctx or {}        
    local ctx = export.ctx 

    -- 回合开始 时设置计数器
    if breakpoint == BreakPoint.ROUND_BEGIN_A then
        ctx.buff20006_counter =  0
    end
    -- 出牌后 加牌
    if breakpoint == BreakPoint.CARD_PLAY_Z then
        ctx.buff20006_counter = (ctx.buff20006_counter or 0) + 1
        print("==============================" .. ctx.buff20006_counter)
        if ctx.buff20006_counter >= 3 then
            print("==============================获得一张普通攻击")
            entity:stack_hold_newone(10150)
            ctx.buff20006_counter =0
        end
    end
end

-- 余震
aftershock = function(entity, card, buff, pick_entity, breakpoint)
    assert(buff ~= nil)
    export.ctx = export.ctx or {}        
    local ctx = export.ctx

    -- 回合开始时,设置计数器
    if breakpoint == BreakPoint.ROUND_BEGIN_A then
        ctx.buff20010_counter =  0
    end
    -- 回合结束前,对所有目标造成伤害
    if breakpoint == BreakPoint.ROUND_END_A then
        local entities = GetAllEnemy();
        for _,e in ipairs(entities) do
            TakeDamage(e,ctx.buff20010_counter)
        end
    end
    -- 出牌后 增加计数器
    if breakpoint == BreakPoint.CARD_PLAY_Z then
        if card or card.base.category == CardCategory.AGGRESSIVE then
            ctx.buff20010_counter = ctx.buff20010_counter + 1
        end
    end
end