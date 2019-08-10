------------------------
--怪物测试员
------------------------
return {
    baseid = 7001,
    name = {cn="怪物工厂",en=""},
    resource = {uiModel="Models/UI/DEFAULT",mapModel="Models/Map/DEFAULT",},
    category = 3,
    block = 1,
    width = 2,
    height = 3,
    require_tile = true,
    endtype = 2,
    script_func = EventScript.pickMonster,
    desc = {cn="默认事件描述",en=""},
}
