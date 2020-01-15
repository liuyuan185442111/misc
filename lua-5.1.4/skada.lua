#! /usr/bin/env lua

function formatnum0(n)
	if math.type(n) == 'integer' then
		return tostring(n)
	else
		return string.format('%.2f', n)
	end
end

function formatnum(n)
	if type(n) ~= 'number' then return '' end
	if n<10000 then
		return formatnum0(n)
	elseif n<100000000 then
		return formatnum0(n/10000)..'w'
	else
		return formatnum0(n/100000000)..'y'
	end
end

print(formatnum(tonumber(arg[1]) or 5))
