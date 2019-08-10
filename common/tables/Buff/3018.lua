------------------------
--备注
------------------------
return {
    baseid = 3018,
    name = {cn="额外能量",en=""},
    desc = {cn="【回合开始时】额外获得{layer}点能量",en=""},
    icon = "Sprites/Icon/Buff/DEFAULT",
    effect_gain = "null",
    effect_lost = "null",
    effect_persist = "null",
    effect_layer_add = "null",
    effect_layer_minus = "null",
    span = 0,
    level = 1,
    levelup_baseid = 0,
    max_layers = 999,
    enable_negative_layer = false,
    params = {},
    script_func = BuffScript.gainMpOnRoundBegin,
}
