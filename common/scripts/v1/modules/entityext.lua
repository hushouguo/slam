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
    return Actions.damageOne(who,attack,damage_type)
end

--- 单纯掉血
-- 正值表示扣血,非正值不处理
-- @param who who
-- @param amount 扣血数量
-- @param damage_effect_type 伤害类型
M.loseHp = function (who, amount)
    Actions.loseHp(who,amount)
end

--- 不算伤害的 hp_add,不通知客户端
-- @param who who
-- @param amount 扣血数量
M.loseHpSilent = function(who, amount)
    Actions.loseHpSilent(who,amount)
end

--- 接受治疗
-- @param who who
-- @param amount 治疗的值，小于0无效，请用takedamage
M.takeHeal = function (who, amount)
    Actions.takeHeal(who,amount)
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
    actions.drawCard(who,amount)
end

--- Deck增加卡牌
-- @param who 玩家
-- @param card_baseid 卡牌ID
M.addCard2Deck = function(who,card_baseid)
    Actions.addCard2Deck(who,card_baseid)
end

--- 获得金钱
-- @param who 玩家
-- @param amount 数量
M.addGold = function (who,amount)
    Actions.addGold(who,amount)
end


--- 新增手牌
-- 这里的牌不在玩家的Deck中,是额外新增的
-- @param who who
-- @param card_baseids {cardbaseid,}
-- @usage handAddNewCard(player,{10025,10026,10025,10025})
M.handAddNewCard = function (who, card_baseids)
    Actions.handAddNewCard(who,card_baseids)
end


--- 增加魔法值
M.addMp = function(who, amount)
    Actions.addMp(who,amount)
end

--- 增加生命值上限
M.addMaxHp = function(who, amount)
    actions.addMaxHp(who,amount)
end


------------------------FUNC


--- 查询是否保留护甲
-- @param who 查询对象
-- @return 返回是否保留护甲
M.isKeepARM = function(who)
    return M.getBuffLayers(who, Stats.KEEPAMR) > 0
end


--- 查询是否可以抽牌
M.canDraw = function(who)
    return M.getBuffLayers(who, Stats.NODRAW) <= 0
end



-- 给目标增加BUFF
M.addBuff = function(who,buff_baseid,amount)
    Actions.addBuff(WHO,buff_baseid,amount)
end


--- 卡牌增加护甲Buff
-- 护甲计算公式:护甲 = (卡牌护甲+敏捷)*(脆弱系数)
-- @param who    who
-- @param amount 护甲层数
M.addARMBuffByCard = function(who, amount)
    Actions.addARMBuffByCard(who,amount)
end

--- 非卡牌增加护甲Buff
-- 护甲计算公式:护甲
-- @param who    who
-- @param amount 护甲层数
M.addARMBuff = function(who, amount)
    Actions.addARMBuff(who,amount)
end

--- 按比例缩放护甲
-- @param who who
-- @param scale 比例
M.scaleARMBuff = function(who, scale)

    if who == nil then return end
    if scale == nil then return end
    if scale <= 0 then return end

    local arm_buff, arm_amount = M.getBuff(who, Stats.ARM) --  who:buff_find(Stats.ARM)

    if arm_buff == nil then return end

    local new_amount = math.floor(arm_amount * scale)

    if new_amount > 0 then
        who:buff_add(Stats.ARM, new_amount - arm_amount)
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
    return M.getBuffLayerValue(who, Stats.ED)
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





----------------------------事件


-- M.onMakeDamage = function(who,damage)

-- end





return M