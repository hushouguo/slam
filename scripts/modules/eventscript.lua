-- 事件 脚本模块


module(...,package.seeall)

-- 事件依赖数据
local data = {
    card ={
        common   = {10010,10020,10030,10040,10050,10060,10070},
        uncommon = {10080,10090,10110,10120},
        rare     = {10140,10130},
        shop     = {19999,19998},
        boss     = {19998,19999},
    },
    monster = {
        common = {1003,1004,1005,1006},
        elite  = {1101,1102,1103,1104},
        boss   = {1201,1202,1203}
    },

    -- shop = {7301,7302},

    -- destroy = {7401},

    item = {
        all = {2001,2002,2003}
    }
}

local LootChoice = {}

--- 创建一个掉落选项
-- @param type_key  'items','cards','gold' 三种
-- @return 返回掉落选项
-- @usage
-- ## 道具掉落
-- local l = LootChoice:New('items')
-- l:add(1001,2,3004,4)       -- 一个ID,一个数量配对好,如果是奇数个参数,那么最后一个道具数量为1
-- ##卡牌掉落
-- local l = LootChoice:New('cards')
-- l:add(3001,4002,...)         -- ID组合,表示单张卡牌
-- ## 金币掉落
-- local l = LootChoice:New('gold')
-- l:add(3,4,5,6)               -- 金币数量组合,一般单个参数就够,多个单数会求和
function LootChoice:New(type_key)
    local t = {}
    t.type_key = type_key
    if type_key == "gold" then
        t.data = 0
    else
        t.data = {}
    end

    local addFuncTable = {
        ["items"] = function(self,...)
            local _id,_count
            if ... then
                for i = 1,select('#',...) ,2 do
                    _id = select(i,...)
                    _count = select(i+1,...)
                    self.data[_id] = (self.data[_id] or 0) +  (_count or 1)
                end
            end
        end,
        ["cards"] = function(self,...)
            if ... then
                for i = 1,select('#',...) do
                    self.data[#self.data+1] = select(i,...)
                end
            end
        end,
        ["gold"] = function(self,...)
            if ... then
                for i = 1,select('#',...) do
                    self.data = self.data + select(i,...)
                end
            end
        end

    }


    function t:add(...)
        return addFuncTable[self.type_key](self,...)
    end

    function t:get_data()
        return {[type_key] = self.data}
    end
    return t
end

local LootGroup = {}

--- 创建reward结构体
-- @usage
-- local a = LootGroup:New()
-- 把a:get_data() 返回给程序
-- @return 一个RewardGroup Lua结构
function LootGroup:New()
    local t = {}
    t._data = {}
    t._choices = {}

    --- 添加一个LootChoice
    function t:addChoice(loot_choice)
        self._choices[#self._choices+1] = loot_choice
    end

    function t:get_data()
        self._data = {}
        for _,v in ipairs(self._choices) do
           self._data[#self._data+1] = v:get_data() or false
        end

        return self._data
    end
    return t
end


local MonsterEvent = {}

function MonsterEvent:New()
    local t = {
        m = {},                 -- 怪物
        l = LootGroup:New()   -- 奖励
    }

    function t:get_data()
        return {
            monster = {
                monsters = self.m or {},
                rewards = self.l:get_data() or {}
            }
        }
    end

    -- 添加怪物,
    -- @usage t:addMonster
    function t:addMonster(monster_baseid)
        t.m[monster_baseid] = (t.m[monster_baseid] or 0) + 1
    end

    -- 添加Loot
    function t:addChoice(loot_choice)
        t.l:addChoice(loot_choice)
    end
    return t
end

test2 = function()

    local a = LootGroup:New()
    -- local g = LootChoice:New("gold")
    -- g:add(10)

    local is = LootChoice:New("cards")
    is:add(1003,1004)
    -- a:addChoice(g)
    a:addChoice(is)

    --     local me = MonsterEvent:New()
    --     me:addMonster(1003)
    --     me:addMonster (1004,2)
    --     me:addReward("Item",{[2004]=3})
    --     me:addReward("Gold",100)
    --     me:addReward("Card",{1,2,3,4})
    print(table.serialize(a:get_data()))
end


local pickRandom = function(tbl,random_func)
    local random_func = random_func or math.random
    return tbl[random_func(#tbl)]
end

--- 随机一个普通怪物事件
--  @param entity_id        职业ID
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 怪物事件结构
function pickMonster(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    local random_func = Random.newRandom(randomseed)

    local me = MonsterEvent:New()

    -- 怪物
    local entity_count = random_func(1,3)
    for _ = 1, entity_count do
        me:addMonster(pickRandom(data.monster.common,random_func))
    end
    -- 奖励卡牌
    local card_choice = LootChoice:New("cards")
    for i=1,3 do
        card_choice:add(pickRandom(data.card.common,random_func))
    end
    me:addChoice(card_choice)

    -- 奖励道具
    local item_choice = LootChoice:New("items")
    item_choice:add(pickRandom(data.item.all))
    me:addChoice(item_choice)

    -- 奖励钱
    local gold_choice = LootChoice:New("gold")
    gold_choice:add(10)
    me:addChoice(gold_choice)

    return me:get_data()
end


--- 随机一个精英事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 怪物事件结构
function pickElite(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    local random_func = Random.newRandom(randomseed)
    -- local monster = {}
    -- monster[1] = {entity_baseid = ,number = 1} -- 先随个厉害的

    local me = MonsterEvent:New()
    -- 先来个elite
    me:addMonster(pickRandom(data.monster.elite,random_func))

    local group_prop = 0.5      -- Magic number

    if random_func() < group_prop then                                                   -- 再看情况随2个小弟
        local common_count = random_func(1,2)
        for i = 1,common_count do
            -- monster[#monster+1] = pickRandom(data.monster.common,random_func)
            me:addMonster(pickRandom(data.monster.common,random_func)) -- 再来个小怪
        end
    end

    -- 奖励卡牌
    local card_choice = LootChoice:New("cards")
    for i=1,3 do
        card_choice:add(pickRandom(data.card.uncommon,random_func))
    end
    me:addChoice(card_choice)

    -- 奖励道具
    local item_choice = LootChoice:New("items")
    item_choice:add(pickRandom(data.item.all))
    me:addChoice(item_choice)

    -- 奖励钱
    local gold_choice = LootChoice:New("gold")
    gold_choice:add(10)
    me:addChoice(gold_choice)

    return me:get_data()

end

--- 随机一个精Boss事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 怪物事件结构
function pickBoss(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    -- local monster = {}
    -- monster[1] =pickRandom(data.monster.boss,random_func)
    -- return constructMonsterEvent(monsters)
    local me = MonsterEvent:New()
    -- 先来个elite
    me:addMonster(pickRandom(data.monster.boss,random_func))


    -- 奖励卡牌
    local card_choice = LootChoice:New("cards")
    for i=1,3 do
        card_choice:add(pickRandom(data.card.boss,random_func))
    end
    me:addChoice(card_choice)

    -- 奖励道具
    local item_choice = LootChoice:New("items")
    item_choice:add(pickRandom(data.item.all))
    me:addChoice(item_choice)

    -- 奖励钱
    local gold_choice = LootChoice:New("gold")
    gold_choice:add(10)
    me:addChoice(gold_choice)


    return me:get_data();
end

--- 随机一个商店
--  @param entity_id        职业ID
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickShop(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    local card_count = 6     -- magic number
    local item_count = 6
    local cards = {}
    local items = {}

    for i = 1,card_count do
        cards[i] = {card_baseid = pickRandom(data.card.common,random_func)}
    end

    for i = 1,item_count do
        items[i] = { item_baseid = pickRandom(data.item.all,random_func)}
    end

    return  {shop = {cards = cards,items = items}}

end

--- 随机一个删卡事件
--  @param entity_id        职业ID
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickDelCard(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return { destroy_card = {price_gold = copy_layers*10+55 }}
end

--- 随机一个剧情事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickStoryEvent(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return {story = { title = "算数题",text = "1+1=?", options = {A="2",B="1",C="我不知道"}}}
end

--- 随机一个休息事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickRest(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return {}
end

--- 入口事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickEnter(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return {}
end
--- 出口事件
--  @param entity_id    职业
--  @param copy_baseid      副本ID
--  @param copy_layers      层
--  @param event_baseid     事件的配置ID
--  @param randomseed       随机种子
--  @return userdata 事件结构
function pickExit(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return {}
end

common = function(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    return pickMonster(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
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


--[[api
    通过entityid获取entity对象: 
        local entity = g_copy.members[entityid]
        assert(entity ~= nil)
        
    获取entity的职业:
        entity.base.career

--]]

-- test()
