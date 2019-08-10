------------------------
--回合开始生效一次，层数-1
------------------------
return {
    baseid = 3027,
    name = {cn="【#27流血】",en=""},
    desc = {cn="回合开始时失去【层】点生命。流血层数每回合-1",en=""},
    icon = "Sprites/Icon/Buff/3027",
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
    script_func = BuffScript.poison,
}
