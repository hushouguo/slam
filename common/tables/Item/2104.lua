------------------------
--临时道具
------------------------
return {
    baseid            = 2104,
    name              = {cn="烟斗",en=""},
    icon              = "ICON/Item/04",
    quality           = 1,
    category          = 2,
    equip_placeholder = 2,
    level             = 0,
    levelup_baseid    = 0,
    max_overlap       = 1,
    consumable_times  = 0,
    span              = 0,
    require_career    = 0,
    require_gender    = 0,
    require_min_level = 0,
    price_gold        = 100,
    price_diamond     = 0,
    enhance_value     = {hp=30,mp=5},
    enable_script     = true,
    script_func       = ItemScript.testEquip,
    desc              = {cn="吸烟有害健康，请尽早戒烟。",en=""},
}
