
#include <stdio.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include "db-constant.h"
#include "db-enum.h"
#include "db-struct.h"
#include "db-io.h"
#include "db-btree.h"




/**
 * 元信息校验
 * 
 * @param  input_buffer [description]
 * @return              [description]
 */
MetaCommandResult do_meta_command(InputBuffer* input_buffer,Table* table){
	if(strcmp(input_buffer->buffer,".exit") == 0 ) {
		close_input_buffer(input_buffer);
		table_close(table);
		exit(EXIT_SUCCESS);
	 } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
    	printf("Tree:\n");
    	print_tree(table->pager, 0, 0);
    	return META_COMMAND_SUCCESS;
  	} else if (strcmp(input_buffer->buffer, ".constants") == 0) {
   		printf("Constants:\n");
    	print_constants();
    	return META_COMMAND_SUCCESS;
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
	//
	void* page = get_page(table->pager,table->root_page_num);
	uint32_t num_cells = (*leaf_node_num_cells(page));
	
	Row* row_to_insert = &(statement->row_to_insert);
	uint32_t key_to_insert = row_to_insert->id;
	Cursor* cursor = table_find(table,key_to_insert);


	if(cursor->cell_num < num_cells) {
		//数据已存在
		uint32_t key_at_index = *leaf_node_key(page, cursor->cell_num);
		if (key_at_index == key_to_insert) {
            return EXECUTE_DUPLICATE_KEY;
        }
	}

	leaf_node_insert(cursor,row_to_insert->id,row_to_insert);
	

	free(cursor);

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
	
	Cursor* cursor = table_start(table);
	while(!(cursor->end_of_table)) {
		deserialize_row(cursor_value(cursor),&row);
		print_row(&row);
		cursor_advance(cursor);
	}
	free(cursor);
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
	 if (argc < 2) {
      printf("Must supply a database filename.\n");
      exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  Table* table = table_open(filename);

	InputBuffer* input_buffer = new_input_buffer();
	while(true){
		//读取输入
		print_prompt();
		read_input(input_buffer);
		//解析输入
		if(input_buffer->buffer[0] == '.') {
			switch (do_meta_command(input_buffer,table)) {
				case (META_COMMAND_SUCCESS):
				  continue;
				case (META_COMMAND_UNRECOGNIZED_COMMAND):
				  printf("Unrcognized command '%s' \n",input_buffer->buffer);
				  continue;
			}
		}
		//预处理
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
		//执行输入的命令
		switch (execute_statement(&statement,table)) {
			case (EXECUTE_SUCCESS):
			  printf("Executed.\n");
			  break;
			case EXECUTE_DUPLICATE_KEY:
              printf("Error: Duplicate key.\n");
              break;
			case (EXECUTE_TABLE_FULL):
			  printf("Error: Table full.\n");
			  break;
		}

	}
	return 0;
}
