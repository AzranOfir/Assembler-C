#ifndef LABELTABLE_H
#define LABELTABLE_H

/* label table configuration constants */
#define MAX_LABEL_NAME 31
#define INITIAL_COUNT 0

/* error message definitions for label table operations */
#define ERROR_MEMORY_ALLOCATION_FAILED "Error: memory allocation failed for label '%s'\n"
#define ERROR_LABEL_NOT_FOUND "Error: label '%s' not found in symbol table\n"
#define ERROR_LABEL_NOT_DEFINED "Error: label '%s' is not defined (no address assigned)\n"
#define ERROR_LABEL_ALREADY_DEFINED "Error: label '%s' is already defined\n"
#define ERROR_DUPLICATE_LABEL "Error: label '%s' already exists (duplicate labels not allowed)\n"

/* label classification types */
typedef enum {
    LABEL_CODE,      /* code labels */
    LABEL_DATA,      /* data labels  */
    LABEL_EXTERNAL,  /* external labels */
    LABEL_ENTRY      /* entry labels */
} label_type;

/* label node structure for linked list implementation */
typedef struct label_node {
    char name[MAX_LABEL_NAME];      /* label identifier */
    int address;                    /* memory address or value */
    label_type type;                /* label classification */
    int is_defined;                 /* 1 if defined, 0 if only declared */
    struct label_node *next;        /* pointer to next node in list */
} label_node;

/* symbol table structure managing all labels */
typedef struct {
    label_node *head;               /* pointer to first node in linked list */
    int count;                      /* total number of labels in table */
} label_table;

/**
 * nitialize empty label table
 * @param table: pointer to label table structure to initialize
 */
void init_label_table(label_table *table);

/**
 * Add new label to symbol table
 * @param table: pointer to label table
 * @param name: label name string
 * @param address: memory address or value for label
 * @param type: label classification type (LABEL_CODE, LABEL_DATA, LABEL_EXTERNAL, LABEL_ENTRY)
 * @return SUCCESS on successful addition, FAILURE otherwise
 */
int add_label(label_table *table, const char *name, int address, label_type type);

/**
 * search for label by name
 * @param table: pointer to label table to search
 * @param name: label name to find
 * @return pointer to label node if found, NULL otherwise
 */
label_node *find_label(const label_table *table, const char *name);

/**
 * find_label_with_error - searches for label with error reporting
 * @table: pointer to label table to search  
 * @name: label name to find
 * 
 * same as find_label but prints error message if label not found
 * used when label is expected to exist
 * returns pointer to label node if found, NULL otherwise
 */
label_node *find_label_with_error(const label_table *table, const char *name);

/**
 * update_label_address - modifies existing label address
 * @table: pointer to label table
 * @name: name of label to update
 * @new_address: new address value to assign
 * 
 * finds existing defined label and updates its address
 * returns SUCCESS if update successful, FAILURE otherwise
 */
int update_label_address(label_table *table, const char *name, int new_address);

/**
 * mark_label_defined - sets label as defined
 * @table: pointer to label table
 * @name: name of label to mark as defined
 * 
 * changes label status from declared to defined
 * prevents multiple definitions of same label
 * returns SUCCESS if marking successful, FAILURE otherwise
 */
int mark_label_defined(label_table *table, const char *name);

/**
 * delete_label - removes label from table
 * @table: pointer to label table
 * @name: name of label to remove
 * 
 * finds and removes label node from linked list
 * frees associated memory
 * returns SUCCESS if deletion successful, FAILURE otherwise
 */
int delete_label(label_table *table, const char *name);

/**
 * free all table memory
 * @param table: pointer to label table to free
 */
void free_label_table(label_table *table);

#endif /* LABELTABLE_H */