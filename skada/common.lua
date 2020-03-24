DEBUG111189 = false

local function num2str_inner(n)
  if math.type(n) == 'integer' then
    return tostring(n)
  else
    return string.format('%.2f', n)
  end
end
local function num2str(n)
  if type(n) ~= 'number' then return 'x' end
  if not (n*0==0) then return '0' end
  local minus = ''
  if n<0 then
    minus='-'
    n=-n
  end
  if n<10000 then
    return minus..num2str_inner(n)
  elseif n<100000000 then
    return minus..num2str_inner(n/10000)..'万'
  else
    return minus..num2str_inner(n/100000000)..'亿'
  end
end

local function per2str(n)
  if type(n) ~= 'number' then return 'x' end
  return string.format('%.1f%%', n*100.0)
end

function seconds2str(n)
  n = math.floor(n+0.5)
  local hour = 0
  local min = n//60
  n = n%60
  if min>0 then
    hour = min//60
    min = min%60
  end
  local s = ''
  if hour>0 then
    s = hour .. '小时'
  end
  if min>0 then
    s = s .. min .. '分钟'
  end
  return s .. n .. '秒'
end

local strtab
local function outputstr(...)
  for _,str in ipairs{...} do
    table.insert(strtab, str)
  end
end
--用以将除以0等情况下产生的异常数字变为0输出
local digits={}
--数字0到9
for i=48,57 do digits[i] = true end
--这里假定str是由数字转换来的字符串
local function isnormalnumstr(str)
  local a = string.byte(str, 1)
  if a ~= 45 then --负号
    return digits[a]
  else
    return digits[string.byte(str, 2)]
  end
end
local function outputnumstr(str)
  if not isnormalnumstr(str) then
    str = '0'
  end
  table.insert(strtab, str)
end
local function serialize(o, prefix, header, tailer)
  if header then outputstr(header) end
  local t = type(o)
  if t=='number' then
    if not (o*0==0) then
      table.insert(strtab, '0')
    else
      if DEBUG111189 then
        table.insert(strtab, tostring(o))
      else
        table.insert(strtab, string.format('%q', o))
      end
    end
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
  strtab = {}
  serialize(o, '', header, '\n\n')
  return table.concat(strtab)
end

local function clone_array(org)
  return {table.unpack(org)}
end

local function clone_table(org)
  local t = {}
  for k,v in pairs(org) do
    t[k] = v
  end
  return t
end

local function trans_table(org)
  local t = {}
  for _,v in pairs(org) do
    table.insert(t, v)
  end
  return t
end

local function trans_table_if(org, pred)
  local t = {}
  for _,v in pairs(org) do
    if pred(v) then
      table.insert(t, v)
    end
  end
  return t
end

local function reverse_array(seq)
  local n = #seq
  for i=1,n//2 do
    local tmp = seq[i]
    seq[i] = seq[n+1-i]
    seq[n+1-i] = tmp
  end
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

------------------------------------------------------------
skada = {}
skada.num2str = num2str
skada.per2str = per2str
skada.seconds2str = seconds2str
skada.dump = dump
skada.clone_array = clone_array
skada.clone_table = clone_table
skada.trans_table = trans_table
skada.trans_table_if = trans_table_if
skada.reverse_array = reverse_array
skada.queue = queue
