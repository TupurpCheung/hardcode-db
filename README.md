##                                                一个硬编码的数据库



### 一：参考

https://cstack.github.io/db_tutorial/



### 二：具体信息

#### 2.1、一个硬编码的数据库，假设存在如下表：

| column   | type         |
| :------- | :----------- |
| id       | integer      |
| username | varchar(32)  |
| email    | varchar(255) |



#### 2.2、可以进行新增和查询操作

+ 新增 

  `insert id username email`

+ 查询

  `select`



#### 2.3、数据持久化



#### 2.4、游标



#### 2.5、B+tree 结构



### 三：内部逻辑

+ `insert` 和 `select` 元（meta）操作校验
+ `insert` 参数预处理校验
+ 执行语句
+ 序列化反序列化

