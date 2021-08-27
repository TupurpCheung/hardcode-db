#include "db-constant.h"
#include "db-btree.h"
#include <stdint.h>



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