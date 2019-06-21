--- 随机相关的一些功能

--+++++++++++++++++++++++++++++++++++++++++++++++++++++++

module(...,package.seeall)

-- require 'extend'

--- 从若干个choice中选择一个choice
-- choices = { a = 3,b = 4,... }或者{1,2,3,4}
-- @param choices    table   权重表
-- @param total       number  总权重
-- @param random_func       function  总权重
-- @return number  选择的id
randomWeighted = function (choices,total,random_func)

    local total = table.total(choices)

    local random_func = random_func or math.random
    local threshold = random_func() * total
    local last_choice
    for k,choice in pairs(choices) do
        threshold = threshold - choice
        if threshold <= 0 then
            return k
        end        
        last_choice = k
    end    
    return last_choice
end


--- 正态分布
--@param mu    number 期望
--@param sigma number 标准差
--@param random_func function 返回一个01之间的随机值
randomNormal = function (mu,sigma,random_func)
    local random_func = random_func or math.random
    v1 = random_func();
    v2 = random_func();
    num = math.sqrt(-2 * math.log(v1)) * math.cos(math.pi * 2 * v2);
    return num * sigma + mu;
end

--- 线性同余器,提供一个种子,返回一个随机函数
-- @usage local random = newRandom(0) 
--local rndValue = random()
-- @param seed 种子
-- @return closure 线性同余的fn
-- 参考网址: https://en.wikipedia.org/wiki/Linear_congruential_generator
function newRandom(seed)
    local seed = seed or os.time
    return function(low,up)  --区间[low,up]
        seed = (seed * 9301 + 49279) % 233280
        if low and up and low < up then
            return math.floor(seed/233280*(up-low+1)+low),seed
        elseif low and up==nil then
            return math.floor(seed/233280*low+1),seed
        else
            return seed / 233280,seed
        end
    end
end



--- 简单随机不放回抽样,从数组中取若干个随机不重复元素
-- @param tbl           数组列表
-- @param pickcount     挑几个
-- @param random_func   随机函数
-- @return table 数组
function SRSWOR(tbl,pickcount,random_func)
    local random_func  = random_func or math.random

    local total = #tbl
    local pickcount = pickcount or 1   

    local index_table = {}
    for i = 1, total do index_table[i] = i end

    local result = {}
    local rnd = 0
    -- 不放回抽取
    for i = 1, pickcount do
        rnd = random_func(total)
        result[i] = tbl[index_table[rnd]]
        table.remove(index_table,rnd)
        total = total - 1        
    end

    return result
end



--- 打乱数组
-- @param tbl  原数组
-- @param random_func 随机函数，默认采用math.random
-- @return 在原数组上就地打乱的结果
function shuffleArray(tbl,random_func)
    random = random or math.random
    local arrayCount = #tbl
    for i = arrayCount, 2, -1 do
        local j = random_func(i) -- math.floor(*+1) --  math.random(1, i)
        tbl[i], tbl[j] = tbl[j], tbl[i]
    end
    return tbl
end

--- 打乱字典的key值
-- @param dict 待打乱的字典
-- @param random_func function 函数
-- @return table 结果
function shuffleKeys(dict,random_func)
    local keys = {}
    for k,v in pairs(dict) do
        table.insert(keys, k)
    end
    return table.shuffle_array(keys,random_func)
end

-- 从表格随机一个
-- @param tbl  table  没有空洞的数组
-- @param random_func 随机函数
-- @return 返回随机的表内元素
function pickRandom(tbl,random_func)
    local random_func = random_func or math.random
    return tbl[random_func(#tbl)]
end