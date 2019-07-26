--- 道具 表 脚本

--++++++++++++++++++++++++++++++++++++++
-- module(...,package.seeall)
local M  ={}

local function CreateEquipData()

    return {
        hp = 10, -- 10, 10%
        mp = 20,
        maxhp = 30,
        round_mp = 40,
        strength = 50,
        armor = 1,
        shield = 1,
        weakness = 1,
        buffs = {10010,10010}
    }
end

--- 测试装备
-- 属性随机
function M.testEquip(entityid,item,randomseed)
    local random = Random.newRandom(randomseed)
    return {
        hp = random(10,30),
        mp = random(1,2),
        armor = random(1,2),
    }
end

-- 测试生命药水
-- 恢复5点HP
M.testHealProp = function(entityid,item,randomseed)
    local entity = CopyUtil.queryEntity(entityid)
    EntityExt.takeHeal(entity,5)
end

M.testEngProp = function(entityid,item,randomseed)
    local entity = CopyUtil.queryEntity(entityid)
    EntityExt.addMp(entity,2)
end

M.testFood = function (entityid,item,randomseed)
    local entity = CopyUtil.queryEntity(entityid)
    EntityExt.takeHeal(entity,math.floor(entity.pack.maxhp*2e-2))
end

return M