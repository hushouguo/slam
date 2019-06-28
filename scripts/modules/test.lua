--- 测试脚本

package.path = package.path ..';..\\?.lua';

require 'modules.module'
require 'tests.test_logic'

-- require 'modules.main'

-- for k,v in pairs(modules) do
--     print(k,v)
-- end

-- print(table.serialize(CopyScript.common(1,2,3,10)))b
-- --[[ 
-- 结果:
-- {
--     ["map_events"]={
--         [1]={["event_baseid"]=7503,["_COMMENT"]="R",},
--         [2]={["event_baseid"]=7003,["_COMMENT"]="M",},
--         [3]={["event_baseid"]=7012,["_COMMENT"]="M",},
--         [4]={["event_baseid"]=7009,["_COMMENT"]="M",},
--         [5]={["event_baseid"]=7502,["_COMMENT"]="E",},
--         [6]={["event_baseid"]=7501,["_COMMENT"]="R",},
--     },
--     ["map_baseid"] = 6000
--     ["map_baseid"]=5000,
-- } 
-- ]]

-- print(Random.randomNormal(3,1))


-- print(table.serialize(constructMonster({101,203,3})))


-- print(table.serialize(EventScript.pickShop(1,2,3,45,5)));
-- print(table.serialize(EventScript.pickBoss(1,2,3,45,5)));
-- print(table.serialize(EventScript.pickMonster(1,2,3,45,5)));



-- local LootChoice = {}


-- --- 创建一个掉落选项
-- -- @param type_key  'items','cards','gold' 三种
-- -- @return 返回掉落选项
-- -- @usage 
-- -- local l = LootChoice:New('items')
-- -- 
-- function LootChoice:New(type_key)
--     local t = {}
--     t.type_key = type_key
--     if type_key == "gold" then
--         t.data = 0
--     else
--         t.data = {}
--     end

--     local addFuncTable = {
--         ["items"] = function(self,...)
--             local _id,_count
--             if ... then
--                 for i = 1,select('#',...) ,2 do
--                     _id = select(i,...)
--                     _count = select(i+1,...)
--                     self.data[_id] = (self.data[_id] or 0) +  (_count or 1)
--                 end
--             end
--         end,
--         ["cards"] = function(self,...)
--             if ... then
--                 for i = 1,select('#',...) do
--                     self.data[#self.data+1] = select(i,...)
--                 end
--             end
--         end,
--         ["gold"] = function(self,...)
--             if ... then
--                 for i = 1,select('#',...) do
--                     self.data = self.data + select(i,...)
--                 end
--             end
--         end

--     }


--     function t:add(...)
--         return addFuncTable[type_key](self,...)
--     end

--     function t:get_data()
--         return {[type_key] = self.data}
--     end
--     return t
-- end

-- local aa = LootChoice:New('items')

-- aa:add(3,4,5)
-- -- aa:addCard(4)
-- -- aa:addGold(3)
-- print(table.serialize(aa:get_data()))
-- print(aa:get_data())

-- local rewardConstructor = {
--     ["Item"] = function(...) return {items = ...}   end,LootChoice:New("gold")
--     ["Card"] = function(...) return {cards = ...}   end,
--     ["Gold"] = function(...) return {gold  = ...}   end,
-- }

-- local RewardGroup = {}

-- --- 创建reward结构体
-- -- @usage 
-- -- local a = RewardGroup:New()  
-- -- a:add("Item",{[1003]=4})         -- 添加4个1003道具
-- -- a:add("Card",{1005,1006,1007})   -- 添加三张卡
-- -- a:add("Gold",100)                -- 添加100块钱
-- -- a.
-- -- @return 一个RewardGroup Lua结构
-- function RewardGroup:New()
--     local t = {}
--     t.data = {}

--     -- 插入一个奖励
--     function t:add(reward_type,...)
--         if ... then
--             t.data[#(t.data)+1] = rewardConstructor[reward_type](...) or false
--         end 
--     end    

--     return t
-- end


-- local a = LootGroup:New()  

-- local c = LootChoice:New()
-- a.add(c) 

-- print(table.serialize(a))