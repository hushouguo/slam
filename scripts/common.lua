require("modules.main")

cc = _G

--
------------------- global instance -------------------
--

g_enable_debug = true

g_runonclient = true    -- single
g_runonserver = false   -- single or multiple

g_copy = nil

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
-- BuffSurviveType
--
BuffSurviveType = {
    NONE = 0, ROUND_BEGIN = 1, ROUND_END = 2, TIMES_EFFECT = 3
}

--
-- BuffSettlePoint
--
BuffSettlePoint = {
    NONE = 0, ROUND_BEGIN = 1, ROUND_END = 2
}

--
-- BuffDefenseDamageType
--
BuffDefenseDamageType = {
    NONE = 0, HP = 1, MP = 2
}

-- 
-- BuffTierType
--
BuffTierType = {
    NONE                =   0,  --无类型
    EXPAND_DURATION     =   1,  --延长生存时间
    ENLARGE_EFFECT      =   2,  --放大BUFF效果
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
}

--
-- BlockCategory
--
BlockCategory = {
	NONE				=	0,	--无阻挡
	STATIC				=	1,	--静态阻挡
	DYNAMIC				=	2,	--动态阻挡, 破损后阻挡消失
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
                Error(entity, card, nil, "pick_entityid is nil")
                return {}
            end
            
            local target = g_copy.scene.match.entities[pick_entityid]
            if target == nil then
                Error(entity, card, nil, "pick_entityid is not exist")
                return {}
            end

            if target.side ~= entity.side then
                Error(entity, card, nil, "target.side: " .. tostring(target.side) .. ", entity.side: " .. tostring(entity.side))
                return {}
            end
            
            return {[pick_entityid] = pick_entityid}
        end,
        
        [CardTargetType.PICK_SINGLE_ENEMY] = function(entity, card, pick_entityid)
            if pick_entityid == nil then
                Error(entity, card, nil, "pick_entityid is nil")
                return {}
            end
            
            local target = g_copy.scene.match.entities[pick_entityid]
            if target == nil then
                Error(entity, card, nil, "pick_entityid is not exist")
                return {}
            end

            if target.side == entity.side then
                Error(entity, card, nil, "target.side: " .. tostring(target.side) .. ", entity.side: " .. tostring(entity.side))
                return {}
            end
            
            return {[pick_entityid] = pick_entityid}
        end,
        
        [CardTargetType.SELF] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil and pick_entityid ~= entity.id then
                Error(entity, card, nil, "entity.id: " .. tostring(entity.id) .. ", pick_entityid: " .. tostring(pick_entityid))
                return {}
            end
            return {[entity.id] = entity.id}
        end,
        
        [CardTargetType.SELF_PUPPET] = function(entity, card, pick_entityid)
            -- TODO: return puppet of mine
            return {}
        end,
        
        [CardTargetType.ALLIES_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(g_copy.scene.match.entities) do
                if entity.side == target.side then targets[targetid] = targetid end
            end
            return targets
        end,
        
        [CardTargetType.ALLIES_PUPPET_ALL] = function(entity, card, pick_entityid)
            -- TODO: return all puppets of allies
            return {}
        end,
        
        [CardTargetType.ENEMY_ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(g_copy.scene.match.entities) do
                if entity.side ~= target.side then targets[targetid] = targetid end
            end
            return targets
        end,
        
        [CardTargetType.ENEMY_PUPPET_ALL] = function(entity, card, pick_entityid)
            -- TODO: return all puppets of enemy
            return {}
        end,
        
        [CardTargetType.ENEMY_RANDOM] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil then
                local target = g_copy.scene.match.entities[pick_entityid]
                if target == nil then
                    --Error(entity, card, nil, "pick_entityid: " .. tostring(pick_entityid) .. " is not exist")
                    return {}
                end

                if target.side == entity.side then
                    --Error(entity, card, nil, "target.side: " .. tostring(target.side) .. ", entity.side: " .. tostring(entity.side))
                    return {}
                end

                return {[pick_entityid] = pick_entityid}
            else
                local targets = {}
                for targetid, target in pairs(g_copy.scene.match.entities) do
                    if entity.side ~= target.side then targets[targetid] = targetid end
                end
                local entityid = table.random(targets, table.size(targets), entity.random_func)
                return entityid ~= nil and {[entityid] = entityid} or {}
            end
        end,
        
        [CardTargetType.ALLIES_RANDOM] = function(entity, card, pick_entityid)
            if pick_entityid ~= nil then
                local target = g_copy.scene.match.entities[pick_entityid]
                if target == nil then
                    --Error(entity, card, nil, "pick_entityid: " .. tostring(pick_entityid) .. " is not exist")
                    return {}
                end

                if target.side ~= entity.side then
                    --Error(entity, card, nil, "target.side: " .. tostring(target.side) .. ", entity.side: " .. tostring(entity.side))
                    return {}
                end

                return {[pick_entityid] = pick_entityid}
            else
                local targets = {}
                for targetid, target in pairs(g_copy.scene.match.entities) do
                    if entity.side == target.side then targets[targetid] = targetid end
                end
                local entityid = table.random(targets, table.size(targets), entity.random_func)
                return entityid ~= nil and {[entityid] = entityid} or {}
            end
        end,
        
        [CardTargetType.ALL] = function(entity, card, pick_entityid)
            local targets = {}
            for targetid, target in pairs(g_copy.scene.match.entities) do targets[targetid] = targetid end
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
    table.dump(targets, "choose target, card: " .. tostring(card.baseid))
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
            local damage = (card.base.damage_value + entity.strength) * entity.weakness
            damage = math.floor(damage + 0.5)
            if damage < target.armor then
                target:armor_modify(-damage)
            else
                damage = damage - target.armor
                target:armor_modify(-target.armor)
                target:hp_modify(-damage)
                cc.Damage(target.id, damage)
            end
        end,
        [CardDamageType.SPELL] = function(entity, card, target)
            local damage = (card.base.damage_value + entity.strength) * entity.weakness
            damage = math.floor(damage + 0.5)
            if damage < target.shield then
                target:shield_modify(-damage)
            else
                damage = damage - target.shield
                target:shield_modify(-target.shield)
                target:hp_modify(-damage)
                cc.Damage(target.id, damage)
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
        target:hp_modify(card.base.effect_hp)

        --
        -- effect mp
        target:mp_modify(card.base.effect_mp)

        --
        -- effect strength
        target:strength_modify(card.base.effect_strength)
        
        --
        -- effect armor
        target:armor_modify(card.base.effect_armor)

        --
        -- effect shield
        target:shield_modify(card.base.effect_shield)

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
                if damage < target.armor then
                    target:armor_modify(-damage)
                else
                    damage = damage - target.armor
                    target:armor_modify(-target.armor)
                    target:hp_modify(-damage)
                    cc.Damage(target.id, damage)
                end
            end
        end,
        [CardDamageType.SPELL] = function(target, buff)
            local damage = buff.layers * buff.base.damage_value * buff.base.layer_value
            if damage > 0 then
                if damage < target.shield then
                    target:shield_modify(-damage)
                else
                    damage = damage - target.shield
                    target:shield_modify(-target.shield)
                    target:hp_modify(-damage)
                    cc.Damage(target.id, damage)
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
        target:mp_modify(buff.base.effect_mp)

        --
        -- effect strength
        target:strength_modify(buff.base.effect_strength)
        
        --
        -- effect armor
        target:armor_modify(buff.base.effect_armor)

        --
        -- effect shield
        -- target:shield_modify(buff.base.effect_shield)
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


