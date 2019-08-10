------------------------
--
------------------------
return {
    baseid = 7801,
    name = {cn="选择事件",en=""},
    resource = {uiModel="Models/UI/DEFAULT",mapModel="Models/Map/Map_ChuanSongZhen",},
    category = 8,
    block = 1,
    width = 1,
    height = 1,
    require_tile = true,
    endtype = 2,
    script_func = EventScript.pickOption,
    desc = {cn="默认事件描述",en=""},
}
