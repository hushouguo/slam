------------------------
--备注
------------------------
return {
    baseid = 3017,
    name = {cn="残暴",en=""},
    desc = {cn="回合开始损失【层】点生命值,抽【层】张牌",en=""},
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
    params = {1},
    script_func = BuffScript.brutality,
}
