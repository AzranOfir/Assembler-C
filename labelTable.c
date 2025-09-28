#include "labelTable.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize empty label table
 * @param table: pointer to label table structure to initialize
 */
void init_label_table(label_table* table) {
    if (!table) {
        return;  
    }
    
    /* initialize empty table */
    table->head = NULL;
    table->count = INITIAL_COUNT;
}

/**
 * Add new label to symbol table
 * @param table: pointer to label table
 * @param name: label name string
 * @param address: memory address or value for label
 * @param type: label classification type (LABEL_CODE, LABEL_DATA, LABEL_EXTERNAL, LABEL_ENTRY)
 * @return SUCCESS on successful addition, FAILURE otherwise
 */
int add_label(label_table* table, const char* name, int address, label_type type) {
    label_node* new_node;
    label_node* existing_label;

    /* determine whether to print validation errors */
    int should_print_errors = (type != LABEL_EXTERNAL);
    
    /* validate label name format */
    if (!is_valid_label(name, should_print_errors)) {
        return FAILURE;
    }

    /* check for existing label with same name */
    existing_label = find_label(table, name);
    if (existing_label != NULL) {
        fprintf(stderr, ERROR_DUPLICATE_LABEL, name);
        return FAILURE;
    }

    /* allocate memory for new label node */
    new_node = (label_node*)malloc(sizeof(label_node));
    if (!new_node) {
        fprintf(stderr, ERROR_MEMORY_ALLOCATION_FAILED, name);
        return FAILURE; 
    }

    /* initialize new label node */
    strcpy(new_node->name, name);
    new_node->address = address;
    new_node->type = type;
    new_node->is_defined = 0; 

    /* add to beginning of linked list */
    new_node->next = table->head;
    table->head = new_node;
    table->count++;

    return SUCCESS;
}

/**
 * Search for label by name
 * @param table: pointer to label table to search
 * @param name: label name to find
 * @return pointer to label node if found, NULL otherwise
 */
label_node* find_label(const label_table* table, const char* name) {
    label_node* current;

    /* validate input parameters */
    if (!table || !name) {
        return NULL;
    }

    /* search through linked list */
    current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current; 
        }
        current = current->next;
    }

    return NULL; /* label not found */
}

/**
 * find_label_with_error - searches for label with error reporting
 * @param table: pointer to label table to search  
 * @param name: label name to find
 * @return pointer to label node if found, NULL otherwise
 * 
 * same as find_label but prints error message if label not found
 * used when label is expected to exist
 */
label_node* find_label_with_error(const label_table* table, const char* name) {
    label_node* result;
    
    /* validate input parameters */
    if (!table || !name) {
        return NULL;
    }
    
    result = find_label(table, name);
    if (!result) {
        fprintf(stderr, ERROR_LABEL_NOT_FOUND, name);
    }
    return result;
}

/**
 * update_label_address - modifies existing label address
 * @param table: pointer to label table
 * @param name: name of label to update
 * @param new_address: new address value to assign
 * @return SUCCESS if update successful, FAILURE otherwise
 * 
 * finds existing defined label and updates its address
 */
int update_label_address(label_table* table, const char* name, int new_address) {
    label_node* label;

    /* validate input parameters */
    if (!table || !name) {
        return FAILURE;
    }

    /* find label in table */
    label = find_label(table, name);
    if (label == NULL) {
        fprintf(stderr, ERROR_LABEL_NOT_FOUND, name);
        return FAILURE; 
    }

    /* check that label is defined */
    if (!label->is_defined) {
        fprintf(stderr, ERROR_LABEL_NOT_DEFINED, name);
        return FAILURE; 
    }

    /* update address */
    label->address = new_address;
    return SUCCESS; 
}

/**
 * mark_label_defined - sets label as defined
 * @param table: pointer to label table
 * @param name: name of label to mark as defined
 * @return SUCCESS if marking successful, FAILURE otherwise
 * 
 * changes label status from declared to defined
 * prevents multiple definitions of same label
 */
int mark_label_defined(label_table* table, const char* name) {
    label_node* label;

    /* validate input parameters */
    if (!table || !name) {
        return FAILURE;
    }

    /* find label in table */
    label = find_label(table, name);
    if (label == NULL) {
        fprintf(stderr, ERROR_LABEL_NOT_FOUND, name);
        return FAILURE; 
    }

    /* check if already defined */
    if (label->is_defined) {
        fprintf(stderr, ERROR_LABEL_ALREADY_DEFINED, name);
        return FAILURE; 
    }

    /* mark as defined */
    label->is_defined = 1;
    return SUCCESS;
}

/**
 * delete_label - removes label from table
 * @param table: pointer to label table
 * @param name: name of label to remove
 * @return SUCCESS if deletion successful, FAILURE otherwise
 * 
 * finds and removes label node from linked list
 * frees associated memory
 */
int delete_label(label_table* table, const char* name) {
    label_node* current_label;
    label_node* prev_label;

    /* validate input parameters */
    if (!table || !name) {
        return FAILURE;
    }

    current_label = table->head;
    prev_label = NULL;

    /* search for label to delete */
    while (current_label != NULL && strcmp(current_label->name, name) != 0) {
        prev_label = current_label;
        current_label = current_label->next;
    }

    /* label not found */
    if (current_label == NULL) {
        fprintf(stderr, ERROR_LABEL_NOT_FOUND, name);
        return FAILURE; 
    }

    /* remove node from linked list */
    if (prev_label == NULL) {
        /* removing first node */
        table->head = current_label->next;
    } else {
        /* removing middle or last node */
        prev_label->next = current_label->next;
    }

    /* free memory and update count */
    free(current_label);
    table->count--;
    return SUCCESS; 
}

/**
 * Release all table memory
 * @param table: pointer to label table to free
 */
void free_label_table(label_table* table) {
    label_node* current_label;
    label_node* next_label;

    /* validate input parameter */
    if (!table) {
        return;
    }

    /* free all label nodes */
    current_label = table->head;
    while (current_label != NULL) {
        next_label = current_label->next;
        free(current_label);
        current_label = next_label;
    }

    /* reset table to empty state */
    table->head = NULL;
    table->count = 0;
}