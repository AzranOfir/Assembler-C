#include "first_pass.h"
#include "utils.h"
#include "parser.h"
#include "labelTable.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>

/* error messages */
#define ERROR_MAT_REQUIRES_DIMENSION "Error: .mat directive requires at least dimension specification\n"
#define ERROR_INVALID_MATRIX_DIMENSIONS "Error: invalid matrix dimensions in '%s'\n"
#define ERROR_MATRIX_VALUES_MISMATCH "Error: matrix expects 0 or %d values, got %d\n"
#include <ctype.h>
#include <stdlib.h>

/* helper functions */

/**
 * is_data_directive - check if command is .data directive
 * @param cmd: command string to check
 * @return YES if .data directive, NO otherwise
 */
static int is_data_directive(const char* cmd) {
    if (cmd == NULL) {
        return NO;
    }
    if (strcmp(cmd, DIRECTIVE_DATA) == 0)
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

/**
 * is_string_directive - check if command is .string directive
 * @param cmd: command string to check
 * @return YES if .string directive, NO otherwise
 */
static int is_string_directive(const char* cmd) {
    if (cmd == NULL) {
        return NO;
    }
    if (strcmp(cmd, DIRECTIVE_STRING) == 0) {
        return YES;
    } else {
        return NO;
    }
}

/**
 * is_mat_directive - check if command is .mat directive
 * @param cmd: command string to check
 * @return YES if .mat directive, NO otherwise
 */
static int is_mat_directive(const char* cmd) {
    if (cmd == NULL) {
        return NO;
    }
    if (strcmp(cmd, DIRECTIVE_MAT) == 0) {
        return YES;
    } else {
        return NO;
    }
}

/**
 * is_extern_directive - check if command is .extern directive
 * @param cmd: command string to check
 * @return YES if .extern directive, NO otherwise
 */
static int is_extern_directive(const char* cmd) {
    if (cmd == NULL) {
        return NO;
    }
    if (strcmp(cmd, DIRECTIVE_EXTERN) == 0) {
        return YES;
    } else {
        return NO;
    }
}

/**
 * is_entry_directive - check if command is .entry directive
 * @param cmd: command string to check
 * @return YES if .entry directive, NO otherwise
 */
static int is_entry_directive(const char* cmd) {
    if (cmd == NULL) {
        return NO;
    }
    if (strcmp(cmd, DIRECTIVE_ENTRY) == 0) {
        return YES;
    } else {
        return NO;
    }
}

/**
 * parse_matrix_dimensions - parse matrix dimensions from operand string
 * @param operand: operand string containing [rows][cols] format
 * @param rows: output parameter for number of rows
 * @param cols: output parameter for number of columns
 * @return SUCCESS if parsing successful, FAILURE otherwise
 */
static int parse_matrix_dimensions(const char* operand, int* rows, int* cols) {
    char* first_bracket;
    char* second_bracket;
    char* third_bracket;
    char* fourth_bracket;
    char rows_str[MATRIX_DIMENSION_STRING_LENGTH], cols_str[MATRIX_DIMENSION_STRING_LENGTH];
    int i;

    first_bracket = strchr(operand, OPEN_BRACKET);
    if (!first_bracket) {
        return FAILURE;
    }

    second_bracket = strchr(first_bracket + 1, CLOSE_BRACKET);
    if (!second_bracket) {
        return FAILURE;
    }

    third_bracket = strchr(second_bracket + 1, OPEN_BRACKET);
    if (!third_bracket) {
        return FAILURE;
    }

    fourth_bracket = strchr(third_bracket + 1, CLOSE_BRACKET);
    if (!fourth_bracket) {
        return FAILURE;
    }

    /* extract rows */
    if (second_bracket - first_bracket - 1 <= 0 || second_bracket - first_bracket - 1 >= MATRIX_DIMENSION_STRING_LENGTH)
        return FAILURE;
    strncpy(rows_str, first_bracket + 1, second_bracket - first_bracket - 1);
    rows_str[second_bracket - first_bracket - 1] = NULL_CHAR;

    /* extract cols */
    if (fourth_bracket - third_bracket - 1 <= 0 || fourth_bracket - third_bracket - 1 >= MATRIX_DIMENSION_STRING_LENGTH)
        return FAILURE;
    strncpy(cols_str, third_bracket + 1, fourth_bracket - third_bracket - 1);
    cols_str[fourth_bracket - third_bracket - 1] = NULL_CHAR;

    /* check if rows string is valid */
    for (i = 0; rows_str[i]; i++) {
        if (!isdigit(rows_str[i])) return FAILURE;
    }

    /* validate cols string */
    for (i = 0; cols_str[i]; i++) {
        if (!isdigit(cols_str[i])) return FAILURE;
    }

    *rows = (int)strtol(rows_str, NULL, BASE_10);
    *cols = (int)strtol(cols_str, NULL, BASE_10);

    if (*rows <= 0 || *cols <= 0) return FAILURE;

    return SUCCESS;
}

/**
 * estimate_ic_words - estimate how many words an instruction will consume
 * @param parts: parsed line structure containing command and operands
 * @return number of words the instruction will consume, 0 on error
 */
static int estimate_ic_words(const separate_line* parts) {
    const command_instructions* inst;
    int words = 1;  /* one word for the opcode itself */
    int src_mode = 0, dst_mode = 0;

    if (!parts || !parts->command) return 0;

    inst = get_instruction(parts->command);
    if (!inst) return 0;

    /* instruction with no operands */
    if (inst->num_of_operands == 0) {
        return words;
    }
    /* instruction with one operand */
    else if (inst->num_of_operands == 1) {
        dst_mode = get_operand_mode(parts->operands[0]);
        if (dst_mode == FAILURE) return 0;

        /* matrix access needs 2 additional words */
        if (dst_mode == MATRIX_ACCESS) {
            words += 2;
        } else {
            words += 1;
        }
    }
    /* instruction with two operands */
    else if (inst->num_of_operands == 2) {
        src_mode = get_operand_mode(parts->operands[0]);
        dst_mode = get_operand_mode(parts->operands[1]);
        if (src_mode == FAILURE || dst_mode == FAILURE) return 0;

        /* optimization: register to register can be packed in one word */
        if (src_mode == REGISTER && dst_mode == REGISTER) {
            words += 1;
        } else {
            /* general case: count words for each operand separately */
            if (src_mode == MATRIX_ACCESS) {
                words += 2;  /* matrix access needs 2 words */
            } else {
                words += 1;  /* other modes need one word */
            }

            if (dst_mode == MATRIX_ACCESS) {
                words += 2;  /* matrix access needs 2 words */
            } else {
                words += 1;  /* other modes need one word */
            }
        }
    }

    return words;
}

/**
 * define_label - safely define a label in the table
 * @param table: label table to add/update label in
 * @param name: label name to define
 * @param address: memory address for the label
 * @param type: type of label (LABEL_CODE, LABEL_DATA, etc.)
 * @return SUCCESS if label defined successfully, FAILURE otherwise
 */
static int define_label(label_table* table, const char* name, int address, label_type type) {
    label_node* existing;
    if (!table || !name) {
        return FAILURE;
    }

    existing = find_label(table, name);
    if (existing) {
        if (existing->is_defined) {
            fprintf(stderr, ERROR_LABEL_ALREADY_DEFINED, name);
            return FAILURE;
        }
        existing->address = address;
        /* update type for label definitions, but preserve ENTRY status */
        if (existing->type != LABEL_ENTRY) {
            existing->type = type;
        }
        existing->is_defined = YES;
        return SUCCESS;
    }

    /* try to add the label */
    if (add_label(table, name, address, type) == SUCCESS) {
        if (mark_label_defined(table, name) == SUCCESS) {
            return SUCCESS;
        }
    } else {
        /* if add_label failed, it might be because the label already exists */
        /* try to find it again and update it */
        existing = find_label(table, name);
        if (existing && !existing->is_defined) {
            existing->address = address;
            if (existing->type != LABEL_ENTRY) {
                existing->type = type;
            }
            existing->is_defined = YES;
            return SUCCESS;
        }
    }
    return FAILURE;
}

/**
 * process_line - process a single line during first pass
 * @param line: input line string to process
 * @param table: label table to populate with labels
 * @param IC: instruction counter (incremented for instructions)
 * @param DC: data counter (incremented for data directives)
 * @param line_number: current line number for error reporting
 * @return SUCCESS if line processed successfully, FAILURE otherwise
 */
static int process_line(const char* line, label_table* table, int* IC, int* DC, int line_number) {
    separate_line* parts;
    int had_label = 0;
    int count = 0;
    int i;
    int values_provided = 0;
    int words = 0;
    const char* s;
    size_t len;
    int rows, cols, total_elements;
    label_node* ex;

    /* skip empty/whitespace lines and comment lines starting with ';' */
    if (!line) return SUCCESS;
    while (*line == SPACE_CHAR || *line == TAB_CHAR)
        line++;
    if (*line == CARRIAGE_RETURN_CHAR) line++;
    if (*line == NULL_CHAR || *line == NEWLINE_CHAR || *line == SEMICOLON_CHAR)
        return SUCCESS;

    parts = parse_line(line);
    if (!parts) {
        fprintf(stderr, ERROR_PARSE_FAILED_LINE, line_number);
        return FAILURE;
    }

    if (parts->label) {
        had_label = 1;
    }

    /* .data: numeric constants handling */
    if (is_data_directive(parts->command) == YES) {
        count = 0;
        for (i = 0; i < parts->how_many_operands; i++) {
            const char* op = parts->operands[i];
            if (*op == PLUS_SIGN || *op == MINUS_SIGN)
                op++;
            if (!*op || !isdigit((unsigned char)*op)) {
                fprintf(stderr, ERROR_INVALID_IMMEDIATE_LINE, line_number, parts->operands[i]);
                free_separate_line(parts);
                return FAILURE;
            }
            while (*op) {
                if (!isdigit((unsigned char)*op)) {
                    fprintf(stderr, ERROR_INVALID_IMMEDIATE_LINE, line_number, parts->operands[i]);
                    free_separate_line(parts);
                    return FAILURE;
                }
                op++;
            }
            count++;
        }
        if (had_label) {
            if (define_label(table, parts->label, *DC, LABEL_DATA) == FAILURE) {
                free_separate_line(parts);
                return FAILURE;
            }
        }
        *DC += count;
        free_separate_line(parts);
        return SUCCESS;
    }

    /* .string: a quoted string handling */
    if (is_string_directive(parts->command) == YES) {
        if (parts->how_many_operands != 1 || parts->operands[0][0] != QUOTE_CHAR) {
            fprintf(stderr, ERROR_INVALID_STRING_LINE, line_number, parts->how_many_operands ? parts->operands[0] : "");
            free_separate_line(parts);
            return FAILURE;
        }
        {
            s = parts->operands[0];
            len = strlen(s);
            if (len < 2 || s[len - 1] != QUOTE_CHAR) {
                fprintf(stderr, ERROR_INVALID_STRING_LINE, line_number, s);
                free_separate_line(parts);
                return FAILURE;
            }
            if (had_label) {
                if (define_label(table, parts->label, *DC, LABEL_DATA) == FAILURE) {
                    free_separate_line(parts);
                    return FAILURE;
                }
            }
            *DC += (int)((len >= 2 ? (len - 2) : 0) + 1);
        }
        free_separate_line(parts);
        return SUCCESS;
    }

    /* .mat: matrix definition handling */
    if (is_mat_directive(parts->command) == YES) {
        values_provided = 0;

        if (parts->how_many_operands < 1) {
            fprintf(stderr, ERROR_MAT_REQUIRES_DIMENSION);
            free_separate_line(parts);
            return FAILURE;
        }

        if (parse_matrix_dimensions(parts->operands[0], &rows, &cols) == FAILURE) {
            fprintf(stderr, ERROR_INVALID_MATRIX_DIMENSIONS, parts->operands[0]);
            free_separate_line(parts);
            return FAILURE;
        }

        total_elements = rows * cols;

        for (i = 1; i < parts->how_many_operands; i++) {
            const char* op = parts->operands[i];
            if (*op == PLUS_SIGN || *op == MINUS_SIGN) op++;

            if (!*op || !isdigit((unsigned char)*op)) {
                fprintf(stderr, ERROR_INVALID_IMMEDIATE_LINE, line_number, parts->operands[i]);
                free_separate_line(parts);
                return FAILURE;
            }
            while (*op) {
                if (!isdigit((unsigned char)*op)) {
                    fprintf(stderr, ERROR_INVALID_IMMEDIATE_LINE, line_number, parts->operands[i]);
                    free_separate_line(parts);
                    return FAILURE;
                }
                op++;
            }
            values_provided++;
        }

        if (values_provided != 0 && values_provided != total_elements) {
            fprintf(stderr, ERROR_MATRIX_VALUES_MISMATCH,
                    total_elements, values_provided);
            free_separate_line(parts);
            return FAILURE;
        }

        if (had_label) {
            if (define_label(table, parts->label, *DC, LABEL_DATA) == FAILURE) {
                free_separate_line(parts);
                return FAILURE;
            }
        }

        *DC += total_elements;
        free_separate_line(parts);
        return SUCCESS;
    }

    /* .extern: declare external labels handling */
    if (is_extern_directive(parts->command) == YES) {
        for (i = 0; i < parts->how_many_operands; i++) {
            ex = find_label(table, parts->operands[i]);
            if (ex && ex->is_defined) {
                fprintf(stderr, ERROR_LABEL_ALREADY_DEFINED, parts->operands[i]);
                free_separate_line(parts);
                return FAILURE;
            }
            if (!ex) {
                add_label(table, parts->operands[i], 0, LABEL_EXTERNAL);
            }
        }
        free_separate_line(parts);
        return SUCCESS;
    }

    /* .entry: mark labels as ENTRY handling */
    if (is_entry_directive(parts->command) == YES) {
        for (i = 0; i < parts->how_many_operands; i++) {
            label_node* en = find_label(table, parts->operands[i]);
            if (en) {
                if (en->is_defined) {
                    /* if label is already defined, mark it as entry but keep original type for data labels */
                    if (en->type != LABEL_DATA) {
                        en->type = LABEL_ENTRY;
                    }
                    /* for data labels, keep LABEL_DATA type but mark as entry - we'll handle this in entries file generation */
                } else {
                    /* if label exists but not defined yet, mark it as entry */
                    en->type = LABEL_ENTRY;
                }
            } else {
                /* create a placeholder entry label - it will be updated when the actual label is defined */
                add_label(table, parts->operands[i], 0, LABEL_ENTRY);
            }
        }
        free_separate_line(parts);
        return SUCCESS;
    }

    /* instruction handeling */
    {
        words = estimate_ic_words(parts);
        if (words == 0) {
            free_separate_line(parts);
            return FAILURE;
        }
        if (had_label) {
            if (define_label(table, parts->label, *IC, LABEL_CODE) == FAILURE) {
                free_separate_line(parts);
                return FAILURE;
            }
        }
        *IC += words;
    }

    free_separate_line(parts);
    return SUCCESS;
}

/**
 * first_pass_on_table - run first pass using given label table
 * @param filename: path to macro-expanded source file (.am)
 * @param table: symbol table to populate during first pass
 * @param outIC: output parameter for final instruction counter
 * @param outDC: output parameter for final data counter
 * @return SUCCESS if first pass completed successfully, FAILURE otherwise
 */
int first_pass_on_table(const char* filename, label_table* table, int* outIC, int* outDC) {
    FILE* file;
    char line[MAX_LINE_LENGTH];
    int IC;
    int DC;
    int line_number;
    int has_errors;
    label_node* current;

    file = open_file_read(filename);

    IC = INITIAL_IC;
    DC = INITIAL_DC;
    line_number = INITIAL_LINE_NUMBER;
    has_errors = 0;

    if (!file) {
        return FAILURE;
    }

    if (table == NULL) {
        fclose(file);
        return FAILURE;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[MAX_LINE_LENGTH - 2] != NEWLINE_CHAR) {
            fprintf(stderr, ERROR_LINE_TOO_LONG, MAX_LINE_LENGTH - 1);
            has_errors = 1;
            line_number++;
            continue;
        }

        if (!process_line(line, table, &IC, &DC, line_number)) {
            has_errors = 1;
        }
        line_number++;
    }

    fclose(file);

    /* update data label addresses after first pass */
    if (!has_errors) {

        current = table->head;
        while (current) {
            if (current->type == LABEL_DATA && current->is_defined) {

                /* add IC_final to data addresses to place them after instructions */
                current->address += IC;
            }
            current = current->next;
        }
    }

    if (outIC) *outIC = IC;
    if (outDC) *outDC = DC;

    return has_errors ? FAILURE : SUCCESS;
}

/**
 * first_pass - run the assembler first pass over a source file
 * @param filename: path to macro-expanded source file (.am)
 * @return SUCCESS if first pass completed successfully, FAILURE otherwise
 */
int first_pass(const char* filename) {
    label_table table;
    int rc;
    int IC, DC;
    init_label_table(&table);
    rc = first_pass_on_table(filename, &table, &IC, &DC);
    free_label_table(&table);
    return rc;
}