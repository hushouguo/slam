------------------------
--回合开始生效一次，层数-1
------------------------
return {
    baseid = 3013,
    name = {cn="多层护甲[抄]",en=""},
    desc = {cn="【回合开始前】获得{layer}【盾】，受到攻击时，多层护甲的层数减少1",en=""},
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
    params = {},
    script_func = BuffScript.multArmor,
}
