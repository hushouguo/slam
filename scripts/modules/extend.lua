---
---   lua 内置库的一些扩展方法
---   string,table,math
--- 


--+++++++++++++++++++++++++++++++++++++++++++++++++++
--               table 
--+++++++++++++++++++++++++++++++++++++++++++++++++++
do
    --- [[ 序列化table表 ]]
    -- @param  obj    待转lua对象
    -- @return string 返回的字符串
    table.serialize = function (obj)
        local lua = ""
        local t = type(obj)
        if t == "number" then
            lua = lua .. obj
        elseif t == "boolean" then
            lua = lua .. tostring(obj)
        elseif t == "string" then
            lua = lua .. string.format("%q", obj)
        elseif t == "table" then
            lua = lua .. "{"
            for k, v in pairs(obj) do
                lua = lua .. "[" .. table.serialize(k) .. "]=" .. table.serialize(v) .. ","
            end
            local metatable = getmetatable(obj)
            if metatable ~= nil and type(metatable.__index) == "table" then
            for k, v in pairs(metatable.__index) do
                lua = lua .. "[" .. table.serialize(k) .. "]=" .. table.serialize(v) .. ","
            end
        end
            lua = lua .. "}"
        elseif t == "nil" then
            return nil
        else
            return "-nil-"
            --error("can not serialize a " .. t .. " type.")
        end
        return lua
    end

    --- [[数组映射]]
    -- @param tbl table 数组
    -- @param fn  function 映射函数
    -- return table 映射后的新表
    table.imap = function (tbl, fn)
        local out = {}
        for i,v in ipairs(tbl) do
            out[i] = fn(v)
        end
        return out
    end

    --- [[字典映射]]
    -- @param tbl table 字典
    -- @param fn  function 映射函数
    -- return table    映射后的新字典
    table.map = function (tbl,fn)
        local out = {}
        for k,v in pairs(tbl) do
            out[k] = fn(v)
        end
        return out
    end

    --- [[表格求和]]
    -- @param tbl table 数组
    table.itotal = function (tbl)
        local r = 0
        for i,v in ipairs(tbl) do
            r = r + tonumber(v) or 0
        end
        return r
    end

    -- [[字典求和]]
    table.total = function (tbl)
        local r = 0
        for k,v in pairs(tbl) do
            r = r + (tonumber(v) or 0)
        end
        return r
    end

    --- 判断tbl每个元素是否满足fn
    -- @param  tbl 数组
    -- @param  fn  一个函数,返回true/false
    -- @return bool 如果每个元素都满足fn，则返回true，否则返回false
    function table.isevery(tbl, fn)
        for i=1,#tbl do
            local pass
            if fn then
                pass = fn(tbl[i])
            else
                pass = tbl[i]
            end
            if not pass then return false end
        end
        return true
    end

    -- [[数组筛选]]
    function table.ifilter (tbl,check_func) 
        local r = {}
        for i,v in ipairs(tbl) do
            if check_func(v) then
                r[#r+1] = v or false
            end
        end
        return r
    end

    -- [[字典筛选]]
    function table.filter (dict,check_func) 
        local r = {}
        for k,v in pairs(dict) do
            if check_func(v) then
                r[k] = v or false
            end
        end
        return r
    end
end

--+++++++++++++++++++++++++++++++++++++++++++++++++++
--               math 
--+++++++++++++++++++++++++++++++++++++++++++++++++++
do 
    --- [[clamp01]]
    -- @param v number
    -- @rreturn 截取后的结果
    math.clamp01 = function (v)
        if v < 0 then return 0 end
        if v > 1 then return 1 end
        return v
    end

    --- [[clamp]]
    -- @param v   待截取的数字
    -- @param min 截取的下限
    -- @param max 截取的上限
    -- @return 截取的结果
    math.clamp = function (v,min,max)
        if v < min then return min end
        if v > max then return max end
        return v
    end

    --- [[截断小数]]
    -- math.round(9.025,0.01)=9.02
    -- math.round(9.025) = 9
    -- @param num  一个数字
    -- @param point number eg:0.001 表示截断3位
    math.round = function (num,point)
        local point = point or 1
        return num - num % point
    end

    --- 返回数字的符号,正数和0返回+1，负数返回-1sign
    -- @param v 数字
    -- @return +1或者-1
    math.sign = function (v)
        return v < 0 and -1 or 1
    end
end

--+++++++++++++++++++++++++++++++++++++++++++++++++++
--               string 
-- API   : string.fun(self,...) 
--       : self:func(...)
--+++++++++++++++++++++++++++++++++++++++++++++++++++
do
    --- python化的字符串Format
    -- string.pyformat("${name} ${age}",{name="hello",age="world"})
    -- @param s       string 待格式化的字符串
    -- @param format    table  用来格式化字符串的table
    -- @return 返回格式化后的字符串
    string.pyformat = function (self,format)
        return (self:gsub('($%b{})', function(w) return format[w:sub(3, -2)] or w end))
    end

    --- 分割字符串
    -- string.split(s,sep) / s:split(sep)
    -- @param sep    分割的符号，默认是冒号：
    -- @return table 结果数组
    string.split = function(self,sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        self:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
    end
end

local function test()
    print(table.serialize{name="bw",age=14,list={1,2,3},dict={a=1,b="abc"}})
    print(table.serialize(table.imap({1,2,3},function (x) return x*2  end)))
    print(table.serialize(table.map({a=3,b=4},function(x) return x*2 end)))
    assert(math.round(9.025,0.01)==9.02)
    assert(math.clamp(9.025,3,4)==4)
    assert(math.clamp01(9.025)==1)
    assert(table.itotal{1,2,3,"4"} == 10)
    print(table.total{a=1,b=2,3,"4","a12"} == 10)
    print(string.pyformat("${name} ${age}",{name="hello",age="world"}))

    print(table.serialize(string.split("a:b:c")))
    print(table.serialize(("a:b:c"):split()))
end
-- test()