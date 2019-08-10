------------------------
--回合的标记BUFF
------------------------
return {
    baseid = 3029,
    name = {cn="【#29免】",en=""},
    desc = {cn="从【卡牌】受到的【直接】【伤害】减少【20%】",en=""},
    icon = "Sprites/Icon/Buff/3029",
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
