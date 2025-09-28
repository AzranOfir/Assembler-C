#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* forward declarations */
static const char* handle_label_extraction(const char* line_copy, separate_line* separate);
static separate_line* allocate_separate_line(void);
static void initialize_separate_line(separate_line* separate);
static void initialize_operands_array(char** operands, int max_operands);

/**
 * parse_line - parse a single assembly line
 * @line: input line to parse
 * @return pointer to allocated separate_line on success, NULL on failure
 */
separate_line* parse_line(const char* line) {
    char line_copy[MAX_LINE_LENGTH];
    const char* rest;
    int i;
    size_t line_len;
    separate_line* separate;
    char** operands;
    const char* rest_ptr;

    /* check if the line is null */
    if (!line) {
        return NULL;
    }

    /* check if the line is empty */
    if (strlen(line) == 0) {
        return NULL;
    }

    /* check if the line is too long */
    line_len = strlen(line);
    if (line_len >= MAX_LINE_LENGTH) {
        fprintf(stderr, ERROR_LINE_TOO_LONG_DETAILED,
                line_len, MAX_LINE_LENGTH - 1);
        return NULL;
    }

    /* check if the line contains only whitespace */
    if (strspn(line, WHITESPACE_CHARS) == line_len) {
        return NULL;
    }

    /* check if the line contains non-printable characters */
    if (strcspn(line, CONTROL_CHARS) != line_len) {
        fprintf(stderr, ERROR_LINE_CONTAINS_NON_PRINTABLE);
        return NULL;
    }

    /* allocate memory for the structure */
    separate = allocate_separate_line();
    if (!separate) {
        return NULL;
    }

    /* initialize structure to null */
    initialize_separate_line(separate);

    /* copy the line to manipulate */
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = NULL_CHAR;

    /* extract label if exists */
    rest = handle_label_extraction(line_copy, separate);
    if (!rest) {
        /* if failed, free the structure and return null */
        free_separate_line(separate);
        return NULL;
    }

  
    
    /* skip spaces/tabs after the label */
    rest_ptr = rest;
    while (*rest_ptr && (*rest_ptr == SPACE_CHAR || *rest_ptr == TAB_CHAR)) {
        rest_ptr++;
    }
    rest = rest_ptr;

    /* check if we haven't reached the end of the line */
    if (!*rest) {
        /* line ends after label, no command or operands */
        separate->command = NULL;
        separate->how_many_operands = 0;
        return separate;
    }

    /* extract command */
    separate->command = extract_command(rest);
    if (!separate->command) {
        /* if failed, free the structure and return null */
        free_separate_line(separate);
        return NULL;
    }

   
    
    /* extract operands */
    operands = extract_operands(rest, &separate->how_many_operands);
    if (operands) {
        /* if succeeded, copy the operands to the structure */
        for (i = 0; i < separate->how_many_operands; i++) {
            separate->operands[i] = operands[i];
        }
        /* free the array, not the strings (strings are now owned by separate) */
        free(operands);
    }
    return separate;
}

/**
 * handle_label_extraction - extract label and return rest of the line
 * @line_copy: mutable copy of the input line
 * @separate: output struct to store extracted label
 * @return pointer to rest of the line after the label, or NULL on error
 */
static const char* handle_label_extraction(const char* line_copy, separate_line* separate) {
    char* colon_pos = strchr(line_copy, COLON);
    /*looking for the colon that means that there is a label*/
    if (colon_pos) {
        separate->label = extract_label(line_copy);
        if (!separate->label) {
            /* the label is not valid */
            return NULL;
        }
        /* return the rest of the line after the label */
        return colon_pos + 1;
    } else {
        /* no colon so there is no label, return start of line */
        return line_copy;
    }
}

/**
 * extract_operands - extract operand strings from a line
 * @line: source line
 * @count: out number of operands parsed
 * @return heap-allocated array of char* (caller frees array and strings)
 */
char** extract_operands(const char* line, int* count) {
    char line_copy[MAX_LINE_LENGTH];
    char** operands = NULL;
    char* current_pos;
    char current_operand[MAX_LINE_LENGTH];
    int operand_count = 0;
    int operand_index;
    int i;
    char* temp_pos;
    *count = 0;

    /* copy the line to manipulate */
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = NULL_CHAR;

    /* point to the start of the line */
    current_pos = line_copy;

    /* skip spaces and tabs in the beginning of the line */
    while (*current_pos == SPACE_CHAR || *current_pos == TAB_CHAR) {
        current_pos++;
    }

    /* skip the command */
    while (*current_pos && *current_pos != SPACE_CHAR && *current_pos != TAB_CHAR && *current_pos != NEWLINE_CHAR) {
        current_pos++;
    }

    /* skip spaces after command */
    while (*current_pos && (*current_pos == SPACE_CHAR || *current_pos == TAB_CHAR)) {
        current_pos++;
    }

    /* check if there are no operands */
    if (!*current_pos || *current_pos == NEWLINE_CHAR) {
        return NULL;
    }

    /* allocate memory for operands */
    operands = (char**)malloc(MAX_OPERANDS * sizeof(char*));
    if (!operands) {
        fprintf(stderr, MALLOC_FAILED);
        return NULL;
    }

    /* initialize array to null */
    initialize_operands_array(operands, MAX_OPERANDS);

    /* get operands one by one */
    while (*current_pos && operand_count < MAX_OPERANDS) {
        /* skip spaces and tabs in the beginning of the operand */
        while (*current_pos == SPACE_CHAR || *current_pos == TAB_CHAR) {
            current_pos++;
        }

        /* check if we are in the end of the line */
        if (!*current_pos || *current_pos == NEWLINE_CHAR || *current_pos == CARRIAGE_RETURN_CHAR) {
            break;
        }

        /* initialize the index of the operand */
        operand_index = 0;

        /* check if this is string operand */
        if (*current_pos == QUOTE_CHAR) {
            /* copy the opening quote */
            current_operand[operand_index++] = *current_pos;
            current_pos++;
            
            /* read until closing quote */
            while (*current_pos && *current_pos != QUOTE_CHAR && operand_index < MAX_LINE_LENGTH) {
                current_operand[operand_index++] = *current_pos;
                current_pos++;
            }
            
            /* add closing quote if found */
            if (*current_pos == QUOTE_CHAR) {
                current_operand[operand_index++] = *current_pos;
                current_pos++;
            }
        } else {
            /* read until comma, space, tab, new line, carriage return or end of line */
            while (*current_pos &&
                   *current_pos != COMMA_CHAR &&
                   *current_pos != SPACE_CHAR &&
                   *current_pos != TAB_CHAR &&
                   *current_pos != NEWLINE_CHAR &&
                   *current_pos != CARRIAGE_RETURN_CHAR &&
                   operand_index < MAX_LINE_LENGTH) {
                current_operand[operand_index++] = *current_pos;
                current_pos++;
            }
        }

        /* add null finish char to the operand */
        current_operand[operand_index] = NULL_CHAR;

        /* allocate memory for this operand */
        operands[operand_count] = (char*)malloc(operand_index + 1);
        if (!operands[operand_count]) {
            fprintf(stderr, MALLOC_FAILED);
            /* free allocated memory and return error */
            for (i = 0; i < operand_count; i++) {
                if (operands[i]) {
                    free(operands[i]);
                }
            }
            free(operands);
            return NULL;
        }

        /* copy the operand */
        strcpy(operands[operand_count], current_operand);
        operand_count++;

        /* skip spaces after operand */
        while (*current_pos == SPACE_CHAR || *current_pos == TAB_CHAR) {
            current_pos++;
        }

        /* skip comma if exists */
        if (*current_pos == COMMA_CHAR) {
            current_pos++;

            /* check if the next char is a space or tab */
            temp_pos = current_pos;
            while (*temp_pos == SPACE_CHAR || *temp_pos == TAB_CHAR) {
                temp_pos++;
            }

            /* if the next char is comma, null, new line or carriage return, it's an error */
            if (*temp_pos == COMMA_CHAR || *temp_pos == NULL_CHAR || *temp_pos == NEWLINE_CHAR || *temp_pos == CARRIAGE_RETURN_CHAR) {
                /* free allocated memory and return error */
                for (i = 0; i < operand_count; i++) {
                    if (operands[i]) {
                        free(operands[i]);
                    }
                }
                free(operands);
                return NULL;
            }
        }
    }

    *count = operand_count;
    return operands;
}



/* generic function to allocate string memory with error context */
static char* allocate_string_memory(size_t length, const char* allocation_purpose) {
    char* str = (char*)malloc(length + 1);
    if (!str) {
        fprintf(stderr, MALLOC_FAILED);
        if (allocation_purpose) {
            fprintf(stderr, ALLOCATION_PURPOSE_FORMAT, allocation_purpose);
        }
        return NULL;
    }
    return str;
}

/**
 * extract_label - extract label from a line
 * @line: source line
 * @return heap-allocated label string or NULL on failure
 */
char* extract_label(const char* line) {
    char* before_colon;
    size_t label_length;
    char* label;

    /* skip the spaces and tabs in the beginning of the line */
    while (*line == SPACE_CHAR || *line == TAB_CHAR) {
        line++;
    }

    /* looking for the colon that means that there is a label*/
    before_colon = strchr(line, COLON);
    if (!before_colon) {
        /* if there is no colon, return null */
        return NULL;
    }

    /* there is a label, so find its length */
    label_length = before_colon - line;
    if (label_length == 0) {
        /*empty*/
        return NULL;
    }

    /* check if label is too long */
    if (label_length > MAX_LABEL_LENGTH - 1) {
        fprintf(stderr, ERROR_LABEL_LENGTH_TOO_LONG, MAX_LABEL_LENGTH - 1);
        return NULL;
    }

    /* everything fine, so allocate memory for label */
    label = allocate_string_memory(label_length, ALLOCATION_PURPOSE_LABEL);
    if (!label) {
        return NULL;
    }

    /* copy the label */
    strncpy(label, line, label_length);
    label[label_length] = NULL_CHAR;

    /* check if the label is valid */
    if (!is_valid_label(label, 1)) {
        /* the label is not valid*/
        free(label);
        return NULL;
    }

    return label;
}

/**
 * extract_command - extract command token from a line
 * @line: source line
 * @return heap-allocated command string or NULL on failure
 */
char* extract_command(const char* line) {
    char* token;
    char line_copy[MAX_LINE_LENGTH];
    char* line_ptr;
    size_t token_len;
    char* command;

    /* copy the line to manipulate */
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = NULL_CHAR;

    /* Skip whitespaces and tabs */
    line_ptr = line_copy;
    while (*line_ptr == SPACE_CHAR || *line_ptr == TAB_CHAR) {
        line_ptr++;
    }

    /* get the first word in the line - maybe the command */
    token = strtok(line_ptr, WHITESPACE_CHARS);
    if (!token) {
        return NULL;
    }

    /* check if the command is not empty */
    token_len = strlen(token);
    if (token_len == 0) {
        return NULL;
    }

    /* check if the command is valid (opcode or directive)*/
    if (!is_valid_opcode(token) && !is_valid_directive(token)) {
        return NULL;
    }

    /* allocate memory for command */
    command = allocate_string_memory(token_len, ALLOCATION_PURPOSE_COMMAND);
    if (!command) {
        return NULL;
    }

    /* copy the command */
    strcpy(command, token);

    return command;
}

/**
 * allocate_separate_line - allocate a separate_line structure
 * @return pointer to allocated struct or NULL on failure
 */
static separate_line* allocate_separate_line() {
    separate_line* separate = (separate_line*)malloc(sizeof(separate_line));
    if (!separate) {
        fprintf(stderr, MALLOC_FAILED);
        return NULL;
    }
    return separate;
}

/**
 * initialize_separate_line - initialize fields to default values
 * @separate: structure to initialize
 */
static void initialize_separate_line(separate_line* separate) {
    int i;

    separate->label = NULL;
    separate->command = NULL;
    separate->how_many_operands = 0;

    /* Initialize all operands to NULL */
    for (i = 0; i < MAX_OPERANDS; i++) {
        separate->operands[i] = NULL;
    }
}

/**
 * initialize_operands_array - set an operands array to NULLs
 * @operands: array to initialize
 * @max_operands: array length
 */
static void initialize_operands_array(char** operands, int max_operands) {
    int i;
    for (i = 0; i < max_operands; i++) {
        operands[i] = NULL;
    }
}
