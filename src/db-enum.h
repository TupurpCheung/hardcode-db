#ifndef _DBENUM_H_

#define _DBENUM_H_
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

#endif