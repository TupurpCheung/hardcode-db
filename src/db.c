#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
/*用户名长度*/
#define COLUMN_USERNAME_SIZE 32
/**用户邮箱长度*/
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
/*页大小，4096字节*/
const uint32_t PAGE_SIZE = 4096;
#define size_of_attribute(Struct,Attribute) sizeof(((Struct*)0)->Attribute)

/**
 * 命令行输入流的抽象
 */
typedef struct {	
	char* buffer;
	size_t buffer_length;
	ssize_t input_length;

} InputBuffer;



/**
 * 语句元操作验证结果
 * 合法的操作类型
 * 非法的操作类型
 * 
 */
typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

/**
 *数据库操作类型
 *插入、查询
 * 
 */
typedef enum {
	STATEMENT_INSERT,
	STATEMENT_SELECT
} StatementType;

/**
 * 语句预处理验证结果
 * 
 */
typedef enum {
	PREPARE_SUCCESS,
	PREPARE_SYNTAX_ERROR,
	PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

/**
 *语句执行结果
 * 
 */
typedef enum {
	EXECUTE_SUCCESS,
	EXECUTE_TABLE_FULL
	
} ExecuteResult;

/**
 *一条记录
 * 
 */
typedef struct {
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE];
	char email[COLUMN_EMAIL_SIZE];	
} Row;

/**
 *表
 * 
 */
typedef struct {
	uint32_t num_rows;
	void* pages[TABLE_MAX_PAGES];
} Table;

/**
 * id属性的大小，也可以直接写死32
 */
const uint32_t ID_SIZE = size_of_attribute(Row,id);
/**
 * 用户名属性的大小，也可以直接写死32
 */
const uint32_t USERNAME_SIZE = size_of_attribute(Row,username);
/**
 * 邮箱属性的大小，也可以直接写死255
 */
const uint32_t EMAIL_SIZE = size_of_attribute(Row,email);

/**
 * 一条记录占用的内存大小
 * 
 */
const uint32_t ROW_SIZE = size_of_attribute(Row,id) + size_of_attribute(Row,username) + size_of_attribute(Row,email);
/**
 * 
 * id偏移量,从0开始
 * 从此偏移量开始，移动ID_SIZE长度的位置，存放的是Id信息
 */
const uint32_t ID_OFFSET = 0;
/**
 * 
 * 用户名偏移量
 * 从此偏移量开始，移动USERNAME_SIZE长度的位置，存放的是 用户名 信息
 */
const uint32_t USERNAME_OFFSET = size_of_attribute(Row,id);
/**
 * 
 * 邮箱偏移量
 * 从此偏移量开始，移动EMAIL_SIZE长度的位置，存放的是 邮箱 信息
 */
const uint32_t EMAIL_OFFSET = size_of_attribute(Row,id) + size_of_attribute(Row,username);

/**
 * 每页（4096）可以存放的记录数
 * @param  PAGE_SIZE [页大小]
 * @param  ROW_SIZE [单条记录大小]
 */
#define ROW_PER_PAGE  (PAGE_SIZE / ROW_SIZE)
/**
 * 表（Table）最多可存放的记录数
 * ROW_PER_PAGE 每页可以存放的记录数
 * TABLE_MAX_PAGES 最多可以使用的页数
 */
#define TABLE_MAX_ROWS  ROW_PER_PAGE * TABLE_MAX_PAGES

/**
 * 初始化表，申请表所需的内存空间
 * 
 * @return [Table 表引用]
 */
Table* new_table(){
	Table* table = malloc(sizeof(Table));
	table->num_rows = 0 ;
	uint32_t i ;
	uint32_t size = TABLE_MAX_PAGES;
	for(i =0 ;i< size;i++) {
		table->pages[i] = NULL;
	}
	return table;
}

/**
 * 释放表占用的内存空间
 * 
 * @param table [表引用]
 */
void free_table(Table* table) {
	
	uint32_t i ;
	uint32_t size = TABLE_MAX_PAGES;
	
	for(i =0 ;i< size;i++) {
		free(table->pages[i]);
	}
	free(table);


}

/**
 * 获取某条记录所在的内存空间（槽）的起始地址
 * 
 * @param table   [description]
 * @param row_num [description]
 */
void* row_slot(Table* table,uint32_t row_num);

void* row_slot(Table* table, uint32_t row_num) {
	uint32_t page_num = row_num / ROW_PER_PAGE;
	void *page = table->pages[page_num];
	if ( page == NULL) {
		page = table->pages[page_num] = malloc(PAGE_SIZE);
	}
	int row_offset = (row_num % ROW_PER_PAGE);
	int byte_offset = row_offset * ROW_SIZE;	
	
	return page + byte_offset;
	
}

/**
 * 将记录序列化到内存空间中
 * 
 * @param source      [description]
 * @param destination [description]
 */
void serialize_row(Row* source,void* destination) {
	memcpy(destination + ID_OFFSET,&(source->id),ID_SIZE);
	memcpy(destination + USERNAME_OFFSET,&(source->username),USERNAME_SIZE);
	memcpy(destination + EMAIL_OFFSET,&(source->email),EMAIL_SIZE);

}

/**
 * 将内存空间中的数据反序列化为一条记录
 * 
 * @param source      [description]
 * @param destination [description]
 */
void deserialize_row(void* source, Row* destination) {
	memcpy(&(destination->id),source + ID_OFFSET,ID_SIZE);
	memcpy(&(destination->username),source + USERNAME_OFFSET,USERNAME_SIZE);
	memcpy(&(destination->email),source + EMAIL_OFFSET,EMAIL_SIZE);

}

/**
 *单次操作
 * 
 */
typedef struct {
	StatementType type;
	Row row_to_insert;
} Statement;


/*
* 初始化InputBuffer
 */
InputBuffer* new_input_buffer() {
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	input_buffer->buffer = NULL;
	input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;

	return input_buffer;
}

/**
 * [print_prompt 打印提示信息]
 */
void print_prompt() { 
	printf("sql-db > ");
}
/**
 * 打印记录
 * 
 * @param row [description]
 */
void print_row(Row* row) {
	printf("(%d, %s, %s) \n",row->id,row->username,row->email);
}

/**
 * 读命令行输入，使用getline函数
 * @param input_buffer [description]
 */
void read_input(InputBuffer* input_buffer) {
	ssize_t  bytes_read =  getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);
	if(bytes_read <=0 ){
		printf("Error reading input \n");
		exit(EXIT_FAILURE);
	}
	
	input_buffer->input_length = bytes_read - 1;
	input_buffer->buffer[bytes_read-1] = 0;

}
/**
 * 关闭输入流
 * @param input_buffer [description]
 */
void close_input_buffer(InputBuffer* input_buffer) {

	free(input_buffer->buffer);
	free(input_buffer);
}

/**
 * 元信息校验
 * 
 * @param  input_buffer [description]
 * @return              [description]
 */
MetaCommandResult do_meta_command(InputBuffer* input_buffer){
	if(strcmp(input_buffer->buffer,".exit") == 0 ) {
		close_input_buffer(input_buffer);
		exit(EXIT_SUCCESS);
	} else {
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

/**
 * sql 预处理，校验参数
 * 
 * @param  input_buffer [description]
 * @param  statement    [description]
 * @return              [description]
 */
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement) {
	if(strncmp(input_buffer->buffer,"insert",6) == 0 ) {
		statement->type = STATEMENT_INSERT;
		int args_assigned = sscanf(
		input_buffer->buffer,"insert %d %s %s", &(statement->row_to_insert.id),
			statement->row_to_insert.username,statement->row_to_insert.email
		);
		if(args_assigned < 3 ) {
			return PREPARE_SYNTAX_ERROR;
		}
		return PREPARE_SUCCESS;
	}
	if(strncmp(input_buffer->buffer,"select",6) == 0 ) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}
	
	return PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * 插入操作
 * @param  statement [description]
 * @param  table     [description]
 * @return           [description]
 */
ExecuteResult execute_insert(Statement* statement,Table* table) {
	if (table->num_rows >= TABLE_MAX_ROWS) {
		return EXECUTE_TABLE_FULL;
	}
	Row* row_to_insert = &(statement->row_to_insert);
	serialize_row(row_to_insert,row_slot(table,table->num_rows));
	table->num_rows += 1;

	return EXECUTE_SUCCESS;

}

/**
 * 查询
 * 
 * @param  statement [description]
 * @param  table     [description]
 * @return           [description]
 */
ExecuteResult execute_select(Statement* statement,Table* table) {
	Row row;
	uint32_t size = table->num_rows;
	uint32_t i;
	for(i = 0 ;i < size; i++) {
		deserialize_row(row_slot(table,i),&row);
		print_row(&row);
	}
	return EXECUTE_SUCCESS;
}

/**
 * 执行sql
 * @param  statement [description]
 * @param  table     [description]
 * @return           [description]
 */
ExecuteResult execute_statement(Statement* statement,Table* table) {
	switch (statement->type) {
		case (STATEMENT_INSERT):
		  return execute_insert(statement,table);
		case (STATEMENT_SELECT):
		  return execute_select(statement,table);
	}
}

/**
 * 程序入口
 */
int main(int argc,char* argv[]){
	Table* table = new_table();
	InputBuffer* input_buffer = new_input_buffer();
	while(true){
		print_prompt();
		read_input(input_buffer);
		if(input_buffer->buffer[0] == '.') {
			switch (do_meta_command(input_buffer)) {
				case (META_COMMAND_SUCCESS):
				  continue;
				case (META_COMMAND_UNRECOGNIZED_COMMAND):
				  printf("Unrcognized command '%s' \n",input_buffer->buffer);
				  continue;
			}
		}
	
		Statement statement;
		switch (prepare_statement(input_buffer,&statement)) {
			case (PREPARE_SUCCESS):
			 break;
			case (PREPARE_SYNTAX_ERROR):
			  printf("Syntax error. Could not parse statement.\n");
			case (PREPARE_UNRECOGNIZED_STATEMENT):
			  printf("unrecognized keyword at start of '%s' .\n",input_buffer->buffer);
			  continue;
		}
		
		switch (execute_statement(&statement,table)) {
			case (EXECUTE_SUCCESS):
			  printf("Executed.\n");
			  break;
			case (EXECUTE_TABLE_FULL):
			  printf("Error: Table full.\n");
			  break;
		}

	}
	return 0;
}
