--
-- dump table
--
table.dump = function(t, prefix)  
    local dump_cache = {}
    local function sub_dump(t, indent)
        if dump_cache[tostring(t)] then
            print(indent .. "*" .. tostring(t))
        else
            dump_cache[tostring(t)] = true
            if type(t) == "table" then
                for pos, val in pairs(t) do
                    if type(val) == "table" then
                        print(indent .. "[" .. tostring(pos) .. "] => " .. tostring(t) .. " {")
                        sub_dump(val, indent .. string.rep(" ", string.len(pos) + 8))
                        print(indent .. string.rep(" ", string.len(pos) + 6) .. "}")
                    elseif type(val) == "string" then
                        print(indent .. "[" .. tostring(pos) .. '] => "' .. val .. '"')
                    else
                        print(indent .. "[" .. tostring(pos) .. "] => " .. tostring(val))
                    end
                end
            else
                print(indent .. tostring(t))
            end
        end
    end
	if prefix ~= nil then
		print(tostring(t) .. ", " ..  tostring(prefix))
	end
    if type(t) == "table" then
		print(tostring(t) .. " {")
        sub_dump(t, "  ")
        print("}")
    else
        sub_dump(t, "  ")
    end
end

