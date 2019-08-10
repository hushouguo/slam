------------------------
--回合结束生效一次，层数-1
------------------------
return {
    baseid = 3014,
    name = {cn="【#14回】",en=""},
    desc = {cn="回合结束时获得【层】治疗，恢复x点生命值，再生层数减少1",en=""},
    icon = "Sprites/Icon/Buff/3014",
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
    script_func = BuffScript.rejuvenation,
}
