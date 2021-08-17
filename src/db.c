#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

/*用户名长度*/
#define COLUMN_USERNAME_SIZE 32
/**用户邮箱长度*/
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
/*页大小，4096字节,即4Kb*/
const uint32_t PAGE_SIZE = 4096;
#define size_of_attribute(Struct,Attribute) sizeof(((Struct*)0)->Attribute)



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
 *b+tree的节点类型定义
 * 
 */
typedef enum {
	//内部节点
	NODE_INTERNAL,
	//叶子节点
	NODE_LEAF
} NodeType;

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
	Table* table;
	uint32_t page_num;
	uint32_t cell_num;
	bool end_of_table;
} Cursor;
//Cursor->Table->Pager->pages[page_nums]
//table_start(table_open(pager_open(filename)))

/**
 * id属性的大小，也可以直接写死4
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
 * 一条记录占用的内存大小255+32+4=291
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
*  叶子节点的头部布局
*  节点类型：1个字节，偏移量0
*  是否是根节点：1个字节，偏移量1个字节
*  父节点指针：4个字节，偏移量2个字节
*  子节点个数：4个字节，偏移量6个字节
*
* 	故叶子节点的头部信息共占用 10个字节
**/

//节点类型的大小和偏移量
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;

//根节点的大小和偏移量
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = sizeof(uint8_t);

//父节点指针的大小和偏移量
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = sizeof(uint8_t) + sizeof(uint8_t);
const uint8_t COMMON_NODE_HEADER_SIZE = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t);

//子节点个数的大小和偏移量
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t);

//叶子节点头部布局的大小，10个字节
const uint32_t LEAF_NODE_HEADER_SIZE = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t);

/**
* 叶子节点的主体布局
*/
//一个叶子节点的主键的key大小
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;

//一个叶子节点的数据大小
const uint32_t LEAF_NODE_VALUE_SIZE = 291;
const uint32_t LEAF_NODE_VALUE_OFFSET =    sizeof(uint32_t);

//一个叶子节点的大小 = 数据+主键
const uint32_t LEAF_NODE_CELL_SIZE = 295;

//一页（4k=4094字节）可用于存储节点数据的空间=4094字节-页头节点
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = 4096 - 10;

//一页最多可以存放多少个叶子节点
const uint32_t LEAF_NODE_MAX_CELLS = 4086 / 295;

//获取存放节点中子节点个数，内存的起始位置指针
uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

//获取节点下指定子节点的key（主键）内存起始位置
void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}
//获取节点下指定子节点的key（主键）内存起始位置
uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}
//获取节点下指定子节点的数据（行记录）内存起始位置
void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

//初始化节点，子节点个数为0
void initialize_leaf_node(void* node) { *leaf_node_num_cells(node) = 0; }



/**
 * 打开文件，初始化Pager，申请表所需的内存空间
 * 
 * @return [Table 表引用]
 */
Pager* pager_open(const char* filename) {
	//打开文件
  int fd = open(filename,
     	  O_RDWR | 	// Read/Write mode
     	      O_CREAT,	// Create file if it does not exist
     	  S_IWUSR |	// User write permission
     	      S_IRUSR	// User read permission
     	  );
	
	//文件不存在
  if (fd == -1) {
     printf("Unable to open file\n");
     exit(EXIT_FAILURE);
  }
		//获取文件长度，当SEEK_END时，文件偏移量=文件长度+offset
  	off_t file_length = lseek(fd, 0, SEEK_END);
		
		//初始胡Pager对象，主要其中有个数组用来存储每个Page在内存的指针
  	Pager* pager = malloc(sizeof(Pager));
  	//指定文件描述符
  	pager->file_descriptor = fd;
  	//指定文件长度
  	pager->file_length = file_length;
  	//指定共有多少页
  	pager->num_pages = (file_length / PAGE_SIZE)
  	//使用B+tree后，文件大小是页对齐的，也就是一定是4K的整倍数
  	if (file_length % PAGE_SIZE != 0) {
  		printf("Db file is not a whole number of pages. Corrupt file.\n");
  		exit(EXIT_FAILURE);
  	}
		uint32_t i ;
		uint32_t size = TABLE_MAX_PAGES;
		for(i =0 ;i< size;i++) {
			pager->pages[i] = NULL;
		}
		return pager;
}


/**
*从内存获取指定页，若不存在，则从文件加载
* @return pager.pages[page_num]
*/
void* get_page(Pager* pager, uint32_t page_num) {
	//不可以大于最大页100
  if (page_num > TABLE_MAX_PAGES) {
     printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
     	TABLE_MAX_PAGES);
     exit(EXIT_FAILURE);
  }

	//指定页不在内存，则申请空间，并从文件加载到内存
  if (pager->pages[page_num] == NULL) {
     // Cache miss. Allocate memory and load from file.
     //申请一页所需的空间4K
     void* page = malloc(PAGE_SIZE);
     //计算文件一共有多少页
     uint32_t num_pages = pager->file_length / PAGE_SIZE;

     // We might save a partial page at the end of the file
     //如果文件数据大小不是页的整数倍，则在上一步的基础上加1
     if (pager->file_length % PAGE_SIZE) {
         num_pages += 1;
     }
		//将指定的页加载到内存
     if (page_num <= num_pages) {
     		 //设置文件偏移量为 页码*4K
         lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
         //从文件偏移量处读4K字节
         ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
         //加载失败
         if (bytes_read == -1) {
     		 		printf("Error reading file: %d\n", errno);
     	    	exit(EXIT_FAILURE);
         }
     }
		 //将页内存空间的起始指针存入数组
     pager->pages[page_num] = page;
     
     if(page_num >= pager->num_pages) {
     		pager->num_pages = page_num + 1;	
     }
  }
	//指定页在内存，直接返回数据
  return pager->pages[page_num];
}
/**
*刷新pager到硬盘
*/
void pager_flush(Pager* pager, uint32_t page_num) {
  if (pager->pages[page_num] == NULL) {
     printf("Tried to flush null page\n");
     exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE,
     		 SEEK_SET);

  if (offset == -1) {
     printf("Error seeking: %d\n", errno);
     exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = write(
     pager->file_descriptor, pager->pages[page_num], PAGE_SIZE
     );

  if (bytes_written == -1) {
     printf("Error writing: %d\n", errno);
     exit(EXIT_FAILURE);
  }
}

/**
* 初始化Table
*/
Table* table_open(const char* filename) {
  Pager* pager = pager_open(filename);
  
  Table* table = malloc(sizeof(Table));
  table->pager = pager;
  table->root_page_num = 0;
  //没有数据，初始化页码为0的页为叶子节点
  if (pager->num_pages == 0) {
  	void* root_node = get_page(pager,0);
  	initialize_leaf_node(root_node);
  }

  return table;
}


Cursor* table_start(Table* table){
	Cursor* cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = table->root_page_num;
	cursor->cell_num = 0;
	void* root_node = get_page(table->pager,table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->end_of_table = (num_cells == 0);
	return cursor;
} 

Cursor* table_end(Table* table){
	Cursor* cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = table->root_page_num;
	void* root_node = get_page(table->pager,table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->cell_num = num_cells;
	cursor->end_of_table = true;
	return cursor;
} 

void table_close(Table* table) {
  	Pager* pager = table->pager;
  	
	uint32_t i;
	uint32_t max = table->num_pages;
  for ( i = 0; i < max; i++) {
     if (pager->pages[i] == NULL) {
         continue;
     }
     pager_flush(pager, i);
     free(pager->pages[i]);
     pager->pages[i] = NULL;
  }



  int result = close(pager->file_descriptor);
  if (result == -1) {
     printf("Error closing db file.\n");
     exit(EXIT_FAILURE);
  }
  for (i = 0; i < TABLE_MAX_PAGES; i++) {
     void* page = pager->pages[i];
     if (page) {
         free(page);
         pager->pages[i] = NULL;
     }
  }

  free(pager);
  free(table);
}



void* cursor_value(Cursor* cursor) {
	
	uint32_t page_num = cursor->page_num;
	void *page = get_page(cursor->table->pager, page_num);	
	return left_node_value(page,cursor->cell_num);
	
}

void cursor_advance(Cursor* cursor) {
	uint32_t page_num = cursor->page_num;
	void* node =get_page(cursor->table->pager,page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
    cursor->end_of_table = true;
  }
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
MetaCommandResult do_meta_command(InputBuffer* input_buffer,Table* table){
	if(strcmp(input_buffer->buffer,".exit") == 0 ) {
		close_input_buffer(input_buffer);
		table_close(table);
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
	Cursor* cursor = table_end(table);
	serialize_row(row_to_insert,cursor_value(cursor));
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
		print_prompt();
		read_input(input_buffer);
		if(input_buffer->buffer[0] == '.') {
			switch (do_meta_command(input_buffer,table)) {
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
