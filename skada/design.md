战斗统计系统分为三个模块，数据收集、数据处理、数据展示。
数据处理模块负责数据的存储、计算、导入导出等，它的数据来源是收集模块，
处理过的数据提供给展示模块使用，它暴露一系列的方法，供另外两个模块调用。
数据收集模块负责收集数据并处理成处理模块需要的格式。
数据展示模块从处理模块获得数据并进行展示。
当前目录下的文件都属于数据处理模块。

## 数据处理模块文件说明
strict.lua：检测未声明便使用的全局变量的工具，这个文件不是必需的，但若使用，应于其他文件之前加载

common.lua：一些基础工具
port.lua：查询必要信息的接口，移植时需要修改该文件
json.lua：将变量转换为json格式，pure_json.lua是原版
export.lua：数据导入导出
base.lua：主要的数据结构和提供数据来源的函数

common.lua需要在除strict.lua之前加载，base.lua需要在port.lua、export.lua之后加载，
json.lua需要在export.lua之前加载，common.lua提供全局的skada变量，
由于在base.lua中定义了几个全局的变量，它们需要port.lua中的nowtime()函数和
export.lua中的import_allbattle()函数和import_allbattle_json()函数，
import_allbattle_json()函数需要json.lua中的decode()函数。
后面的几个文件提供的都是函数，且没有被全局变量直接调用，所以顺序无关紧要。

death.lua：死亡统计
damage.lua：伤害类信息的统计
heal.lua：治疗类信息的统计

test.lua：简单测试

## skada
数据处理模块提供的接口都置于skada表中，现做说明：
类型：配置参数为0，函数为1，其他为2
用途：模块内部使用为0，为数据收集模块为1，为数据展示模块为2
| 成员 | 说明 | 属性(类型/用途) |
|:--|:--|:--|
**common.lua** |
num2str | 将一个数字转换为字符串 | 10
per2str | 将一个百分数转换为字符串 | 10
seconds2str | 将秒数转换为几时几分几秒的字符串 | 10
dump | 将一个变量转换成字符串形式 | 10
clone_array | 返回一个数组的简单拷贝（浅拷贝） | 10
clone_table | 返回一个map的简单拷贝（浅拷贝） | 10
trans_table | 将一个map转换为数组（浅拷贝） | 10
reverse_array | 翻转一个数组 | 10
queue | 实现了一个队列 | 20
**port.lua** |
getcampinfo | 获取与host的相对阵营 | 10
isplayer | 判断是否是玩家 | 10
is_self_or_teammate | 判断是否是自己或队友 | 10
nowtime | 获取当前时间（秒） | 10
isbaoji | 判断是否是暴击 | 10
getroleinfo | 获取指定角色的名字和职业 | 10
nullname | 某些时候获取角色名字失败时返回该值 | 10
getroleinfo2  | 获取指定角色的名字和职业，如果找不到该角色则返回nullname和0 | 10
getpawnname | 获取指定角色或npc的名字 | 10
getskillname | 获取指定技能或buff的名字 | 10
getrolemaxhp | 获取指定角色的最大血量 | 10
getrivalinfo | 获取对方的等级、类型等信息 | 10
savedata | 保存数据 | 10
loaddata | 加载数据 | 10
MAX_BATTLES | 最多保留战斗的场数 | 00
MAX_DEATH_ACTIVITIES | 生前记录的最大数目 | 00
**export.lua** |
export_allbattle | 以lua格式导出allbattle | 10
import_allbattle | 导入lua格式的allbattle | 10
export_allbattle_json | 以json格式导出allbattle | 10
import_allbattle_json | 导入json格式的allbattle | 10
export_battle | 导出指定battle | 10
**base.lua** |
onlogin | 玩家登录时调用 | 11
begin_battle | 开始一场战斗时调用 | 11
finish_battle | 结束一场战斗时调用 | 11
add_damage_or_heal | 如果在一场战斗中，产生一条伤害或治疗时调用 | 11
protect_battles | 将指定战斗置为保护状态 | 12
protect_a_battle | 将指定战斗置为保护状态 | 12
unprotect_a_battle | 将指定战斗置为非保护状态 | 12
rm_a_battle | 删除一场战斗 | 12
rm_all_battles | 尝试删除所有战斗 | 12
**death.lua** |
add_death_activity | 为当前战斗添加一条血量增减事件 | 10
finish_death_record | 整理当前战斗的死亡记录，在结束当前战斗时调用 | 10
cal_death | 整理指定战斗的死亡记录 | 12
**damage.lua** |
cal_fsd | 整理指定战斗的友方造成伤害记录 | 12
其他伤害记录... |
cal_curr_damage | 整理当前战斗的友方/敌方的造成/受到伤害记录 | 10
**heal.lua** |
cal_weheal | 整理指定战斗的我方治疗记录 | 12
cal_heheal | 整理指定战斗的敌方治疗记录 | 12
cal_curr_heal | 整理当前战斗的我方和敌方治疗记录 | 10
**json.lua** |
encode | 将指定变量编码为json格式 | 10
decode | 将json字符串解码为lua变量 | 10

## 数据组织和计算
base.lua提供了三个全局变量，分别是currbattle，sumbattle，allbattle。
currbattle表示当前战斗，allbattle是一个数组，包含最近的若干场战斗，
当前战斗结束时，currbattle会放入allbattle，sumbattle是allbattle中所有战斗的总计。
数据处理模块的核心便是一场战斗的数据组织，它的主要成员在newbattle函数中说明了，
但还有一些细节需要说明，这里以友方造成伤害为例。
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

由于skillset需要按照伤害量排序并展示，但中间的计算过程中以tid分组比较容易计算，所以调用cal_fsd_curr时，
会生成一个skillsort_NS成员，它是skillset的一个拷贝，并以伤害量从大到小进行了排序。这个成员不会存盘。
同理也会生成targetsort_NS。

以上是当前战斗的计算过程，对于从磁盘加载的历史战斗，是没有skillsort_NS的，因为它是skillsort的一个冗余，没必要存盘。
在cal_fsd_old时会重新生成skillsort_NS和targetsort_NS。对于sumbattle则更复杂一些，因为sumbattle不存盘，
cal_fsd_sum会将历史战斗的fsd_summary进行合并，然后生成skillsort_NS和targetsort_NS。
cal_fsd函数根据请求的是currbattle，sumbattle，还是其他历史战斗调用相应的函数。

fsd_sort1将fsd_summary拷贝后按伤害量进行排序，fsd_sort2将fsd_summary拷贝后按每秒伤害进行排序，
它们也不存盘，因为它们是fsd_summary的冗余，在cal_fsd函数中会重新进行计算，在有需要的时候。
```

## 数据收集模块
比较简单，不再介绍。

## 数据展示模块
数据展示模块与具体界面联系比较紧密，代码不在这里，简单介绍下框架。
界面大体上可以分为3个部分：
1. 主体界面，用来显示当前选中的战斗、模式下的详细统计数据
2. 悬浮窗界面，根据鼠标所在的条目显示不同的内容
3. 设置界面
主体界面较为复杂，要支持两种前进方式，后退，在任意界面中更改战斗和模式。
悬浮窗界面和主体界面联系紧密，应统一考虑和设计。设置界面相对独立和简单，便不介绍了。

### 主体界面的数据结构
主体界面的展示空间是一个窗口，而且会有多个，每个窗口都包含：
widget 一个窗口绑定一个widget
frame 标识当前所在的界面，frame是具体界面的封装，下面详细介绍
stack frame的栈，用于返回时查找上一界面的frame，进入下一级界面时会将当前frame入栈
battle 标识是哪场战斗，一级界面转到二级界面时对其进行赋值
id 标识三级界面转到四级界面时点击的条目
subid 标识四级界面转到五级界面时点击的条目

### 悬浮窗界面的数据结构
悬浮窗只有一个，被命名为全局的变量ToolTip，它包含：
winid 当前与之绑定的窗口的id
pos 触发悬浮窗时的位置
触发悬浮窗时，通过winid能找到具体的窗口，就能找到battle,id等参数，在加上pos参数便能找到所有需要的数据。

### frame
frame是窗口中单个界面的封装，将所有的frame组织成一个树形结构，frame主要包括刷新当前界面的逻辑、所有子节点指针、悬浮窗指针，
打开窗口时将根frame交给窗口，以后进入下一级界面时查找当前frame的子节点，每个frame都包含：

index
一个字符串，标识该节点在树中的位置。
根节点/一级界面的index为空字符串，二级界面的index的长度是1，三级界面的index的长度是2，依此类推。
设某节点的index是"xyz"，其每个子节点，如果通过右键点击进入，其index为"xyz2"；
如果左键点击"xyz"节点的任何条目进入相同的子节点，子节点的index为"xyz1"；
如果点击"xyz"节点不同条目进入不同的子节点，点击位置0条目进入的子节点的index为"xyza"，
点击位置1条目进入的子节点的index为"xyzb"，依此类推。

left
指示左键点击时的子节点，如无则为nil。

right
指示右键点击时的子节点，如无则为nil。

next
next是一个数组，默认为空，同样指示左键点击时的子节点，在left为nil时有效。
从二级界面进入三级界面时，点击不同的位置进入的三级界面不同（这里的不同指的是update函数的逻辑有很大差别，
所以用不同的frame表示），这时候便不再使用left，而是将下一级的frame放到next里，根据点击位置的不同选择不同的下一级frame。
当然也可以将下一级的不同界面都由一个frame来实现，将点击位置传给下一级frame的update函数，让其根据点击位置刷出不一样的界面来。

tips
tips是一个数组，默认为空，指示触发哪个悬浮窗，后面详细介绍。

update
函数，根据参数刷出界面来。

comeon
函数，点击进入下一级界面时通过点击位置为窗口的battle，id，subid进行赋值，
这些变量将在update函数里被使用，同时它会阻拦不正确的位置参数。

### tip
封装悬浮窗的刷新逻辑。
一个frame可能只有一个tip，意为不同条目的显示内容大致相同，但在二级界面里，不同条目的显示内容大不相同，
所以每个条目都将触发不同的tip。
用frame的tips数组来表示这些不同情况，具体是如果所有条目的tip都相同，将其放在tips[-1]中，
如果不同条目的tip不同，将条目位置作为tips的索引放入tips中。触发悬浮窗时优先检测tips[-1]是否存在，
如果存在便使用tips[-1]的tip，如果不存在则用当前位置作为tips的索引去寻找tip。

index
一个字符串，标识自身是哪个frame或frame条目的tip。
例如："1a"界面的所有条目tip都相同，这个tip的index就是"1a-"；"1b"界面的每个条目有不同的tip，
其位置0的tip的index就是"1aa"，位置1的tip的index就是"1ab"，依此类推。

updatetip
函数，刷新悬浮窗的逻辑。

### 驱动
通过一个定时器，周期调用每个窗口的frame的update函数和当前frame的tip的updatetip函数来进行界面的刷新。
刷新界面时尽量做到如果自上次以来数据并无变化便不做什么了。
没有使用数据处理模块数据更新时通知数据展示模块的驱动方式，一来想使这些模块尽量解耦，二来想简单一些。
用户鼠标点击时也会立即进行一次刷新。

## 潜在问题
1. 阵营信息有可能会获取不到，现在是直接判为友方。因为添加伤害记录时会将获取不到tid的记录丢弃，
所以获取不到阵营信息的只能是其他玩家（怪物能从xid获取到tid就一定能得到阵营信息），一般情况下其他玩家就是被认定为友方，
所以暂时问题不大。
2. 少数情况下目标的名字可能会获取不到，现在是显示为空。这个问题已经通过getroleinfo2解决了大部分，
但还有个别情况下有问题，有时间再解决吧。
3. 四级界面中得到的id是进入三级界面时点击的pos，也就是说如果三级界面中条目顺序发生变化，
四级界面（当前界面）中显示的内容就会跟着改变。这只在当前战斗中可能会有问题，因为已结束的战斗中各条目的顺序已经确定不再改变。

