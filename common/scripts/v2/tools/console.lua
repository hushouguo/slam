--
-- console
--

console = {}

CONSOLE_COLOR = {
        BLACK   =   30, 
        RED     =   31, 
        GREEN   =   32, 
        BROWN   =   33, 
        BLUE    =   34, 
        MAGENTA =   35, 
        CYAN    =   36, 
        GREY    =   37, 
        LRED    =   41, 
        LGREEN  =   42, 
        YELLOW  =   43, 
        LBLUE   =   44, 
        LMAGENTA=   45, 
        LCYAN   =   46, 
        WHITE   =   47  	
}

--_G.println = _G.print
--_G.print = _G.io.write

console.setcolor = function(color)
	io.write(string.format("\x1b[%d%sm", 
		(color >= CONSOLE_COLOR.LRED and (color - 10) or color), 
		(color >= CONSOLE_COLOR.LRED and ";1" or "")))
end

console.resetcolor = function()
	io.write("\x1b[0m")
end


