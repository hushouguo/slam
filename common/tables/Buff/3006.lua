------------------------
--回合的标记BUFF
------------------------
return {
    baseid = 3006,
    name = {cn="【#6破】",en=""},
    desc = {cn="从【卡牌】获得的【护甲】减少【25%】",en=""},
    icon = "Sprites/Icon/Buff/3006",
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
    params = {0.25},
    script_func = BuffScript.roundBeginReduce,
}
