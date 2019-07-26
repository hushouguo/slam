-- Buff  扩展方法

-- 操作Buff实例的一些功能封装

--- functional api

local M = {}

--- BUFF层数自减
-- @param buff buff实例
M.reduce = function(buff)
    M.modifyLayers(buff,-1);
end

--- 修改BUFF层数
-- 到0会删除BUFF
-- @param buff   buff实例
-- @param layers 层数
M.modifyLayers = function (buff,layers)
	if buff == nil then return end
    if buff.entity == nil then
        print("buffSelfReduce 错误,buff entity为空"..table.serialize(buff))
        return
    end

    buff:layers_add(layers)
    if buff.layers == 0 then
        buff.entity:buff_remove(buff.id)
    end
end

return M