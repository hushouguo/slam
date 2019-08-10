require('tools/astar')

function unittest()
	--[[
	{2,0,0,0,0},
	{1,1,1,1,0},
	{0,0,3,1,0},
	{0,1,1,1,0},
	{0,0,0,0,0},
	]]
	local map = {
		[0]={[0]=2,[1]=0,[2]=0,[3]=0,[4]=0},
		[1]={[0]=1,[1]=1,[2]=1,[3]=1,[4]=0},
		[2]={[0]=0,[1]=0,[2]=3,[3]=1,[4]=0},
		[3]={[0]=0,[1]=1,[2]=1,[3]=1,[4]=0},
		[4]={[0]=0,[1]=0,[2]=0,[3]=0,[4]=0},
	}
	local t, errorstring = FindPath({x=0,y=0}, {x=2,y=2}, 1, 5, function(coord)
		return coord.x < 5 and coord.y < 5 and coord.x >= 0 and coord.y >= 0 and map[coord.y][coord.x] == 0
	end)
	if t == nil then
		print("error: " .. tostring(errorstring))
	else
		print("fullPath:")
		for _, coord in pairs(t) do
			print(string.format("(%d, %d)", coord.x, coord.y))
		end
	end
end

unittest()
