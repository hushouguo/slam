-- 选项事件选项Code

--++++++++++++++++++++++++++++++++++++++



-- TODO  这个很重要
-- 现在option 表内填的是如下lua结构
--[[
{
    [1] = {
        caption = {cn='恢复所有生命值',en=''},
        script_func = OptionScript.restoreAllHp,
        nextid = 0, -- 下一个选项事件的ID
    },
    [2] = {
        caption = {cn='获得少量金币点金币',en=''},
        script_func = OptionScript.gainBitGold,
        nextid = 0,
    }

}
--]]
-- 通过封装,使得填表结构变成
--[[
    {Option(NEXT),  OptionNameB(Next),...}
--]]

--[[

**使用方法**

options可以这样配置:

return {
    baseid   = 8000,
    title    = {cn="事件模板",en="test event option"},
    text     = {cn="这是一个测试",en=""},
    options  = {
        OptionScript.getGold(nextid), -- nextid 不填表示 0
        OptionScript.restoreAllHp(nextid),
        ...
    },
}



--]]


-- module(..., package.seeall)
local M= {}

function M.test(nextid)
    local function randomContent()
        local t = {
            [1] = "李白要走",
            [2] = "杜甫不走",
            [3] = "白居易说",
            [4] = "云对雨,雪对风",
            [5] = "晚照对晴空",
            [6] = "来鸿对去燕",
            [7] = "三尺剑,六钧弓",
        }
        return t[math.random(1,#t)]
    end
    return{
        caption = {
            cn = randomContent(),
            en = randomContent(),
        },
        script_func = function (...)
            print("你好,这是一个测试选项!" .. randomContent())
        end,
        nextid = nextid or 0,
    }
end


-- 获得金币的选项
function M.getGold(nextid)
    return {
        caption = {cn='获得30-50枚金币',en='Gain a little coin.'},
        script_func = function(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
            local entity = CopyUtil.queryEntity(entity_id)
            local random = Random.newRandom(randomseed)
            EntityExt.addGold(entity,random(30,50))
        end,
        nextid = nextid or 0,
    }
end


-- 获得金币的选项
function M.restoreAllHp(nextid)
    return {
        caption = {cn='恢复所有生命值',en='re'},
        script_func = function(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
            local entity = CopyUtil.queryEntity(entity_id)
            EntityExt.restoreHp(entity)
        end,
        nextid = nextid or 0,
    }
end

------------------------------------------------------------
--                  选项的事件函数
------------------------------------------------------------

-- do -- option script_func

--     -- 恢复所有生命值
--     function M.restoreAllHp(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
--         local entity = CopyUtil.queryEntity(entity_id)
--         EntityExt.restoreHp(entity)
--     end

--     -- 获取金币
--     function M.gainGold(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
--         local entity = CopyUtil.queryEntity(entity_id)
--         local random = Random.newRandom(randomseed)
--         EntityExt.addGold(entity,random(randomseed)(30,50))
--     end

--     -- 获得一张诅咒
--     function M.gainCurseCard(entity_id, copy_baseid, copy_layers, event_baseid, randomseed)
--         -- print("todo")
--     end


-- end

return M