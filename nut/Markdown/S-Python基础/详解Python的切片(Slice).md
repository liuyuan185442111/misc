先看例子：
```
array = [0, 1, 2, 3, 4, 5]
print(array[:])
print(array[::-1])
print(array[::2])
print(array[:-4:-1])
print(array[-4:5])
print(array[-10:3])
```
结果是：

	[0, 1, 2, 3, 4, 5]
	[5, 4, 3, 2, 1, 0]
	[0, 2, 4]
	[5, 4, 3]
	[2, 3, 4]
	[0, 1, 2]
这就是 Python 的切片（Slice）操作。

切片的语法是：

	m[start:stop:step]
其中：

	start，stop，step，:step 均可以省略，省略的 start 和 stop 会被 None 替换，省略的 step 会被 1 替换
	m 可以是 list，tuple，str，range，或支持 getitem 等几个方法的自定义类
	start 和 stop 可以是 None，整数，或拥有 __index__ 方法的对象（__index__方法须返回一个整数）
	step 必须是非零整数
	返回一个包含 m 部分元素的副本（浅复制）

结果的获得：

	1) 找到 start 和 stop 在 m 中对应的位置
	2) step 为正时，如果 start 为 None，可认为 start 为0，如果 stop 为 None，可认为 stop 为 len(m)；	step 为负时，如果 start 为 None，可认为 start 为 -1，如果 stop 为 None，可认为 stop 为 -1-len(m)
	3) step 为正时，stop 应在 start 的右边，step 为负时，stop 应在start 的左边，否则得到空序列
	4) 在 [start,stop) 所标识的区间之内以 step 步进取得相应元素

实际动作相当于（对应后边的简单实现来看）：

	1) step 为正时，m 的边界为 [0,len(m))；step 为负时，m 的边界为 [-1,-1-len(m))
	2) 如果 start 或 stop 为 None，根据 step 不同将其替换为 m 的边界，比如 start==None and step>0，将 start 替换为 0
	3) 如果 start 或 step 的值超出 m 的索引范围，将其限制为 m 的边界
	4) 在 [start,stop) 范围内查找相应元素：step 为正时，从左向右找元素；step 为负时，从右向左找元素
##作为左值进行赋值
	array = [0, 1, 2, 3, 4, 5]
	array[2:4] = [8,8,8]
	print(array)
	# [0, 1, 8, 8, 4, 5]
对于list有几点需要说明和注意：

1. 右侧对象必须是iterable。
2. step 不为 1 时，= 两侧区间的大小必须相同，新元素将替换旧元素；
step 为 1 时，即切片标识一段从左向右的连续区间，两侧区间大小可以不同，新区间将替换旧区间。
3. 切片标识的区间为空时，将在 start 位置之前插入新区间，此规则不与 规则2 冲突：此时如果 step 不是 1，新区间的大小则必须是 0。

例子如下：

	array = [0, 1, 2, 3, 4, 5]
	array[2:0] = [9.9, 8.8]
	print(array)
	# [0, 1, 9.9, 8.8, 2, 3, 4, 5]
##自定义类型
自定义类型如想使用下标运算符[]，需酌情定义以下几个函数：

	__getitem__(self, key)
		m[k]  =>  m.__getitem__(k)
	
	__setitem__(self, key, value)
		m[k]=v  =>  m.__setitem__(k,v)
	
	__delitem__(self,key)
		del m[k]  =>  m.__delitem__(k)
此时[ ]内可接受任何类型参数，[ ]内的切片表达式将自动形成slice实例，比如：

	[:]     =>  slice(None,None,None)
	[1:3:2] =>  slice(1,3,2)
##附字符串的部分切片实现
```
# m是原字符串, 返回切片后的字符串副本
def slice_L(m, start, stop, step):
    if step>0:
        # 将start和stop都调整为正数
        if start==None:
            start=0
        if stop==None:
            stop=len(m)
        # 如果start太左, 限制为0
        if start<=-len(m):
            start=0
        elif -len(m)<start<0:
            start+=len(m)
        # 如果stop太右, 限制为len(m)
        if stop>len(m):
            stop=len(m)
        elif -len(m)<stop<0:
            stop+=len(m)
        # start太右或stop太左的情况
        if start>=stop:
            return ''

        s=''
        for i in range(start,stop,step):
            s+=m[i]
        return s

    if step<0:
        if start==None:
            start=-1
        if stop==None:
            stop=-1-len(m)
        if start>=len(m):
            start=-1
        elif 0<=start<len(m):
            start-=len(m)
        if stop<-len(m):
            stop=-1-len(m)
        elif 0<=stop<len(m):
            stop-=len(m)
        if start<=stop:
            return ''
        s=''
        for i in range(start,stop,step):
            s+=m[i]
        return s
```