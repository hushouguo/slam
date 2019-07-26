-- 事件 脚本模块

-- module(...,package.seeall)

local M = {}
-- setfenv(1,M)

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
    item = {
        all = {2000,2001,2002,2003}
    },

    puppet = {
        all = {1102,1103}
    },

    option = {
        all = {8000,8001,8002}
    },
    reward = {
        all = {7901,7902,7903,7904}
    }
}




------------DATA STRUCT DEFINE-----------------------

-- 3
--- 怪物数据
local CreateMonsterData  = function ()
    local t = {}
    t._data = {}
    t._next_id = nil

    function t:get_data()
        return {
            monster = self._data or {},
            trigger_event_baseid = self._next_id
        }
    end

    --- 添加怪物
    -- @param monster_baseid 怪物ID
    -- @usage t:addMonster
    function t:addMonster(monster_baseid)
        self._data[monster_baseid] = (self._data[monster_baseid] or 0) + 1
    end

    function t:setNextID(next_id)
        self._next_id = next_id
    end

    return t
end

-- 4
--- 商店数据
--@return
local CreateShopData = function ()
    local t = {}
    t._cards   = {}
    t._items   = {}
    t._next_id = nil

    function t:addItem(item_baseid)
        self._items[#self._items+1] = {item_baseid = item_baseid }
    end
    function t:addCard(card_baseid)
        self._cards[#self._cards+1] = {card_baseid = card_baseid }
    end
    function t:setNextID(next_id)
        self._next_id = next_id
    end
    function t:get_data()
        return {
            shop = {
                cards = self._cards,
                items = self._items,
            }
        }
    end
    return t
end

-- 5
--- 删卡数据
-- @param price 删卡价格
-- @return
local CreateDestroyCardData = function (price)
    local t = {}
    -- t._gold = 0
    t._next_id = nil

    -- function t:setPrice(price)
    --     t._gold = price
    -- end
    function t:setNextID(next_id)
        self._next_id = next_id
    end
    function t:get_data()
        return {
            destroy_card = {
                price_gold = price
            },
            trigger_event_baseid = self._next_id,
        }
    end

    return t
end

-- 6
--- 剧情数据
-- @param story_id 对白id
-- @return 剧情数据
local CreateStoryData = function (story_id)
    local t = {}
    t._next_id = nil
    function t:setNextID(v)
        self._next_id = v
    end
    function t:get_data()
        return {
            story = story_id,
            trigger_event_baseid = self._next_id
        }
    end
    return t
end

-- 7
--- 卡牌升级数据
-- @param price 升级卡牌价格
-- @return carddata
local CreateLevelUpCardData = function (price)
    local t = {}
    t._next_id = nil

    function t:setNextID(v)
        self._next_id = v
    end
    function t:get_data()
        return {
            levelup_card = {
                price_gold = price
            },
            trigger_event_baseid = self._next_id
        }
    end

    return t
end

-- 7
--- 宠物升级数据
-- @param price 升级卡牌价格
-- @return carddata
local CreateLevelUpPuppetData = function (price)
    local t = {}
    t._next_id = nil

    function t:setNextID(v)
        self._next_id = v
    end
    function t:get_data()
        return {
            levelup_puppet  = {
                price_gold = price
            },
            trigger_event_baseid = self._next_id
        }
    end

    return t
end

-- 8
local CreateOptionData = function (option_id)
    local t = {}
    t._next_id = nil
    function t:setNextID(v)
        self._next_id = v
    end
    function t:get_data()
        return {
            storyoption = option_id,
            trigger_event_baseid = self._next_id
        }
    end
    return t
end

-- 9


--- 创建一个掉落的选项
-- @param type_key  'items','cards','gold','puppets' 4种
-- @return 返回掉落选项
-- @usage
-- ## 道具掉落
-- local l = CreateRewardItemData('items')
-- l:add(1001,2,3004,4)       -- (一个ID,一个数量)键值对,如果是奇数个参数,那么最后一个道具数量为1
-- ## 卡牌掉落
-- local l = CreateRewardItemData('cards')
-- l:add(3001,4002,...)
-- ## 金币掉落
-- local l = CreateRewardItemData('gold')
-- l:add(3,4,5,6)               -- 金币数量组合,一般单个参数就够,多个单数会求和
-- ## 宠物
-- local l = CreateRewardItemData('puppets')
-- l:add(1,2,3)
local CreateRewardItemData = function (type_key)
    local t = {}
    -- t.type_key = type_key

    if type_key == "gold" then
        t.data = 0
    else
        t.data = {}
    end

    function t:add(...)
        if type_key == "cards" or type_key == "puppets" then
            for i = 1,select('#',...) do
                self.data[#self.data+1] = select(i,...)
            end
        elseif type_key == "gold" then
            self.data = self.data + ...
            -- for i = 1,select('#',...) do
            --      + select(i,...)
            -- end
        elseif type_key == "items" then
            local id,count
            for i = 1,select('#',...) ,2 do
                id = select(i,...)
                count = select(i+1,...)
                self.data[id] = (self.data[id] or 0) +  (count or 1)
            end
        end
    end

    function t:get_data()
        return {[type_key] = self.data}
    end

    return t
end

-- 9
--- 创建reward结构体
-- 掉落结构是一个list,每个list里面(非金币)都是n选1
--[[
reward = {
    {
     -- 每组里面是N选1
     cards = { card_baseid, ... },
     items = { item_baseid = number, ... },
     gold = ?,
     puppets = { entity_baseid, ... }
    },
    ...
 },
 --]]
-- @usage
-- local a = CreateRewardData()
-- 把a:get_data() 返回给程序
-- @return 一个RewardGroup Lua结构
local CreateRewardData = function ()
    local t = {}
    -- t._data = {}
    t._choices = {}
    t._next_id = nil

    --- 添加一个LootChoice
    function t:addChoice(loot_choice)
        self._choices[#self._choices+1] = loot_choice
    end

    function t:setNextID(v)
        self._next_id = v
    end
    function t:get_data()
        -- print("aa"..table.serialize(self._choices))
        local tt = {}
        for _,v in ipairs(self._choices) do

           tt[#tt+1] = v:get_data()
        end
        -- print("tt" .. table.serialize(tt))
        return  {
            reward = tt,
            trigger_event_baseid = self._next_id
        }
    end

    return t
end



----------------------------------------- test pick func ---------------------------------------------


do -- test pick func

    local pickRandom = function(tbl,random_func)
        local random_func = random_func or math.random
        return tbl[random_func(#tbl)]
    end

    -- 随机测试用一个普通怪物事件
    function M.pickMonster(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)

        local random_func = Random.newRandom(randomseed)

        local r = CreateMonsterData()

        local entity_count = random_func(1,3)
        for _ = 1, entity_count do
            r:addMonster(pickRandom(data.monster.common,random_func))
        end

        r:setNextID(7902)

        return r:get_data()
    end

    -- 随机测试用一个精英事件
    function M.pickElite(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)

        local random_func = Random.newRandom(randomseed)

        local r = CreateMonsterData()
        r:addMonster(pickRandom(data.monster.elite,random_func))

        local group_prop = 0.5      -- Magic number
        if random_func() < group_prop then        -- 再看情况随2个小弟
            local common_count = random_func(1,2)
            for i = 1,common_count do
                -- monster[#monster+1] = pickRandom(data.monster.common,random_func)
                r:addMonster(pickRandom(data.monster.common,random_func)) -- 再来个小怪
            end
        end
        r:setNextID(7903)
        return r:get_data()
    end

    -- 随机返回一个Boss事件
    function M.pickBoss(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateMonsterData()
        r:addMonster(pickRandom(data.monster.boss,random_func))

        r:setNextID(7904)
        return r:get_data()
    end

    -- 随机测试奖励事件
    function M.pickReward(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateRewardData()

        -- 奖励卡牌
        local card_choice = CreateRewardItemData("cards")
        for _=1,3 do   -- TODO :magic number
            card_choice:add(pickRandom(data.card.uncommon,random_func))
        end
        r:addChoice(card_choice)
        -- print(table.serialize(card_choice:get_data()))
        -- 奖励道具
        local item_choice = CreateRewardItemData("items")
        item_choice:add(pickRandom(data.item.all),10)
        r:addChoice(item_choice)

        -- 奖励钱
        local gold_choice = CreateRewardItemData("gold")
        gold_choice:add(10)
        r:addChoice(gold_choice)

        -- 奖励宠物
        local puppets_choice = CreateRewardItemData("puppets")
        puppets_choice:add(1102,1103)
        r:addChoice(puppets_choice)
        -- print("lg" .. table.serialize(lg))
        return r:get_data()
    end

    -- 弱怪奖励
    function M.pickRewardA(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)

        local random_func = Random.newRandom(randomseed)
        local r = CreateRewardData()

        -- 奖励卡牌
        local card_choice = CreateRewardItemData("cards")
        for _=1,3 do   -- TODO :magic number
            card_choice:add(pickRandom(data.card.uncommon,random_func))
        end
        r:addChoice(card_choice)
        -- 奖励道具
        local item_choice = CreateRewardItemData("items")
        item_choice:add(pickRandom(data.item.all),10)
        r:addChoice(item_choice)

        -- 奖励钱
        local gold_choice = CreateRewardItemData("gold")
        gold_choice:add(random_func(10,20))
        r:addChoice(gold_choice)

        return r:get_data()
    end

    -- 普通怪奖励
    function M.pickRewardB(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateRewardData()

        -- 奖励卡牌
        local card_choice = CreateRewardItemData("cards")
        for _= 1,3 do   -- TODO :magic number
            card_choice:add(pickRandom(data.card.uncommon,random_func))
        end
        r:addChoice(card_choice)
        -- 奖励道具
        local item_choice = CreateRewardItemData("items")
        item_choice:add(pickRandom(data.item.all),10)
        r:addChoice(item_choice)
        -- 奖励钱
        local gold_choice = CreateRewardItemData("gold")

        gold_choice:add(random_func(20,30))
        r:addChoice(gold_choice)
        return r:get_data()
    end

    -- 精英奖励
    function M.pickRewardC(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateRewardData()

        -- 奖励卡牌
        local card_choice = CreateRewardItemData("cards")
        for _=1,3 do   -- TODO :magic number
            card_choice:add(pickRandom(data.card.uncommon,random_func))
        end
        r:addChoice(card_choice)
        -- 奖励钱
        local gold_choice = CreateRewardItemData("gold")
        gold_choice:add(random_func(30,40))
        r:addChoice(gold_choice)
        -- 奖励道具
        local item_choice = CreateRewardItemData("items")
        item_choice:add(pickRandom(data.item.all),10)
        r:addChoice(item_choice)

        return r:get_data()
    end

    -- boss奖励
    function M.pickRewardD(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        return M.pickReward(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
    end

    --- 随机一个商店
    function M.pickShop(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local card_count = 6     -- magic number

        local r = CreateShopData()

        for _ = 1,card_count do
            r:addCard(pickRandom(data.card.common,random_func))
        end

        -- local item_count = 6
        -- for i = 1,item_count do
        --     r:addItem(pickRandom(data.item.all,random_func))
        -- end

        return r:get_data()
    end

    -- 随机一个删卡事件
    function M.pickDelCard(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local r = CreateDestroyCardData(copy_layers*10+55)
        return r:get_data()
    end

    --- 升级卡牌事件
    function M.pickUpgradeCard(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local r = CreateLevelUpCardData(0)
        return r:get_data()
    end

    -- 随机一个剧情事件
    function M.pickStory(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local r = CreateStoryData(0)
        return r:get_data()
    end

    --- 选项事件
    function M.pickOption(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateOptionData(pickRandom(data.option.all,random_func))
        return r:get_data()
    end

    -- 宠物升级
    function M.pickPuppetLevelup(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
        local random_func = Random.newRandom(randomseed)
        local r = CreateLevelUpPuppetData(0)
        return r:get_data()
    end
    -- 入口事件
    function M.pickEnter(...)
        return {}
    end

    -- 出口事件
    function M.pickExit(...)
        return {}
    end
end



return  M