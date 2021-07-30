##                                                一个硬编码的数据库



### 一：参考

https://cstack.github.io/db_tutorial/



### 二：具体信息

#### 2.1、一个硬编码的数据库，假设存在如下表：

| column   | type         | 占用空间 |
| :------- | :----------- | -------- |
| id       | integer      | 4字节    |
| username | varchar(32)  | 32字节   |
| email    | varchar(255) | 255字节  |



#### 2.2、可以进行新增和查询操作

+ 新增 

  `insert id username email`

+ 查询

  `select`



#### 2.3、数据持久化

+ 运行

  `./db mydb.db`

+ 新增

  ```
  insert 1 soc sock@example.com
  insert 2 cat cat@animal.com
  ```

+ 查询

  `select`

+ 退出并持久化

  `.exit`

+ 验证

  使用`vim`编辑,使用`:`进入底线命令模式,输入命令`%!xxd`

  

#### 2.4、游标

数据嵌套结构

+ 记录（Row）

  大小为 id->int（4） + username->char（32） + email->char（255）  =  `291`字节。

+ 页（Pager）

  页设定大小为`4096`字节，则一页最多可存取 `4096/291）= 14`。

+ 表（Table）

  表设定最多可存取`100`页。

+ 游标（Cursor），支持顺序读取

  从表头依次读到表尾，判断条件为当前读取记录条数是否等于`140`。



#### 2.5、B+tree 结构



### 三：内部逻辑

+ `insert` 和 `select` 元（meta）操作校验
+ `insert` 参数预处理校验
+ 执行语句
+ 序列化反序列化

