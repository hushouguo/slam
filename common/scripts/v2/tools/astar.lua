--
-- astar.lua
--

--
-- args:
-- 		src, dest: coord->{x=XX,y=XX}
-- 		step: 1, 2, ...
-- 		radius: like MAX(map.width, map.height)
-- 		moveable: bool moveable(coord)
-- return value: 
--		nil, errorstring or	{coord, coord, ...}
--

function FindPath(src, dest, step, radius, moveable)
--	local MAX_COST = radius * radius * radius
	local function COST(startpos, endpos)
		return math.abs(endpos.x - startpos.x) + math.abs(endpos.y - startpos.y)
	end

	local function passable(coord, moveable)
		local adjust = {
			[0] = {  x = 1,  y = 0,  cost = 1 },
			[1] = {  x = 0,  y = -1, cost = 1 },
			[2] = {  x = 0,  y = 1,  cost = 1 },
			[3] = {  x = -1, y = 0,  cost = 1 },
--			[4] = {  x = 1,  y = -1, cost = MAX_COST },
--			[5] = {  x = -1, y = -1, cost = MAX_COST },
--			[6] = {  x = -1, y = 1,  cost = MAX_COST },
--			[7] = {  x = 1,  y = 1,  cost = MAX_COST },
		}
--		for i = 0, 7 do
		for i = 0, 3 do
			if moveable({x = coord.x + adjust[i].x, y = coord.y + adjust[i].y}) then return true end
		end
		return false
	end

	local function intoHeap(heapStack, newPoint)
		heapStack.heapStack[heapStack.heapStackCursor] = newPoint
		heapStack.heapStackCursor = heapStack.heapStackCursor + 1
		local endpoint = heapStack.heapStackCursor - 1
		while endpoint ~= 0 do
			local e = heapStack.heapStack[endpoint]
			local p = heapStack.heapStack[math.floor(endpoint/2)]
			if e.f < p.f then
				local temp = e
				heapStack.heapStack[endpoint] = p
				heapStack.heapStack[math.floor(endpoint/2)] = temp
				endpoint = math.floor(endpoint/2)
			else
				break
			end
		end
	end

	local function exitHeap(heapStack)
		if heapStack.heapStackCursor == 0 then return nil end
		local ret = heapStack.heapStack[0]
		heapStack.heapStack[0] = heapStack.heapStack[heapStack.heapStackCursor - 1]
		heapStack.heapStackCursor = heapStack.heapStackCursor - 1
		local endpoint = 0
		while (endpoint * 2 + 2) < heapStack.heapStackCursor do
			local e = heapStack.heapStack[endpoint]
			local pl = heapStack.heapStack[endpoint * 2 + 1]
			local pr = heapStack.heapStack[endpoint * 2 + 2]
			if e.f > pl.f then
				local temp = e
				heapStack.heapStack[endpoint] = pl
				heapStack.heapStack[endpoint * 2 + 1] = temp
				endpoint = endpoint * 2 + 1
			elseif e.f > pr.f then
				local temp = e
				heapStack.heapStack[endpoint] = pr
				heapStack.heapStack[endpoint * 2 + 2] = temp
				endpoint = endpoint * 2 + 2
			else
				break
			end
		end
		return ret
	end

	if not passable(src, moveable) then
		return nil, "src coord is unreachable"
	end

	if not passable(dest, moveable) then
		return nil, "dest coord is unreachable"
	end

	local MAX_DISMETER = radius * 2 + 1
	local MAX_NUMBER = MAX_DISMETER * MAX_DISMETER

	local xd = math.abs(dest.x - src.x)
	local yd = math.abs(dest.y - src.y)

	if xd > (radius * step) or yd > (radius * step) then 
		return nil, "destination too far" 
	end

--	if xd <= step and yd <= step then
	if (xd <= step and src.y == dest.y) or (yd <= step and src.x == dest.x) then
		if moveable(dest) then return {dest}
		else 
--		    cc.ScriptErrorLog(string.format(">>>>> src:(%d,%d), dest:(%d,%d), step: %d, radius:%d, dest not moveable", src.x, src.y, dest.x, dest.y, step, radius))
		    return {} 
		end
	end

	local adjust = {
		[0] = {  x = 1 * step,  y = 0 * step,  cost = 1 },
		[1] = {  x = 0 * step,  y = -1 * step, cost = 1 },
		[2] = {  x = 0 * step,  y = 1 * step,  cost = 1 },
		[3] = {  x = -1 * step, y = 0 * step,  cost = 1 },
--		[4] = {  x = 1 * step,  y = -1 * step, cost = MAX_COST },
--		[5] = {  x = -1 * step, y = -1 * step, cost = MAX_COST },
--		[6] = {  x = -1 * step, y = 1 * step,  cost = MAX_COST },
--		[7] = {  x = 1 * step,  y = 1 * step,  cost = MAX_COST },
	}

	local heapStack = {
		heapStackCursor = 0,
		heapStack = {}	 -- MAX_NUMBER * 8 + 1
	}
	local costStack = {} -- MAX_NUMBER + 1
	local pathStack = {} -- MAX_NUMBER * 8 + 1
	for i = 0, MAX_NUMBER * 8 do 
		pathStack[i] = {
			pos = { x = -1, y = -1 },
			f = 0.00,
			g = 0.00,
			father = nil
		}
	end

	local root = pathStack[MAX_NUMBER * 8]
	root.pos.x = src.x
	root.pos.y = src.y
	root.g = 0.00
	root.f = root.g + COST(root.pos, dest)
	root.father = nil

	intoHeap(heapStack, root)
	
	local count = 0
	while count <= MAX_NUMBER do
--		if math.abs(root.pos.x - dest.x) <= step and math.abs(root.pos.y - dest.y) <= step then
		if (math.abs(root.pos.x - dest.x) <= step and root.pos.y == dest.y) 
			or (math.abs(root.pos.y - dest.y) <= step and root.pos.x == dest.x) 
		then
			break
		end

--		for i = 0, 7 do
		for i = 0, 3 do
			local tempPos = {
				x = root.pos.x + adjust[i].x, y = root.pos.y + adjust[i].y
			}

			if moveable(tempPos) then
				local cost = root.g + adjust[i].cost
				local index = (tempPos.y - dest.y + radius) * MAX_DISMETER + (tempPos.x - dest.x + radius)
				if (index >= 0 and index < MAX_NUMBER and (costStack[index] == nil or cost < costStack[index] or costStack[index] == 0)) then
					costStack[index] = cost;
					local node = pathStack[count * 8 + i]
					node.pos.x = tempPos.x
					node.pos.y = tempPos.y
					node.g = cost
					node.f = node.g + COST(tempPos, dest)
					node.father = root
					intoHeap(heapStack, node);
				end
			end
		end

		root = exitHeap(heapStack)
		if root == nil then
			return nil, "destination unreachable"
		end

		count = count + 1
	end

	if count <= MAX_NUMBER then
		local fullPath = {}
		if moveable(dest) then table.insert(fullPath, dest) end
		while root ~= nil do
			table.insert(fullPath, root.pos)
			if root.father ~= nil and root.father.father == nil then
				return fullPath
			end
			root = root.father
		end
		return nil, "not available path"
	end

	return nil, "destination unreachable, overflow"
end
