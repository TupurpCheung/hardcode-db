#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "db-constant.h"
#include "db-struct.h"
#include "db-btree.h"


/*
* 初始化InputBuffer,用于读取用户输入
 */
InputBuffer* new_input_buffer() {
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	input_buffer->buffer = NULL;
	input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;

	return input_buffer;
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
 * [print_prompt 打印提示信息]
 */
void print_prompt() { 
	printf("sql-db > ");
}
/**
 * 打印常量
 * 
 */
void print_constants() {
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

//打印缩进
void indent(uint32_t level) {
   uint32_t i;
    for (i = 0; i < level; i++) {
        printf("  ");
    }
}
/**
 * 
 * 打印树
 */
void print_tree(Pager *pager, uint32_t page_num, uint32_t indentation_level) {
    uint32_t  i;
    uint32_t  num_keys;
    uint32_t  child;
    void     *node = get_page(pager, page_num);

    switch (get_node_type(node)) {
    case NODE_LEAF:
        num_keys = *leaf_node_num_cells(node);
        indent(indentation_level);
        printf("- leaf node (size %d)\n", num_keys);
        for (i = 0; i < num_keys; i++) {
            indent(indentation_level + 1);
            printf("- %d\n", *leaf_node_key(node, i));
        }
        break;
    case NODE_INTERNAL:
        num_keys = *internal_node_num_keys(node);
        indent(indentation_level);
        printf("- internal node (size %d)\n", num_keys);
        printf("- internal node (page_num %d)\n", page_num);
        printf("\n");
        for (i = 0; i < num_keys; i++) {
            child = *internal_node_child(node, i);
            printf("  - leaf node (page_num %d)\n", *internal_node_child(node, i));
            print_tree(pager, child, indentation_level + 1);

            indent(indentation_level + 1);
            printf("- leaf node (max_key %d)\n", *internal_node_key(node, i));
            printf("\n");
        }

		
        child = *internal_node_right_child(node);
        printf("  - Rleaf node (page_num %d)\n", child);
        print_tree(pager, child, indentation_level + 1);
        break;
    }
}
/**
 * 打印记录
 * 
 * @param row [description]
 */
void print_row(Row* row) {
	printf("(%d, %s, %s) \n",row->id,row->username,row->email);
}

