-- Entity  扩展方法

-- 操作Entity实例的一些功能封装

--- functional api

local M = {}


--- 对Entity制造伤害
-- @usage EntityExtend.takeDamage(who,attack,damage_type)
-- @param who           承受伤害的人
-- @param attack        伤害的总攻击力
-- @param damage_type   伤害类型
M.takeDamage = function(who, attack, damage_type)
    -- check
    if who == nil then return end
    if attack == nil then return end
    if attack <= 0 then return end
    -- local damage_effect_type = damage_effect_type or -1

    local damage_type = damage_type or CardDamageType.PHYSICAL -- 默认物理伤害

    -- print("角色受伤过程: 伤害值" .. attack)

    local ed_factor = 1 + M.getEDValue(who)

    -- print("\t易伤系数:"..ed_factor)
    attack = math.floor(attack * ed_factor) -- 攻击
    -- print("\t需要承受伤害:"..attack)
    -- 无实体只受1点伤害
    local unreal_buff, unreal_count = M.getBuff(who, SPLBUFF.UNREAL)
    if unreal_buff and unreal_count >= 1 and attack > 1 then
        attack = 1
        BuffExt.reduce(unreal_buff)
        -- print("\t目标无实体,伤害变成" .. attack )
    end

    if damage_type == CardDamageType.PHYSICAL then
        local arm_buff, arm_amount = M.getBuff(who, SPLBUFF.ARM)

        -- print("\t物理伤害")
        local damage = math.max(attack - arm_amount, 0)

        if damage == 0 then -- 护甲够伤害扣除
            if arm_buff then
                -- print("\t免受伤害,扣除护甲"..-attack)
                arm_buff:layers_add(-attack)
                if arm_buff.layers == 0 then
                    who:buff_remove(arm_buff.id) -- 清空护甲
                    -- print("\t护甲破损")
                end
            else
                print("\t目标不可能在没有护甲的情况下受到0点伤害,因为攻击总是大于0")
            end
        else -- 护甲不够伤害扣除
            if arm_buff then
                -- print("\t护甲破损-" .. arm_amount)
                who:buff_remove(arm_buff.id) -- 清空护甲
            end
            -- print("\t扣除血量:" .. damage)
            M.loseHp(who, damage)
        end
    else
        -- print("\t魔法伤害")
        -- print("\t扣除血量:" .. attack)
        M.loseHp(who, attack)
    end
end


--- 单纯掉血
-- 正值表示扣血,非正值不处理
-- @param who who
-- @param amount 扣血数量
-- @param damage_effect_type 伤害类型
M.loseHp = function (who, amount)
    if who == nil and amount <= 0 then return end
    who:hp_add(-amount)
    cc.Damage(who.id, amount)
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
    who:hp_add(amount)
    -- TODO: 调用客户端治疗接口
end

--- 恢复所有生命值
M.restoreHp = function(who)
    M.takeHeal(who,1e10)
end

--- 抽卡
-- 从抽卡堆抽卡
-- @param who who
-- @param amount 抽卡数量
M.drawCard = function(who, amount)

    if amount <= 0 then return end

    if M.canDraw(who) then
        who:card_draw_from_stackdeal(amount)
    end
end

--- Deck增加卡牌
-- @param who 玩家
-- @param card_baseid 卡牌ID
M.addCard2Deck = function(who,card_baseid)
    who:add_card(card_baseid)
end

--- 获得金钱
-- @param who 玩家
-- @param amount 数量
M.addGold = function (who,amount)
    -- todo check can get money
    who:add_gold(amount)
end

--- 查询是否保留护甲
-- @param who 查询对象
-- @return 返回是否保留护甲
M.isKeepARM = function(who)
    return M.getBuffLayers(who, SPLBUFF.KEEPAMR) > 0
end

--- 查询是否可以抽牌
M.canDraw = function(who)
    return M.getBuffLayers(who, SPLBUFF.NODRAW) <= 0
end

--- 新增手牌
-- 这里的牌不在玩家的Deck中,是额外新增的
-- @param who who
-- @param card_baseids {cardbaseid,}
-- @usage handAddNewCard(player,{10025,10026,10025,10025})
M.handAddNewCard = function (who, card_baseids)
    for _, v in ipairs(card_baseids) do
        who:stack_hold_newone(v)
    end
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

-------------------------------------------BUFF 相关------------------------------------------------------------------------------

--- 卡牌增加护甲Buff
-- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
-- @param who    who
-- @param amount 护甲层数
M.addARMBuffByCard = function(who, amount)
    if who == nil then return end
    if amount <= 0 then
        print("不要在这里减护甲")
        return
    end
    -- 灵巧
    local dex = M.getBuffLayers(who, SPLBUFF.DEX)
    amount = math.max(0, amount + dex)
    who:buff_add(SPLBUFF.ARM, amount)
end

--- 非卡牌增加护甲Buff
-- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
-- @param who    who
-- @param amount 护甲层数
M.addARMBuff = function(who, amount)
    if who == nil then return end
    if amount <= 0 then return end
    who:buff_add(SPLBUFF.ARM, amount)
end

--- 按比例缩放护甲
-- @param who who
-- @param scale 比例
M.scaleARMBuff = function(who, scale)

    if who == nil then return end
    if scale == nil then return end
    if scale <= 0 then return end

    local arm_buff, arm_amount = M.getBuff(who, SPLBUFF.ARM)--  who:buff_find(SPLBUFF.ARM)

    if arm_buff == nil then return end

    local new_amount = math.floor(arm_amount * scale)

    if new_amount > 0 then
        who:buff_add(SPLBUFF.ARM, new_amount - arm_amount)
    else
        who:buff_remove(arm_buff.id)
    end
end

--- 查找Entity身上的buff
-- 没有buff返回0
-- @param who who
-- @param buff_baseid buff的baseid
-- @return (buff,layers)返回buff和buff的层数,如果没有这个buff,返回(nil,0)
M.getBuff = function(who, buff_baseid)
    if who == nil or buff_baseid <= 0 then return nil, 0 end

    local buff = who:buff_find(buff_baseid)
    if buff then
        return buff, buff.layers
    else
        return nil, 0
    end
end

--- 查询Entity身上的BUFF层数
-- @param who who
-- @param buff_baseid buff的baseid
-- @return buff的层数,没有buff返回0
M.getBuffLayers = function (who, buff_baseid)
    local _, r = M.getBuff(who, buff_baseid)
    return r
end

--- 查询易伤百分比
-- @param who
-- @return 返回ED 的layer_value
M.getEDValue = function (who)
    return M.getBuffLayerValue(who, SPLBUFF.ED)
end

-- 查询角色身上的BUFF
-- 返回buff,buff_layers
M.getBuffLayerValue = function(who, buff_baseid)
    -- assert(who~=nil and buff_baseid~=nil)
    if who == nil or buff_baseid <= 0 then return 0 end
    local buff, layers = M.getBuff(who, buff_baseid)
    if buff and layers > 0 then
        return buff.base.layer_value
    else
        return 0
    end
end

return M