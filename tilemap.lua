--
-- Map
--

require('tools')
require('astar')

local table_map = {
    [1] = {
        baseid = 1,
        width = 30,
        height = 30,
        obstacle_entry = 1,
        obstacle_exit = 1,
        obstacle_event = 10,
        obstacle_building = 15,
    },
}

local OBSTACLE_TILE     =   1
local OBSTACLE_EVENT    =   2
local OBSTACLE_ENTRY    =   3
local OBSTACLE_EXIT     =   4
local OBSTACLE_WALL     =   5
local OBSTACLE_WATER    =   6
local OBSTACLE_TREE     =   7
local OBSTACLE_STONE    =   8
local OBSTACLE_BUILDING =   9

local table_obstacle = {
	[1] = { -- tile
	    baseid = 1,
		desc = "地砖",
		block = false,
		tilechar = '.',
		width = 1,
		height = 1,
		unify = true,
	},

	[2] = { -- event
	    baseid = 2,
		desc = "事件点",
		block = false,
		tilechar = 'X',
		width = 1,
		height = 1,
		unify = false,
	},

	[3] = { -- entry
	    baseid = 3,
		desc = "入口",
		block = false,
		tilechar = 'E',
		width = 1,
		height = 1,
		unify = false,
	},

	[4] = { -- exit
	    baseid = 4,
		desc = "出口",
		block = false,
		tilechar = 'Q',
		width = 1,
		height = 1,
		unify = false,
	},

	[5] = { -- wall
	    baseid = 5,
		desc = "墙",
		block = true,
		tilechar = '#',
		width = 1,
		height = 1,
		unify = true,
	},

	[6] = { -- water
	    baseid = 6,
		desc = "水",
		block = true,
		tilechar = '~',
		width = 1,
		height = 1,
		unify = true,
	},

	[7] = { -- tree
	    baseid = 7,
		desc = "树",
		block = true,
		tilechar = '!',
		width = 1,
		height = 1,
		unify = false,
	},

	[8] = { -- stone
	    baseid = 8,
		desc = "石头",
		block = true,
		tilechar = '*',
		width = 1,
		height = 1,
		unify = false,
	},
	
	[9] = { -- building
	    baseid = 9,
		desc = "建筑物",
		block = true,
		tilechar = '@',
		width = 2,
		height = 3,
		unify = false,
	},
}

--
-- Obstacle class
--

Obstacle = {
    baseid = nil,
    base = nil,
    
    constructor = function(self, baseid)
        self.baseid = baseid
        self.base = table_obstacle[baseid]
        assert(self.base ~= nil)
    end
}

function Obstacle:new(baseid)
	local obstacle = {}
	self.__index = self -- obstacle.__index = function(key) return obstacle[key] end
	setmetatable(obstacle, self)
	obstacle:constructor(baseid)
	return obstacle
end

--
-- Coordinate class
--

Coordinate = {
    x = nil, 
    y = nil,

    constructor = function(self, x, y)
        self.x = x
        self.y = y
    end
}

function Coordinate:new(x, y)
	local coordinate = {}
	self.__index = self -- coordinate.__index = function(key) return coordinate[key] end
	setmetatable(coordinate, self)
	coordinate:constructor(x, y)
	return coordinate
end


--
------------------ map class
--

Map = {
    baseid = nil,
    base = nil,
    randomseed = nil,
    randomfunc = nil,
    tiles = nil, -- {[y][x] = obstacle, ... }
    
    entry_coord = nil,    -- coord: {x=XX,y=YY}
    exit_coord = nil,     -- coord: {x=XX,y=YY}
    event_coords = nil,   -- {coord, coord, ...}
    
    valid_coord = function(self, coord)
        return coord.x >= 0 and coord.y >= 0 and coord.x < self.base.width and coord.y < self.base.height
    end,
    
    fill = function(self, coord, obstacle)
        assert(obstacle.base.width > 0 and obstacle.base.height > 0)
        for y = 0, obstacle.base.height - 1 do
            local yy = y + coord.y
            assert(yy < self.base.height)
            for x = 0, obstacle.base.width - 1 do
                local xx = x + coord.x
                assert(xx < self.base.width)
				assert(self.tiles[yy][xx] == nil)
                self.tiles[yy][xx] = obstacle
            end
        end
    end,
    
    generator = function(self)
        local function tiles_init(self)
            table.clear(self.tiles)
			for y = 0, self.base.height - 1 do self.tiles[y] = {} end
        end

        local function tiles_entry_generator(self)
            self.entry_coord = {x = 0, y = 0}
            self:fill(self.entry_coord, Obstacle:new(OBSTACLE_ENTRY))
        end

        local function tiles_exit_generator(self)
            self.exit_coord = {x = self.base.width - 1, y = self.base.height - 1}
            self:fill(self.exit_coord, Obstacle:new(OBSTACLE_EXIT))
        end

        local function tiles_available_grid(self, rect)
        	local function tiles_check_grid(self, coord, rect)
        		assert(rect.width > 0 and rect.height > 0)
        		for y = 0, rect.height - 1 do
        		    local yy = y + coord.y
        		    if yy >= self.base.height then return false end -- overflow

       				for x = 0, rect.width -1 do
       				    local xx = x + coord.x
       				    if xx >= self.base.width then return false end -- overflow
       				    
       				    if self.tiles[yy][xx] ~= nil then return false end
        			end
        		end
        		return true
        	end
        
        	local t = {} -- {coord, ...}
        	for y = 0, self.base.height - 1 do
        		for x = 0, self.base.width - 1 do
        		    local coord = {x = x, y = y}
        			if tiles_check_grid(self, coord, rect) then
        			    table.insert(t, coord)
        			end
        		end
        	end
        	return t
        end

        local function tiles_event_generator(self)
            local obstacle = table_obstacle[OBSTACLE_EVENT]
        	for _ = 0, self.base.obstacle_event - 1 do
				local t = tiles_available_grid(self, {width = obstacle.width, height = obstacle.height})
				local t_size = table.size(t)
				if t_size == 0 then return false end -- not enough grid for generator
        		local i, coord = table.random(t, t_size, self.random_func)
        		assert(coord ~= nil)
        		self:fill(coord, Obstacle:new(OBSTACLE_EVENT))
        		table.insert(self.event_coords, coord)
        	end
        	return true
        end

        local function tiles_building_generator(self)
            local obstacle = table_obstacle[OBSTACLE_BUILDING]
        	for _ = 0, self.base.obstacle_building - 1 do
				local t = tiles_available_grid(self, {width = obstacle.width, height = obstacle.height})
				local t_size = table.size(t)
				if t_size == 0 then return false end -- not enough grid for generator
        		local i, coord = table.random(t, t_size, self.random_func)
        		assert(coord ~= nil)
        		self:fill(coord, Obstacle:new(OBSTACLE_BUILDING))
        	end
        	return true
        end

        local function tiles_link_event(self)
            assert(self.entry_coord ~= nil and self.exit_coord ~= nil)
            local from = self.entry_coord
            local to = {}
            for _, coord in pairs(self.event_coords) do table.insert(to, coord) end

            local to_size = table.size(to)
            while to_size > 0 do
                for index, to_coord in pairs(to) do
                    local rc, errorstring = FindPath(from, to_coord, 1, math.max(self.base.width, self.base.height), 
                        function(coord)
    		                return self:valid_coord(coord)
    		                    and (self.tiles[coord.y][coord.x] == nil or self.tiles[coord.y][coord.x].base.block == false)
    	                end)
    	            if rc ~= nil then
    	                assert(type(rc) == "table")
    	                to[index] = nil
    	                from = to_coord
    	                for _, coord in pairs(rc) do
    	                    assert(self:valid_coord(coord))
    	                    if self.tiles[coord.y][coord.x] == nil then
    	                        self:fill(coord, Obstacle:new(OBSTACLE_TILE))
    	                    end
    	                end
    	                break
    	            end
                end
                if to_size == table.size(to) then return false end
                to_size = table.size(to)
            end

            -- to exit, from is the coord of last event
            local rc, errorstring = FindPath(from, self.exit_coord, 1, math.max(self.base.width, self.base.height), 
                function(coord)
    		        return self:valid_coord(coord)
    		            and (self.tiles[coord.y][coord.x] == nil or self.tiles[coord.y][coord.x].base.block == false)
    	        end)
    	    if rc ~= nil then
    	        assert(type(rc) == "table")
    	        for _, coord in pairs(rc) do
    	            assert(self:valid_coord(coord))
    	            if self.tiles[coord.y][coord.x] == nil then
    	                self:fill(coord, Obstacle:new(OBSTACLE_TILE))
    	            end
    	        end
    	    else
    	        print("Error: " .. tostring(errorstring))
    	    end

            return true
        end

        local function tiles_dump(self)
            for y = 0, self.base.height - 1 do
                local s = ''
                for x = 0, self.base.width - 1 do
                    local obstacle = self.tiles[y][x]
                    if obstacle ~= nil then
                        s = s .. ' ' .. obstacle.base.tilechar
                    else
                        s = s .. ' ' .. ' ' 
                    end 
                end 
                print(s)
            end
        end

        tiles_init(self)
        tiles_entry_generator(self)
        tiles_exit_generator(self)
        tiles_event_generator(self)
        tiles_building_generator(self)
        tiles_link_event(self)
        tiles_dump(self)
    end,
    
    constructor = function(self, baseid, randomseed)
        self.baseid = baseid
        self.base = table_map[baseid]
        assert(self.base ~= nil)
        assert(self.base.width > 0 and self.base.height > 0)
        self.randomseed = randomseed
        self.randomfunc = NewRandom(self.randomseed)
        self.entry_coord = nil
        self.exit_coord = nil
        self.event_coords = {}
        self.tiles = {}
    end
}

function Map:new(baseid, randomseed)
	local map = {}
	self.__index = self -- map.__index = function(key) return map[key] end
	setmetatable(map, self)
	map:constructor(baseid, randomseed)
	return map
end


--
---------------------------------- unittest ------------------------------
--

local tilemap = Map:new(1, os.time())
tilemap:generator()


