--++++++++++++++++++++++++++++++++++++++
-- 事件 脚本模块
--++++++++++++++++++++++++++++++++++++++

module(...,package.seeall)

local data = {
    card ={
        common = {10010,10020,10030,10040,10050,10060,10070},
        uncommon = {10080,10090,10110,10120},
        rare = {10140,10130},
        shop = {19999,19998},
        boss = {19998,19999},
    },
    monster = {
        common = {1003,1004,1005,1006},
        elite = {1101,1102,1103,1104},
        boss = {1201,1202,1203}
    },

    item = {
        all = {
            2001,2002,2003
        }
    }
}


local pickRandom = function(tbl,random_func)
    local random_func = random_func or math.random
    return tbl[random_func(#tbl)]
end




function pickMonster(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)

    local random_func = Random.newRandom(randomseed)

    local monster = {}

    local entity_count = random_func(1,3)
    for i = 1, entity_count do
        monster[i] = {entity_baseid = pickRandom(data.monster.common,random_func),number = 1}
    end

    return {entities = monster}
end

-- return monster
function pickElite(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    local random_func = Random.newRandom(randomseed)
    local monster = {}
    monster[1] = { entity_baseid = pickRandom(data.monster.elite,random_func),number = 1}

    local group_prop = 0.5      -- Magic number 

    if random_func() < group_prop then
        local common_count = random_func(1,2)
        for i = 1,common_count do
            monster[#monster+1] = {entity_baseid = pickRandom(data.monster.common,random_func),number = 1}
        end
    end

    return {entities = monster}
    
end

-- return monster
function pickBoss(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    local monster = {}
    monster[1] = { entity_baseid = pickRandom(data.monster.boss,random_func),number = 1}     -- DOTO 配置奖励
    return {entities = monster}
end

function pickShop(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    local card_count  = 6     -- magic number
    local item_count = 6
    local cards = {}
    local items = {}

    for i = 1,card_count do
        cards[i] = {card_baseid = pickRandom(data.card.common,random_func)}
    end

    for i = 1,item_count do
        items[i] = { item_baseid = pickRandom(data.item.all,random_func)} 
    end

    return {cards = cards,items = items}

end

-- TODO
function pickDelCard(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return { destroy_card = {}}
end
-- TODO
function pickStoryEvent(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return {story = {options = {}}}
end

function pickRest(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return nil
end

function pickEnter(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return nil
end

function pickExit(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return nil
end

common = function(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
    return pickMonster(entity_career, copy_baseid, copy_layers, event_baseid, randomseed)
end

test = function()
    assert (common(1,2,3,4,5)~=nil)
    assert (pickMonster(1,2,3,4,5)~=nil)
    assert (pickElite(1,2,3,4,5)~=nil)
    -- assert (pickRest()~=nil)
    assert (pickBoss(1,2,3,4,5)~=nil)
    assert (pickShop(1,2,3,4,5)~=nil)
    assert (pickDelCard(1,2,3,4,5)~=nil)
    assert (pickStoryEvent(1,2,3,4,5 )~=nil)
    -- -- assert (pickEnter()~=nil)
    -- assert (pickExit()~=nil)
end

-- test()
