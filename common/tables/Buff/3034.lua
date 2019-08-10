------------------------
--备注
------------------------
return {
    baseid = 3034,
    name = {cn="【#34长剑】",en=""},
    desc = {cn="【回合开始时】对【随机敌人】造成{[1]}点【物理伤害】",en=""},
    icon = "Sprites/Icon/Buff/3034",
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
    params = {2},
    script_func = BuffScript.damageRandomEnemyOnRoundBegin,
}
