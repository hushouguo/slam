------------------------
--备注
------------------------
return {
    baseid = 3036,
    name = {cn="【#36刃甲】",en=""},
    desc = {cn="【回合开始时】获得{[2]}层【荆棘】",en=""},
    icon = "Sprites/Icon/Buff/3036",
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
    params = {3011,2},
    script_func = BuffScript.gainThornOnRoundBegin,
}
