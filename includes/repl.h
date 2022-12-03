#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define print_prompt() printf("db > ");
#define true 1
#define false 0

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define ID_SIZE size_of_attribute(Row, id)
#define USERNAME_SIZE size_of_attribute(Row, username)
#define EMAIL_SIZE size_of_attribute(Row, email)
#define ID_OFFSET (0)
#define USERNAME_OFFSET ((ID_OFFSET) + (ID_SIZE))
#define EMAIL_OFFSET ((USERNAME_OFFSET) + (USERNAME_SIZE))
#define ROW_SIZE ((ID_SIZE) + (USERNAME_SIZE) + (EMAIL_SIZE))

#define PAGE_SIZE 4096 // check if it should be 4096
#define TABLE_MAX_PAGES 100
#define ROWS_PER_PAGE ((PAGE_SIZE) / (ROW_SIZE))

#define TABLE_MAX_ROWS ((ROWS_PER_PAGE) * (TABLE_MAX_PAGES))
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

// btree constants
// node header for all nodes
#define NODE_TYPE_SIZE sizeof(uint8_t)
#define NODE_TYPE_OFFSET 0
#define IS_ROOT_SIZE sizeof(uint8_t)
#define IS_ROOT_OFFSET (NODE_TYPE_SIZE)
#define PARENT_POINTER_SIZE sizeof(uint32_t)
#define PARENT_POINTER_OFFSET (IS_ROOT_OFFSET) + (IS_ROOT_SIZE)
#define COMMON_NODE_HEADER_OFFSET 0
#define COMMON_NODE_HEADER_SIZE ((NODE_TYPE_SIZE) + (IS_ROOT_SIZE) + (PARENT_POINTER_SIZE))

// leaf node
// has common header + leaf node header (num rows field)
#define LEAF_NODE_NUM_CELLS_SIZE sizeof(uint32_t)
#define LEAF_NODE_NUM_CELLS_OFFSET (COMMON_NODE_HEADER_SIZE) + (COMMON_NODE_HEADER_OFFSET)
#define LEAF_NODE_HEADER_OFFSET 0
#define LEAF_NODE_HEADER_SIZE (COMMON_NODE_HEADER_SIZE) + (LEAF_NODE_NUM_CELLS_SIZE)

// leaf node body
#define LEAF_NODE_KEY_SIZE sizeof(uint32_t)
#define LEAF_NODE_KEY_OFFSET 0
#define LEAF_NODE_VALUE_SIZE ROW_SIZE
#define LEAF_NODE_VALUE_OFFSET ((LEAF_NODE_KEY_OFFSET) + (LEAF_NODE_KEY_SIZE))
#define LEAF_NODE_CELL_SIZE ((LEAF_NODE_KEY_SIZE) + (LEAF_NODE_VALUE_SIZE))
#define LEAF_NODE_SPACE_FOR_CELLS ((PAGE_SIZE) - (LEAF_NODE_HEADER_SIZE))
#define LEAF_NODE_MAX_CELLS ((LEAF_NODE_SPACE_FOR_CELLS) / (LEAF_NODE_CELL_SIZE))

#define LEAF_NODE_LEFT_SPLIT_COUNT ((LEAF_NODE_MAX_CELLS + 1) / 2)
#define LEAF_NODE_RIGHT_SPLIT_COUNT (LEAF_NODE_MAX_CELLS - LEAF_NODE_LEFT_SPLIT_COUNT + 1)

// internal node header
#define INTERNAL_NODE_NUM_KEYS_SIZE sizeof(uint32_t)
#define INTERNAL_NODE_NUM_KEYS_OFFSET (COMMON_NODE_HEADER_SIZE)
#define INTERNAL_NODE_RIGHT_CHILD_SIZE sizeof(uint32_t )
#define INTERNAL_NODE_RIGHT_CHILD_OFFSET ((INTERNAL_NODE_NUM_KEYS_OFFSET) + (INTERNAL_NODE_NUM_KEYS_SIZE))
#define INTERNAL_NODE_HEAD_SIZE ((COMMON_NODE_HEADER_SIZE) + (INTERNAL_NODE_RIGHT_CHILD_SIZE) + (INTERNAL_NODE_NUM_KEYS_SIZE))

#define INTERNAL_NODE_KEY_SIZE sizeof(uint32_t)
#define INTERNAL_NODE_CHILD_SIZE sizeof(uint32_t )
#define INTERNAL_NODE_CELL_SIZE ((INTERNAL_NODE_CHILD_SIZE) + (INTERNAL_NODE_KEY_SIZE))
typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
} PrepareCommandResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_DUPLICATE_KEY
} ExecuteResult;

typedef enum
{
    NODE_INTERNAL,
    NODE_LEAF
} NodeType;

typedef struct
{
    u_int32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct
{
    Row rows[PAGE_SIZE / ROW_SIZE];
} Page;

typedef struct
{
    int file_descriptor;
    uint32_t file_length;

    void *pages[TABLE_MAX_PAGES];
    uint32_t num_pages;
} Pager;
typedef struct
{
    // uint32_t num_rows;
    uint32_t root_page_num;
    Pager *pager;
} Table;
typedef struct
{
    StatementType type;
    Row row;
} Statement;
typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef struct
{
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; // Indicates a position one past the last element
} Cursor;
