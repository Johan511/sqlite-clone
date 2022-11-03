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

#define PAGE_SIZE 4088 // check if it should be 4096
#define TABLE_MAX_PAGES 100
#define ROWS_PER_PAGE ((PAGE_SIZE) / (ROW_SIZE))

#define TABLE_MAX_ROWS ((ROWS_PER_PAGE) * (TABLE_MAX_PAGES))
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

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
    EXECUTE_TABLE_FULL
} ExecuteResult;

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
    Page *pages[TABLE_MAX_PAGES];
} Pager;
typedef struct
{
    uint32_t num_rows;
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
    uint32_t row_num;
    bool end_of_table ; // Indicates a position one past the last element
} Cursor;
