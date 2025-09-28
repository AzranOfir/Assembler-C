#include "utils.h"
#include "commands.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * check if this is a valid directive
 * @param name: directive name to check
 * @return SUCCESS if valid directive, FAILURE otherwise
 */
int is_valid_directive(const char *name) {
    if (!name) {
        return FAILURE;
    }

    /* check for data directives */
    if (strcmp(name, DIRECTIVE_DATA) == 0) return SUCCESS;
    if (strcmp(name, DIRECTIVE_STRING) == 0) return SUCCESS;
    if (strcmp(name, DIRECTIVE_MAT) == 0) return SUCCESS;

    /* check for label directives */
    if (strcmp(name, DIRECTIVE_EXTERN) == 0) return SUCCESS;
    if (strcmp(name, DIRECTIVE_ENTRY) == 0) return SUCCESS;

    return FAILURE;
}

/**
 * check if this is a valid opcode
 * @param name: instruction name to check
 * @return SUCCESS if valid opcode, FAILURE otherwise
 */
int is_valid_opcode(const char *name) {
    int i;

    /* check if name is valid */
    if (!name) {
        return FAILURE;
    }
    /*  name cannot be an opcode */
    for (i = 0; i < NUM_OF_OPCODES; i++) {
        if (strcmp(name, instruction_table[i].name) == 0) {
            return SUCCESS;
        }
    }
    return FAILURE;
}

/**
 * check if this is a valid macro name
 * @param name: macro name to check
 * @return SUCCESS if valid macro name, FAILURE otherwise
 */
int is_valid_macro_name(const char *name) {
    int i;
    size_t len;

    /* check if name is valid */
    if (!name) {
        return FAILURE;
    }

    len = strlen(name);
    if (len < MIN_MACRO_NAME_LENGTH) {
        return FAILURE;
    }

    /* macro needs to start letter */
    if (!isalpha(name[0])) {
        return FAILURE;
    }

    /* macro cannot be opcode */
    if (is_valid_opcode(name)) {
        return FAILURE;
    }

    /* macro cannot be a register(r0-r7) */
            if (len == REGISTER_NAME_LENGTH &&
            name[0] == REGISTER_PREFIX_CHAR) {
        if (name[1] >= MIN_REGISTER_CHAR && name[1] <= MAX_REGISTER_CHAR) {
            return FAILURE;
        }
    }

    /* macro contains only numbers' letters and underscore */
    for (i = 0; name[i]; i++) {
        if (!isalnum(name[i]) && name[i] != UNDERSCORE_CHAR) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * check if the name is a valid label name
 * @param label: label name to check
 * @param print_errors: whether to print error messages (1) or not (0) - depends on the calling function
 * @return SUCCESS if valid label, FAILURE otherwise
 */
int is_valid_label(const char* label, int print_errors) {
    int i;
    size_t len;

    /* check if the label is not null */
    if (!label) {
        if (print_errors) {
            fprintf(stderr, ERROR_UNLABEL);
        }
        return FAILURE;
    }

    len = strlen(label);

    /* check if the length is valid */
    if (len == 0) {
        if (print_errors) {
            fprintf(stderr, ERROR_UNLABEL);
        }
        return FAILURE;
    }
    if (len > MAX_LABEL_LENGTH-1) {
        if (print_errors) {
            fprintf(stderr, ERROR_LABEL_LENGTH_TOO_LONG, MAX_LABEL_LENGTH-1);
        }
        return FAILURE;
    }

    /* label needs to start with a letter */
    if (!isalpha(label[0])) {
        if (print_errors) {
            fprintf(stderr, ERROR_INVALID_LABEL, label);
        }
        return FAILURE;
    }

    /* check if all other characters are letters or digits */
    for (i = 1; label[i]; i++) {
        if (!isalpha(label[i]) && !isdigit(label[i])) {
            if (print_errors) {
                fprintf(stderr, ERROR_INVALID_LABEL, label);
            }
            return FAILURE;
        }
    }

    /* label cannot be an opcode */
    if (is_valid_opcode(label)) {
        if (print_errors) {
            fprintf(stderr, ERROR_INVALID_LABEL, label);
        }
        return FAILURE;
    }

    /* label cannot be a register (r0-r7) */
            if (len == REGISTER_NAME_LENGTH &&
            label[0] == REGISTER_PREFIX_CHAR) {
        if (label[1] >= MIN_REGISTER_CHAR && label[1] <= MAX_REGISTER_CHAR) {
            if (print_errors) {
                fprintf(stderr, ERROR_INVALID_LABEL, label);
            }
            return FAILURE;
        }
    }

    return SUCCESS;
}

/* helper function to check if register name is valid */
static int is_valid_register(const char *reg_str) {
    /*check the len of the input*/
    if (strlen(reg_str) != REGISTER_NAME_LENGTH){return FAILURE;}
    /* check if the first char is r (lowercase only) */
    if (reg_str[0] != REGISTER_PREFIX_CHAR) {return FAILURE;}
    /* check if the second char is a digit between 0 and 7 */
    if (reg_str[1] < MIN_REGISTER_CHAR || reg_str[1] > MAX_REGISTER_CHAR) {return FAILURE;}
    return SUCCESS;
}

/**
 * parse_matrix_access - validates matrix addressing operand syntax
 * @operand: input string expected in the form: label[reg][reg]
 * Returns:
 * - SUCCESS if the operand matches matrix addressing syntax
 * - FAILURE otherwise
 */
static int parse_matrix_access(const char *operand) {
    char *copy;
    char *first_bracket;
    char *first_close;
    char *second_bracket;
    char *second_close;
    char reg1[REGISTER_BUFFER_SIZE], reg2[REGISTER_BUFFER_SIZE];
    int result = FAILURE;

    /* check if the input is not null*/
    if (!operand) {
        return FAILURE;
    }

    /* allocate memory for the copy of the matrix so the real one is not changed*/
    copy = malloc(strlen(operand) + 1);
    if (!copy) {return FAILURE;}
    strcpy(copy, operand);

    /* find the first bracket, so we can check if there's a label before it*/
    first_bracket = strchr(copy, OPEN_BRACKET);
    if (!first_bracket) {
        free(copy);
        return FAILURE;
    }

    /* find the second bracket, so we can check if there's a label after it*/
    first_close = strchr(first_bracket + 1, CLOSE_BRACKET);
    if (!first_close) {
        free(copy);
        return FAILURE;
    }

    /* find the third bracket, so we can check if there's a label after it*/
    second_bracket = strchr(first_close + 1, OPEN_BRACKET);
    if (!second_bracket) {
        free(copy);
        return FAILURE;
    }
    /* find the fourth bracket, so we can check if there's a label after it*/
    second_close = strchr(second_bracket + 1, CLOSE_BRACKET);
    if (!second_close) {
        free(copy);
        return FAILURE;
    }

    /* check that there is a label before the first brackets and that it is valid */
    *first_bracket = NULL_CHAR;
    if (strlen(copy) == 0 || !is_valid_label(copy, 0)) {
        free(copy);
        return FAILURE;
    }

    /* find the first register*/
    if (first_close - first_bracket - 1 != 2) {
        free(copy);
        return FAILURE;
    }
    /* copy the first register so we can check if it is ok*/
    strncpy(reg1, first_bracket + 1, 2);
    reg1[2] = NULL_CHAR;

    /* find the second register*/
    if (second_close - second_bracket - 1 != 2) {
        free(copy);
        return FAILURE;
    }
    /* copy the second register so we can check if it is ok*/
    strncpy(reg2, second_bracket + 1, 2);
    reg2[2] = NULL_CHAR;

    /* check if the registers are valid */
    if (is_valid_register(reg1) && is_valid_register(reg2)) {
        result = SUCCESS;
    }

    /* free the copy of the matrix, because we don't need it anymore*/
    free(copy);
    return result;
}

/**
 * free all allocated memory in a separate_line structure
 * @param s: pointer to separate_line structure we want to free
 */
void free_separate_line(separate_line *s) {
    int i;
    if (!s) return;
    if (s->label) free(s->label);
    if (s->command) free(s->command);
    /* Free only the operands that were actually allocated */
    for (i = 0; i < s->how_many_operands; i++) {
        if (s->operands[i]) free(s->operands[i]);
    }
    free(s);
}

/* open file to read */
FILE *open_file_read(const char *filename) {
    FILE *f = fopen(filename, FILE_READ_MODE);
    if (!f) {
        fprintf(stderr, ERROR_CANNOT_OPEN_FILE, filename);
        return NULL;
    }
    return f;
}

/* open file to write */
FILE *open_file_write(const char *filename) {
    FILE *f = fopen(filename, FILE_WRITE_MODE);
    if (!f) {
        fprintf(stderr, ERROR_CANNOT_OPEN_FILE_WRITE, filename);
        return NULL;
    }
    return f;
}

FILE *open_file_write_with_suffix(const char *base_filename, const char *suffix) {
    size_t base_len;
    size_t suffix_len;
    char buffer[MAX_LABEL_LENGTH];

    /* check if the base filename and suffix are not null*/
    if (!base_filename || !suffix) {
        return NULL;
    }

    /* find the length of the base filename and suffix*/
    base_len = strlen(base_filename);
    suffix_len = strlen(suffix);

    /* check if the base filename and suffix are under MAX_LABEL_LENGTH -1 chars*/
    if (base_len + suffix_len >= sizeof(buffer)) {
        /* allocate memory*/
        char *dynamic_filename = (char *) malloc(base_len + suffix_len + 1);
        FILE *f;
        /* check if the memory allocation failed*/
        if (!dynamic_filename) {
            fprintf(stderr, MALLOC_FAILED);
            return NULL;
        }
        /* copy the base filename and suffix to the allocated buffer*/
        strcpy(dynamic_filename, base_filename);
        /* concat the base filename and suffix*/
        strcat(dynamic_filename, suffix);
        /* open the new file name for writing*/
        f = open_file_write(dynamic_filename);
        /* free the allocated memory*/
        free(dynamic_filename);
        return f;
    }

    strcpy(buffer, base_filename);
    strcat(buffer, suffix);
    return open_file_write(buffer);
}

/* extract base filename without suffix */
char *extract_base_filename(const char *filename) {
    char *base;
    char *suffix_start;
    size_t len;

    /* check if the filename is not null*/
    if (!filename) {return NULL;}

    /* find the length of the filename*/
    len = strlen(filename);
    /* allocate memory for the base filename*/
    base = malloc(len + 1);
    /* check if the memory allocation failed*/
    if (!base) {
        fprintf(stderr, MALLOC_FAILED);
        return NULL;
    }

    /* copy the filename to the base filename*/
    strcpy(base, filename);

    /* remove the suffix if exists*/
    suffix_start = strrchr(base, DOT_CHAR);
    if (suffix_start) {
        *suffix_start = NULL_CHAR;
    }

    return base;
}


/**
 * number_to_base4_letters - converts number to base-4 letter encoding
 * @value: integer value to convert
 * returns static string containing base-4 representation
 */
char *number_to_base4_letters(int value) {
    static char result[5];
    unsigned int word = (unsigned int)value;
    int i;

    /* convert to base-4 using 4 digits (for addresses) */
    /* fill from right to left (least significant first) */
    for (i = 3; i >= 0; i--) {
        result[i] = 'a' + (word % 4);
        word /= 4;
    }

    result[4] = '\0';
    return result;
}


/**
 * number_to_base4_code - converts number to base-4 code representation
 * @value: integer value to convert
 * returns static string containing base-4 code
 */
char *number_to_base4_code(int value) {
    static char result[6];
    unsigned int word = (unsigned int)value & 0x3FF; /* 10-bit mask */
    int i;

    /* convert to base-4 using 5 digits (standard base conversion) */
    /* fill from right to left (least significant first) */
    for (i = 4; i >= 0; i--) {
        result[i] = 'a' + (word % 4);
        word /= 4;
    }

    result[5] = '\0';
    return result;
}

/* check if the string is a valid number */
int is_valid_number(const char *str) {
    int i;
    int start_pos;

    /*check if the input is not null and not empty*/
    if (!str || strlen(str) == 0) return FAILURE;

    /* find where the number starts*/
    start_pos = 0;
    if (str[0] == PLUS_SIGN || str[0] == MINUS_SIGN) {
        start_pos = 1;
    }

    /* check that all remaining characters are digits */
    for (i = start_pos; str[i]; i++) {
        if (!isdigit(str[i])) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * find operand mode for operand
 * @param operand: operand string
 * @return addressing mode bit mask, or FAILURE if invalid
 */
int get_operand_mode(const char *operand) {
    int i;
    const char *num_part;
    size_t len;
    int is_label_like;

    /* check if the input is not null*/
    if (!operand) {
        return FAILURE;
    }



    /* check if this is a string (starts and ends with quotes) */
    if (operand[0] == QUOTE_CHAR) {
        /* check if the string is valid*/
        len = strlen(operand);
        if (len >= MIN_STRING_LENGTH && operand[len - 1] == QUOTE_CHAR) {
            return IMMEDIATE;
        } else {
            fprintf(stderr, ERROR_INVALID_OPERAND, operand);
            return FAILURE;
        }
    }

    /* check if this is a register (r0-r7) */
    if (strlen(operand) == REGISTER_NAME_LENGTH && operand[0] == REGISTER_PREFIX_CHAR) {
        if (operand[1] >= MIN_REGISTER_CHAR && operand[1] <= MAX_REGISTER_CHAR) {
            return REGISTER;
        } else {
            fprintf(stderr, ERROR_INVALID_REGISTER, operand);
            return FAILURE;
        }
    }

    /* check if this is immediate addressing (starts with #) */
    if (operand[0] == IMMEDIATE_PREFIX) {
        num_part = operand + 1;
        /* check if there is nothing after the #*/
        if (strlen(num_part) == 0) {
            fprintf(stderr, ERROR_INVALID_IMMEDIATE, operand);
            return FAILURE;
        }
        /* if there is a +/-, skip it*/
        if (*num_part == PLUS_SIGN || *num_part == MINUS_SIGN) {
            num_part++;
        }
        /* check all the next are digits */
        for (i = 0; num_part[i]; i++) {
            if (!isdigit(num_part[i])) {
                fprintf(stderr, ERROR_INVALID_IMMEDIATE, operand);
                return FAILURE;
            }
        }
        return IMMEDIATE;
    }

    /* check if this is matrix access: label[reg][reg] */
    /* check if there is a [ and ]*/
    if (strchr(operand, OPEN_BRACKET) && strchr(operand, CLOSE_BRACKET)) {

        /* check if the matrix is valid by parsing it*/
        if (parse_matrix_access(operand) == SUCCESS) {

            return MATRIX_ACCESS;
        }
        /* if couldn't parse, it's an error */

        fprintf(stderr, ERROR_INVALID_OPERAND, operand);
        return FAILURE;
    }

    /* check if this is direct addressing (simple label) */
    /* direct addressing: any string that starts with letter contains only letters and digits*/
    if (strlen(operand) > 0 && isalpha(operand[0])) {
        is_label_like = 1;

        /* check if it is like a label */
        for (i = 0; operand[i]; i++) {
            if (!isalpha(operand[i]) && !isdigit(operand[i])) {
                /* it's not a label */
                is_label_like = 0;
                break;
            }
        }

        if (is_label_like) {

            return DIRECT;
        }
    }

    /* invalid */
    fprintf(stderr, ERROR_INVALID_OPERAND, operand);
    return FAILURE;
}