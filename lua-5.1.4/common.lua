#! /usr/bin/env lua

local function num2str_inner(n)
  if math.type(n) == 'integer' then
    return tostring(n)
  else
    return string.format('%.2f', n)
  end
end

local function num2str(n)
  if type(n) ~= 'number' then return 'x' end
  if n<10000 then
    return num2str_inner(n)
  elseif n<100000000 then
    return num2str_inner(n/10000)..'w'
  else
    return num2str_inner(n/100000000)..'y'
  end
end

local function per2str(n)
  if type(n) ~= 'number' then return 'x' end
  return string.format('%.1f%%', n*100.0)
end

local outstrtab = {}
local function outputstr(...)
  for _,str in ipairs{...} do
    table.insert(outstrtab, str)
  end
end
local function outputnumstr(str)
	if str=='inf' or str=='nan' or str=='-inf' or str=='-nan' then
		str = '0'
	end
	table.insert(outstrtab, str)
end

local function serialize(o, prefix, header, tailer)
  if header then outputstr(header) end
  local t = type(o)
  if t=='number' then
	outputnumstr(tostring(o))
    --outputnumstr(string.format('%q', o))
  elseif t=='string' or t=='boolean' or t=='nil' then
    --o will change to string before Lua 5.3.3
    outputstr(string.format('%q', o))
  elseif t=='table' then
    outputstr('{\n')
    local newprefix = prefix..'\t'
    for k,v in pairs(o) do
		--do not save items whose key contains '_NS'
		if not string.find(k, '_NS', 2, true) then
      if type(k) == 'number' then
        outputstr(newprefix, '[', k, '] = ')
      else
        outputstr(newprefix, k, ' = ')
      end
      serialize(v, newprefix)
      outputstr(',\n')
		end
    end
    outputstr(prefix, '}')
  else
    error('cannot serialize a '..t)
  end
  if tailer then outputstr(tailer) end
end

local function dump(o, header)
  outstrtab = {}
  serialize(o, '', header, '\n\n')
  print(table.concat(outstrtab))
  return table.concat(outstrtab)
end

local function clonetable(org)
  return {table.unpack(org)}
end

meter.num2str = num2str
meter.per2str = per2str
meter.dump = dump
meter.clonetable = clonetable
