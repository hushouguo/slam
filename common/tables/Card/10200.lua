------------------------
--备注
------------------------
return {
    baseid = 10200,
    name = {cn="诅咒",en=""},
    desc = {cn="【无法被打出】回合结束时失去相当于手牌数量的生命值",en=""},
    icon = "Sprites/CardFace/com06",
    play_animate = "Cast",
    play_effect = "null",
    effect_type = 0,
    effect_slot = "0",
    quality = 0,
    category = 5,
    level = 1,
    levelup_baseid = 0,
    require_career = 0,
    require_gender = 0,
    enable_play = false,
    enable_equip = false,
    enable_discard = true,
    cost_mp = 0,
    resume_mp = 0,
    settle_play = false,
    settle_discard = true,
    settle_equip = false,
    settle_times = 0,
    require_target = 0,
    damage_type = 0,
    damage_value = 0,
    effect_hp = 0,
    effect_mp = 0,
    effect_strength = 0,
    effect_armor = 0,
    effect_shield = 0,
    effect_buff = {},
    draw_stacknewone = {},
    draw_stackdeal = 0,
    draw_stackdiscard = 0,
    draw_stackexhaust = 0,
    shuffle_stackdiscard = false,
    shuffle_stackexhaust = false,
    into_stackdiscard = false,
    into_stackexhaust = false,
    into_destroy = false,
    price_gold = 100,
    price_diamond = 100,
    script_func = CardScript.curse,
}
