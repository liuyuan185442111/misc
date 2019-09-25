很简单的用户登录管理，设定登陆成功在session里写入super属性来存储权限信息，所以只有登陆成功后session里才会有super这个属性。
##FilterChain的逻辑
1、/login.html过滤器
这个过滤器一定要放在/*过滤器之前，否则会造成死循环。
```java
Object logined = session.getAttribute("super");
if (logined == null)
	forward to "/login.html";
else
	redirect to "/main.html";
```
2、/check.jsp过滤器
这个过滤器一定要放在/*过滤器之前，否则会造成无法验证登陆。
```java
Object logined = session.getAttribute("super");
if (logined == null)
	forward to "/check.html";
else
	redirect to "/main.html";
```
3、/*过滤器
```java
Object logined = session.getAttribute("super");
if (logined == null)
	redirect to "/login.html";
else
	chain.doFilter(request, response);
```
<br>1-2顺序可变，但一定要在3之前；3之后的过滤器顺序可变，但一定要在3之后。
##check.jsp
```java
<%@ page contentType="text/html;charset=UTF-8" pageEncoding="utf-8" %>
<html>
	<meta http-equiv=Content-Type content="text/html;charset=utf-8">
<head>
	<title>check</title>
</head>
<body>
	<%
	response.setHeader("progma","no-cache");
	response.setHeader("Cache-Control","no-cache");
	response.setDateHeader("Expires",0);
	String username = request.getParameter("username");
	String password = request.getParameter("password");
	if (username == null || password == null)
		response.sendRedirect(request.getContextPath()+"/login.html");
	else if (username.equals("admin") && password.equals("123")) {
		session.setAttribute("super","super");
		response.sendRedirect(request.getContextPath()+"/main.html");
	}
	else {
		response.setHeader("refresh","3;url=login.html");
	%>
	用户不存在或密码错误，3秒后自动返回登陆界面。<br>
	如果没有返回，请点击<a href="login.html">此处</a>。
	<%}%>
</body>
</html>
```
##login.html
```html
<html>
<meta http-equiv=Content-Type content="text/html;charset=utf-8">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="expires" content="0">
<head>
	<title>login</title>
	<script type="text/javascript">
	function Check() {
		with(document.info) {
			var uname = username.value;
			var psd = password.value;
			if(uname == null || uname == "") alert("用户名为空");
			else if (psd == null || psd == "") alert("密码为空");
			else submit();
		}
	}
	</script>
</head>
<body><br><br><br><br><center>
	<form name="info" action="check.jsp" method="post">
		<table>
			<tr><td colspan="2" align="center">用户登录</td></tr>
			<tr><td>登录名：</td><td><input type="text" name="username"></td></tr>
			<tr><td>密码：</td><td><input type="password" name="password"></td></tr>
		</table>
		<input type="button" value="登录" onclick="Check()">&nbsp;&nbsp;&nbsp;
		<input type="reset" value="重置">
	</form>
</center></body>
</html>
```
注意，check.jsp和login.html里需要设置页面不缓存。
check.jsp密码验证部分应当从数据库里取用户名、密码、权限信息等，这里为方便简化了。