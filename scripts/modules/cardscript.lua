--++++++++++++++++++++++++++++++++++++++
-- 卡牌 脚本
--++++++++++++++++++++++++++++++++++++++

module(...,package.seeall)

BattleUtil = BattleUtil or require ('modules.battleutil')


-- 通用结算
common = function (entity, card, buff, pick_entity, breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then 
        CardSettle(entity, card, pick_entity)    
    end 
end

-- 多重打击
multiStrike = function (entity, card, buff, pick_entity, breakpoint)

    local DrawEnemy = function(enemies)
        return enemies[table.random(enemies,#enemies)]
    end

    -- settle
    if breakpoint == BreakPoint.CARD_PLAY_Z then    

        local enemies = BattleUtil.getAllEnemy()

        local damage = (card.base.damage_value+entity.strength)*entity.weakness -- (攻击+力量)

        for i = 1,3 do -- 抽三次
            local target = DrawEnemy(enemies)
            if target then                  
                BattleUtil.takeDamage(target,damage)    
            end
        end
    end
end

-- 巩固
entrench = function (entity,card,buff,pick_entity,BreakPoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then   
        entity:armor_modify(entity.armor)
    end
end


-- 全身撞击
bodyslam = function(entity,card,buff,pick_entity,BreakPoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then  
        local damage = (entity.armor + entity.strength)*entity.weakness
        BattleUtil.takeDamage(target,damage)
    end
end

-- 诅咒
curse = function(entity,card,buff,pick_entity,BreakPoint)
    if breakpoint == BreakPoint.ROUND_END_A then  
        local n = #(entity.stack_hold)
        entity:hp_modify(-n)
    end
end