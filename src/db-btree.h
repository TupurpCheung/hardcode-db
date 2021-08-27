#ifndef _DBBTREE_H_
#define _DBBTREE_H_
#include <stdio.h>
#include <stdint.h>
#include "db-struct.h"








//一个页page就是一个节点node
//当前页共存放了多少个节点
uint32_t* leaf_node_num_cells(void* page);
//获取指定节点
void* leaf_node_cell(void* page, uint32_t cell_num);
//获取指定节点的key
uint32_t* leaf_node_key(void* page, uint32_t cell_num);
//获取指定节点的值
void* leaf_node_value(void* page, uint32_t cell_num);
//初始化页（节点）
void initialize_leaf_node(void* page) ;
void leaf_node_insert(Cursor* cursor, uint32_t key,Row* row);






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
 * 读取表尾
 */
Cursor* table_end(Table* table);
/**
 * 关闭表
 */
void table_close(Table* table);







void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);


#endif
