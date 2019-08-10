
--
------------------- God class -------------------
--

God = {}

God.dispatch = function(args)
    if not g_enable_god then return false end
--    if g_copy == nil then 
--        cc.ScriptErrorLog(string.format(">>>>>> g_copy is nil, func: %s", Function()))
--        return false
--    end
    local t = SplitString(args, ' ')
    local cmd = t[1]
    if cmd == nil then
        cc.ScriptErrorLog(string.format(">>>>>> cmd is nil, args: %s, func: %s", args, Function()))
        return false
    end
    local table_cmds = {
        ['chunk'] = {
            desc = '执行一段lua代码',
            args = 'lua string',
            func = function(table_cmds, t)
                local chunk = ''
                for i, str in pairs(t) do
                    if i ~= 1 and str ~= nil then chunk = chunk .. ' ' .. tostring(str) end
                end
                local func = loadstring(chunk)
                if func == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> error chunk: %s", chunk))
                    return false
                end
                func()
                return true
            end
        },


        ['addgold'] = {
            desc = '添加金币',
            args = 'value(100)',
            func = function(table_cmds, t)
                g_copy.entity_master:add_gold(t[2] == nil and 100 or tonumber(t[2]))
                return true
            end
        },

        ['addhp'] = {
            desc = '回复血量',
            args = 'value(base.maxhp)',
            func = function(table_cmds, t)
                g_copy.entity_master:hp_add(t[2] == nil and g_copy.entity_master.maxhp or tonumber(t[2]))
                return true
            end
        },

        ['addmp'] = {
            desc = '回复能量',
            args = 'value(base.round_mp)',
            func = function(table_cmds, t)
                g_copy.entity_master:mp_add(t[2] == nil and g_copy.entity_master.base.round_mp or tonumber(t[2]))
                return true
            end
        },

        ['addcard'] = {
            desc = '添加卡牌, 探索场景里添加到背包里, 战斗场景里添加到手牌',
            args = 'card_baseid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:card_baseid is nil, func: %s", Function()))
                    return false
                end        
                if g_copy.scene.match ~= nil then
                    g_copy.entity_master:stack_hold_newone(tonumber(t[2]))
                else
                    g_copy.entity_master:add_card(tonumber(t[2]))
                end
                return true
            end
        },

        ['additem'] = {
            desc = '添加物品到背包里',
            args = 'item_baseid number(1)',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:item_baseid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:add_item(tonumber(t[2]), t[3] == nil and 1 or tonumber(t[3]))
                return true
            end
        },

        ['useitem'] = {
            desc = '使用物品或装备',
            args = 'itemid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:itemid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:use_item(tonumber(t[2]))
                return true
            end
        },

        ['addpuppet'] = {
            desc = '添加宠物到背包里',
            args = 'entity_baseid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:entity_baseid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:add_puppet(tonumber(t[2]))
                return true
            end
        },

        ['addbuff'] = {
            desc = '添加buff到自身',
            args = 'buff_baseid buff_layer(1)',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:buff_baseid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:buff_add(tonumber(t[2]), t[3] == nil and 1 or tonumber(t[3]))
                return true
            end
        },

        ['endevent'] = {
            desc = '完成事件, 在战斗中则直接获胜退出',
            args = '',
            func = function(table_cmds, t)
                if g_copy.scene.match ~= nil then
                    if g_copy.scene.match.isdone then
                        cc.ScriptErrorLog(string.format(">>>>>>> g_copy.scene.match is done, func: %s", Function()))
                        return false
                    end
            	    g_copy.scene.match:done(g_copy.entity_master.side)
                else
                    if g_copy.scene.trigger_event == nil then -- trigger_event must be hold
                        cc.ScriptErrorLog(string.format(">>>>>>> g_copy.scene.trigger_event is nil, func: %s", Function()))
                        return false
                    end

                    if g_copy.scene.trigger_event.accomplish then
                        cc.ScriptErrorLog(string.format(">>>>>>> g_copy.scene.trigger_event already accomplish, func: %s", Function()))
                        return false
                    end

                    g_copy.scene:accomplish_event(g_copy.scene.trigger_event)
                end
                return true
            end
        },

        ['map'] = {
            desc = '跳转到指定层地图',
            args = 'layer(+1)',
            func = function(table_cmds, t)
                local layer = t[2] == nil and 1 or tonumber(t[2])
                g_copy:set_current_layer(layer < 1 and 0 or layer - 1)
                g_copy.enter_next_layer = true
                return true
            end
        },

        ['info'] = {
            desc = '列出实体所有信息',
            args = 'entityid(self)',
            func = function(table_cmds, t)
                local entity = g_copy.entity_master
                if t[2] ~= nil then
                    local target_entityid = tonumber(t[2])
                    if g_copy.scene.match ~= nil then
                        -- 列出参战的某个成员
                        entity = g_copy.scene.match.entities[target_entityid]
                    end
                    if entity == nil then
                        -- 列出某个盟友
                        entity = g_copy.members[target_entityid]
                    end
                    if entity == nil then
                        -- 列出自身某个宠物
                        entity = g_copy.entity_master.pack.puppets[target_entityid]
                    end
                end
                if entity == nil then
                    cc.ScriptErrorLog(string.format(">>>>>>> target: %s not found, func: %s", tostring(t[2]), Function()))
                    return false
                end
                
                cc.ScriptDebugLog(string.format("entity: %d,%d,%s", entity.id, entity.baseid, entity.base.name.cn))
                
                -- base information
                cc.ScriptDebugLog(string.format("    category: %s", entity.base.category == EntityCategory.PLAYER and 'Player' or 'Monster'))
                cc.ScriptDebugLog(string.format("    level: %d", entity.base.level))
                cc.ScriptDebugLog(string.format("    newbie: %s", tostring(entity.newbie)))
                cc.ScriptDebugLog(string.format("    createtime: %s", tostring(entity.createtime)))
                cc.ScriptDebugLog(string.format("    seed: %s", tostring(entity.seed)))
                
                -- Copy info
                cc.ScriptDebugLog(string.format("    Copy: %d,%s", g_copy.baseid, g_copy.base.name.cn))
                cc.ScriptDebugLog(string.format("        layer: %d", g_copy:current_layer()))
                cc.ScriptDebugLog(string.format("        seed: %d", g_copy:current_seed()))
                cc.ScriptDebugLog(string.format("        members: %d", table.size(g_copy.members)))
                
                -- Scene info
--                cc.ScriptDebugLog(string.format("    Scene: %d,%s", g_copy.baseid, g_copy.base.name.cn))
--                cc.ScriptDebugLog(string.format("        layer: %d", g_copy:current_layer()))
--                cc.ScriptDebugLog(string.format("        seed: %d", g_copy:current_seed()))
--                cc.ScriptDebugLog(string.format("        members: %d", table.size(g_copy.members)))

                -- pack info
                cc.ScriptDebugLog(string.format("    Pack"))
                cc.ScriptDebugLog(string.format("        death: %s", tostring(entity.death)))
                cc.ScriptDebugLog(string.format("        side: %s", entity.side == Side.ALLIES and 'allies' or 'enemy'))
                cc.ScriptDebugLog(string.format("        hp: %d", entity.pack.hp))
                cc.ScriptDebugLog(string.format("        mp: %s", tostring(entity.pack.mp)))
                cc.ScriptDebugLog(string.format("        maxhp: %s", tostring(entity.pack.maxhp)))
                cc.ScriptDebugLog(string.format("        round_mp: %s", tostring(entity.pack.round_mp)))
                cc.ScriptDebugLog(string.format("        strength: %s", tostring(entity.pack.strength)))
                cc.ScriptDebugLog(string.format("        armor: %s", tostring(entity.pack.armor)))
                cc.ScriptDebugLog(string.format("        shield: %s", tostring(entity.pack.shield)))
                cc.ScriptDebugLog(string.format("        weakness: %s", tostring(entity.pack.weakness)))
                
                if entity.coord ~= nil then
                cc.ScriptDebugLog(string.format("        coord: (%d, %d)", entity.coord.x, entity.coord.y))
                else
                cc.ScriptDebugLog(string.format("        coord: nil"))
                end
                                
                cc.ScriptDebugLog(string.format("        gold: %d", entity.pack.gold))
                cc.ScriptDebugLog(string.format("        cards: %d", table.size(entity.pack.cards)))
                for _, card in pairs(entity.pack.cards) do
                cc.ScriptDebugLog(string.format("            card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
                end
                cc.ScriptDebugLog(string.format("        items: %d", table.size(entity.pack.items)))
                for _, item in pairs(entity.pack.items) do
                cc.ScriptDebugLog(string.format("            item: %d,%d,%s", item.id, item.baseid, item.base.name.cn))
                end
                cc.ScriptDebugLog(string.format("        buffs: %d", table.size(entity.pack.buffs)))
                for _, buff in pairs(entity.pack.buffs) do
                cc.ScriptDebugLog(string.format("            buff: %d,%d,%s", buff.id, buff.baseid, buff.base.name.cn))
                end
                cc.ScriptDebugLog(string.format("        puppets: %d", table.size(entity.pack.puppets)))
                for _, puppet in pairs(entity.pack.puppets) do
                cc.ScriptDebugLog(string.format("            puppet: %d,%d,%s", puppet.id, puppet.baseid, puppet.base.name.cn))
                end
                cc.ScriptDebugLog(string.format("        placeholders: %d", table.size(entity.pack.placeholders)))
                for placeholder, puppet in pairs(entity.pack.placeholders) do
                cc.ScriptDebugLog(string.format("            [%d]: %d,%d,%s", placeholder, puppet.id, puppet.baseid, puppet.base.name.cn))
                end

                -- stack_hold
                cc.ScriptDebugLog(string.format("        stack hold: %d", table.size(entity.pack.stack_hold)))
                for _, card in pairs(entity.pack.stack_hold) do
                cc.ScriptDebugLog(string.format("            card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
                end
                -- stack_deal
                cc.ScriptDebugLog(string.format("        stack deal: %d", table.size(entity.pack.stack_deal)))
                for _, card in pairs(entity.pack.stack_deal) do
                cc.ScriptDebugLog(string.format("            card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
                end
                -- stack_discard
                cc.ScriptDebugLog(string.format("        stack discard: %d", table.size(entity.pack.stack_discard)))
                for _, card in pairs(entity.pack.stack_discard) do
                cc.ScriptDebugLog(string.format("            card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
                end
                -- stack_exhaust
                cc.ScriptDebugLog(string.format("        stack exhaust: %d", table.size(entity.pack.stack_exhaust)))
                for _, card in pairs(entity.pack.stack_exhaust) do
                cc.ScriptDebugLog(string.format("            card: %d,%d,%s", card.id, card.baseid, card.base.name.cn))
                end
                
                -- match info
                if g_copy.scene.match ~= nil then
                cc.ScriptDebugLog(string.format("    Match"))
                cc.ScriptDebugLog(string.format("        isstart: %s", tostring(g_copy.scene.match.isstart)))
                cc.ScriptDebugLog(string.format("        isdone: %s", tostring(g_copy.scene.match.isdone)))
                cc.ScriptDebugLog(string.format("        side_victory: %s", tostring(g_copy.scene.match.side_victory)))
                cc.ScriptDebugLog(string.format("        round_total: %s", tostring(g_copy.scene.match.round_total)))
                cc.ScriptDebugLog(string.format("        round_entityid: %s", tostring(g_copy.scene.match.round_entityid)))
                cc.ScriptDebugLog(string.format("        round_iterator: %s", tostring(g_copy.scene.match.round_iterator)))
                cc.ScriptDebugLog(string.format("        round_list_size: %s", tostring(g_copy.scene.match.round_list_size)))
                cc.ScriptDebugLog(string.format("        round_list: %d", table.size(g_copy.scene.match.round_list)))
                for _, entityid in pairs(g_copy.scene.match.round_list) do
                cc.ScriptDebugLog(string.format("            entityid: %d", entityid))
                end
                cc.ScriptDebugLog(string.format("        entities: %d", table.size(g_copy.scene.match.entities)))
                for _, target_entity in pairs(g_copy.scene.match.entities) do
                cc.ScriptDebugLog(string.format("            entity: %d,%d,%s, death: %s", target_entity.id, target_entity.baseid, target_entity.base.name.cn, tostring(target_entity.death)))
                end
                else
                cc.ScriptDebugLog(string.format("    Match: nil"))
                end

                return true
            end
        },
        

        ['levelupcard'] = {
            desc = '升级背包里的卡牌',
            args = 'cardid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:cardid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:levelup_card(tonumber(t[2]), {price_gold = 0})
                return true
            end
        },

        ['leveluppuppet'] = {
            desc = '升级背包里的宠物',
            args = 'entityid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:entityid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:levelup_puppet(tonumber(t[2]), {price_gold = 0})
                return true
            end
        },

        ['destroycard'] = {
            desc = '销毁背包里的卡牌',
            args = 'cardid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:cardid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:destroy_card(tonumber(t[2]), {price_gold = 0})
                return true
            end
        },

        ['destroyitem'] = {
            desc = '销毁背包里的道具',
            args = 'itemid',
            func = function(table_cmds, t)
                cc.ScriptErrorLog(string.format("cmd: destroyitem not implement yet!"))
                return true
            end
        },

        ['destroypuppet'] = {
            desc = '解散背包里的宠物',
            args = 'entityid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:entityid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:destroy_puppet(tonumber(t[2]))
                return true
            end
        },

        ['arrangeplaceholder'] = {
            desc = '宠物排位',
            args = 'entityid placeholder',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:entityid is nil, func: %s", Function()))
                    return false
                end
                g_copy.entity_master:arrange_placeholder(tonumber(t[2]), t[3] == nil and Placeholder.NONE or tonumber(t[3]))
                return true
            end
        },
        
		['buycard'] = {
            desc = '商店买卡',
            args = 'card_baseid',
            func = function(table_cmds, t)
                if t[2] == nil then
                    cc.ScriptErrorLog(string.format(">>>>>> t[2]:card_baseid is nil, func: %s", Function()))
                    return false
                end
				if g_copy.scene.trigger_event == nil then				
					cc.ScriptDebugLog(string.format("current not shop"))
				else
					g_copy.scene:purchase_card(g_copy.entity_master.id, g_copy.scene.id, g_copy.scene.trigger_event.id, tonumber(t[2]))
				end
                return true
            end
        },

        ['help'] = {
            desc = '帮助',
            args = '',
            func = function(table_cmds, t)
                cc.ScriptDebugLog(string.format("God commands list:"))
                for cmd, o in pairs(table_cmds) do
                    cc.ScriptDebugLog(string.format("    %s %s  --- %s", cmd, o.args, o.desc))
                end
                return true
            end
        }
    }
    if table_cmds[tostring(cmd)] == nil then
        cc.ScriptErrorLog(string.format(">>>>>> cmd: %s not implement, func: %s", tostring(cmd), Function()))
        return false
    end
    return table_cmds[tostring(cmd)].func(table_cmds, t)    
end


