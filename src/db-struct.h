#ifndef _DBSTRUCT_H_
#define _DBSTRUCT_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "db-enum.h"


/*用户名长度*/
#define COLUMN_USERNAME_SIZE 32
/**用户邮箱长度*/
#define COLUMN_EMAIL_SIZE 255
/**单表最多可以有100页*/
#define TABLE_MAX_PAGES 100



/**
 * 命令行输入流的抽象
 */
typedef struct {	
	char* buffer;
	size_t buffer_length;
	ssize_t input_length;

} InputBuffer;

/**
 *一条记录
 * 
 */
typedef struct {
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE];
	char email[COLUMN_EMAIL_SIZE];	
} Row;


typedef struct {
	//文件名
  int file_descriptor;
  //文件长度
  uint32_t file_length;
  //文件当前有多少页
  uint32_t num_pages;
  //存放页数据的数组
  void* pages[TABLE_MAX_PAGES];
} Pager;


/**
 *表
 * 
 */
typedef struct {
	//根节点页的页码
	uint32_t root_page_num;
	Pager* pager;
} Table;


typedef struct {
	//表句柄
	Table* table;
	//当前是第几页
	uint32_t page_num;
	//页内的记录数量
	uint32_t cell_num;
	//是否是表尾
	bool end_of_table;
} Cursor;
//Cursor->Table->Pager->pages[page_nums]
//table_start(table_open(pager_open(filename)))
//
/**
 *单次操作
 * 
 */
typedef struct {
	StatementType type;
	Row row_to_insert;
} Statement;

#endif