--- 战斗辅助功能

--++++++++++++++++++++++++++++++++++++++]]

module(..., package.seeall)

require "modules.extend" -- TODO 整理require
-- 特殊BUFF,战斗结算相关BUFF
local SPLBUFF = {
    STR     = 3001,     -- 强壮(增加卡牌造成的伤害)
    DEX     = 3002,     -- 灵巧(从卡牌获得的护甲增加)
    INT     = 3003,     -- 领悟(第三职业主属性)
    ARM     = 3004,     -- 护甲
    WEAK    = 3005,     -- 弱小(卡牌造成的伤害减少)
    BROKEN  = 3006,     -- 破损(从卡牌获得的护甲减少)
    ED      = 3007,     -- 易伤(受到的伤害增加)  
    THORN   = 3011,     -- 荆棘
    
    KEEPAMR = 3010,     -- 保留护甲
    UNREAL  = 3012,     -- 无实体
}

local old_print = print
local print = function (log)
    old_print("<color=maroon><b>".. log .. "</b></color>")
end


------------------------------------------------------------
--
--      常用卡牌效果
--      
------------------------------------------------------------
do

    --- 对单位制造伤害
    -- @param who           承受伤害的人
    -- @param attack        伤害的总攻击力
    -- @param damage_type   伤害类型
    entityTakeDamage = function(who, attack, damage_type)       
        -- check
        if who == nil then return end
        if attack <= 0 then return end
        local damage_type = damage_type or CardDamageType.PHYSICAL

        -- print("□伤害结算过程:")
        -- print("卡牌攻击力:"..attack)
        -- local ed_buff = queryEntityBuff(who,SPLBUFF.ED)
        local ed_factor = 1 + queryEntityEDValue(who)
        -- print("易伤系数:"..ed_factor)
        attack = attack * ed_factor -- 攻击
        -- print("易伤后攻击:"..attack)
        -- 无实体只受1点伤害
        local unreal_buff = queryEntityBuff(who,SPLBUFF.UNREAL)
        if unreal_buff and attack > 1 then
            attack = 1
            -- print("目标无实体,伤害变成" .. attack )
        end

        if damage_type == CardDamageType.PHYSICAL then
            local arm_buff,arm_amount  = queryEntityBuff(who,SPLBUFF.ARM)  

            -- print("物理伤害")
            local damage = math.max(attack - arm_amount,0)

            if damage == 0 then     -- 护甲够伤害扣除
                if arm_buff then
                    -- print("免受伤害,扣除护甲"..-attack)
                    arm_buff:layers_add(-attack)
                    if arm_buff.layers == 0 then
                        who:buff_remove(arm_buff.id) -- 清空护甲
                        -- print("护甲破损")
                    end 
                else
                    print("目标不可能在没有护甲的情况下受到0点伤害,因为攻击总是大于0")
                end
            else    -- 护甲不够伤害扣除
                if arm_buff then
                    -- print("护甲破损")
                    who:buff_remove(arm_buff.id) -- 清空护甲
                end
                -- print("扣除血量:" .. damage)
                entityLoseHp(who,damage)
            end  
        else
            -- print("魔法伤害")
            -- print("扣除血量:" .. attack)
            entityLoseHp(who,attack)     
        end
    end

    --- 单纯掉血
    -- 正值表示扣血,非正值不处理
    -- @param who entity
    -- @param amount 扣血数量    
    entityLoseHp = function (who,amount)
        if who == nil and amount <= 0 then return end;
        who:hp_add(-amount)
        cc.Damage(who.id, amount)  
    end

    --- 不算伤害的Damage
    -- @param who entity
    -- @param amount 扣血数量
    entityLoseHpSilent = function(who,amount)
        if who == nil and amount <= 0 then return end;
        who:hp_add(-amount)
    end

    --- 接受治疗
    -- @param who entity
    -- @param amount 治疗的值，小于0无效，请用takedamage
    entityTakeHeal = function (who,amount)
        if who==nil or amount <=0 then return end
        who:hp_add(amount)
        -- TODO: 调用客户端接口
    end

    --- 抽卡
    -- 从抽卡堆抽卡
    -- @param who entity
    -- @param amount 抽卡数量
    entityDrawCard = function(who,amount)
        if amount <= 0 then return end
        who:card_draw_from_stackdeal(amount)
    end

    --- 新增手牌
    -- 这里的牌不在玩家的Deck中,是额外新增的
    -- @param who entity
    -- @param card_baseids {cardbaseid,}
    -- @usage entityHandNewCard(player,{10025,10026,10025,10025})
    entityHandAddNewCard = function (who,cardbaseid_group)
        for _,v in ipairs(cardbaseid_group) do
            who:stack_hold_newone(v)
        end
    end

    --- 卡牌增加护甲Buff
    -- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
    -- @param who    entity
    -- @param amount 护甲层数
    entityGainArmorBuffByCard = function(who, amount)
        if who == nil then return end
        if amount <= 0 then 
            print("不要在这里减护甲")
            return 
        end
        -- 灵巧
        local dex = queryEntityBuffLayers(who,SPLBUFF.DEX)        
        amount = math.max(0,amount + dex)
        who:buff_add(SPLBUFF.ARM,amount)
    end

    --- 非卡牌增加护甲Buff
    -- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
    -- @param who    entity
    -- @param amount 护甲层数
    entityGainArmorBuffNotByCard = function(who,amount)
        if who == nil then return end
        if amount <= 0 then return end
        who:buff_add(SPLBUFF.ARM,amount)
    end     

    -- --- 去掉Entity身上所有护甲
    -- -- @param who           entity
    -- -- @param buff_baseid   baseid
    -- brokenAMR = function (who)
    --     assert(who~=nil)
    --     local arm = SPLBUFF.ARM
    --     local buff = who:buff_find(arm)
    --     if buff then
    --         who:buff_add(arm,-buff.layers)
    --         who:buff_remove(buff.id)
    --     end
    -- end


    -- --- 添加Entity身上的BUFF层数
    -- -- @param  who entity
    -- -- @param  buff_baseid buff的baseid
    -- -- @param  layers buff的层数
    -- addEntityBuffLayers = function (who,buff_baseid,layers)
    --     assert(who~=nil and buff_baseid>0)
    --     who:buff_add(buff_baseid,layers)
    --     if queryEntityBuffLayers(who,buff_baseid)==0 then
    --         who:buff_remove(buff_baseid)
    --     end 
    -- end

    --- 卡牌增加护甲Buff
    -- TODO :旧的API
    -- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
    -- @param who    entity
    -- @param amount 护甲层数
    addArmorBuffByCard = function(who, amount)
        entityGainArmorBuffByCard(who,amount)        
    end

    -- --- BUFF层数减少1
    -- buffSelfReduce1Layers = function(buff)
    --     buff:layers_add(-1)

    -- end
end


----------------修改卡-----------------
--      void Card:set_field( name, value)
--      void Card:reset_field( name)
--------------------------------------
do
    -- 修改卡牌【能耗】
    modifyCardMp = function (card,value)
        if card == nil then return end

        if value == -1 then
            card:reset_field("cost_mp")
        else
            card:set_field("cost_mp",value)
        end
    end

    -- 使卡牌变成【消耗】
    modifyCardDiscardTarget = function(card,value)
        if card == nil then return end
        if value == -1 then
            card:reset_field("into_stackdiscard")
            card:reset_field("into_stackexhaust")
        else
            card:set_field("into_stackdiscard",false)
            card:set_field("into_stackexhaust",true)
        end
    end

    -- -- 修改卡牌攻击
    -- -- TODO:
    -- modifyCardBaseAttack = function(card,value)
    --     if card == nil then return end       

    --     if value  == -1 then

    --     else

    --     end

    -- end
end



-------------------------- Util ----------------------------------
--
--          主要是一些数据查询操作
-- 
-------------------------- Util ----------------------------------


local function _getSideMembers(side)
    local r = {}
    for _, v in pairs(g_copy.scene.match.entities) do 
        if v.side == side then -- Side.ENEMY
            r[#r + 1] = v or false
        end
    end
    return r
end

--- 获取战斗的所有敌人    
-- @return entity实例数组{entity,entity...}
getAllEnemy = function() return _getSideMembers(Side.ENEMY) end

--- 获取战斗的所有友方单位
-- @return entity实例数组{entity,entity...}
getAllAllies = function() return _getSideMembers(Side.ALLIES) end

--- 查找Entity身上的buff
-- @param who entity
-- @param buff_baseid buff的baseid
-- @return (buff,layers)返回buff和buff的层数,如果没有这个buff,返回(nil,0)
queryEntityBuff = function(who,buff_baseid)
    if who == nil or buff_baseid<=0 then return nil,0 end

    local buff = who:buff_find(buff_baseid)
    if buff then 
        return buff,buff.layers
    else
        return nil,0
    end
end




--- 查询Entity身上的BUFF层数
-- @param who entity
-- @param buff_baseid buff的baseid
-- @return buff的层数,没有buff返回0
queryEntityBuffLayers = function (who,buff_baseid)
    local _,r  = queryEntityBuff(who,buff_baseid)
    return r
end

-- --- 增加护甲
-- -- @param who entity
-- -- @amount 护甲数量
-- addARM = function(who,amount)
--     assert(who~=nil and amount > 0)
--     who:buff_add(SPLBUFF.ARM,amount)
-- end

-- --- 丢失护甲
-- -- @param who entity
-- -- @amount 护甲数量
-- loseARM = function(who,amount)
--     assert(who~=nil and amount > 0)
--     who:buff_add(SPLBUFF.ARM,-amount)
-- end

-- 查询角色身上的BUFF
-- 返回buff,buff_layers
local _queryEntityBuffLayerValue = function(who,buff_baseid)
    -- assert(who~=nil and buff_baseid~=nil)
    if who==nil or buff_baseid <=0 then return 0 end
    local buff,layers = queryEntityBuff(who,buff_baseid)
    if buff and layers > 0 then
        return buff.base.layer_value
    else
        return 0
    end
end

--- 查询Entity的弱小DeBUFF数值
-- @param who
-- @return 返回弱小BUFF的layer_value
queryEntityWeakValue = function (who)    
    -- assert(who~=nil)
    return _queryEntityBuffLayerValue(who,SPLBUFF.WEAK)
end

--- 查询易伤百分比
-- @param who
-- @return 返回ED 的layer_value
queryEntityEDValue = function (who)    
    return _queryEntityBuffLayerValue(who,SPLBUFF.ED)
end

-- queryEntityFlag = {
--     ["KEEPARM"] = function(who) return queryEntityBuffLayers(who,SPLBUFF.KEEPAMR) > 0 end,
--     ["UNREAL"]  = function(who) return queryEntityBuffLayers(who,SPLBUFF.UNREAL) > 0 end,
-- }


-- --- 查询玩家身上的标记BUFF
-- queryEntityFlagBuff = function (who,flag)
--     if flag = "KEEPARM" then         
-- end


--- 查询是否保留护甲
-- @param who 查询对象
-- @return 返回是否保留护甲
queryFlagEntityKeepARM = function(who)
    return queryEntityBuffLayers(who,SPLBUFF.KEEPAMR) > 0 
end

-- queryFlagEntityUnreal = function(who)

-- end

--- 计算卡牌的攻击力
-- (攻击+力量)*(1-弱小系数)
-- @param player entity
-- @param card_attack  卡牌面板攻击值
-- @return 计算后的真实攻击力
calcCardAttack = function(player,card_attack)
    if player == nil or card_attack < 0 then return end
    -- assert(player~=nil )
    -- assert(card_attack >= 0)

    local str_amount = queryEntityBuffLayers(player,SPLBUFF.STR) -- 力量
    local weak_value = queryEntityWeakValue(player)              -- 弱小
        -- 攻击力计算公式
    local attack = math.max(0,
        math.floor(card_attack + str_amount) * math.clamp01(1 - weak_value)
        ) 
    print("卡牌攻击力=(攻击+力量)*(1-弱小系数):"..string.pyformat(
            "卡牌攻击计算公式(${base_attack}+${str})*(1-${weak_value})=${attack}",
            {base_attack = card_attack,str = str_amount,weak_value = weak_value,attack = attack}
            )
        )
    return attack
end




------------------ CardSettle -------------------
--
--                   出卡逻辑
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

        local attack = calcCardAttack(player,card_attack)

        for i = 1, count do
            for _, id in pairs(target_ids) do
                local target = g_copy.scene.match.entities[id]
                entityTakeDamage(target, attack, damage_type)
                -- 荆棘BUFF
                local thorn_value = _queryEntityBuffLayerValue(target,SPLBUFF.THORN)
                if  thorn_value > 0 then
                    entityLoseHp(player,thorn_value)
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
            target:armor_add(card_base.effect_armor)    -- 增加护甲 TODO:属性有效期
            target:shield_add(card_base.effect_shield)  -- 增加护盾 TODO:属性有效期

            if card_base.effect_buff ~= nil then -- 增加BUFF
                for buff_baseid, buff_layers in pairs(card_base.effect_buff) do
                    if(buff_baseid==SPLBUFF.ARM) then
                        addArmorBuffByCard(target,buff_layers)          -- 特殊BUFF
                    else
                        target:buff_add(buff_baseid, buff_layers)
                    end
                end -- end for
            end
        end -- end for
    end

    -- 3.对自己结算 -- resume_mp效果已经执行过了,不要在这里执行第二次
    local cardSettleSelfEffect = function(player, card)

        if player.death then return end -- perhaps entity is death

        local card_base = card.base
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
    function useCard(player, card, pick_entity)
        -- 获取目标ID集合
        local target_ids = CardGetTargets(player, card, pick_entity ~= nil and pick_entity.id or nil)
        local card_base = card.base

        local double_buff = 3015

        -- 判断双发BUFF
        local useTime = 1 
        local hasDoubleUse = player:buff_find(double_buff)
        local needDouble  = card_base.category == CardCategory.AGGRESSIVE and hasDoubleUse
        if needDouble then
            useTime  = useTime + (hasDoubleUse.layers or 0)
        end

        for i = 1,useTime do     
            -- 卡牌结算伤害
            if card_base.damage_type ~= CardDamageType.NONE 
                and card_base.damage_value > 0 
                and card_base.settle_times > 0 then
                cardSettleDamage(player, target_ids, card_base)            
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



