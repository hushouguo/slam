------------------------
--
------------------------
return {
    baseid = 7401,
    name = {cn="卡牌回收处",en=""},
    resource = {uiModel="Models/UI/DEFAULT",mapModel="Models/Map/Map_Zhong",},
    category = 5,
    block = 2,
    width = 1,
    height = 1,
    require_tile = true,
    endtype = 2,
    script_func = EventScript.pickDelCard,
    desc = {cn="默认事件描述",en=""},
}
