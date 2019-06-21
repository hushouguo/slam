--- 卡牌 脚本模块
-- 在这里实现特殊需求的卡牌

--++++++++++++++++++++++++++++++++++++++

module(...,package.seeall)

local BattleUtil = BattleUtil or require ('modules.battleutil')


--- 卡牌通用脚本,基础的卡牌执行逻辑
-- @param entity 当前打出卡牌或拥有BUFF的实体对象, entity肯定不是nil
-- @param card   当前打出的卡牌对象, 如果不是打牌的BreakPoint, card 是 nil
-- @param buff   当前处理的buff对象，如果不是处理buff的BreakPoint, buff 是 nil
-- @param pick_entity   打牌时玩家自主选择的目标对象， pick_entity也可能是nil
-- @param breakpoint 是BreakPoint的枚举
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

        local damage = (card.base.damage_value + entity.strength) * entity.weakness -- (攻击+力量)*虚弱

        for i = 1,3 do -- 抽三次
            local target = DrawEnemy(enemies)
            if target then                  
                BattleUtil.takeDamage(target,damage)    
            end
        end
    end
end

-- 巩固
entrench = function (entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then   
        entity:armor_modify(entity.armor)
    end
end


-- 全身撞击
bodyslam = function(entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.CARD_PLAY_Z then  
        local damage = (entity.armor + entity.strength)*entity.weakness
        BattleUtil.takeDamage(entity,damage)
    end
end

-- 诅咒
curse = function(entity,card,buff,pick_entity,breakpoint)
    if breakpoint == BreakPoint.ROUND_END_A then  
        local n = #(entity.stack_hold)
        entity:hp_modify(-n)
    end
end
