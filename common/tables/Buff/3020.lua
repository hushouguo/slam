------------------------
--
------------------------
return {
    baseid = 3020,
    name = {cn="金属化",en=""},
    desc = {cn="【回合结束时】获得{layer*[2]}点【盾】",en=""},
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
    params = {3004,1},
    script_func = BuffScript.gainBuffOnRoundEnd,
}
