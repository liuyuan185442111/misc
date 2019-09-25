用java产生一个验证码图片的代码如下：
```java
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.Random;
import javax.imageio.ImageIO;

public class IdentifyingImage
{
	static private int width = 80, height = 27;
	static	private Random random = new Random();
	static private Font font = new Font("Comic Sans", Font.BOLD|Font.ITALIC, 24);

	static private Color getRandomColor()
	{
		int r = random.nextInt(255);
		int g = random.nextInt(255);
		int b = random.nextInt(255);
		return new Color(r, g, b);
	}

	static public void getItem()
	{
		//创建一个图片缓冲区
		BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
		//获取图片处理对象
		Graphics graphics = image.getGraphics();
		//填充背景色
		graphics.setColor(getRandomColor());
		graphics.fillRect(0, 0, width, height);
		//设定边框颜色
		graphics.setColor(getRandomColor());
		graphics.drawRect(0, 0, width-1, height-1);
		//设置干扰线
		for (int i=0; i<10; ++i)
		{
			int x1 = random.nextInt(width-2);
			int y1 = random.nextInt(height-2);
			int x2 = random.nextInt(width-2);
			int y2 = random.nextInt(height-2);
			graphics.setColor(getRandomColor());
			graphics.drawLine(x1+1, y1+1, x2+1, y2+1);
		}
		//写入文字
		StringBuffer strbuf = new StringBuffer("");
		graphics.setFont(font);
		for (int i=0; i<4; ++i)
		{
			graphics.setColor(getRandomColor());
			int n = random.nextInt(10);
			strbuf.append(n);
			graphics.drawString(""+n, 1+20*i, height-4);
		}

		//输出文件
		File file = new File("D:\\123.gif");
		try
		{
			ImageIO.write(image, "GIF", file);
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		//释放资源
		graphics.dispose();
	}


	public static void main(String[] args)
	{
		IdentifyingImage.getItem();
	}
}
```
下面就做一个带验证码的简单登陆界面。

先将上面的类封装为一个JavaBean：
```java
package advance;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.Random;
import javax.imageio.ImageIO;

public class IdentifyingImage
{
    static private int width = 78, height = 27;
    static	private Random random = new Random();
    static private Font font = new Font("Comic Sans", Font.BOLD|Font.ITALIC, 24);

    static private Color getRandomColor()
    {
        int r = random.nextInt(255);
        int g = random.nextInt(255);
        int b = random.nextInt(255);
        return new Color(r, g, b);
    }

    static public BufferedImage getItem(String s)
    {
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
        Graphics graphics = image.getGraphics();
        graphics.setColor(getRandomColor());
        graphics.fillRect(0, 0, width, height);
        graphics.setColor(getRandomColor());
        graphics.drawRect(0, 0, width-1, height-1);
        for (int i=0; i<10; ++i)
        {
            int x1 = random.nextInt(width-2);
            int y1 = random.nextInt(height-2);
            int x2 = random.nextInt(width-2);
            int y2 = random.nextInt(height-2);
            graphics.setColor(getRandomColor());
            graphics.drawLine(x1+1, y1+1, x2+1, y2+1);
        }
        graphics.setFont(font);
        for (int i=0; i<4; ++i)
        {
            graphics.setColor(getRandomColor());
            int n = random.nextInt(10);
            graphics.drawString(""+s.charAt(i), 2+20*i, height-4);
        }
        graphics.dispose();
        return image;
    }
}
```
然后编写将验证码传至客户端的Servlet，将验证码信息放到了session里，以备验证。
```java
package advance;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Random;
import javax.imageio.ImageIO;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import advance.IdentifyingImage;

public class GenImage extends HttpServlet
{
    public GenImage()
    {
        super();
    }
    public void destroy()
    {
        super.destroy();
    }
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException
    {
        String code="";
        Random rm = new Random();
        for(int i=0; i<4; ++i) code += rm.nextInt(10);
        HttpSession session = request.getSession();
        session.setAttribute("piccode",code);
        //禁止缓存
        response.setHeader("Prama","no-cache");
        response.setHeader("Cache-Control","no-cache");
        response.setDateHeader("Expires",0);
        response.setContentType("image/gif");
        ImageIO.write(IdentifyingImage.getItem(code), "gif", response.getOutputStream());
        response.getOutputStream().close();
    }
    public void doPost(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException
    {
        doGet(request, response);
    }
    public void init() throws ServletException {}
}
```
web.xml配置
```xml
<servlet>
  <servlet-name>image</servlet-name>
  <servlet-class>advance.GenImage</servlet-class>
</servlet>
<servlet-mapping>
  <servlet-name>image</servlet-name>
  <url-pattern>/image</url-pattern>
</servlet-mapping>
```
登陆界面 login.html：
```jsp
<html>
    <meta http-equiv=Content-Type content="text/html;charset=utf-8">
    <script type="text/javascript">
    function reloadImage(t) {
    	t.src="image?flag="+Math.random();
    }
    </script>
<head>
    <title>login</title>
</head>
<body>
	<form action="check.jsp" method="post">
    <input type="text" name="checkcode">
    <img src="image" onclick="reloadImage(this)">
    <br><br>
	<input type="submit" value="登陆">
	</form>
</body>
</html>
```
验证页面 check.jsp，从session取得正确的验证码，从request参数中取得输入的验证码：
```java
<%@ page contentType="text/html;charset=UTF-8" pageEncoding="utf-8" %>
<html>
	<meta http-equiv=Content-Type content="text/html;charset=utf-8">
<head>
	<title>check</title>
</head>
<body>
	<%
	String checkcode=request.getParameter("checkcode");
	String piccode=new String();
	Object o = session.getAttribute("piccode");
	if(o != null) piccode=o.toString();
	else piccode="木有取到";
	%>
	你输入的验证码是: <%=checkcode%><br>
	正确的验证码是: <%=piccode%>
</body>
</html>
```