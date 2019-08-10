------------------------
--备注
------------------------
return {
    baseid = 3035,
    name = {cn="【#35圆盾】",en=""},
    desc = {cn="【回合开始时】获得{[2]}层【盾】",en=""},
    icon = "Sprites/Icon/Buff/3035",
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
    params = {3004,2},
    script_func = BuffScript.gainBuffOnRoundBegin,
}
