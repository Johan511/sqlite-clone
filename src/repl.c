#include "../includes/repl.h"

void serialize_row(Row *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, EMAIL_SIZE);
}

uint32_t *leaf_node_num_cells(void *node)
{
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void *leaf_node_cell(void *node, uint32_t cell_num)
{
    return node + LEAF_NODE_HEADER_OFFSET + LEAF_NODE_HEADER_SIZE + (cell_num * LEAF_NODE_CELL_SIZE);
}

uint32_t *leaf_node_key(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num);
}

void *leaf_node_value(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void *node)
{
    *leaf_node_num_cells(node) = 0;
}

void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

Page *get_page(Pager *pager, uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               +TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    if (pager->pages[page_num] == NULL)
    {
        Page *page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE)
        {
            // we need ceil of the division result not floor;
            num_pages++;
        }
        if (page_num <= num_pages)
        {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            if (bytes_read == -1)
            {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
        if (page_num >= pager->num_pages)
        {
            pager->num_pages = page_num + 1;
        }
    }
    return pager->pages[page_num];
}

void pager_flush(Pager *pager, uint32_t page_num)
{
    if (pager->pages[page_num] == NULL)
    {
        printf("Tried to flush NULL page\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    if (offset == -1)
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);
    if (bytes_written == -1)
    {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void db_close(Table *table)
{
    Pager *pager = table->pager;
    for (uint32_t i = 0; i < pager->num_pages; i++)
    {
        if (pager->pages[i] == NULL)
            continue;
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }
    int result = close(pager->file_descriptor);
    if (result == -1)
        printf("Error while closing db file\n");
}

Cursor *table_start(Table *table)
{
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;
    void *root_node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->end_of_table = (num_cells == 0);
    return cursor;
}

Cursor *table_end(Table *table)
{
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    void *root_node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->cell_num = num_cells;
    cursor->end_of_table = true;
    return cursor;
}

void cursor_advance(Cursor *cursor)
{
    uint32_t page_num = cursor->page_num;
    void *page = get_page(cursor->table->pager, page_num);
    cursor->cell_num += 1;
    if (cursor->cell_num >= *leaf_node_num_cells(page))
    {
        cursor->end_of_table = true;
    }
}

Row *cursor_value(Cursor *cursor)
{
    uint32_t page_num = cursor->page_num;
    Pager *pager = cursor->table->pager;
    void *page = get_page(pager, page_num);
    return leaf_node_value(page, cursor->cell_num);
}

Row *row_slot(Table *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    Page *page = get_page(table->pager, page_num);
    // rows in a page are stored consecutively but not the pages themselves
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return (Row *)page + row_num;
}

// Table *new_table()
// {
//     Table *table = (Table *)malloc(sizeof(Table));
//     table->num_rows = 0;
//     for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
//     {
//         table->pages[i] = NULL;
//     }
//     return table;
// }

Pager *pager_open(const char *filename)
{
    int file_descriptor = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if (file_descriptor == -1)
    {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(file_descriptor, 0, SEEK_END);
    Pager *pager = malloc(sizeof(Pager));
    pager->num_pages = file_length / PAGE_SIZE;
    pager->file_descriptor = file_descriptor;
    pager->file_length = file_length;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pager->pages[i] = NULL;
    }
    return pager;
}

Table *db_open(const char *filename)
{
    Pager *pager = pager_open(filename);
    Table *table = malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;
    if (pager->num_pages == 0)
    {
        // if db file empty
        void *root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
    }
    return table;
}

InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void read_input(InputBuffer *input_buffer)
{
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    // line ends with endl followed by \0
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0';
    // getline ignores the last \0 chars
}

void close_input_buffer(InputBuffer *input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareCommandResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row.id),
                                   (statement->row.username), (statement->row.email));
        if (args_assigned < 3)
            return PREPARE_SYNTAX_ERROR;
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
    Pager *pager = cursor->table->pager;
    void *node = get_page(pager, cursor->page_num);
    uint32_t num_cells = *(leaf_node_num_cells(node));
    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        printf("Need to implement splitting a leaf node.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = num_cells; i > cursor->cell_num; i--)
    {
        memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
    }
    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

ExecuteResult execute_insert(Statement *statement, Table *table)
{
    Cursor *cursor = table_end(table);
    void *node = get_page(table->pager, cursor->page_num);
    if ((*leaf_node_num_cells(node)) >= LEAF_NODE_MAX_CELLS)
    {
        return EXECUTE_TABLE_FULL;
    }
    Row *row_to_insert = &(statement->row);
    // serialize_row(row_to_insert, cursor_value(cursor));
    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
    free(cursor);
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
    Cursor *cursor = table_start(table);
    Row row;
    while (cursor->end_of_table == false)
    {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

void execute_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        execute_insert(statement, table);
        break;
    case (STATEMENT_SELECT):
        execute_select(statement, table);
        break;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    Table *table = db_open(filename);

    InputBuffer *input_buffer = new_input_buffer();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Unrecognized command\n");
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            execute_statement(&statement, table);
            printf("Executed.\n");
            continue;

        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("Unrecognized keyword at start of %s.\n", input_buffer->buffer);
            continue;
        }
    }
}
