------------------------
--回合的标记BUFF
------------------------
return {
    baseid = 3030,
    name = {cn="【#30闪】",en=""},
    desc = {cn="有几率闪避【卡牌】,【闪避成功，层数-1】",en=""},
    icon = "Sprites/Icon/Buff/3030",
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
    params = {0.2},
    script_func = BuffScript.roundBeginReduce,
}
