#ifndef _DBBTREE_H_
#define _DBBTREE_H_
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "db-enum.h"
#include "db-struct.h"



/**
 * 根节点的操作
 * 
 */
//判断节点是否是根节点
bool is_node_root(void *page);
//设置节点是否根节点
void set_node_root(void *page,bool is_root);



/**
 *  节点（node）实际上就是页（page）
 *  节点的通用操作
 */
//获取节点类型
NodeType get_node_type(void *page);
//设置节点类型
void set_node_type(void *page,NodeType type);
//获取节点存储记录的最大主键，对于内部节点，最大键始终是其右键。对于叶节点，它是最大索引处的键
uint32_t get_node_max_key(void *page);


/**
 *内部节点的操作
 * 
 */
//获取内部节点有多少个叶子节点
uint32_t* internal_node_num_keys(void *page);
//获取主键最右边叶子节点地址
uint32_t* internal_node_right_child(void  *page);
//获取第 cell_num 个叶子节点
uint32_t* internal_node_cell(void *page,uint32_t cell_num);
//获取第 child_num 个叶子节点
uint32_t* internal_node_child(void *page,uint32_t child_num);
//获取第 child_num 个叶子节点中记录的最大主键
uint32_t* internal_node_key(void *page,uint32_t key_num);
//初始化内部节点
void initialize_internal_node(void *page);



/**
 *叶子节点的操作
 * 
 */
//当前页共存放了多少个记录
uint32_t* leaf_node_num_cells(void* page);
//获取指定记录
void* leaf_node_cell(void* page, uint32_t cell_num);
//获取指定记录的key
uint32_t* leaf_node_key(void* page, uint32_t cell_num);
//获取指定记录值
void* leaf_node_value(void* page, uint32_t cell_num);
//初始化叶子节点
void initialize_leaf_node(void* page) ;
//插入数据（记录）
void leaf_node_insert(Cursor* cursor, uint32_t key,Row* row);
//查找数据（记录）
Cursor* leaf_node_find(Table *table,uint32_t page_num, uint32_t key);





/**
 * 打开文件，初始化Pager，申请表所需的内存空间
 * 
 * @return [Table 表引用]
 */
Pager* pager_open(const char* filename);
/**
*从内存获取指定页，若不存在，则从文件加载
* @return pager.pages[page_num]
*/
void* get_page(Pager* pager, uint32_t page_num); 
/**
*刷新pager到硬盘
*/
void pager_flush(Pager* pager, uint32_t page_num);





//打开表
Table* table_open(const char* filename);
//定位到表头
Cursor* table_start(Table* table);
/**
 * 通过key在表中查找记录
 */
Cursor* table_find(Table* table,uint32_t key);
/**
 * 关闭表
 */
void table_close(Table* table);







void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);


#endif
