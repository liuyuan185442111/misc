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
    --do not save items whose key ends with 'NS'
    if not(type(k)=='string' and #k>3 and string.byte(k,-2)==78 and string.byte(k,-1)==83) then
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
  return table.concat(outstrtab)
end

local function clonevector(org)
  return {table.unpack(org)}
end
local function clonetable(org)
  local t = {}
  for k,v in pairs(org) do
    t[k] = v
  end
  return t
end
local function transtable(org)
  local t = {}
  for _,v in pairs(org) do
    table.insert(t, v)
  end
  return t
end

local queue = {}
function queue.new(maxsize)
  --if setmetatable not permitted
  local q = {
    left = 1,
    right = 0,
    maxsize = maxsize or 99999999,
    size = queue.size,
    push = queue.push,
    pop = queue.pop,
    front = queue.front,
    back = queue.back,
    clear = queue.clear,
  }
  return q
end
function queue:size()
  return self.right + 1 - self.left
end
function queue:push(val)
  self.right = self.right + 1
  self[self.right] = val
  if self.right + 1 - self.left > self.maxsize then
    self[self.left] = nil
    self.left = self.left + 1
  end
end
function queue:pop()
  self[self.left] = nil
  self.left = self.left + 1
end
function queue:back()
  return self[self.right]
end
function queue:front()
  return self[self.left]
end
function queue:clear()
  self = queue.new(self.maxsize)
end

local function reverse(seq)
  local n = #seq
  for i=1,n//2 do
    local tmp = seq[i]
    seq[i] = seq[n+1-i]
    seq[n+1-i] = tmp
  end
end

skada = skada or {}
skada.num2str = num2str
skada.per2str = per2str
skada.dump = dump
skada.clonevector = clonevector
skada.clonetable = clonetable
skada.transtable = transtable
skada.queue = queue
skada.reverse = reverse
