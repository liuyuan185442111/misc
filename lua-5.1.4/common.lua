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

local function serialize(o, prefix, header, tailer)
  if header then io.write(header) end
  local t = type(o)
  if t=='number' or t=='string' or t=='boolean' or t=='nil' then
    --o will change to string before Lua 5.3.3
    io.write(string.format('%q', o))
  elseif t=='table' then
    io.write('{\n')
    local newprefix = prefix..'\t'
    for k,v in pairs(o) do
      if type(k) == 'number' then
        io.write(newprefix, '[', k, '] = ')
      else
        io.write(newprefix, k, ' = ')
      end
      serialize(v, newprefix)
      io.write(',\n')
    end
    io.write(prefix, '}')
  else
    error('cannot serialize a '..t)
  end
  if tailer then io.write(tailer) end
end

local function dump(o, header)
  serialize(o, '', header, '\n\n')
end

local function clonetable(org)
  return {table.unpack(org)}
end

meter.num2str = num2str
meter.per2str = per2str
meter.dump = dump
meter.clonetable = clonetable
