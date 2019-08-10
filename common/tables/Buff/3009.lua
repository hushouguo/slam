------------------------
--回合开始生效一次，层数-1
------------------------
return {
    baseid = 3009,
    name = {cn="【#9毒】",en=""},
    desc = {cn=" 中毒的生物会在他们的回合开始时失去【层】点生命。中毒层数每回合-1",en=""},
    icon = "Sprites/Icon/Buff/3009",
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
