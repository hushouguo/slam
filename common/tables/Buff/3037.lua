------------------------
--备注
------------------------
return {
    baseid = 3037,
    name = {cn="【#37自动武器槽】",en=""},
    desc = {cn="【回合开始时】获得1张【普通攻击*】",en=""},
    icon = "Sprites/Icon/Buff/3037",
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
    params = {10150},
    script_func = BuffScript.handAddNewCardOnRoundBegin,
}
