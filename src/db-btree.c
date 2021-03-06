#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "db-constant.h"
#include "db-enum.h"
#include "db-struct.h"
#include "db-io.h"
#include "db-btree.h"

/**
 * 根节点的操作
 * 
 */
//判断节点是否是根节点
bool is_node_root(void *page) {
   uint8_t value = *((uint8_t *) (page + IS_ROOT_OFFSET));
   return (bool) value;
}
//设置节点是否根节点
void set_node_root(void *page,bool is_root) {
    uint8_t value = is_root;
	*((uint8_t *) (page + IS_ROOT_OFFSET)) = value;
}
//创建根节点
//参数table 表
//参数right_child_page_num，最右侧子节点的页码
void create_new_root(Table *table,uint32_t right_child_page_num) {
  uint32_t left_child_max_key;
  //获取原来的根节点，稍后数据会被拷贝到新申请的左节点
  void *root = get_page(table->pager,table->root_page_num);
  //获取右节点
  //void *right_child = get_page(table->pager,right_child_page_num);
  //申请新的页空间，稍后用来存放左节点
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void *left_child = get_page(table->pager,left_child_page_num);

  //将旧的根节点数据拷贝到左节点
  memcpy(left_child,root,PAGE_SIZE);
  set_node_root(left_child,false);

  //更新旧的根节点为内部节点
  initialize_internal_node(root);
  set_node_root(root,true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root,0) = left_child_page_num;

  //左节点的最大主键值
  left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;

  *internal_node_right_child(root) = right_child_page_num;
}



/**
 *  节点（node）实际上就是页（page）
 *  节点的通用操作
 */
//获取节点类型
NodeType get_node_type(void *page) {
	 uint8_t value = *((uint8_t *)(page + NODE_TYPE_OFFSET));
	 return (NodeType) value;
}
//设置节点类型
void set_node_type(void *page,NodeType type) {
	uint8_t value = type;
	*((uint8_t *)(page + NODE_TYPE_OFFSET)) = value;
}
//获取节点存储记录的最大主键，对于内部节点，最大键始终是其右键。对于叶节点，它是最大索引处的键
uint32_t get_node_max_key(void *page) {
    switch (get_node_type(page)) {
    case NODE_INTERNAL:
        return *internal_node_key(page, *internal_node_num_keys(page) - 1);
    case NODE_LEAF:
        return *leaf_node_key(page, *leaf_node_num_cells(page) - 1);
    }
}



/**
 *内部节点的操作
 * 
 */
//获取内部节点有多少个叶子节点
uint32_t* internal_node_num_keys(void *page) {
	return page + INTERNAL_NODE_NUM_KEYS_OFFSET;
}
//获取内部节点的最右边（最大）子节点所在的页
uint32_t* internal_node_right_child(void  *page) {
	return page + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}
//获取第 cell_num 个子节点所在的页
uint32_t* internal_node_cell(void *page,uint32_t cell_num) {
	return page + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}
//获取第 child_num（child_num >=0） 个叶子节点所在的页
uint32_t* internal_node_child(void *page,uint32_t child_num) {
	uint32_t num_keys = *internal_node_num_keys(page);
    if (child_num > num_keys) {
        printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
        exit(EXIT_FAILURE);
    } else if (child_num == num_keys) {
        return internal_node_right_child(page);
    }

    return internal_node_cell(page, child_num);
}
//获取第 child_num 个子节点包含记录的最大主键值的句柄
uint32_t* internal_node_key(void *page,uint32_t key_num) {
	return internal_node_cell(page, key_num) + INTERNAL_NODE_CHILD_SIZE;
}
//初始化内部节点
void initialize_internal_node(void *page) {
    set_node_type(page, NODE_INTERNAL);
    set_node_root(page, false);
    *internal_node_num_keys(page) = 0;
}
//在内部节点中查找指定记录，先找到叶子节点，再从叶子节点中查找数据
Cursor *internal_node_find(Table *table,uint32_t page_num,uint32_t key) {
	//获取内部节点的指针
	void *page = get_page(table->pager,page_num);
	//查询当前内部节点有多少叶子节点
	uint32_t num_keys = *internal_node_num_keys(page);

	uint32_t child_page_num;
	uint32_t *child_page;

	uint32_t min_index = 0;
	uint32_t max_index = num_keys;

	//假设一种情况，共有11个子节点，即从0到10，要查找的值在节点5上
	//则二分查找的左侧节点，右侧节点，查找节点的情形如下，注意，退出条件为min = max
	//* min  * max  *  middle
	//* 0 * 10 * 5  这个时候，按照上帝视角，我们已经查到所在的叶子节点了，但不满足退出条件，还要继续，max = middle = 5
	//* 0 * 5 * 2  继续 min = middle + 1 = 3
	//* 3 * 5 * 4  继续 min  = middle + 1 = 5
	// max = min  = 5 退出，查得要找的key在节点5上
	while(min_index != max_index) {
		//二分查找
		uint32_t index = (min_index + max_index) / 2;
		//二分查找到的中间的值，internal_node_key方法得到的是指针，需要*号取值
		uint32_t key_to_right = *internal_node_key(page,index);

		//中间的值大于等于目标值，则数据在左边，缩小二分查找的最大值
		if(key_to_right >= key) {
			max_index = index;
		}else{
			//反之，增大二分查找的最小值
			min_index = index + 1;
		}
	}

	//获得子节点所在的页号
	child_page_num = *internal_node_child(page,min_index);
	//获取子节点的页
	child_page = get_page(table->pager,child_page_num);
	switch (get_node_type(child_page)) {
		case NODE_LEAF:
			return leaf_node_find(table,child_page_num,key);
		case NODE_INTERNAL:
			return internal_node_find(table,child_page_num,key);	
	}

}






/**
 *叶子节点的操作
 * 
 */
//获取存放节点中子节点个数，内存的起始位置指针
uint32_t* leaf_node_num_cells(void* page) {
  return page + LEAF_NODE_NUM_CELLS_OFFSET;
}

//获取节点下指定子节点内存起始位置
void* leaf_node_cell(void* page, uint32_t cell_num) {
  return page + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}
//获取节点下指定子节点的key（主键）内存起始位置
uint32_t* leaf_node_key(void* page, uint32_t cell_num) {
  return leaf_node_cell(page, cell_num);
}
//获取节点下指定子节点的数据（行记录）内存起始位置
void* leaf_node_value(void* page, uint32_t cell_num) {
  return leaf_node_cell(page, cell_num) + LEAF_NODE_KEY_SIZE;
}
//初始化节点，子节点个数为0
void initialize_leaf_node(void* page) { 
	set_node_type(page,NODE_LEAF);
	//默认不是根节点
	set_node_root(page, false);
	//默认记录数为0
	*leaf_node_num_cells(page) = 0; 
	//叶子节点初始化时，默认后继兄弟叶子节点的page_num为0
	*leaf_node_next_leaf(page) = 0;
}

//插入数据
void leaf_node_insert(Cursor* cursor, uint32_t key,Row *row) {
	//获取记录所在的页
	void* page = get_page(cursor->table->pager,cursor->page_num);

	//计算该页存放了多少个记录
	uint32_t num_cells = *leaf_node_num_cells(page);

    //当前页存放子节点已满
	if (num_cells >= LEAF_NODE_MAX_CELLS) {
		//切分叶子节点
		leaf_node_split_and_insert(cursor,key,row);
		return;
	}

    //继续在当前叶子节点插入，参见leaf_node_find 方法里的注释，可以解释为什么在当前cursor前面插入
	if (cursor->cell_num < num_cells) {
    //将数据后移
		uint32_t i ;
		for (i = num_cells; i > cursor->cell_num; --i) {
      //destin-- 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针。
      //source-- 指向要复制的数据源，类型强制转换为 void* 指针。
      //n-- 要被复制的字节数。
      //索引从0开始，所以有10个数据的时候，最大的cell_num  = 9
			memcpy(leaf_node_cell(page,i),leaf_node_cell(page,i-1),LEAF_NODE_CELL_SIZE);			
		}
	}

	//更新页中子节点数量 加1
	*(leaf_node_num_cells(page)) += 1;
	//更新节点的主键
	*(leaf_node_key(page,cursor->cell_num)) = key;
  	//序列化记录到内存
	serialize_row(row, leaf_node_value(page, cursor->cell_num));

}

Cursor* leaf_node_find(Table *table,uint32_t page_num, uint32_t key) {
  //子节点数量计数从0开始
	uint32_t min_index = 0;
	uint32_t one_past_max_index;
	void *page = get_page(table->pager,page_num);
	uint32_t num_cells = *leaf_node_num_cells(page);
	Cursor *cursor = (Cursor *) malloc(sizeof(Cursor));

	cursor->table = table;
	cursor->page_num = page_num;

	//二分查找
	//最大值=当前页存放的子记录数
	one_past_max_index = num_cells;

	//假设待查找的主键在当前页存在，则返回对应的cursor即可

	//假设当前叶子节点已有7条数据，即0-6，且要查找的key不存在,小于page[3],大于page[2]
	// * min * max * mid
	// * 0 * 7 * 3  此时 下标3处记录的值的主键 大于 形参key，则 max = 3,mid = （0 + 3）/ 2 = 1
	// * 0 * 3 * 1  此时 下标1处记录的值的主键 小于 形参key，则 min = 2,mid = （2 + 3）/ 2 = 2
	// * 2 * 3 * 2  此时 下标2处记录的值的主键 小于 形参key，则 min = 3
	// min = max  =3 ，退出，所以待查找的主键在当前页不存在时，返回的是比传入主键大的记录Cursor
	// 即要求随后插入这条新记录时，要插入在返回cursor的前面
	// 
	// 
	// 当要查找的key不存在，且大于此节点所有记录，则cursor->cell_num = 14
	while(one_past_max_index != min_index) {
		//二分
		uint32_t index = (min_index + one_past_max_index) / 2;
		//二分后查到的记录主键
		uint32_t key_at_index = *leaf_node_key(page,index);	

		//查找到记录
		if(key == key_at_index) {
			cursor->cell_num = index;
			return cursor;
		}

		//二分查找到的记录大于要查询的记录，调整最大值=二分值，否则调整最小值=二分值
		if(key < key_at_index) {
			one_past_max_index = index;
		}else{
			min_index = index + 1;
		}

	}

	//
	cursor->cell_num = min_index;
	return cursor;

}

//下一个叶子节点所在页的指针
uint32_t *leaf_node_next_leaf(void *page) {
	return page + LEAF_NODE_NEXT_LEAF_OFFSET;
}







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
  	//指定文件中共有多少页
  	pager->num_pages = (file_length / PAGE_SIZE);
  	//使用B+tree后，文件大小是页对齐的，也就是一定是4K的整倍数
  	if (file_length % PAGE_SIZE != 0) {
  		printf("Db file is not a whole number of pages. Corrupt file.\n");
  		exit(EXIT_FAILURE);
  	}

    //指定表最大可以存放的的页数并初始化为null
		uint32_t i ;
		uint32_t size = TABLE_MAX_PAGES;
		for(i = 0 ;i< size;i++) {
			pager->pages[i] = NULL;
		}
		return pager;
}
/**
*从内存获取指定页，若不存在，则从文件加载，文件中若也没有该页数据，则仅申请空间
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
  //没有数据，初始化页码为0的页为叶子节点,并设置为根节点
  if (pager->num_pages == 0) {
  	void* root_node = get_page(pager,0);
  	//没有数据时，初始化的节点使用叶子节点
  	initialize_leaf_node(root_node);
      //设置为根节点
    set_node_root(root_node, true);
  }

  return table;
}

/**
* 读取表头 
*/
Cursor* table_start(Table* table) {
	Cursor *cursor = table_find(table,0);
	void *page = get_page(table->pager,cursor->page_num);
	uint32_t num_cells = *leaf_node_num_cells(page);
	cursor->end_of_table = (num_cells == 0);
	return cursor;
} 

/**
 * 通过key在表中查找记录
 */
Cursor* table_find(Table* table,uint32_t key) {
	uint32_t root_page_num = table->root_page_num;
	void *root_node = get_page(table->pager,root_page_num);
	if(get_node_type(root_node) == NODE_LEAF) {
     return leaf_node_find(table, root_page_num, key);
	}else {
		return internal_node_find(table, root_page_num, key);
  }	

} 

/**
 * 关闭表
 */
void table_close(Table* table) {
  	Pager* pager = table->pager;
  	
	uint32_t i;
	uint32_t max = table->pager->num_pages;
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
	return leaf_node_value(page,cursor->cell_num);
	
}

void cursor_advance(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* page =get_page(cursor->table->pager,page_num);
  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(page))) {
    
    //当前节点已经迭代结束，遍历下一个节点
    uint32_t next_page_num = *leaf_node_next_leaf(page);
    if(0 == next_page_num){
    	//到达最后
    	cursor->end_of_table = true;
    }else{
    	cursor->page_num = next_page_num;
    	cursor->cell_num =0;
    }

  }
}
//获取当前数据库未被使用的页码（最大被使用页）
uint32_t get_unused_page_num(Pager *pager) {

  //页码从0开始，所以数据库现在一共有10页的时候，被使用的最大页为9，则未被使用的页码为10
  return pager->num_pages;
}

//分裂旧的叶子节点，并插入数
void leaf_node_split_and_insert(Cursor *cursor,uint32_t key,Row *value) {

    //创建新的叶子节点，并从旧的叶子节点复制一半数据到新节点
    //并更新两个节点的父节点（内部节点），如果没有，则创建
    

    //获取旧节点的句柄
    void *old_node = get_page(cursor->table->pager,cursor->page_num); 

    //计算未被使用的页号
    uint32_t new_page_num = get_unused_page_num(cursor->table->pager);

    //创建新页（叶子节点）
    void *new_node = get_page(cursor->table->pager,new_page_num);
    //初始化新节点
    initialize_leaf_node(new_node);
    //旧节点的兄弟节点变成新节点的兄弟，新节点变成旧节点的兄弟
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node); 
    *leaf_node_next_leaf(old_node) = new_page_num;

    //将旧节点的数据复制到新节点
    //一个叶子节点最多可存放14个节点，cell_num为 0-13
    int32_t i;

    //循环保证的是，值从大到小遍历，包括旧节点的全部14个节点（下标为0-13）和新插入的1个节点
    for ( i = LEAF_NODE_MAX_CELLS; i >= 0; i--){
      uint32_t index_within_node;
      void *destination;
      void *destination_node;

      //一半放右节点，一半放左节点      
      if(i >= LEAF_NODE_LEFT_SPLIT_COUNT){
      	// 7-14 共8个放新节点
        destination_node = new_node;
      }else{
      	// 0-6 共7个放旧节点
        destination_node = old_node;
      }

      //计算数据即将被存放的节点位置，以保证旧数据的有序性
      //此程序旧的叶子节点放7个，新的叶子节点放7个
      // 14 % 7 = 0
      // 13 % 7 = 6
      // 8 % 7 = 1
      // 7 % 7 = 0
      // 6 % 7 = 6
      // 5 % 7 = 5
      // 0 % 7 = 0  
      index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
      //数据被拷贝的目的地址
      destination = leaf_node_cell(destination_node,index_within_node);


      if((uint32_t)i == cursor->cell_num) {
        //新插入的值
      	serialize_row(value,leaf_node_value(destination_node, index_within_node));
      	*leaf_node_key(destination_node, index_within_node) = key;

      }else if((uint32_t)i > cursor->cell_num){
      	//这个逻辑里面，所有的数据都是大于新插入的值的,真实的位置被新插入的值向后挤了一个位置，所以下标需要减去1
      	//举个列子，假设cursor->cell_num = 11，则新插入的值是小于下标 11，12，13的
      	//循环从14开始，则旧节点是根本没有下标14的，所以需要减去1
      
        memcpy(destination,leaf_node_cell(old_node,i-1),LEAF_NODE_CELL_SIZE);
      }else{
      	//这个逻辑里，所有的值都是小于新插入的值的
      
        memcpy(destination,leaf_node_cell(old_node,i),LEAF_NODE_CELL_SIZE);
      }

      //更新两个节点的单元格（记录）计数
      *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
      *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;
    }

    if(is_node_root(old_node)) {
        //如果旧节点是根节点，则需要创建新的内部根节点，并将旧节点下挂
        return create_new_root(cursor->table,new_page_num);
    }else {
      printf("Need to implement update parent after split\n");
      exit(EXIT_FAILURE);
    }



}