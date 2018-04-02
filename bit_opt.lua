
BIT_OPT = {}
BIT_OPT.Bit = {
1, 			2, 			4, 			8,
16, 		32, 		64, 		128,
256, 		512, 		1024, 		2048,
4096, 		8192, 		16384, 		32768,
65536, 		131072, 	262144, 	524288,
1048576,	2097152,	4194304,	8388608,
16777216,	33554432,	67108864,	134217728,
268435456,	536870912,	1073741824,	2147483648, }

function BIT_OPT:BIT_OR(val1, val2)
	local bit1 = BIT_OPT:GET_BIT(val1)
	local bit2 = BIT_OPT:GET_BIT(val2)
	local val = 0
	local num = math.max(#bit1, #bit2)
	for i= 1,num do
		if (bit1[i] or 0) >= 1 or (bit2[i] or 0) >= 1 then
			val = val + BIT_OPT.Bit[i]
		end
	end
	return val
end

function BIT_OPT:BIT_AND(val1, val2)
	local bit1 = BIT_OPT:GET_BIT(val1)
	local bit2 = BIT_OPT:GET_BIT(val2)
	local val = 0
	local num = math.max(#bit1, #bit2)
	for i= 1,num do
		if (bit1[i] or 0) >= 1 and (bit2[i] or 0) >= 1 then
			val = val + BIT_OPT.Bit[i]
		end
	end
	return val
end

function BIT_OPT:GET_BIT(val)
	local bit = {}
	local num = 0
	while val ~= 0 do
		num = num + 1
		bit[num] = math.mod(val, 2)
		val = (val - bit[num])/2
	end
	return bit
end

function BIT_OPT:GET_VAL(bit)
	local val = 0
	if type(bit) == "table" then
		for i,v in ipairs(bit) do
			if v >= 1 then
				val = val + BIT_OPT.Bit[i]
			end
		end
	end
	return val
end

local bit1 = BIT_OPT:GET_BIT(123123123)
print(BIT_OPT:GET_VAL(bit1))
local bit2 = BIT_OPT:GET_BIT(2323123678)
print(BIT_OPT:GET_VAL(bit2))

print(BIT_OPT:BIT_AND(123123123, 2323123678))
print(BIT_OPT:BIT_OR(123123123, 2323123678))


