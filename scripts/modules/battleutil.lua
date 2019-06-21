--- 卡牌脚本和状态等战斗相关脚本通用逻辑 
-- TODO: 和common脚本查重

--++++++++++++++++++++++++++++++++++++++]]


module(...,package.seeall)

require 'modules.extend'  -- TODO 整理require

--- 优先计算护甲的扣血方式
-- @param who entity  扣血对象       
-- @param how_much number  大于0的伤害数值
takeDamage = function(who,how_much)
    assert(how_much >= 0,"不接受负的伤害!")
    if who:buff_find(20000) then -- 易伤
       how_much = math.round(how_much * 1.5)  -- require 'extend'  -- 扣血采用舍去小数的方式
    end 
    local diff = who.shield - how_much
    if diff >= 0 then
        who:shield_modify(-how_much)
    else
        who:hp_modify(diff)
        cc.Damage(who.id, -diff)
    end                    
end


--- 获取所有敌人
-- @return table 敌人数组
getAllEnemy = function()
    local r = {}
    for _,v in pairs(g_copy.scene.match.entities) do
        if v.side == Side.ENEMY then
            r[#r+1] = v or false
        end 
    end
    return r
end


--- 应用一个伤害,把受伤逻辑封装到一个函数内
-- TODO: 函数式API
-- @param who      entity 掉血的人
-- @param attacker    攻击者()  
-- @param card        攻击的卡
-- @param damage      伤害数值
-- @param damage_type 伤害类型
local function applyDamage(attacker,target,damage,damage_type)

end
