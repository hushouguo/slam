------------------------
--layer_value 表示初始BUFF层数
------------------------
return {
    baseid                = 3016,
    name                  = {cn="【狂暴】",en="windfury"},
    icon = "Sprites/Icon/Buff/3016",
    effect_gain           = "null",
    effect_lost           = "null",
    effect_persist        = "null",
    effect_layer_add      = "null",
    effect_layer_minus    = "null",
    span                  = 0,
    level                 = 0,
    levelup_baseid        = 0,
    max_layers            = 999,
    enable_negative_layer = false,
    layer_value           = 3,
    enable_script         = true,
    script_func           = BuffScript.windfury,
    desc                  = {cn="每使用layer_value张攻击牌获得一张临时的【攻击牌】",en=""},
}
