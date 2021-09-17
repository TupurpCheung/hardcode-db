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
  + 有多少个子节点（num_keys），4个字节
  + 最右侧（最大）子节点的所在的页（right_child_page_num），4个字节
  + 子节点所在的页（child_page_num），4个字节，左侧子节点包含记录的最大主键，4个字节，此布局不停的重复
+ 叶子节点
  + 公共头节点，6个字节
  + 当前页存放的记录个数（num_cells），4个字节
  + 下一个叶子节点所在的页（next_leaf）,4个字节
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

### 实现逻辑

+ 读取数据逻辑

  + 申请`Pager`所需的内存空间。
    + 从磁盘读取文件长度，计算出磁盘文件存放的页数，更新到`Pager->num_pages`。
    + 初始化`Pager->pages[0-表可以使用的最大页数]`为null
    + 获取指定页（不大于表可以使用的最大页数）时，若指定页在内存，则直接返回。若指定页不在内存且指定页不大于`num_pages`，则从磁盘文件读入。
  + 申请`Table`所需的内存空间
    + 初始化`Pager`，并设置`Table->Pager`
    + 若`Pager`加载的文件无数据，则初始化叶子节点为根节点

  + 申请`Cursor`所需的内存空间
    + 设置`Cursor->table`，设置`Cursor->page_num`，设置`Cursor->cell_num=0`。即将游标的位置放在第一页的第一个数据前面。
    + 游标一页一页，一个记录一个记录的顺序读取下去

+ 插入逻辑

