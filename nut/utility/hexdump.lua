#!/usr/bin/env lua
-- hexdump -C file
assert(arg[1] and not arg[2], "usage: lua "..arg[0].." file")
local file=io.input(arg[1])
local offset=0
while true do
	local s=file:read(16)
	if s==nil then return end
	io.write(string.format("%08x  ",offset))
	string.gsub(s,"(.)",function(c) io.write(string.format("%02x ",string.byte(c))) end)
	io.write(string.rep(" ",3*(16-string.len(s)))) -- for last line
	io.write(" |",string.gsub(s,"[^%w%p ]","."),"|\n") 
	offset=offset+16
end
