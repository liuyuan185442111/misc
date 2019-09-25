JDBC：Java database connectivity。
##一个静态查询的例子
```java
import java.sql.*;

public class test
{
	static String driver = "com.mysql.jdbc.Driver";
	static String url = "jdbc:mysql://10.108.27.48/thunder";
	static String username = "thor";
	static String password = "password";
	static String sql = "SELECT * FROM user";

	public static void main(String[] args)
	{
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		try
		{
			Class.forName(driver);
			System.out.println("驱动程序加载成功.");
			conn = DriverManager.getConnection(url, username, password);
			System.out.println("数据库连接成功.");
			stmt = conn.createStatement();
			System.out.println("语句建立成功.");
			rs = stmt.executeQuery(sql);
			System.out.println("获取结果成功.");
			while (rs.next())
			{
				String name = rs.getString("username");
				System.out.println(name);
			}
		} catch (Exception e)
		{
			e.printStackTrace();
		} finally
		{
			try
			{
				if (rs != null)
					rs.close();
				if (stmt != null)
					stmt.close();
				if (conn != null)
					conn.close();
			} catch (Exception e)
			{
				e.printStackTrace();
			}
		}
	}
}
```
##eclipse安装支持MySQL的JDBC驱动
下载MySQL支持JDBC的驱动程序，地址为http://dev.mysql.com/downloads/connector/j/。
打开下载得到的压缩包（mysql-connector-java-5.1.34.zip），将其中的Java包（mysql-connector-java-5.1.34-bin.jar），复制到某个目录下，以备加载驱动程序时使用。
将下载得到的MySQL驱动程序包（mysql-connector-java-5.1.34-bin.jar）添加到工程的Build path中。菜单Project->Properties，然后选择Java Build Path选项->Libraries选项卡，选中J2EE 1.3 Libraries，然后点击右方Add External JARs按钮。
##一些说明
分三部分：不含参数的静态查询、含有参数的静态查询、获取元数据。
主要涉及四个类（或接口）：DriverManager、Connection、Statement、ResultSet。
另外，PreparedStatement是Statement的派生类，SQLException是异常类。
|类|描述|
|:-|:-|
|DriverManager|负责加载各种不同驱动程序，并根据不同的请求，向调用者返回相应的数据库连接。|
|Connection（接口）|数据库连接，负责与数据库进行通信，SQL执行以及事务处理都是在某个特定Connection环境下进行的。可以产生用以执行SQL的Statement对象。|
|Statement（接口）|用来执行不含参数的静态SQL查询和更新，并返回执行结果。|
|ResultSet（接口）|用来获得SQL查询结果。|
|PreparedStatement（接口）|用来执行包含参数的动态SQL查询和更新。|
|SQLException|代表在数据库连接的建立、关闭或SQL语句的执行过程中发生了异常。|