LeetCode 131：Palindrome Partitioning
> Given a string s, partition s such that every substring of the partition is a palindrome.
> 
> Return all possible palindrome partitioning of s.
> 
>For example, given s = "aab"
>
>Return
>
	[
		["aa","b"],
		["a","a","b"]
	]
>

该问题简单来说就是给定一个字符串，将字符串分成多个部分，满足每一部分都是回文串，输出所有可能的情况。
思路是：首先通过动态规划的方法获得字符串中每两个字符之间确定的子串是否为回文，然后用深度优先搜索的方法获得所有可能解。

可以通过的Python代码为：
```Python
import copy
class Solution:
    # 得到回文关系图
    # @param {string} s
    # @return {string[]}
    def getmap(self, s):
        arr=[]
        for i in range(len(s)):
            arr.append([False]*len(s)) # attention 1
        for i in range(len(s)-1,-1,-1):
            for j in range(i,len(s)):
                if i==j:
                    arr[i][j]=True
                elif s[i]==s[j]:
                    if j==i+1 or arr[i+1][j-1]==True:
                        arr[i][j]=True
        return arr

    # 深度优先搜索
    # @param {string,int,int[],string[],string[][]} s,i,arr,once,result
    def dfs(self,s,i,arr,once,result):
        for j in range(i,len(s)):
            if arr[i][j]==True:
                once.append(s[i:j+1])
                if j+1==len(s):
                    result.append(copy.copy(once)) # attention 2
                self.dfs(s,j+1,arr,once,result)
                once.pop()

    # @param {string} s
    # @return {string[][]}
    def partition(self, s):
        # 获得s的回文关系图
        arr = self.getmap(s)
        # result保存最终结果, once保存每个分割, 也就是result的元素
        once = []
        result = []
        self.dfs(s,0,arr,once,result)
        return result
```
用dp[i][j]（j≥i）为True表示从i到j之间（包括i和j）的子串是回文序列，生成dp的递推公式为：
$$
dp[i][j] = \begin{cases}
dp[i+1][j-1]\ and\ s[i]==s[j] &\text{$j-i>1$}
\\ture &\text{$i==j$}
\\s[i]==s[j] &\text{$j-i==1$}
\end{cases}
$$

此题结束，但有两个地方需要说明。
##attention 1
生成那个二维数组的时候，用的是：

	arr=[]
	for i in range(len(s)):
		arr.append([False]*len(s))
而没用

	arr=[[False]*len(s)]*len(s)
例如：

	arr=[[0]*3]*3
	arr[0][0]=1
	print(arr)
	# 输出是[[1, 0, 0], [1, 0, 0], [1, 0, 0]]
因为这样初始化，arr的三个元素都指向同一个变量，是同一变量的3个引用，变一个另外两个也会变，更明显的例子是：`arr=[[0,1,2]]*3`。
##attention 2
	result.append(copy.copy(once))
这里先对once进行了拷贝再加到result尾部，如果不先进行拷贝，result的每个元素都将是最后一次append中once的引用。简单的例子：

	a=[]
	b=[]
	a.append(1)
	b.append(a)
	a.append(1)
	b.append(a)
	print(b)
	# 输出是[[1, 1], [1, 1]]
因为append执行的就是实际操作是**增加引用计数**，所以最后b的每个元素都是相同的。
>copy.copy( )：浅拷贝，而不是增加引用计数，只拷贝父对象，不会拷贝对象的内部的子对象
>copy.deepcopy( )：深拷贝，拷贝对象及其子对象

一个很好的例子：
```
import copy
a = [1, 2, 3, 4, ['a', 'b']]

b = a  # 赋值, 传对象的引用
c = copy.copy(a)  # 对象拷贝, 浅拷贝
d = copy.deepcopy(a)  # 对象拷贝, 深拷贝

a.append(5)  # 修改对象a
a[4].append('c')  # 修改对象a中的['a', 'b']数组对象

print('a =', a)
print('b =', b)
print('c =', c)
print('d =', d)
```
输出结果：

	a = [1, 2, 3, 4, ['a', 'b', 'c'], 5]
	b = [1, 2, 3, 4, ['a', 'b', 'c'], 5]
	c = [1, 2, 3, 4, ['a', 'b', 'c']]
	d = [1, 2, 3, 4, ['a', 'b']]
<br>
现在看来，attention 1中的*也可以看作是增加引用计数：

	b = [(1,2)]*2
	print(b)
	print(id(b[0]), id(b[1]))
其输出是：

	[(1, 2), (1, 2)]
	37936520 37936520
果然如此。
##其他
1）几乎所有的赋值（=）都是增加引用计数，包括函数的默认参数。（切片赋值不是）
2）切片取得的是原序列的浅复制副本。
3）不可变对象的复制没什么意义的，反正它的值又不会变，单纯的增加引用计数也就够了。str是不可变对象。
4）x = x + y，x 出现两次，必须执行两次，性能不好
x += y，x 只出现一次，也只会计算一次，性能好
当 x、y 为list时， += 会自动调用 extend 方法进行合并运算，in-place change，而第一种方式会生成新的 x，然后进行拷贝。
6）有些内置函数，例如 list，能够生成拷贝 list(L)。
7）deepcopy 本质上是递归 copy。
8）字典的 copy 方法，D.copy() 能够复制字典，但此法只能浅层复制。
##参考
[leetcode之 Palindrome Partitioning I&II](http://blog.csdn.net/yutianzuijin/article/details/16850031)
[Python 拷贝对象（深拷贝deepcopy与浅拷贝copy）](http://www.jb51.net/article/15714.htm)
[python基础（5）：深入理解 python 中的赋值、引用、拷贝、作用域](http://my.oschina.net/leejun2005/blog/145911#OSC_h3_3)