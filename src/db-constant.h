#include <stdint.h>

/*页大小，4096字节,即4Kb*/
extern const uint32_t PAGE_SIZE;

/**
 * 
 * id偏移量,从0开始
 * 从此偏移量开始，移动ID_SIZE长度的位置，存放的是Id信息
 */
extern const uint32_t ID_OFFSET ;
/**
 * id属性的大小，也可以直接写死4
 */
extern const uint32_t ID_SIZE;

/**
 * 
 * 用户名偏移量
 * 从此偏移量开始，移动USERNAME_SIZE长度的位置，存放的是 用户名 信息
 */
extern const uint32_t USERNAME_OFFSET ;

/**
 * 用户名属性的大小，也可以直接写死32
 */
extern const uint32_t USERNAME_SIZE ;


/**
 * 
 * 邮箱偏移量
 * 从此偏移量开始，移动EMAIL_SIZE长度的位置，存放的是 邮箱 信息
 */
extern const uint32_t EMAIL_OFFSET ;

/**
 * 邮箱属性的大小，也可以直接写死255
 */
extern const uint32_t EMAIL_SIZE ;

/**
 * 一条记录占用的内存大小255+32+4=291
 * 
 */
extern const uint32_t ROW_SIZE ;





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
extern const uint32_t NODE_TYPE_OFFSET ;
extern const uint32_t NODE_TYPE_SIZE ;


//根节点的大小和偏移量
extern const uint32_t IS_ROOT_OFFSET ;
extern const uint32_t IS_ROOT_SIZE ;


//父节点指针的大小和偏移量
extern const uint32_t PARENT_POINTER_OFFSET ;
extern const uint32_t PARENT_POINTER_SIZE ;

extern const uint8_t COMMON_NODE_HEADER_SIZE ;

//子节点个数的大小和偏移量
extern const uint32_t LEAF_NODE_NUM_CELLS_OFFSET ;
extern const uint32_t LEAF_NODE_NUM_CELLS_SIZE ;


//叶子节点头部布局的大小，10个字节
extern const uint32_t LEAF_NODE_HEADER_SIZE ;

/**
* 叶子节点的主体布局
*/
//一个叶子节点的主键的key大小
extern const uint32_t LEAF_NODE_KEY_OFFSET;
extern const uint32_t LEAF_NODE_KEY_SIZE;


//一个叶子节点的数据大小
extern const uint32_t LEAF_NODE_VALUE_OFFSET;
extern const uint32_t LEAF_NODE_VALUE_SIZE ;


//一个叶子节点的大小 = 数据+主键
extern const uint32_t LEAF_NODE_CELL_SIZE;

//一页（4k=4094字节）可用于存储节点数据的空间=4094字节-页头节点
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;

//一页最多可以存放多少个叶子节点
extern const uint32_t LEAF_NODE_MAX_CELLS ;