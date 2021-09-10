#ifndef _DBIO_H_
#define _DBIO_H_

#include "db-struct.h"


InputBuffer* new_input_buffer();
void serialize_row(Row* source,void* destination);
void deserialize_row(void* source, Row* destination);
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer) ;
void print_prompt();
void print_constants();
//打印缩进
void indent(uint32_t level);
//打印树
void print_tree(Pager *pager, uint32_t page_num, uint32_t indentation_level);
void print_row(Row* row) ;

#endif