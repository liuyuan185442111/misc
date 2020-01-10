-- example of for with generator functions

function generatefib (n)
  return coroutine.wrap(function ()
    local a, b = 1, 1
    while a <= n do
      coroutine.yield(a)
      a, b = b, a+b
    end
  end)
end

function generatefib2 (n)
  local a, b = 1, 1
  return function()
    if a <= n then
      a, b = b, a+b
      return b-a
    end
  end
end

for i in generatefib(1000) do print(i) end
for i in generatefib2(1000) do print(i) end
