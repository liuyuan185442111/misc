#! /usr/bin/env lua

local function format_number_base(n)
  if math.type(n) == 'integer' then
    return tostring(n)
  else
    return string.format('%.2f', n)
  end
end

local function format_number(n)
  if type(n) ~= 'number' then return 'x' end
  if n<10000 then
    return format_number_base(n)
  elseif n<100000000 then
    return format_number_base(n/10000)..'w'
  else
    return format_number_base(n/100000000)..'y'
  end
end

local function serialize(o, prefix, header, tailer)
  if header then io.write(header) end
  local t = type(o)
  --from lua 5.3.3
  if t=='number' or t=='string' or t=='boolean' or t=='nil' then
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

local function pack(o, header)
  serialize(o, '', header, '\n')
end

t={v=1,[2]={y1=3,y2={z1=4,z2=5}}}
t[1]='hello world'
y=1/3
pack(y, 'y = ')
pack(t, 't = ')
