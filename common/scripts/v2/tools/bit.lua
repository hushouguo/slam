--
-- bit
--

bit = {}
bit.bxor = function(num1,num2)	
	local tmp1 = num1
	local tmp2 = num2
	local str = ""
	repeat
		local s1 = tmp1 % 2
		local s2 = tmp2 % 2
		if s1 == s2 then
			str = "0"..str
		else
			str = "1"..str
		end
		tmp1 = math.modf(tmp1/2)		
		tmp2 = math.modf(tmp2/2)	
	until(tmp1 == 0 and tmp2 == 0)	
	return tonumber(str,2)
end

bit.band = function(num1,num2)	
	local tmp1 = num1	
	local tmp2 = num2	
	local str = ""	
	repeat		
		local s1 = tmp1 % 2		
		local s2 = tmp2 % 2		
		if s1 == s2 then			
			if s1 == 1 then				
				str = "1"..str			
			else				
				str = "0"..str			
			end		
		else			
			str = "0"..str		
		end		
		tmp1 = math.modf(tmp1/2)		
		tmp2 = math.modf(tmp2/2)	
	until(tmp1 == 0 and tmp2 == 0)	
	return tonumber(str,2)
end

bit.bor = function(num1,num2)	
	local tmp1 = num1	
	local tmp2 = num2	
	local str = ""	
	repeat 		
		local s1 = tmp1 % 2		
		local s2 = tmp2 % 2		
		if s1 == s2 then			
			if s1 == 0 then				
				str = "0"..str			
			else				
				str = "1"..str			
			end		
		else			
			str = "1"..str		
		end		
		tmp1 = math.modf(tmp1/2)		
		tmp2 = math.modf(tmp2/2)	
	until(tmp1 == 0 and tmp2 == 0)	
	return tonumber(str,2)
end

