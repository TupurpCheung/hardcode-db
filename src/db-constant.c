#include <stdint.h>
#include "db-constant.h"


/*页大小，4096字节,即4Kb*/
const uint32_t PAGE_SIZE = 4096;

/**
 * 
 * id偏移量,从0开始
 * 从此偏移量开始，移动ID_SIZE长度的位置，存放的是Id信息
 */
const uint32_t ID_OFFSET = 0;
/**
 * id属性的大小，也可以直接写死4
 */
const uint32_t ID_SIZE = 4;

/**
 * 
 * 用户名偏移量
 * 从此偏移量开始，移动USERNAME_SIZE长度的位置，存放的是 用户名 信息
 */
const uint32_t USERNAME_OFFSET = 4;

/**
 * 用户名属性的大小，也可以直接写死32
 */
const uint32_t USERNAME_SIZE = 32;


/**
 * 
 * 邮箱偏移量
 * 从此偏移量开始，移动EMAIL_SIZE长度的位置，存放的是 邮箱 信息
 */
const uint32_t EMAIL_OFFSET = 36;

/**
 * 邮箱属性的大小，也可以直接写死255
 */
const uint32_t EMAIL_SIZE = 255;

/**
 * 一条记录占用的内存大小255+32+4=291
 * 
 */
const uint32_t ROW_SIZE = 291;





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
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t NODE_TYPE_SIZE = 1;


//根节点的大小和偏移量
const uint32_t IS_ROOT_OFFSET = 1;
const uint32_t IS_ROOT_SIZE = 1;


//父节点指针的大小和偏移量
const uint32_t PARENT_POINTER_OFFSET = 2;
const uint32_t PARENT_POINTER_SIZE = 4;

const uint8_t COMMON_NODE_HEADER_SIZE = 6;

//子节点个数的大小和偏移量
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = 6;
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = 4;


//叶子节点头部布局的大小，10个字节
const uint32_t LEAF_NODE_HEADER_SIZE = 10;

/**
* 叶子节点的主体布局
*/
//一个叶子节点的主键的key大小
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_KEY_SIZE = 4;


//一个叶子节点的数据大小
const uint32_t LEAF_NODE_VALUE_OFFSET =  4;
const uint32_t LEAF_NODE_VALUE_SIZE = 291;


//一个叶子节点的大小 = 数据+主键
const uint32_t LEAF_NODE_CELL_SIZE = 295;

//一页（4k=4094字节）可用于存储节点数据的空间=4094字节-页头节点
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = 4096 - 10;

//一页最多可以存放多少个叶子节点
const uint32_t LEAF_NODE_MAX_CELLS = 4086 / 295;