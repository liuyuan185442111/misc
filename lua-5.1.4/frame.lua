swin = {
	battle,mode,id,subid,frame
}

click调用comeon，返回true再调用update
goback重置frame即可，不必将四个参数都还原
item身上只存swin id和pos即可

sum = {}
table.insert(sum, {
	index = '',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
		if pos == 0 then
			win.battle = currbattle
		elseif pos == 1 then
			win.battle = sumbattle
		else
			pos = pos - 2
			if pos>=0 and pos<=#allbattle then
				win.battle = allbattle[pos]
			else
				return false
			end
		end
		return true
	end,
})
table.insert(sum, {
	index = '1',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
		if pos>=0 and pos<=10 then
			win.mode = pos
			return true
		end
		return false
	end,
})
table.insert(sum, {
	index = '1a',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1a1',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1a11',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1a12',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1b',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1b1',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1b11',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})
table.insert(sum, {
	index = '1b12',
	next = {},
	update = function()
	end,
	comeon = function(win, pos)
	end,
})

table.sort(sum, function(a,b)
	if #a.index<#b.index then
		return true
	else
		if #a.index==#b.index then
			return a.index<b.index
		end
	end
end)
skada.reverse_array(sum)

function find(t, s)
	for i=#t,1,-1 do
		if t[i].index == s then
			local r = t[i]
			table.remove(t, i)
			return r
		end
	end
end
root = find(sum, '')

function getchar(i)
	return string.char(string.byte('abcdefghijklmnopqrstuvwxyz', i))
end

function recurse(root)
	if #sum == 0 then return end
	root.left = find(sum, root.index .. '1')
	if root.left then
		recurse(root.left)
	end
	root.right = find(sum, root.index .. '2')
	if root.right then
		recurse(root.right)
	end
	for i=1,16 do
		local temp = find(sum, root.index .. getchar(i))
		if temp then
			root.next[i] = temp
			recurse(root.next[i])
		end
	end
end

recurse(root)

print(skada.dump(sum))
print(skada.dump(root))
