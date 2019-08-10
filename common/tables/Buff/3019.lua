------------------------
--层数表示持续回合
------------------------
return {
    baseid = 3019,
    name = {cn="无法抽牌",en=""},
    desc = {cn="回合结束前你无法再抽牌",en=""},
    icon = "Sprites/Icon/Buff/DEFAULT",
    effect_gain = "null",
    effect_lost = "null",
    effect_persist = "null",
    effect_layer_add = "null",
    effect_layer_minus = "null",
    span = 0,
    level = 1,
    levelup_baseid = 0,
    max_layers = 1,
    enable_negative_layer = false,
    params = {0},
    script_func = BuffScript.roundEndReduce,
}
