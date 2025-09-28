#include "macro.h"

#include <ctype.h>

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * init_macro_list - initialize the macro list to empty state
 * @macro_list: pointer to macro list structure
 */
void init_macro_list(macro_list* macro_list) {
    macro_list->head = INITIAL_MACRO_LIST_HEAD;
    macro_list->count = INITIAL_MACRO_COUNT;
}

/**
 * check_if_macro_start - checks if line is a macro definition start
 * @line: line string to check
 * @return SUCCESS if valid macro start, FAILURE otherwise
 */
int check_if_macro_start(const char* line) {
    const char* name_start;
    char temp_name[MAX_WORD_LENGTH];
    char* newline;

    /* remove all tabs and spaces */
    while (*line == SPACE_CHAR || *line == TAB_CHAR) {
        line++;
    }
    /* check if the line starts with the word mcro */
    if (strncmp(line, MCRO_KEYWORD, MCRO_LENGTH) != 0) {
        return FAILURE;
    }
    /* check if there is space after 'mcro', if not this is illegal */
    if (line[MCRO_LENGTH] != SPACE_CHAR) {
        return FAILURE;
    }

    /* find the name of the macro */
    name_start = line + MCRO_SPACE_OFFSET;
    while (*name_start == SPACE_CHAR) name_start++;

    /* copy the name of the macro to a temp string */
    strcpy(temp_name, name_start);

    /* look for new line - so the macro name is before it and we will get a pointer to it */
    newline = strchr(temp_name, NEWLINE_CHAR);
    if (newline) {
        /* find the name, now put end of line */
        *newline = NULL_CHAR;
    }

    /* check macro name length */
    if (strlen(temp_name) > MAX_MACRO_NAME - NULL_TERMINATOR_SIZE) {
        fprintf(stderr, ERROR_MACRO_NAME_TOO_LONG, MAX_MACRO_NAME - NULL_TERMINATOR_SIZE, temp_name);
        return FAILURE;
    }

    /* comprehensive validation - this will be called from expand_macros with the macro_list */
    /* For now, just check basic syntax */
    if (!is_valid_macro_name(temp_name)) {
        return FAILURE;
    }

    return SUCCESS;
}
/**
 * check_if_macro_end - detects macro definition end
 * @line: line string to check
 * @return SUCCESS if macro end detected, FAILURE otherwise
 */
int check_if_macro_end(const char* line) {
    char temp_line[MAX_WORD_LENGTH];
    char* newline;
    int result;

    /* remove all tabs and spaces */
    while (*line == SPACE_CHAR || *line == TAB_CHAR) {
        line++;
    }

    /* copy the line to a temp string */
    strcpy(temp_line, line);

    /* look for new line - so the macro name is before it and we will get a pointer to it */
    newline = strchr(temp_line, NEWLINE_CHAR);
    if (newline) {
        *newline = NULL_CHAR;
    }

    /* check if the line is a macro end */
    result = (strcmp(temp_line, MCROEND_KEYWORD) == LOOP_START_INDEX) ? SUCCESS : FAILURE;
    return result;
}

/**
 * extract_macro_name - extracts macro name from definition line
 * @line: macro definition line
 * @name: output buffer for macro name
 */
void extract_macro_name(const char* line, char* name) {
    int i;

    /* remove all tabs and spaces */
    while (*line == SPACE_CHAR || *line == TAB_CHAR) {
        line++;
    }
    /* skip the word mcro */
    line += MCRO_SPACE_OFFSET;
    /* skip all spaces */
    while (*line == SPACE_CHAR) {
        line++;
    }
    i = 0;
    /* copy the name of the macro to name */
    while (*line && *line != SPACE_CHAR && *line != TAB_CHAR && *line != NEWLINE_CHAR && i < MAX_MACRO_NAME - 1) {
        name[i++] = *line++;
    }
    name[i] = NULL_CHAR;
}

/**
 * is_macro_call - checks if line contains macro invocation
 * @line: line string to check
 * @macro_list: table of defined macros
 * @return SUCCESS if macro call detected, FAILURE otherwise
 */
int is_macro_call(const char* line, const macro_list* macro_list) {
    char first_word[MAX_MACRO_NAME];
    int i;
    int j;

    i = 0;
    j = 0;

    while (line[j] == SPACE_CHAR || line[j] == TAB_CHAR) {
        j++;
    }
    /* copy the first word in line */
    while (line[j] && line[j] != SPACE_CHAR && line[j] != TAB_CHAR && line[j] != NEWLINE_CHAR && i < MAX_MACRO_NAME - 1) {
        first_word[i++] = line[j++];
    }
    /* put end of string */
    first_word[i] = NULL_CHAR;
    /* check if the macro is in the macro list */
    return find_macro(macro_list, first_word) != NULL;
}
/**
 * add_macro - adds new macro to table
 * @macro_list: pointer to macro table
 * @name: macro name
 * @content: macro body content
 * @return SUCCESS on successful addition, FAILURE otherwise
 */
int add_macro(macro_list* macro_list, const char* name, const char* content) {
    macro_node* new_node;

    new_node = (macro_node*)malloc(sizeof(macro_node));
    if (!new_node) {
        fprintf(stderr, MALLOC_FAILED);
        free_macro_list(macro_list);
        return FAILURE;
    }
    /*copy the name*/
    strcpy(new_node->macro.name, name);
    /*copy the macro code*/
    strcpy(new_node->macro.content, content);
    /*add the macro to the macro list*/
    new_node->next = macro_list->head;
    macro_list->head = new_node;
    macro_list->count++;

    return SUCCESS; /* success */
}

/**
 * find_macro - searches for macro by name
 * @macro_list: pointer to macro table to search
 * @name: macro name to find
 * @return pointer to macro if found, NULL otherwise
 */
macro* find_macro(const macro_list* macro_list, const char* name) {
    macro_node* current;

    current = macro_list->head;
    while (current != NULL) {
        if (strcmp(current->macro.name, name) == 0) {
            return &current->macro;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * free_macro_list - releases all macro table memory
 * @macro_list: pointer to macro table to free
 */
void free_macro_list(macro_list* macro_list) {
    macro_node* current;
    macro_node* next;

    current = macro_list->head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    init_macro_list(macro_list);
}

/**
 * expand_macros - expands macros from input file to output file
 * @input_file: source file containing macro definitions and calls
 * @output_file: destination file for macro-expanded code
 */
void expand_macros(const char* input_file, const char* output_file) {
    FILE* input;
    FILE* output;
    macro_list macro_list;
    int in_macro_definition;
    char current_macro_name[MAX_MACRO_NAME];
    char current_macro_content[MAX_MACRO_BODY];
    char line[MAX_LINE_LENGTH];
    int content_length;
    int line_number;

    /*open the input file for reading */
    input = fopen(input_file, FILE_READ_MODE);
    if (!input) {
        fprintf(stderr, ERROR_CANNOT_OPEN_INPUT, input_file);
        return;
    }
    output = fopen(output_file, FILE_WRITE_MODE);
    if (!output) {
        fprintf(stderr, ERROR_CANNOT_CREATE_OUTPUT, output_file);
        fclose(input);
        return;
    }

    init_macro_list(&macro_list);
    in_macro_definition = 0;
    content_length = INITIAL_CONTENT_LENGTH;
    line_number = 0;

    /* first pass: collect macro definitions */
    while (fgets(line, sizeof(line), input)) {
        line_number++;

        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[MAX_LINE_LENGTH - 2] != NEWLINE_CHAR) {
            fprintf(stderr, ERROR_LINE_TOO_LONG, MAX_LINE_LENGTH - 1);
            free_macro_list(&macro_list);
            fclose(input);
            fclose(output);
            return;
        }

        /*check if their a macro start in the line*/
        if (check_if_macro_start(line)) {
            /* extract macro name to check if valid */
            extract_macro_name(line, current_macro_name);

            /* check if valid */
            if (!validate_macro_name(current_macro_name, &macro_list)) {
                fprintf(stderr, ERROR_INVALID_MACRO_NAME, current_macro_name, line_number);
                free_macro_list(&macro_list);
                fclose(input);
                fclose(output);
                return;
            }

            in_macro_definition = 1;
            content_length = INITIAL_CONTENT_LENGTH;
            current_macro_content[0] = NULL_CHAR;
        }
        /*check if their a macro end in the line*/
        else if (check_if_macro_end(line)) {
            /*there is a macro end, so  add the macro if we are in the first pass*/
            if (in_macro_definition) {
                add_macro(&macro_list, current_macro_name, current_macro_content);
                in_macro_definition = 0;
            }
        }
        /*if we are in the first pass, add the macro content to the macro list*/
        else if (in_macro_definition) {
            size_t line_len;

            line_len = strlen(line);
            /*check if there is enough place*/
            if (content_length + line_len < sizeof(current_macro_content) - 1) {
                /*add the line to the macro content*/
                strcat(current_macro_content, line);
                content_length += (int)line_len;
            }
        }
    }

    /* check if we're still in a macro definition - there is no endmcro */
    if (in_macro_definition) {
        fprintf(stderr, ERROR_MISSING_ENDMCRO, current_macro_name);
        free_macro_list(&macro_list);
        fclose(input);
        fclose(output);
        return;
    }

    /*return to the start of the file*/
    rewind(input);
    line_number = 0;

    /* seond pass: expand macros */
    in_macro_definition = 0; /* Reset for second pass */
    while (fgets(line, sizeof(line), input)) {
        line_number++;

        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[MAX_LINE_LENGTH - 2] != NEWLINE_CHAR) {
            fprintf(stderr, ERROR_LINE_TOO_LONG, MAX_LINE_LENGTH - 1);
            free_macro_list(&macro_list);
            fclose(input);
            fclose(output);
            return;
        }
        /*if the line is a macro start, skip it and enter macro definition mode*/
        if (check_if_macro_start(line)) {
            in_macro_definition = 1;
            continue;
        }
        /*if the line is a macro end, skip it and exit macro definition mode*/
        else if (check_if_macro_end(line)) {
            in_macro_definition = 0;
            continue;
        }
        /*if we're inside a macro definition, skip*/
        else if (in_macro_definition) {
            continue;
        }
        /*if macro call, expand the macro*/
        else if (is_macro_call(line, &macro_list)) {
            char macro_name[MAX_MACRO_NAME];
            macro* macro;
            int i;
            int j;

            i = 0;
            j = 0;
            while (line[j] == SPACE_CHAR || line[j] == TAB_CHAR) j++;
            while (line[j] && line[j] != SPACE_CHAR && line[j] != TAB_CHAR && line[j] != NEWLINE_CHAR && i < MAX_MACRO_NAME - 1) {
                macro_name[i++] = line[j++];
            }
            macro_name[i] = NULL_CHAR;

            /*find the macro in the macro list*/
            macro = find_macro(&macro_list, macro_name);
            if (macro) {
                /*copy the macro content to the output file*/
                fputs(macro->content, output);
            }
        }
        else {
            fputs(line, output);
        }
    }

    free_macro_list(&macro_list);
    fclose(input);
    fclose(output);
}

/**
 * validate_macro_name - macro name to check
 * @name: macro name to validate
 * @macro_list: existing macro table for duplicate checking
 * @return SUCCESS if valid, FAILURE otherwise
 */
int validate_macro_name(const char* name, const macro_list* macro_list) {
    int i;

    /* check 1: not empty */
    if (!name || strlen(name) == 0) {
        return FAILURE;
    }

    /* check 2: not start with a digit */
    if (isdigit(name[0])) {
        return FAILURE;
    }

    /* check 3: not aמ opcode */
    if (is_valid_opcode(name)) {
        return FAILURE;
    }

    /* check 4: not a register (r0-r7) */
    if (strlen(name) == REGISTER_NAME_LENGTH && name[0] == REGISTER_PREFIX_CHAR) {
        if (name[1] >= MIN_REGISTER_CHAR && name[1] <= MAX_REGISTER_CHAR) {
            return FAILURE;
        }
    }

    /* check 5: contains only letters, digits and underscore */
    for (i = 0; name[i]; i++) {
        if (!isalnum(name[i]) && name[i] != UNDERSCORE_CHAR) {
            return FAILURE;
        }
    }

    /* check 6: not already defined as a macro */
    if (find_macro(macro_list, name) != NULL) {
        return FAILURE;
    }

    return SUCCESS;
}

