--- 副本相关辅助代码
-- copy action
---------------------------------


local M = {}


--- 根据entityID获取entity
-- 副本相关接口都是传入的角色ID作为实参
-- @param entityid ID
-- @return entity 可能为nil
function M.queryEntity(entityid)
    local entity = g_copy.members[entityid]
    return entity
end





return M