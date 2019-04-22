--
-- dump table
function dump(t)  
    local dump_cache={}
    local function sub_dump(t,indent)
        if (dump_cache[tostring(t)]) then
            cc.log_trace(indent.."*"..tostring(t))
        else
            dump_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        cc.log_trace(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_dump(val,indent..string.rep(" ",string.len(pos)+8))
                        cc.log_trace(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        cc.log_trace(indent.."["..pos..'] => "'..val..'"')
                    else
                        cc.log_trace(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                cc.log_trace(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        cc.log_trace(tostring(t).." {")
        sub_dump(t,"  ")
        cc.log_trace("}")
    else
        sub_dump(t,"  ")
    end
--    cc.log_trace()
end
