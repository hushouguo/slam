require('tools/bit')
require('tools/noise')

local function unittest()
    local tiles = {} -- [y][x] = float
    local width = 15
    local height = 15
    for y = 0, height - 1 do tiles[y] = {} end

    local function tiles_dump()
    	print("--------------------------------")
    	for y = 0, height - 1 do
    		local s = ''
    		for x = 0, width - 1 do
    --			s = s .. ' ' .. string.format("%.2f", tiles[y][x])
    			s = s .. ' ' .. tiles[y][x]
    --			s = s .. ' ' .. math.floor(math.abs(tiles[y][x]))
    		end 
    		print(s)
    	end
    	print("--------------------------------")
    end

    for y = 0, height - 1 do 
    	for x = 0, width - 1 do
--    		tiles[y][x] = math.floor((SimplexNoise2D(x, y) + 1) * 50)
    --		tiles[y][x] = SimplexNoiseInRange2D(x, y, 1, 10)
            tiles[y][x] = SimplexNoise2DInteger(x, y)
    	end
    end

    tiles_dump()
end

unittest()


