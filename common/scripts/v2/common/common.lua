-- require("modules.main")

cc = _G

--
------------------- global instance -------------------
--

g_enable_debug = true
g_enable_god = true

g_runonclient = true    -- single
g_runonserver = false   -- single or multiple

g_copy = nil

--
------------------- setting -------------------
--

FLAG_monster_round_interval = 0 -- 1 second

--
------------------- enum declare -------------------
--

--
-- EntityGender
-- 
EntityGender = {
    NONE = 0, MALE = 1, FEMALE = 2
}

--
-- EntityCareer
--
EntityCareer = {
    
}

--
-- EntityCategory
--
EntityCategory = {
    NONE = 0, PLAYER = 1, MONSTER = 2
}

--
-- CardQuality
--
CardQuality = {
}

--
-- CardCategory
--
CardCategory = {
    NONE                =   0,  -- 无种类
    AGGRESSIVE          =   1,  -- 攻击类
    SPELL               =   2,  -- 技能类
    ABILITY             =   3,  -- 能力类
    DUMMY               =   4,  -- 状态类
    CURSE               =   5,  -- 诅咒类
    EQUIPMENT           =   6,  -- 装备类
    PUPPET              =   7,  -- 随从类  
}

--
-- CardTargetType
--
CardTargetType = {
    NONE                =   0,  -- 无需目标
    PICK_SINGLE_ALLIES  =   1,  -- 对自选单个本方目标
    PICK_SINGLE_ENEMY   =   2,  -- 对自选单个敌方目标
    SELF                =   3,  -- 对自己
    SELF_PUPPET         =   4,  -- 对自己的随从
    ALLIES_ALL          =   5,  -- 对所有盟友
    ALLIES_PUPPET_ALL   =   6,  -- 对所有盟友随从
    ENEMY_ALL           =   7,  -- 对所有敌人
    ENEMY_PUPPET_ALL    =   8,  -- 对所有敌人随从
    ENEMY_RANDOM        =   9,  -- 对随机一个敌人
    ALLIES_RANDOM       =   10, -- 对随机一个盟友
    ALL                 =   11, -- 对全体
}

--
-- CardDamageType
--
CardDamageType = {
    NONE = 0, PHYSICAL = 1, SPELL = 2
}

--
-- ObstacleCategory
--
ObstacleCategory = {
	NONE				=	0,	--无类型
	WALL				=	1,	--墙
	HIGHWALL			=	2,	--高墙,城墙
	WATER				=	3,	--水
	TREE				=	4,	--树
	GRASS				=	5,	--草
	STONE				=	6,	--石头	
	TILE				=	7,	--地砖
	BUILDING			=	8,	--建筑物
	MOUNTAIN			=	9,	--山丘
	DECORATION			=	10,	--装饰
	CORNER              =   11, --转角
	GATE                =   12, --门
}

--
-- BlockCategory
--
BlockCategory = {
	NONE				=	0,	--无阻挡
	DYNAMIC				=	1,	--动态阻挡, 破损后阻挡消失
	STATIC				=	2,	--静态阻挡
}

--
-- EventCategory
--
EventCategory = {
	NONE				=	0,	--无事件
	ENTRY 				=	1,	--入口
	EXIT 				=	2,	--出口
	MONSTER				=	3,	--怪物
	SHOP_BUY_CARD		=	4,	--购卡商店
	SHOP_DESTROY_CARD	=	5,	--销卡商店
	STORY 				=	6,	--剧情事件
	SHOP_LEVELUP_CARD   =   7,  --升级卡商店
	STORY_OPTION		=	8,	--选项事件
	REWARD				=	9,	--奖励事件
    SHOP_LEVELUP_PUPPET	=	10,	--升级宠物	
}


--
-- GridObjectCategory
--
GridObjectCategory = {
    NONE                =   0,  -- 无分类
    EVENT               =   1,  -- 事件
    OBSTACLE            =   2,  -- 障碍
}

--
-- Side
--
Side = {
    ALLIES = 0, ENEMY = 1
}


--
-- BreakPoint
--
BreakPoint = {
    ROUND_BEGIN_A       =   1,  -- 回合开始前
    ROUND_BEGIN_Z       =   2,  -- 回合开始后
    CARD_DEAL_A         =   3,  -- 发牌前
    CARD_DEAL_Z         =   4,  -- 发牌后
    CARD_PLAY_A         =   5,  -- 出牌前
    CARD_PLAY_Z         =   6,  -- 出牌后
    CARD_DISCARD_A      =   7,  -- 弃牌前
    CARD_DISCARD_Z      =   8,  -- 弃牌后
    ROUND_END_A         =   9,  -- 回合结束前
    ROUND_END_Z         =   10, -- 回合结束后
    BUFF_ADD_A          =   11, -- 添加BUFF开始
    BUFF_ADD_Z          =   12, -- 添加BUFF结束
    BUFF_REMOVE_A       =   13, -- 移除BUFF开始
    BUFF_REMOVE_Z       =   14, -- 移除BUFF结束
    ENTITY_SUFFER_DAMAGE=   15, -- 受到伤害
}


--
-- EventEnd
--
EventEnd = {
	NONE				=	0,	-- 永远不结束
	TRIGGER				=	1,	-- 触发时结束
	REWARD				=	2,	-- 操作时结束, 操作指：商店购买、销卡、升级卡牌、进入战斗等
}


--
-- MapPolicy
--
MapPolicy = {
	NONE				=	0,	-- 无策略
	CHESSBOARD			=	1,	-- 棋盘地表, 棋盘地表要求map.obstacles里至少包含两块以上地砖
	WAVE_WALL           =   2,  -- 波浪城墙, 波浪城墙要求map.obstacles里至少包含墙和高墙两种墙
}


--
-- placeholder
--
Placeholder = {
    NONE                =   0,  -- 无排位
    FRONT               =   1,  -- 前排
    MIDDLE              =   2,  -- 中间
    BACK                =   3,  -- 后排
}

--
-- ItemQuality
--
ItemQuality = {
	NONE				=	0,	-- 无品阶
	WHITE				=	1,	-- 白
	GREEN				=	2,	-- 绿
	BLUE				=	3,	-- 蓝
	PURPLE				=	4,	-- 紫
	ORANGE				=	5,	-- 橙
}

--
-- ItemCategory
--
ItemCategory = {
	NONE				=	0,	-- 无种类
	USABLE				=	1,	-- 可使用
	EQUIPABLE			=	2,	-- 可装备
	MATERIAL			=	3,	-- 材料类
}

--
-- Span
--
Span = {
    NONE                =   0,  -- 无持续范围
    GLOBAL              =   9,  -- 全局
    COPY                =   5,  -- 副本内 
    MATCH               =   1,  -- 战斗内
}

--
-- BuffType
--
BuffType = {
    NONE                =   0,  -- 无类型
    TALENT              =   1,  -- 祝福
    EQUIP               =   2,  -- 装备
    CARD                =   3,  -- 卡牌
    ITEM                =   4,  -- 道具
}

--
-- MapPattern
--
MapPattern = {
    NONE                =   0,  -- 无样式
    I                   =   1,  -- 直路
    L                   =   2,  -- 左转
    R                   =   3,  -- 右转
    LR                  =   4,  -- 左转和右转
    LI                  =   5,  -- 带左转的直路   
    RI                  =   6,  -- 带右转的直路
    LRI                 =   7,  -- 三岔路
}

--
------------------- Card Settle -------------------
--


--
-- return targets: {entityid, ...}
--
function CardGetTargets(entity, card, pick_entityid)
    local switch_target_type = {
        [CardTargetType.NONE] = function(entity, card, pick_entityid)
            return pick_entityid ~= nil and {[pick_entityid] = pick_entityid} or {}
        end,
        
        [CardTargetType.PICK_SINGLE_ALLIES] = function(entity, card, pick_entityid)
            if pick_entityid == nil then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, pick_entityid is nil", entity.id, card.id, card.baseid))
                return {}
            end
            
            local target = entity.copy.scene.match.entities[pick_entityid]
            if target == nil then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, pick_entityid is not exist", entity.id, card.id, card.baseid))
                return {}
            end

            if target.side ~= entity.side then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, side is equal", entity.id, card.id, card.baseid))
                return {}
            end
            
            return {[pick_entityid] = pick_entityid}
        end,
        
        [CardTargetType.PICK_SINGLE_ENEMY] = function(entity, card, pick_entityid)
            if pick_entityid == nil then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, pick_entityid is nil", entity.id, card.id, card.baseid))
                return {}
            end
            
            local target = entity.copy.scene.match.entities[pick_entityid]
            if target == nil then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, pick_entityid is not exist", entity.id, card.id, card.baseid))
                return {}
            end

            if target.side == entity.side then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, side not equal", entity.id, card.id, card.baseid))
                return {}
            end
            
            return {[pick_entityid] = pick_entityid}
        end,
        
        [CardTargetType.SELF] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil and pick_entityid ~= entity.id then
--				cc.ScriptErrorLog(string.format(">>>>>>> entity: %d, card: %d,%d, pick_entityid not self", entity.id, card.id, card.baseid))
                return {}
            end
            return {[entity.id] = entity.id}
        end,
        
        [CardTargetType.SELF_PUPPET] = function(entity, card, pick_entityid)
            local targets = {}
            for _, target in pairs(entity.pack.placeholders) do
                if target.id ~= entity.id then
                    targets[target.id] = target.id
                end
            end
            if table.size(targets) > 0 then
                local entityid = table.random(targets, table.size(targets), entity.random_func)
                return {[entityid] = entityid}
            end
            return {} -- no puppet
        end,
        
        [CardTargetType.ALLIES_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(entity.copy.scene.match.entities) do
                if entity.side == target.side then targets[targetid] = targetid end
            end
            return targets
        end,
        
        [CardTargetType.ALLIES_PUPPET_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(entity.copy.scene.match.entities) do
                if entity.side == target.side then 
                    for _, puppet in pairs(target.pack.placeholders) do
                        if puppet.id ~= target.id then
                            targets[puppet.id] = puppet.id
                        end
                    end
                end
            end
            return targets
        end,
        
        [CardTargetType.ENEMY_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(entity.copy.scene.match.entities) do
                if entity.side ~= target.side then targets[targetid] = targetid end
            end
            return targets
        end,
        
        [CardTargetType.ENEMY_PUPPET_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(entity.copy.scene.match.entities) do
                if entity.side ~= target.side then 
                    for _, puppet in pairs(target.pack.placeholders) do
                        if puppet.id ~= target.id then
                            targets[puppet.id] = puppet.id
                        end
                    end
                end
            end
            return targets
        end,
        
        [CardTargetType.ENEMY_RANDOM] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil then
                local target = entity.copy.scene.match.entities[pick_entityid]
                if target == nil then
                    return {}
                end

                if target.side == entity.side then
                    return {}
                end

                return {[pick_entityid] = pick_entityid}
            else
                local targets = {}
                for targetid, target in pairs(entity.copy.scene.match.entities) do
                    if entity.side ~= target.side then targets[targetid] = targetid end
                end
                local entityid = table.random(targets, table.size(targets), entity.random_func)
                return entityid ~= nil and {[entityid] = entityid} or {}
            end
        end,
        
        [CardTargetType.ALLIES_RANDOM] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil then
                local target = entity.copy.scene.match.entities[pick_entityid]
                if target == nil then
                    return {}
                end

                if target.side ~= entity.side then
                    return {}
                end

                return {[pick_entityid] = pick_entityid}
            else
                local targets = {}
                for targetid, target in pairs(entity.copy.scene.match.entities) do
                    if entity.side == target.side then targets[targetid] = targetid end
                end
                local entityid = table.random(targets, table.size(targets), entity.random_func)
                return entityid ~= nil and {[entityid] = entityid} or {}
            end
        end,
        
        [CardTargetType.ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(entity.copy.scene.match.entities) do targets[targetid] = targetid end
            return targets
        end,
    }

    assert(switch_target_type[card.base.require_target] ~= nil)
    local targets = nil
    if pick_entityid == nil then
        if card.base.require_target == CardTargetType.PICK_SINGLE_ALLIES then
            targets = switch_target_type[CardTargetType.ALLIES_RANDOM](entity, card, pick_entityid)
        end
        if card.base.require_target == CardTargetType.PICK_SINGLE_ENEMY then
            targets = switch_target_type[CardTargetType.ENEMY_RANDOM](entity, card, pick_entityid)
        end
    end
    if targets == nil then
        targets = switch_target_type[card.base.require_target](entity, card, pick_entityid)
    end
--    table.dump(targets, "choose target, card: " .. tostring(card.baseid))
    return targets    
end


--
-- target is entity object
--
function CardSettleDamage(entity, card, target)
    local switch_damage_type = {
        [CardDamageType.NONE] = function(entity, card, target)
            -- no damage
        end,
        [CardDamageType.PHYSICAL] = function(entity, card, target)
            local damage = (card.base.damage_value + entity.pack.strength) * entity.pack.weakness
            damage = math.floor(damage + 0.5)
            if damage < target.armor then
                target:armor_add(-damage)
            else
                damage = damage - target.pack.armor
                target:armor_add(-target.pack.armor)
                target:hp_add(-damage)
                cc.ScriptMatchEventDamage(target.id, damage)
            end
        end,
        [CardDamageType.SPELL] = function(entity, card, target)
            local damage = (card.base.damage_value + entity.pack.strength) * entity.pack.weakness
            damage = math.floor(damage + 0.5)
            if damage < target.pack.shield then
                target:shield_add(-damage)
            else
                damage = damage - target.pack.shield
                target:shield_add(-target.pack.shield)
                target:hp_add(-damage)
                cc.ScriptMatchEventDamage(target.id, damage)
            end
        end,
    }

    assert(target ~= nil)
    assert(switch_damage_type[card.base.damage_type] ~= nil)
    switch_damage_type[card.base.damage_type](entity, card, target)
end

--
-- void CardSettle(entity, card, pick_entity)
--
function CardSettle(entity, card, pick_entity)
    local CardSettleTargetStage1 = function(entity, card, target_entityid)
        local target = g_copy.scene.match.entities[target_entityid]
        -- assert(target ~= nil)
        if target == nil then return end -- perhaps target is die

        -- TODO: buff_defense_attack
        
        CardSettleDamage(entity, card, target)
    end

    local CardSettleTargetStage2 = function(entity, card, target_entityid)
        local target = g_copy.scene.match.entities[target_entityid]
        -- assert(target ~= nil)
        if target == nil then return end -- perhaps target is die
        
        --
        -- effect hp
        target:hp_add(card.base.effect_hp)

        --
        -- effect mp
        target:mp_add(card.base.effect_mp)

        --
        -- effect strength
        target:strength_add(card.base.effect_strength)
        
        --
        -- effect armor
        target:armor_add(card.base.effect_armor)

        --
        -- effect shield
        target:shield_add(card.base.effect_shield)

        --
        -- effect buff
        if card.base.effect_buff ~= nil then
            for buff_baseid, buff_layers in pairs(card.base.effect_buff) do
                target:buff_add(buff_baseid, buff_layers)
            end
        end
    end

    local targets = CardGetTargets(entity, card, pick_entity ~= nil and pick_entity.id or nil)
    for _, target_entityid in pairs(targets) do
        local settle_times = card.base.settle_times
        while settle_times > 0 do
            CardSettleTargetStage1(entity, card, target_entityid)
            settle_times = settle_times - 1
        end
        CardSettleTargetStage2(entity, card, target_entityid)
    end

    if entity.death then return end -- perhaps entity is death
    
    --
    -- draw_stackdeal
    if card.base.draw_stackdeal > 0 then
        entity:card_draw_from_stackdeal(card.base.draw_stackdeal)
    end

    --
    -- draw_stackdiscard
    if card.base.draw_stackdiscard > 0 then
        entity:card_draw_from_stackdiscard(card.base.draw_stackdiscard)
    end

    --
    -- draw_stackexhaust
    if card.base.draw_stackexhaust > 0 then
        entity:card_draw_from_stackexhaust(card.base.draw_stackexhaust)
    end

    --
    -- shuffle_stackdiscard
    if card.base.shuffle_stackdiscard then
        entity:shuffle_stackdiscard()
    end
    
    --
    -- shuffle_stackexhaust
    if card.base.shuffle_stackexhaust then
        entity:shuffle_stackexhaust()
    end
end


--
------------------- Buff Settle -------------------
--

--
-- target is entity object
--
function BuffSettleDamage(target, buff)
    local switch_damage_type = {
        [CardDamageType.NONE] = function(target, buff)
            -- no damage
        end,
        [CardDamageType.PHYSICAL] = function(target, buff)
            local damage = buff.layers * buff.base.damage_value * buff.base.layer_value
            if damage > 0 then
                if damage < target.pack.armor then
                    target:armor_add(-damage)
                else
                    damage = damage - target.pack.armor
                    target:armor_add(-target.pack.armor)
                    target:hp_add(-damage)
                    cc.ScriptMatchEventDamage(target.id, damage)
                end
            end
        end,
        [CardDamageType.SPELL] = function(target, buff)
            local damage = buff.layers * buff.base.damage_value * buff.base.layer_value
            if damage > 0 then
                if damage < target.pack.shield then
                    target:shield_add(-damage)
                else
                    damage = damage - target.pack.shield
                    target:shield_add(-target.pack.shield)
                    target:hp_add(-damage)
                    cc.ScriptMatchEventDamage(target.id, damage)
                end
            end
        end,
    }

    assert(target ~= nil)    
    assert(switch_damage_type[buff.base.damage_type] ~= nil)
    switch_damage_type[buff.base.damage_type](target, buff)
end

function BuffSettle(entity, buff)
    local BuffSettleTarget = function(target, buff)
        assert(target ~= nil)

        BuffSettleDamage(target, buff)
        if target.death then return end
        
        --
        -- effect mp
        target:mp_add(buff.base.effect_mp)

        --
        -- effect strength
        target:strength_add(buff.base.effect_strength)
        
        --
        -- effect armor
        target:armor_add(buff.base.effect_armor)

        --
        -- effect shield
        -- target:shield_add(buff.base.effect_shield)
    end,

    assert(entity ~= nil)
    assert(buff ~= nil)

    if not entity.death then
        BuffSettleTarget(entity, buff)
    end
    
    --
    -- TODO: reserve attribute values   
end


--
------------------- common functions -------------------
--

-- common = require("module_common")


