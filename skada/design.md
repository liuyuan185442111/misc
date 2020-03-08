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

## 数据处理模块数据结构

## skada表
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
MODE_SIZE | 战斗统计条目的数量 | 00
**export.lua** |
export_allbattle | 导出allbattle | 10
import_allbattle | 导入allbattle | 10
export_battle | 导出指定battle | 10
**base.lua** |
allbattle | 是一个数组，包含最近的若干场战斗，这是一个全局变量，并不包含在skada中
currbattle | 当前战斗，这是一个全局变量，并不包含在skada中
sumbattle | allbattle的总计，这是一个全局变量，并不包含在skada中
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

## 数据展示模块框架

## 计划
week1:
退出战斗加一个弹窗
为每个界面添加日志 统计重刷次数
review frame代码
reviewc++代码 日志里Skada->skada
退出战斗的方式再思考一下
标题显示问题
让getskillname支持npc技能
实现悬浮窗

周二
实现直选功能

周三窗口管理
添加功能全局开关和窗口管理功能

周四完善item
实现item的图标, 进度条功能

周五写文档
把ui框架整理成文档
把数据处理模块数据结构整理出来

week0:
3个专题
金融市场公开课
公开课

week2:
误伤
友方受到伤害

week3:
敌方造成伤害
敌方受到伤害

week4-6:
buff统计

week7:
治疗统计

week8:
法力
怒气

week9:
打断
驱散

week10-12:
威胁值
设置功能

week1314:
原子操作
环形队列
协议生成优化
自动删库
