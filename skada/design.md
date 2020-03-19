战斗统计系统分为三个模块，数据收集、数据处理、数据展示。
数据处理模块负责数据的存储、计算、导入导出等，它的数据来源是收集模块，处理过的数据提供给展示模块使用，它暴露一系列的方法，供另外两个模块调用。
数据收集模块负责收集数据并处理成处理模块需要的格式。
数据展示模块从处理模块获得数据并进行展示。

## 数据处理模块文件说明
strict.lua：检测未声明便使用的全局变量的工具
这个文件不是必需的。

common.lua：一些基础工具
port.lua：查询必要信息的接口，移植时需要修改该文件
export.lua：数据导入导出
base.lua：主要的数据结构和提供数据来源的函数
base.lua需要在common.lua，port.lua，export.lua之后加载，common.lua提供全局的skada变量，由于在base.lua中定义了几个全局的变量，它们需要port.lua中的nowtime()函数和export.lua中的import_allbattle()函数。
后面的几个文件提供的都是函数，且没有被全局变量直接调用，所以顺序无关紧要。

death.lua
死亡统计
damage.lua
伤害类信息的统计

test.lua
简单测试

## skada
数据处理模块提供的接口都置于skada表中，现将其做一下说明。
类型：配置参数为0，函数为1，其他为2
用处：模块内部使用为0，为数据收集模块为1，为数据展示模块为2
| 成员 | 说明 | 属性(类型/用处) |
|:--|:--|:--|
**common.lua** |
num2str | 将一个数字转换为字符串 | 10
per2str | 将一个百分数转换为字符串 | 10
seconds2str | 将秒数转换为几时几分几秒的字符串 | 10
dump | 将一个变量转换成字符串形式 | 10
clone_array | 返回一个数组的简单拷贝（浅拷贝） | 10
clone_table | 返回一个map的简单拷贝（浅拷贝） | 10
trans_array | 将一个map转换为数组（浅拷贝） | 10
trans_array_if | 将一个map中符合条件的元素放到数组中（浅拷贝） | 10
reverse_array | 翻转一个数组 | 10
queue | 实现了一个队列 | 20
**port.lua** |
getcampinfo | 获取与host的相对阵营 | 10
isplayer | 判断是否是玩家 | 10
is_self_or_teammate | 判断是否是自己或队友 | 10
isbaoji | 判断是否是暴击 | 10
getroleoccu | 获取指定角色的职业 | 10
getrolename | 获取指定角色的名字 | 10
getnpcname | 获取指定npc的名字 | 10
getskillname | 获取指定技能或buff的名字 | 10
getrolemaxhp | 获取指定角色的最大血量 | 10
getrivalinfo | 获取对方的等级、类型等信息 | 10
savedata | 保存数据 | 10
loaddata | 加载数据 | 10
MAX_BATTLES | 最多保留战斗的场数 | 00
MAX_DEATH_ACTIVITIES | 生前记录的最大数目 | 00
**export.lua** |
export_allbattle | 导出allbattle | 10
import_allbattle | 导入allbattle | 10
export_battle | 导出指定battle | 10
**base.lua** |
onlogin | 玩家登录时调用 | 11
begin_battle | 开始一场战斗时调用 | 11
finish_battle | 结束一场战斗时调用 | 11
add_damage_or_heal | 如果在一场战斗中，产生一条伤害或治疗时调用 | 11
protect_battles | 将指定战斗置为保护状态 | 12
rm_a_battle | 删除一场战斗 | 12
rm_all_battles | 尝试删除所有战斗 | 12
**death.lua** |
add_death_activity | 为当前战斗添加一条血量增减事件 | 10
finish_death_record | 整理当前战斗的死亡记录，在结束当前战斗时调用 | 10
cal_death | 整理指定战斗的死亡记录 | 12
**damage.lua** |
cal_fsd_curr | 整理当前战斗的友方造成伤害记录 | 10
cal_fsd | 整理指定战斗的友方造成伤害记录 | 12

## 一场战斗
base.lua提供了三个全局变量，分别是currbattle，sumbattle，allbattle，currbattle表示当前战斗，allbattle是一个数组，包含最近的若干场战斗，当前战斗结束时，currbattle会放入allbattle，sumbattle是allbattle中所有战斗的总计。
数据处理模块的核心便是一场战斗的数据组织，它的主要成员在newbattle函数中说明了，但有一些细节需要说明一下，这里以友方造成伤害为例。
```
total_wesend_damage=0,
friend_send_damage={}, --友方造成伤害
fsd_summary={}, --以攻击者tid分组
fsd_sort1={}, --以伤害量排序
fsd_sort2={}, --以每秒伤害排序

这几个成员负责“造成伤害界面”和“每秒伤害界面”的数据，新来的伤害记录如果属于友方造成的伤害，
便会将记录先放于friend_send_damage中，在调用cal_fsd_curr函数时，
会将friend_send_damage先做一个整理，然后合并到fsd_summary中。

fsd_summary以友方每个人的tid为索引，元素是一个table，其成员有：
id --tid
name --tid的名字
occu --tid的职业
damage --伤害量
damage_rate --每秒伤害量
damage_ratio --伤害量占友方造成的所有伤害的比例
firsttime --所有记录里最早的一条的时间
lasttime --所有记录里最晚的一条的时间
active_time --活跃时间，firsttime与lasttime之差
skillset --一个table，以技能id为索引，元素格式稍后说明
targetset --一个table，以目标tid为索引，元素格式稍后说明

skillset元素格式：
id --技能id
name --技能名字
damage --总伤害量
maxdmg --该技能造成的最高伤害
mindmg --该技能造成的最低伤害
avgdmg --该技能造成伤害的平均值
count --该技能造成伤害的次数
baoji --产生暴击的次数
ratio --该技能造成的伤害占该玩家造成总伤害的比例

targetset元素格式：
id --目标tid
name --目标名字
damage --对该目标造成的总伤害
ratio --对该目标造成的总伤害占该玩家造成总伤害的比例

由于skillset需要按照伤害量排序并展示，但中间的计算过程中skillset以tid分组比较容易计算，所以调用cal_fsd_curr时，
会生成一个skillsort_NS成员，它是skillset的一个拷贝，并以伤害量从大到小进行了排序。这个成员不会存盘。
同理也会生成targetsort_NS。

以上是当前战斗的计算过程，对于从磁盘加载的历史战斗，是没有skillsort_NS的，
在cal_fsd_old时会重新生成skillsort_NS和targetsort_NS。对于sumbattle更复杂一些，
因为sumbattle不存盘，cal_fsd_sum会将历史战斗的fsd_summary进行合并，然后生成skillsort_NS和targetsort_NS。
cal_fsd函数根据请求的是currbattle，sumbattle，还是其他历史战斗调用相应的函数。

fsd_sort1是将fsd_summary拷贝后按伤害量进行排序，fsd_sort2是将fsd_summary拷贝后按每秒伤害进行排序，
它们也不存盘，因为它们是fsd_summary的冗余，在cal_fsd函数中会重新进行计算。
```

## 数据展示模块
### 窗口
窗口可能有多个，每个窗口都包含：
widget 一个窗口绑定一个widget
frame 标识当前具体的界面
stack frame的栈，用于返回时查找frame，进入下一级界面时会将当前frame入栈
battle 标识是哪场战斗，一级界面转到二级界面时对其进行赋值
id 标识三级界面转到四级界面时点击的位置
subid 标识四级界面转到五级界面时点击的位置

### Tooltip
悬浮窗只有一个，它包含：
winid 当前与之绑定的窗口的id
pos 触发悬浮窗时的位置
触发悬浮窗时，通过winid能找到具体的窗口，就能找到battle,id等参数，在加上pos参数便能找到所有需要的数据。

### frame
frame是窗口中界面的封装。将所有的frame组织成一个树形结构，打开窗口时将根frame交给窗口，以后进入下一级界面时查找当前frame的子节点，每个frame都包含：
index
一个字符串，标识该节点在树中的位置。
根节点的index为空字符串。
设某节点的index是xxx，其每个子节点，如果通过右键点击进入，其index为xxx2；如果左键点击xxx节点的任何条目进入相同的子节点，子节点的index为xxx1；如果点击xxx节点不同条目进入不同的子节点，点击位置0条目进入的子节点的index为xxxa，点击位置1条目进入的子节点的index为xxxb，依此类推。

left
指示左键点击时的子节点，如无则为nil。

right
指示右键点击时的子节点，如无则为nil。

next
next是一个数组，默认为空，同样指示左键点击时的子节点，在left为nil时有效。
从二级界面进入三级界面时，点击不同的位置进入的三级界面不同（这里的不同指的是update函数的逻辑有很大差别），这时候便不再使用left，而是将下一级的frame放到next里，根据点击位置的不同选择不同的下一级frame。
当然也可以将下一级的不同界面都由一个frame来实现，将点击位置传给下一级frame的update函数，让其根据点击位置刷出不一样的界面来。

tips
tips是一个数组，默认为空，指示触发哪个悬浮窗。

update
函数，根据参数刷出界面来。

comeon
函数，点击进入下一级界面时通过点击位置为窗口的battle，id，subid进行赋值，这些变量将在update函数里被使用，同时它会阻拦不正确的位置参数。

### Tip
封装悬浮窗的刷新逻辑。
一个frame可能只有一个Tip，此时不同条目的Tip逻辑大概相同，但在二级界面里，不同条目的Tip逻辑并不相同，所以每个条目都将触发不同的Tip。
用frame的tips数组来指示这些不同情况，具体是如果所有条目的Tip都相同，将其放在tips['-']中，如果不同条目的Tip不同，将条目位置作为tips的索引放入tips中。触发悬浮窗时优先检测tips['-']是否存在，如果存在便使用tips['-']的Tip，如果不存在则用当前位置作为tips的索引去寻找Tip。

index
一个字符串，标识自身是哪个frame或frame条目的Tip。
比如1a界面的所有条目Tip都相同，这个Tip的index就是1a-；1b界面的每个条目有不同的Tip，其位置0的Tip的index就是1aa，位置1的Tip的index就是1ab，依此类推。

update
函数，刷新悬浮窗的逻辑。

### item池
每个窗口自己都有一个item池，创建item时，便已确定好了其归属于哪个窗口和自身位置，以后也不会改变。
Tooltip有一个item池，它的item没什么特殊的。

### 驱动
通过一个定时器，周期调用每个窗口的frame的update函数和Tooltip的update函数来进行界面的刷新。
刷新界面时尽量做到如果自上次以来数据并无变化便不做什么了。
没有使用数据处理模块数据更新时通知数据展示模块的驱动方式，一来想使这些模块尽量解耦，二来想简单一些。

## 计划
删除mode_size,写完伤害的核心代码先，然后转移到内网

week3:323
1敌方造成伤害核心代码
2敌方受到伤害核心代码
3敌方造成伤害UI代码
4敌方受到伤害UI代码

week4-5:330
buff统计

week6:413
已方治疗

week7:420
敌方治疗

week8:427
法力
怒气

week9:54
打断
驱散

week10:511
威胁值

week11:518
设置功能

待定:
协议生成优化
自动删库
原子操作
