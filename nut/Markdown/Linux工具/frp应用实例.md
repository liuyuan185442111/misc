frp 是一个用于内网穿透的反向代理应用，这是一个非常优秀的开源项目，开发者是 [fatedier](https://blog.fatedier.com/about/)。
##server端配置
```
# frps.ini
[common]
bind_port = 7000
token = passwd

# 可选 通过浏览器查看frp的状态以及代理统计信息展示
dashboard_port = 7500
dashboard_user = admin
dashboard_pwd = passwd

# 可选 点对点内网穿透使用
bind_udp_port = 7001
```
	./frps -c frps.ini
##client端配置
先展示3种基础用法：
```
# frpc.ini
[common]
server_addr = xx.xx.xx.xx
server_port = 7000
token = passwd
# 可选 启动的代理 默认全部启动
start = everything,msts,http_proxy

# 每个代理均可按需配置use_encryption和use_compression

# Everything提供的HTTP Server
[everything]
type = tcp
# local_ip也可配置成其他提供服务的服务器的ip地址, 如果是本机提供
# 该服务, 则设置127.0.0.1, 以下服务均由本机提供, 故省略该配置项
local_ip = 127.0.0.1
local_port = 80
remote_port = 7080

# 远程桌面连接
[msts]
type = tcp
local_port = 3389
remote_port = 7389
use_compression = true

# HTTP代理
[http_proxy]
type = tcp
remote_port = 7100
plugin = http_proxy
plugin_http_user = abc
plugin_http_passwd = abc
use_encryption = true
use_compression = true
```
	frpc.exe -c frpc.ini

以上3种用法，本质上是将处于内网的C的端口 local_ip:p1 映射到处于公网的S的端口 p2，访问 S:p2，即是访问 C:local_ip:p1，这些流量都要通过S中转，但对于访问者都是透明的。

接下来是两种高端玩法
##安全地暴露内网服务
作者称该玩法为 stcp（secret tcp），访问者也需要运行另外一个 frpc，模式可用下图描述：

	frpc::local_ip:port1 ----- frps ----- visitor::port2
	访问本地的port2端口，相当于访问远端的frpc上的local_ip:port1端口
上节中的 frpc.ini 需要添加如下配置：
```
# frpc.ini
[secret_tcp]
type = stcp
sk = abcdefg
local_port = 80
```
visitor 的 frpc 需要做如下配置：
```
# frpc2.ini
[common]
server_addr = xx.xx.xx.xx
server_port = 7000
token = passwd

[secret_service]
role = visitor
type = stcp
# server_name和sk要和上面frpc.ini中一致
server_name = secret_tcp
sk = abcdefg
bind_port = 8000
```
##点对点内网穿透
>frp 提供了一种新的代理类型 xtcp 用于应对在希望传输大量数据且流量不经过服务器的场景。
>使用方式同 stcp 类似，需要在两边都部署上 frpc 用于建立直接的连接。
>目前处于开发的初级阶段，并不能穿透所有类型的 NAT 设备，所以穿透成功率较低。穿透失败时可以尝试 stcp 的方式。

frps 除正常配置外需要额外配置一个 udp 端口用于支持该类型的客户端，在上面 frps.ini 中已添加。frpc.ini 需要额外添加：
```
[p2p_service]
type = xtcp
sk = abcdefg
local_port = 80
```
在要访问这个服务的机器上启动另外一个 frpc，配置如下:
```
# frpc.ini
[common]
server_addr = xx.xx.xx.xx
server_port = 7000
token = passwd

[p2p_visitor]
type = xtcp
role = visitor
# server_name和sk要和上面frpc.ini中一致
server_name = p2p_service
sk = abcdefg
bind_port = 9000
```
##参考
frp还提供很多其他功能，如有需求，可查看：
[frp项目地址](https://github.com/fatedier/frp)
[frp中文文档](https://github.com/fatedier/frp/blob/master/README_zh.md)
frps_full.ini
frpc_full.ini