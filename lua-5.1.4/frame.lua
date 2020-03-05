local AllFrames = {}

local function find_and_remove(t, s)
	for i=#t,1,-1 do
		if t[i].index == s then
			local r = t[i]
			table.remove(t, i)
			return r
		end
	end
end
local function getchar(i)
	return string.char(string.byte('abcdefghijklmnopqrstuvwxyz', i))
end
local function addchildren(root, sum)
	if #sum == 0 then return end
	local temp = find_and_remove(sum, root.index .. '1')
	if temp then
		root.left = temp
		addchildren(temp, sum)
	end
	temp = find_and_remove(sum, root.index .. '2')
	if temp then
		root.right = temp
		addchildren(temp, sum)
	end
	for i=1,16 do
		temp = find_and_remove(sum, root.index .. getchar(i))
		if temp then
			root.next[i] = temp
			addchildren(temp, sum)
		end
	end
end

local function BuildFrameTree()
	local sum = {}
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
		end
		if #a.index==#b.index then
			return a.index<b.index
		end
		return false
	end)

	AllFrames = skada.clone_array(sum)
	skada.reverse_array(sum)
	local root = find_and_remove(sum, '')
	addchildren(root, sum)
	return root
end

local RootFrame = BuildFrameTree()

local function NewWindow(index)
	local widget = 'CreateWidget()'
	return {
		frame = RootFrame,
		widget = widget,
		--battle,mode,id,subid are nil
	}
end
local SWin = {}
table.insert(SWin, NewWindow())
table.insert(SWin, NewWindow())
table.insert(SWin, NewWindow())

local FrameStack = {}

local function onleftclick(winid, pos)
	local win = SWin[winid]
	local currframe = win.frame
	local nextframe = currframe.left or currframe.next[pos]
	if not nextframe then
		return false
	end
	table.insert(FrameStack, currframe)
	win.frame = nextframe
	win.frame.comeon(win, pos)
	win.frame.update(win.widget, win)
	return true
end
local function onrightclick(winid, pos)
	local win = SWin[winid]
	local currframe = win.frame
	local nextframe = currframe.right
	if not nextframe then
		return false
	end
	table.insert(FrameStack, currframe)
	win.frame = nextframe
	win.frame.comeon(win, pos)
	win.frame.update(win.widget, win)
	return true
end
local function ongoback(winid, pos)
	if #FrameStack == 0 then
		return false
	end
	local win = SWin[winid]
	win.frame = FrameStack[#FrameStack]
	table.remove(FrameStack)
	win.frame.update(win.widget, win)
	return true
end
local function ontick(winid)
	local win = SWin[winid]
	win.frame.update(win.widget, win)
end
