--
------------------- Scene pattern -------------------
--

--
-- 15 x 15
--

local pattern_width = 15
local pattern_height = 15

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- 4
	{ X, X, X, X, I, ., ., ., ., ., I, X, X, X, X }, -- 5
	{ X, X, X, X, I, ., ., ., ., ., I, X, X, X, X }, -- 6
	{ X, X, X, X, I, ., ., ., ., ., I, X, X, X, X }, -- 7
	{ X, X, X, X, I, ., ., ., ., ., I, X, X, X, X }, -- 8
	{ X, X, X, X, I, ., ., ., ., ., I, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_L = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}

local pattern_I = {
----  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E	
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 0
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 1
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 2
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 3
	{ X, X, X, X, X, X, X, X, X, X, X, X, X, X, X }, -- 4
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 5
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 6
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 7
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 8
	{ X, X, X, X, X, ., ., ., ., ., X, X, X, X, X }, -- 9
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- A
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- B
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- C
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- D
	{ X, X, X, X, X, I, ., ., ., I, X, X, X, X, X }, -- E
}
