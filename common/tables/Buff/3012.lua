------------------------
--备注
------------------------
return {
    baseid = 3012,
    name = {cn="无实体[抄]",en=""},
    desc = {cn="使你受到的所有【伤害】变为1",en=""},
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
    script_func = BuffScript.roundBeginReduce,
}
