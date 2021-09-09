##                                                一个硬编码的数据库



### 一：参考

https://cstack.github.io/db_tutorial/

https://blog.japinli.top/db_tutorial_trans/

https://www.kancloud.cn/kancloud/theory-of-mysql-index/41846

https://www.wangdoc.com/clang/



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

+ 编译

  `make`

+ 运行

  `./db-engine.o mydb.db`

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

`一页（Page）的大小为4Kb，读取到内存中就是一个节点（Node）。`

+ 公共头节点
  + 节点类型（node_type），1个字节
  + 是否为根节点（is_root），1个字节
  + 父节点指针（parent_poiner），4个字节
+ 内部节点
  + 公共头节点，6个字节
  + 有多少个叶子节点（num_keys），4个字节
  + 最右侧（最大）叶子节点的指针（right_child_pointer），4个字节
  + 叶子节点X指针，4个字节，左侧叶子节点包含记录的最大主键，4个字节，此布局不停的重复
+ 叶子节点
  + 公共头节点，6个字节
  + 当前页存放的记录个数（num_cells），4个字节
  + 行记录主键（key），4个字节，行记录（数据），291个字节，此布局不停重复





### 三：源码结构

+ db-constant 

  定义了全局变量，主要是一些起始位置和偏移量

+ db-enum 

  操作、返回值的枚举

+ db-struct

  行记录、页、表、游标的结构定义

+ db-io

  获取用户输入，文件的磁盘读写

+ db-btree

  将页数据当作b+tree来处理

+ db-engine

  程序入口

