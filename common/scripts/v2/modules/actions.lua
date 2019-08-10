--- Actions Module
-- 游戏中的行为
-- 2019-8-6
-- 整合BuffExt 和 EntityExt 中的需要处理BreakPoint的部分


local M = {}


local _DEBUG = true

local old_print = print
local print = function (...)
    if _DEBUG then
        old_print(...)
    end
end

local Debug = {
    Log = function (log)
        if _DEBUG then print(log) end
    end,
    Warn = function (log)
        if _DEBUG then print([[<color="yellow"><b>]] .. log .. [[</color></b>]] ) end
    end,
    Error = function (log)
        if _DEBUG then print([[<color="red"><b>]] .. log .. [[</color></b>]] ) end
    end
}


do-- 血量/伤害相关Action

    --- 对Entity制造伤害
    -- @usage EntityExtend.takeDamage(who,attack,damage_type)
    -- @param who           承受伤害的人
    -- @param attack        伤害的总攻击力
    -- @param damage_type   伤害类型(默认物理伤害)
    M.damageOne = function (who,attack,damage_type)
        -- check
        if who == nil then return end
        if attack == nil then return end
        if attack <= 0 then return end

        if who.death then return end

        -- local damage_effect_type = damage_effect_type or -1

        local damage_type = damage_type or CardDamageType.PHYSICAL -- 默认物理伤害

        -- print("角色受伤过程: 伤害值" .. attack)

        -- TODO
        --[[
            把这些对伤害有影响的东西封装起来,在这里loop处理,而不是每加一个东西改一次代码
            这个系统暂时叫  伤害处理中间件,负责回调伤害处理
        --]]

        local ed_factor = 1 + EntityExt.getEDValue(who)

        -- print("\t易伤系数:"..ed_factor)
        attack = math.floor(attack * ed_factor) -- 攻击
        -- print("\t需要承受伤害:"..attack)
        -- 无实体只受1点伤害
        local unreal_buff, unreal_count = EntityExt.getBuff(who, Stats.UNREAL)
        if unreal_buff and unreal_count >= 1 and attack > 1 then
            attack = 1
            BuffExt.reduce(unreal_buff)
            -- print("\t目标无实体,伤害变成" .. attack )
        end

        if damage_type == CardDamageType.PHYSICAL then
            local arm_buff, arm_amount = EntityExt.getBuff(who, Stats.ARM)

            -- print("\t物理伤害")
            local damage = math.max(attack - arm_amount, 0)

            if damage == 0 then -- 护甲够伤害扣除
                if arm_buff then
                    -- print("\t免受伤害,扣除护甲"..-attack)
                    arm_buff:layers_add(-attack)
                    if arm_buff.layers == 0 then
                        print("清空护甲")
                        who:buff_remove(arm_buff.id) -- 清空护甲
                        -- print("\t护甲破损")
                    end
                else
                    print("\t目标不可能在没有护甲的情况下受到0点伤害,因为攻击总是大于0")
                end
                return 0
            else -- 护甲不够伤害扣除
                if arm_buff then
                    -- print("\t护甲破损-" .. arm_amount)
                    who:buff_remove(arm_buff.id) -- 清空护甲
                end
                -- print("\t扣除血量:" .. damage)
                -- TODO【回调：受到伤害】
                M.loseHp(who, damage)
                return damage
            end
        else
            -- print("\t魔法伤害")
            -- print("\t扣除血量:" .. attack)
            -- TODO【回调：受到伤害】
            M.loseHp(who, attack)
            return attack
        end
    end

    ---对自己造成伤害
    M.damageSelf = function (who,attack,damage_type)
        M.damageOne(who,attack,damage_type)
    end

    --- 对所有敌方造成伤害
    M.damageAllEnemies = function (attack,damage_type)
        -- local enemies = MatchUtil.getAllEnemy()
        -- -- local attack  = MatchUtil.calcCardAttack(player,card.base.damage_value)
        -- for _, id in pairs(enemies) do
        --     local target = g_copy.scene.match.entities[id]
        --     local dmg = EntityExt.takeDamage(target, attack, damage_type)

        --     do --TODO 【回调：玩家造成伤害】
        --         -- 攻守兼备(造成伤害时获得护甲)BUFF
        --         if dmg > 0 then
        --             local gsjb = EntityExt.getBuffLayers(player,Stats.GongShouJianBei)
        --             if gsjb > 0 then
        --                 EntityExt.addARMBuff(player,math.floor(gsjb*dmg))
        --             end
        --         end

        --         -- 荆棘BUFF
        --         local thron_buff, thorn_value = EntityExt.getBuff(target, Stats.THORN)
        --         if thorn_value > 0 then
        --             EntityExt.loseHp(player, thorn_value)
        --             BuffExt.reduce(thron_buff)
        --         end
        --     end
        -- end
    end

    --- 对随机敌人造成伤害
    M.damageRandomEnemy = function (attack,damage_type)
        local target = MatchUtil.getRandomEnemy()
        if target then
            M.damageOne(target,attack,damage_type)
        end
    end


    --- 单纯掉血
    -- 正值表示扣血,非正值不处理
    -- @param who who
    -- @param amount 扣血数量
    -- @param damage_effect_type 伤害类型
    M.loseHp = function (who, amount)
        if who == nil  then return end
        if amount <= 0 then return end
        who:hp_add(-amount)
        cc.ScriptMatchEventDamage(who.id, amount)
    end

    --- 不算伤害的 hp_add,不通知客户端
    -- @param who who
    -- @param amount 扣血数量
    M.loseHpSilent = function(who, amount)
        if who == nil and amount <= 0 then return end;
        who:hp_add(-amount)
    end

    --- 接受治疗
    -- @param who who
    -- @param amount 治疗的值，小于0无效，请用takedamage
    M.takeHeal = function (who, amount)
        if who == nil or amount <= 0 then return end

        -- 致死打击
        local zhisi = EntityExt.getBuffLayerValue(Stats.ZhiSi)

        amount = math.floor(amount*(1-zhisi))
        who:hp_add(amount)
        -- TODO: 调用客户端治疗接口
        -- TODO【回调：接受治疗】
    end
end


do -- 牌相关Action

    --- 抽卡
    -- 从抽卡堆抽卡
    -- @param who who
    -- @param amount 抽卡数量
    M.drawCard = function(who, amount)
        if amount <= 0 then
            print("抽卡数不能<=0")
            return
        end
        if EntityExt.canDraw(who) then
            who:card_draw_from_stackdeal(amount)
        end
    end

    --- 增加一张卡到背包(Deck)
    -- @param who 玩家
    -- @param card_baseid 卡牌ID
    M.addCard2Deck = function(who,card_baseid)
        who:add_card(card_baseid)
    end

    --- 增加一张手牌
    -- 只能一张一张的增加卡牌
    -- 这里的牌不在玩家的Deck中,是额外新增的
    -- @param who who
    -- @param card_baseids {cardbaseid,}
    -- @usage handAddNewCard(player,10010)
    M.handAddNewCard = function (who, card_baseid)
        who:stack_hold_newone(card_baseid)
    end

    --- 随机丢弃一张牌
    -- @param who entity
    -- @param amount 弃牌的数量
    M.randomDiscardCard = function (who,amount)
    end

    -- 随机消耗一张牌
    -- @param who entity
    -- @param amount 烧牌的数量
    M.randomExhaustCard = function (who,amount)
    end

end-- end do


do -- buff相关ACTION

    -- 给目标增加BUFF（广义上的BUFF）
    M.addBuff = function(who,buff_baseid,amount)
        if who == nil then return end
        assert(amount>0,"addBUFFAction传入了负的BUFF层数")
        -- TODO【回调：获得BUFF】
        who:buff_add(buff_baseid,amount)
    end

    --- 从卡牌获得护甲
    -- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
    -- @param who    who
    -- @param amount 护甲层数
    M.addARMBuffByCard = function(who, amount)
        if who == nil then return end
        if amount <= 0 then
            print("加了负数的护甲")
            return
        end

        -- 【防】BUFF
        local def = EntityExt.getBuffLayers(who, Stats.DEF) + who.pack.armor or 0

        -- 【防护专精】
        local fhzj = EntityExt.getBuffLayers(who,Stats.FangHuZhuanJing)
        local fhzj_value = EntityExt.getBuffLayerValue(who,Stats.FangHuZhuanJing)

        amount = math.max(0, amount + def*(1+fhzj*fhzj_value))
        amount = math.floor(amount)

        M.addBuff(who,Stats.ARM,amount)
        -- who:buff_add(Stats.ARM, amount)
    end

    --- 非卡牌增加护甲Buff
    -- 护甲计算公式:护甲
    -- @param who    who
    -- @param amount 护甲层数
    M.addARMBuff = function(who, amount)
        if who == nil then return end
        if amount <= 0 then return end

        -- TODO【回调：从卡牌获得护甲】
        -- 获得护甲时对所有敌方造成伤害

        local dunci = EntityExt.getBuffLayers(who,Stats.dunci)

        M.addBuff(who,Stats.ARM,amount)
    end
end -- end do


--- 获得金钱
-- @param who 玩家
-- @param amount 数量
M.addGold = function (who,amount)
    -- todo check can get money
    -- TODO【回调：获得金币】
    who:add_gold(amount)
end


--- 增加魔法值
M.addMp = function(who, amount)
    if who == nil then return end
    if amount <= 0 then return end
    who:mp_add(amount)
end

--- 增加生命值上限
M.addMaxHp = function(who, amount)
    if who == nil then return end
    if amount <= 0 then return end
    who:maxhp_add(amount)
end

return M
