------------------------
--备注
------------------------
return {
    baseid = 3000,
    name = {cn="GM",en=""},
    desc = {cn="【回合开始后】获得一张【GM】卡牌",en=""},
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
    enable_negative_layer = true,
    params = {10000},
    script_func = BuffScript.handAddNewCardOnRoundBegin,
}
