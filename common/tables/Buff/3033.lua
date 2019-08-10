------------------------
--备注
------------------------
return {
    baseid = 3033,
    name = {cn="【#33短剑】",en=""},
    desc = {cn="【回合开始时】对【随机敌人】造成{[1]}点【物理伤害】",en=""},
    icon = "Sprites/Icon/Buff/3033",
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
    params = {1},
    script_func = BuffScript.damageRandomEnemyOnRoundBegin,
}
