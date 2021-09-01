#include <stdint.h>

/**
 * 页大小，4096字节,即4Kb
 * 一个页就是B+tree的一个节点
*/
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
 * 节点（node）的公共头布局
 * 内部节点和叶子节点都有这些页（节点）头
 */
//节点类型（分为内部节点和叶子节点）的大小和偏移量
extern const uint32_t NODE_TYPE_OFFSET ;
extern const uint32_t NODE_TYPE_SIZE ;


//是否是根节点的大小和偏移量
extern const uint32_t IS_ROOT_OFFSET ;
extern const uint32_t IS_ROOT_SIZE ;

//父节点指针的大小和偏移量
extern const uint32_t PARENT_POINTER_OFFSET ;
extern const uint32_t PARENT_POINTER_SIZE ;

//节点公共头的大小
extern const uint8_t COMMON_NODE_HEADER_SIZE ;


/**
 * 内部节点（internal node）的头布局
 *
 */
//子节点个数
extern const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET ;
extern const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE ;

//最右边节点的指针
extern const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET ;
extern const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE;

//内部节点头部布局的大小，公共头+内部节点特有头
extern const uint32_t INTERNAL_NODE_HEADER_SIZE ;

/**
* 内部节点的主体布局，由cell组成的数组
*/
//子节点指针
extern const uint32_t INTERNAL_NODE_CHILD_SIZE;
//左侧子节点中最大的key
extern const uint32_t INTERNAL_NODE_KEY_SIZE;
//cell大小
extern const uint32_t INTERNAL_NODE_CELL_SIZE;

/**
*  叶子节点（leaf node）的头部布局
*  子节点个数：4个字节，偏移量6个字节
*
**/
//子节点个数的大小和偏移量
extern const uint32_t LEAF_NODE_NUM_CELLS_OFFSET ;
extern const uint32_t LEAF_NODE_NUM_CELLS_SIZE ;


//叶子节点头部布局的大小，公共头+叶子节点特有头
extern const uint32_t LEAF_NODE_HEADER_SIZE ;

/**
* 叶子节点(leaf node)的主体布局
*/
//一个叶子节点的主键的(key)大小
extern const uint32_t LEAF_NODE_KEY_OFFSET;
extern const uint32_t LEAF_NODE_KEY_SIZE;


//一个叶子节点的数据(row)大小
extern const uint32_t LEAF_NODE_VALUE_OFFSET;
extern const uint32_t LEAF_NODE_VALUE_SIZE ;


//一个节点数据 = key+row
extern const uint32_t LEAF_NODE_CELL_SIZE;

//一页（4k=4094字节）可用于存储节点数据(key+row)的空间=4094字节-头节点大小
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;

//一个叶子节点最多可以存放多少个节点数据 key+row
extern const uint32_t LEAF_NODE_MAX_CELLS ;



/**
 * 叶子节点的分裂
 *
 * 为了保持树的平衡，我们在两个新节点之间均匀地分配 cell。
 * 如果一个叶子节点可以容纳 N 个 cell，那么在分裂的过程中，
 * 我们需要在两个节点之间分配 N+1 个 cell（N 个原 cell 加一个新的 cell）。
 * 如果 N+1 是奇数，我就选择左节点来获得一个以上的 cell。
 * 
 */
extern const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT;
extern const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT;