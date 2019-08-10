-- 影响逻辑的一些特殊buff 标记

-- 作为战斗中属性使用



return {
    STR     = 3001, -- 强壮(增加卡牌造成的伤害)
    DEF     = 3002, -- 防御(从卡牌获得的护盾增加)
    INT     = 3003, -- 领悟(第三职业主属性)
    ARM     = 3004, -- 护甲
    WEAK    = 3005, -- 弱小(卡牌造成的伤害减少)
    BROKEN  = 3006, -- 破损(从卡牌获得的护甲减少)
    ED      = 3007, -- 易伤(受到的伤害增加)

    KEEPAMR = 3010, -- 保留护甲
    THORN   = 3011, -- 荆棘
    UNREAL  = 3012, -- 无实体

    NODRAW  = 3019,         -- 无法再抽牌
    FangHuZhuanJing = 3021, -- 防护专精
    CHUXUE = 3026,          -- 出血BUFF
    GongShouJianBei = 3031,
    XingFenJi = 3032,       -- 兴奋剂
    ZhiSi     = 3038,        -- 致死
}