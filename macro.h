#ifndef MACRO_H
#define MACRO_H

#include "utils.h"

/* macro initialization */
#define INITIAL_MACRO_LIST_HEAD NULL
#define INITIAL_MACRO_COUNT 0
#define INITIAL_CONTENT_LENGTH 0
#define INITIAL_MACRO_FLAG 0

/* for loop and index */
#define LOOP_START_INDEX 0
#define NAME_BUFFER_START_INDEX 0
#define FIRST_CHAR_INDEX 0

/* macro name validation helpers */
#define REGISTER_NAME_PREFIX_LOWER 'r'
#define MIN_REGISTER_DIGIT '0'
#define MAX_REGISTER_DIGIT '7'
#define REGISTER_NAME_EXACT_LENGTH 2

/* macro buffers size */
#define MACRO_CONTENT_BUFFER_SIZE 1000
#define LINE_LENGTH_CHECK_OFFSET 2

/* error message definitions for macro processing */
#define ERROR_LINE_TOO_LONG "Error: line exceeds maximum length of %d characters\n"
#define ERROR_MACRO_NAME_TOO_LONG "Error: macro name exceeds maximum length of %d characters: %s\n"
#define ERROR_CANNOT_OPEN_INPUT "Error: cannot open input file '%s'\n"
#define ERROR_CANNOT_CREATE_OUTPUT "Error: cannot create output file '%s'\n"
#define ERROR_INVALID_MACRO_NAME "Error: invalid macro name '%s' at line %d\n"
#define ERROR_MISSING_ENDMCRO "Error: macro '%s' missing 'endmcro' directive\n"

/* macro definition structure */
typedef struct {
    char name[MAX_MACRO_NAME];      /* macro identifier */
    char content[MAX_MACRO_BODY];   /* macro body content */
} macro;

/* macro table node for linked list */
typedef struct macro_node {
    macro macro;                    /* macro data */
    struct macro_node* next;        /* pointer to next node */
} macro_node;

/* macro table s managing all macros */
typedef struct {
    macro_node* head;               /* pointer to first node in list */
    int count;                      /* total number of macros */
} macro_list;

/**
 * init_macro_list - initialize the macro list to empty state
 * @macro_list: pointer to macro list structure
 */
void init_macro_list(macro_list* macro_list);

/**
 * check_if_macro_start - checks if line is a macro definition start
 * @line: line string to check
 * @return SUCCESS if valid macro start, FAILURE otherwise
 */
int check_if_macro_start(const char* line);

/**
 * check_if_macro_end - detects macro definition end
 * @line: line string to check
 * @return SUCCESS if macro end detected, FAILURE otherwise
 */
int check_if_macro_end(const char* line);

/**
 * extract_macro_name - extracts macro name from definition line
 * @line: macro definition line
 * @name: output buffer for macro name
 */
void extract_macro_name(const char* line, char* name);

/**
 * is_macro_call - checks if line contains macro invocation
 * @line: line string to check
 * @macro_list: table of defined macros
 * @return SUCCESS if macro call detected, FAILURE otherwise
 */
int is_macro_call(const char* line, const macro_list* macro_list);

/**
 * add_macro - adds new macro to table
 * @macro_list: pointer to macro table
 * @name: macro name
 * @content: macro body content
 * @return SUCCESS on successful addition, FAILURE otherwise
 */
int add_macro(macro_list* macro_list, const char* name, const char* content);

/**
 * find_macro - searches for macro by name
 * @macro_list: pointer to macro table to search
 * @name: macro name to find
 * @return pointer to macro if found, NULL otherwise
 */
macro* find_macro(const macro_list* macro_list, const char* name);

/**
 * free_macro_list - releases all macro table memory
 * @macro_list: pointer to macro table to free
 */
void free_macro_list(macro_list* macro_list);

/**
 * expand_macros - expands macros from input file to output file
 * @input_file: source file containing macro definitions and calls
 * @output_file: destination file for macro-expanded code
 */
void expand_macros(const char* input_file, const char* output_file);

/**
 * validate_macro_name - macro name validation
 * @name: macro name to validate
 * @macro_list: existing macro table for duplicate checking
 * @return SUCCESS if valid, FAILURE otherwise
 */
int validate_macro_name(const char* name, const macro_list* macro_list);

#endif /* MACRO_H */