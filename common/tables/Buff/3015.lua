------------------------
--回合结束生效一次，层数-1
------------------------
return {
    baseid = 3015,
    name = {cn="双发[抄]",en=""},
    desc = {cn="你使用的下一张攻击牌生效【层】次",en=""},
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
    params = {1},
    script_func = BuffScript.rejuvenation,
}
