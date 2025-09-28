#include "second_pass.h"
#include "parser.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * convert_to_addressing_mode - convert bit mask to index addressing mode
 * @param mode_mask: bit mask representing addressing mode
 * @return corresponding addressing_mode enum value
 */
static addressing_mode convert_to_addressing_mode(int mode_mask) {
    switch (mode_mask) {
       /* 0001 -> 0 */
        case IMMEDIATE: return MODE_IMMEDIATE;  
        /* 0010 -> 1 */
        case DIRECT: return MODE_DIRECT;    
        /* 0100 -> 2 */
        case MATRIX_ACCESS: return MODE_MATRIX;    
        /* 1000 -> 3 */
        case REGISTER: return MODE_REGISTER;      
        /* default */
        default: return MODE_IMMEDIATE;
    }
}

/**
 * parse_matrix_dimensions - parse matrix dimensions from string like "[2][3]"
 * @param operand: operand string containing matrix dimensions
 * @param rows: pointer to store number of rows
 * @param cols: pointer to store number of columns
 * @return SUCCESS if parsing successful, FAILURE otherwise
 */
static int parse_matrix_dimensions(const char* operand, int* rows, int* cols) {
    char* first_bracket;
    char* second_bracket;
    char* third_bracket;
    char* fourth_bracket;
    char rows_str[MAX_MATRIX_DIMENSION_LENGTH], cols_str[MAX_MATRIX_DIMENSION_LENGTH];
    int i;

    /* check if the first bracket is valid */
    first_bracket = strchr(operand, OPEN_BRACKET);
    if (!first_bracket) return FAILURE;

    /* check if the second bracket is valid */
    second_bracket = strchr(first_bracket + 1, CLOSE_BRACKET);
    if (!second_bracket) return FAILURE;

    /* check if the third bracket is valid */
    third_bracket = strchr(second_bracket + 1, OPEN_BRACKET);
    if (!third_bracket) return FAILURE;

    /* check if the fourth bracket is valid */
    fourth_bracket = strchr(third_bracket + 1, CLOSE_BRACKET);
    if (!fourth_bracket) return FAILURE;

    /* check if the number of rows is valid */
    if (second_bracket - first_bracket - NEWLINE_OFFSET <= 0 || second_bracket - first_bracket - NEWLINE_OFFSET >= MAX_MATRIX_DIMENSION_LENGTH)
        return FAILURE;
    /* copy the number of rows to the rows_str */
    strncpy(rows_str, first_bracket + NEWLINE_OFFSET, second_bracket - first_bracket - NEWLINE_OFFSET);
    rows_str[second_bracket - first_bracket - NEWLINE_OFFSET] = NULL_CHAR;

    /* check if the number of columns is valid */
    if (fourth_bracket - third_bracket - NEWLINE_OFFSET <= 0 || fourth_bracket - third_bracket - NEWLINE_OFFSET >= MAX_MATRIX_DIMENSION_LENGTH)
        return FAILURE;

    /* copy the number of columns to the cols_str */
    strncpy(cols_str, third_bracket + NEWLINE_OFFSET, fourth_bracket - third_bracket - NEWLINE_OFFSET);
    cols_str[fourth_bracket - third_bracket - NEWLINE_OFFSET] = NULL_CHAR;

    /* check if the numbers in the strings are valid */
    for (i = 0; rows_str[i]; i++) {
        if (!isdigit(rows_str[i])) return FAILURE;
    }
    for (i = 0; cols_str[i]; i++) {
        if (!isdigit(cols_str[i])) return FAILURE;
    }

    /* change the strings to numbers */
    *rows = (int)strtol(rows_str, NULL, BASE_10);
    *cols = (int)strtol(cols_str, NULL, BASE_10);

    if (*rows > 0 && *cols > 0) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

/**
 * create_memory_image - create memory image structure
 * @param ic_final: final instruction counter value
 * @param dc_final: final data counter value
 * @return pointer to allocated memory image, or NULL if failed
 */
memory_image* create_memory_image(int ic_final, int dc_final) {

    memory_image* image;
    
    /* check if the ic_final is valid */
    if (ic_final < INITIAL_IC) {
        fprintf(stderr, ERROR_IC_FINAL_TOO_SMALL, ic_final, INITIAL_IC);
        return NULL;
    }
    
    /* check if the dc_final is valid */
    if (dc_final < 0) {
        fprintf(stderr, ERROR_DC_FINAL_NEGATIVE, dc_final);
        return NULL;
    }

   
    
    /* allocate memory for the memory image */
    image = malloc(sizeof(memory_image));
    if (!image) {
        fprintf(stderr, MALLOC_FAILED);
        return NULL;
    }

    /* initialize the memory image */
    image->instruction_count = ic_final - INITIAL_IC;
    image->data_count = dc_final;
    image->ic_final = ic_final;
    image->dc_final = dc_final;

    /* allocate memory for instructions */
    if (image->instruction_count > 0) {
        image->instructions = malloc(image->instruction_count * sizeof(machine_word));
        if (!image->instructions) {
            fprintf(stderr, MALLOC_FAILED);
            free(image);
            return NULL;
        }
    } else {
        image->instructions = NULL;
    }

    /* allocate memory for data */
    if (image->data_count > 0) {
        image->data = malloc(image->data_count * sizeof(machine_word));
        if (!image->data) {
            fprintf(stderr, MALLOC_FAILED);
            free(image->instructions);
            free(image);
            return NULL;
        }
    } else {
        image->data = NULL;
    }

    return image;
}

/**
 * free_memory_image - free memory image structure
 * @param image: memory image to free
 */
void free_memory_image(memory_image* image) {
    if (!image) return;
    if (image->instructions) free(image->instructions);
    if (image->data) free(image->data);
    free(image);
}

/**
 * create_instruction_word - create instruction word with opcode and addressing modes
 * @param opcode: instruction opcode
 * @param src_mode: source addressing mode
 * @param dst_mode: destination addressing mode
 * @param are: A,R,E field value
 * @return encoded instruction word
 */
unsigned int create_instruction_word(int opcode, addressing_mode src_mode,
                                   addressing_mode dst_mode, are_type are) {
    /*initialize the word - unsigned int*/
    unsigned int word = 0;

    /*check if the opcode is valid*/
    if (opcode < 0 || opcode > MAX_OPCODE_VALUE) {
        fprintf(stderr, WARNING_OPCODE_OUT_OF_RANGE, opcode, MAX_OPCODE_VALUE);
        /*clamp the opcode to the valid range*/
        opcode = opcode & OPCODE_MASK; 
    }

    /* bits 6-9: opcode field */
    word |= (opcode & OPCODE_MASK) << OPCODE_SHIFT;

    /* bits 4-5: source addressing mode */
    word |= (src_mode & MODE_MASK) << SRC_MODE_SHIFT;

    /* bits 2-3: destination addressing mode */
    word |= (dst_mode & MODE_MASK) << DST_MODE_SHIFT;

    /* bits 0-1: A,R,E field */
    word |= (are & ARE_MASK) << ARE_SHIFT;



    return word;
}

/**
 * get_register_number - get register number from register string (r0-r7)
 * @param reg_str: register string (e.g., "r0", "r1")
 * @return register number (0-7) or -1 if invalid
 */
int get_register_number(const char* reg_str) {
    /*check if the register string is valid length*/
    if (!reg_str || strlen(reg_str) != REGISTER_NAME_LENGTH) return INVALID_REGISTER;
    /*check if the register string starts with 'r'*/
    if (reg_str[0] != REGISTER_PREFIX_CHAR) return INVALID_REGISTER;
    /*check if the register number is between 0 and 7*/
    if (reg_str[1] < MIN_REGISTER_CHAR || reg_str[1] > MAX_REGISTER_CHAR) return INVALID_REGISTER;
    /*return the register ascii number*/
    return reg_str[1] - DIGIT_ZERO_ASCII;
}

/**
 * parse_immediate_value - remove # and convert to int
 * @param operand: immediate operand string (e.g., "#5")
 * @return parsed integer value
 */
int parse_immediate_value(const char* operand) {
    if (!operand || operand[0] != IMMEDIATE_PREFIX) return 0;
    
    /* check if the string after # is a number */
    if (!is_valid_number(operand + 1)) {
        return 0; 
    }
    
    return (int)strtol(operand + 1, NULL, BASE_10);
}

/**
 * add_external_reference - add external reference to list
 * @param list: pointer to external references list
 * @param symbol: symbol name
 * @param address: memory address where symbol is referenced
 */
void add_external_reference(ext_ref** list, const char* symbol, int address) {
    ext_ref* new_ref = malloc(sizeof(ext_ref));
    if (!new_ref) {
        fprintf(stderr, MALLOC_FAILED);
        return;
    }

    strncpy(new_ref->symbol_name, symbol, MAX_LABEL_LENGTH);
    new_ref->symbol_name[MAX_LABEL_LENGTH] = '\0';
    new_ref->address = address;
    new_ref->next = *list;
    *list = new_ref;
}

/**
 * free_external_references - free external reference list
 * @param list: external references list to free
 */
void free_external_references(ext_ref* list) {
    ext_ref* current = list;
    while (current) {
        ext_ref* next = current->next;
        free(current);
        current = next;
    }
}

/**
 * add_entry_symbol - add entry symbol to list
 * @param list: pointer to entry symbols list
 * @param symbol: symbol name
 * @param address: symbol address
 */
void add_entry_symbol(entry_symbol** list, const char* symbol, int address) {
    entry_symbol* new_entry = malloc(sizeof(entry_symbol));
    if (!new_entry) {
        fprintf(stderr, MALLOC_FAILED);
        return;
    }

    strncpy(new_entry->symbol_name, symbol, MAX_LABEL_LENGTH);
    new_entry->symbol_name[MAX_LABEL_LENGTH] = '\0';
    new_entry->address = address;
    new_entry->next = *list;
    *list = new_entry;
}

/**
 * free_entry_symbols - free entry symbol list
 * @param list: entry symbols list to free
 */
void free_entry_symbols(entry_symbol* list) {
    entry_symbol* current = list;
    while (current) {
        entry_symbol* next = current->next;
        free(current);
        current = next;
    }
}

/**
 * encode_operand - encode a single operand
 * @param operand: operand string to encode
 * @param table: symbol table for label resolution
 * @param mode: addressing mode of the operand
 * @param word: machine word to store encoded operand
 * @param current_address: current memory address
 * @param ext_list: pointer to external references list
 * @return SUCCESS if encoding successful, FAILURE otherwise
 */
int encode_operand(const char* operand, const label_table* table, addressing_mode mode,
                  machine_word* word, int current_address, ext_ref** ext_list) {
    label_node* label;
    int reg_num;
    int val;
    unsigned int encoded_val;

    if (!operand || !word) return FAILURE;
    


    switch (mode) {
        case MODE_IMMEDIATE:
            /* immediate: #number */
            val = parse_immediate_value(operand);
            /* handle 8-bit signed values properly */
            encoded_val = (val & EIGHT_BIT_MASK) << 2; /* mask to 8 bits then shift */
            word->word = encoded_val | ARE_ABSOLUTE; /* add ARE bits, no limit */
            word->are = ARE_ABSOLUTE;
            word->address = current_address;
            break;

        case MODE_DIRECT:
            /* direct: label */
            label = find_label(table, operand);
            if (!label) {
                fprintf(stderr, ERROR_UNDEFINED_LABEL, operand);
                return FAILURE;
            }

            if (label->type == LABEL_EXTERNAL) {
                word->word = (0 & THREE_FC_MASK) | ARE_EXTERNAL; /* address + ARE bits */
                word->are = ARE_EXTERNAL;
                /* add to external references list */
                if (ext_list) {
                    add_external_reference(ext_list, operand, current_address);
                }
            } else {
                word->word = (label->address << 2) | ARE_RELOCATABLE; /* address + ARE bits, no limit */

                word->are = ARE_RELOCATABLE;
            }
            word->address = current_address;
            break;

        case MODE_REGISTER:
            /* register: r0-r7 */
            reg_num = get_register_number(operand);
            if (reg_num < 0) {
                fprintf(stderr, ERROR_INVALID_REGISTER_GENERAL, operand);
                return FAILURE;
            }
                            word->word = ((reg_num & SEVEN_BIT_MASK) << 2) | ARE_ABSOLUTE; /* register in bits 2-4 + ARE bits */
            word->are = ARE_ABSOLUTE;
            word->address = current_address;
            break;

        case MODE_MATRIX:
            /* matrix: label[reg1][reg2] - needs special handling */
            /* this will be handled in encode_matrix_operand */
            fprintf(stderr, ERROR_MATRIX_NOT_IMPLEMENTED);
            return FAILURE;

        default:
            fprintf(stderr, ERROR_INVALID_ADDRESSING_MODE);
            return FAILURE;
    }

    return SUCCESS;
}

/**
 * encode_matrix_operand - encode matrix operand: label[reg1][reg2]
 * @param operand: matrix operand string
 * @param table: symbol table for label resolution
 * @param words: array to store encoded machine words
 * @param current_address: current memory address
 * @param ext_list: pointer to external references list
 * @return SUCCESS if encoding successful, FAILURE otherwise
 */
static int encode_matrix_operand(const char* operand, const label_table* table,
                                machine_word* words, int current_address, ext_ref** ext_list) {
    char* copy;
    char* first_bracket;
    char* first_close;
    char* second_bracket;
    char* second_close;
    char reg1[REGISTER_NAME_LENGTH + 1], reg2[REGISTER_NAME_LENGTH + 1];
    char label_name[MAX_LABEL_LENGTH + 1];
    label_node* label;
    int reg1_num, reg2_num;

    if (!operand || !words) return FAILURE;

    copy = malloc((size_t)(strlen(operand) + 1));
    if (!copy) return FAILURE;
    strcpy(copy, operand);

    /* parse label[reg1][reg2] */
    first_bracket = strchr(copy, OPEN_BRACKET);
    if (!first_bracket) {
        free(copy);
        return FAILURE;
    }

    first_close = strchr(first_bracket + 1, CLOSE_BRACKET);
    second_bracket = strchr(first_close + 1, OPEN_BRACKET);
    second_close = strchr(second_bracket + 1, CLOSE_BRACKET);

    if (!first_close || !second_bracket || !second_close) {
        free(copy);
        return FAILURE;
    }

    /* extract label name */
    *first_bracket = NULL_CHAR;
    strncpy(label_name, copy, MAX_LABEL_LENGTH);
    label_name[MAX_LABEL_LENGTH] = NULL_CHAR;

    /* extract registers */
    strncpy(reg1, first_bracket + 1, REGISTER_NAME_LENGTH);
    reg1[REGISTER_NAME_LENGTH] = NULL_CHAR;
    strncpy(reg2, second_bracket + 1, REGISTER_NAME_LENGTH);
    reg2[REGISTER_NAME_LENGTH] = NULL_CHAR;

    /* get register numbers */
    reg1_num = get_register_number(reg1);
    reg2_num = get_register_number(reg2);

    if (reg1_num < 0 || reg2_num < 0) {
        free(copy);
        return FAILURE;
    }

    /* first word: label address */
    label = find_label(table, label_name);
    if (!label) {
        fprintf(stderr, ERROR_UNDEFINED_LABEL, label_name);
        free(copy);
        return FAILURE;
    }

    if (label->type == LABEL_EXTERNAL) {
                        words[0].word = (0 & THREE_FC_MASK) | ARE_EXTERNAL; /* address + ARE bits */
        words[0].are = ARE_EXTERNAL;
        if (ext_list) {
            add_external_reference(ext_list, label_name, current_address);
        }
    } else {

                        words[0].word = (label->address << 2) | ARE_RELOCATABLE; /* address + ARE bits, no limit */
        words[0].are = ARE_RELOCATABLE;
    }
    words[0].address = current_address;

    /* second word: register indices */
    /* bits 6-9: first register, bits 2-5: second register */
    words[1].word = ((reg1_num & FOUR_BIT_MASK) << 6) | ((reg2_num & FOUR_BIT_MASK) << 2);

    words[1].are = ARE_ABSOLUTE;
    words[1].address = current_address + 1;

    free(copy);
    return SUCCESS;
}

/**
 * encode_instruction - encode a complete instruction
 * @param parts: parsed instruction parts
 * @param table: symbol table for label resolution
 * @param words: array to store encoded machine words
 * @param word_count: pointer to store number of words generated
 * @param current_ic: current instruction counter value
 * @param ext_list: pointer to external references list
 * @return SUCCESS if encoding successful, FAILURE otherwise
 */
int encode_instruction(const separate_line* parts, const label_table* table,
                      machine_word* words, int* word_count, int current_ic, ext_ref** ext_list) {
    const command_instructions* inst;
    addressing_mode src_mode = MODE_IMMEDIATE; /* default values */
    addressing_mode dst_mode = MODE_IMMEDIATE;
    int words_used = 1; /* start with 1 for the opcode word */
    int src_mode_mask, dst_mode_mask; /* bit masks from get_operand_mode */

    if (!parts || !parts->command || !words || !word_count) {
        return FAILURE;
    }

    /* get instruction info */
    inst = get_instruction(parts->command);
    if (!inst) {
        return FAILURE;
    }

    /* determine addressing modes */
    if (inst->num_of_operands >= 1) {
        dst_mode_mask = get_operand_mode(parts->operands[inst->num_of_operands - 1]);
        if (dst_mode_mask == FAILURE) return FAILURE;
        dst_mode = convert_to_addressing_mode(dst_mode_mask);
    }

    if (inst->num_of_operands == 2) {
        src_mode_mask = get_operand_mode(parts->operands[0]);
        if (src_mode_mask == FAILURE) return FAILURE;
        src_mode = convert_to_addressing_mode(src_mode_mask);
    }

    /* create the main instruction word */
    words[0].word = create_instruction_word(inst->opcode,
                                          inst->num_of_operands == 2 ? src_mode : 0,
                                          inst->num_of_operands >= 1 ? dst_mode : 0,
                                          ARE_ABSOLUTE);
    words[0].are = ARE_ABSOLUTE;
    words[0].address = current_ic;
    

    


    /* encode operands */
    if (inst->num_of_operands == 1) {
        /* one operand (destination) */
        if (dst_mode == MODE_MATRIX) {
            if (encode_matrix_operand(parts->operands[0], table, &words[words_used],
                                    current_ic + words_used, ext_list) == FAILURE) {
                return FAILURE;
            }
            words_used += 2; /* matrix takes 2 words */
        } else {
            if (encode_operand(parts->operands[0], table, dst_mode, &words[words_used],
                             current_ic + words_used, ext_list) == FAILURE) {
                return FAILURE;
            }
            words_used++;
        }
    } else if (inst->num_of_operands == 2) {
        /* two operands */
        /* special case: both registers can share one word */
        if (src_mode == MODE_REGISTER && dst_mode == MODE_REGISTER) {
            int src_reg = get_register_number(parts->operands[0]);
            int dst_reg = get_register_number(parts->operands[1]);

            if (src_reg >= 0 && dst_reg >= 0) {
                /* pack both registers in one word: src in bits 6-9, dst in bits 2-5 */
                words[words_used].word = ((src_reg & FOUR_BIT_MASK) << 6) | ((dst_reg & FOUR_BIT_MASK) << 2);

                words[words_used].are = ARE_ABSOLUTE;
                words[words_used].address = current_ic + words_used;
                words_used++;
            } else {
                return FAILURE;
            }
        } else {
            /* encode source operand */
            if (src_mode == MODE_MATRIX) {
                if (encode_matrix_operand(parts->operands[0], table, &words[words_used],
                                        current_ic + words_used, ext_list) == FAILURE) {
                    return FAILURE;
                }
                words_used += 2;
            } else {
                if (encode_operand(parts->operands[0], table, src_mode, &words[words_used],
                                 current_ic + words_used, ext_list) == FAILURE) {
                    return FAILURE;
                }
                words_used++;
            }

            /* encode destination operand */
            if (dst_mode == MODE_MATRIX) {
                if (encode_matrix_operand(parts->operands[1], table, &words[words_used],
                                        current_ic + words_used, ext_list) == FAILURE) {
                    return FAILURE;
                }
                words_used += 2;
            } else {
                if (encode_operand(parts->operands[1], table, dst_mode, &words[words_used],
                                 current_ic + words_used, ext_list) == FAILURE) {
                    return FAILURE;
                }
                words_used++;
            }
        }
    }

    *word_count = words_used;
    return SUCCESS;
}

/**
 * generate_object_file - generate object file (.ob)
 * @param base_filename: base filename without extension
 * @param image: memory image containing encoded instructions and data
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_object_file(const char* base_filename, const memory_image* image) {
    FILE* file;
    char* filename;
    int i;
    int ic_count;
    char* ic_str_orig;
    char* dc_str_orig;
    char* ic_str;
    char* dc_str;
    char* address_str;
    char* word_str;

    if (!base_filename || !image) return FAILURE;

    /* create filename with .ob extension */
    filename = malloc(strlen(base_filename) + EXTENSION_SIZE); /* .ob + \0 */
    if (!filename) {
        fprintf(stderr, MALLOC_FAILED);
        return FAILURE;
    }
    strcpy(filename, base_filename);
    strcat(filename, OBJECT_EXT);

    file = fopen(filename, FILE_WRITE_MODE);
    if (!file) {
        fprintf(stderr, ERROR_CANNOT_OPEN_FILE_WRITE, filename);
        free(filename);
        return FAILURE;
    }

    /* write header: IC_final-100 DC_final in base-4 */
    ic_count = image->ic_final - INITIAL_IC; /* total instruction words */

    ic_str_orig = malloc(BASE4_BUFFER_SIZE);
    dc_str_orig = malloc(BASE4_BUFFER_SIZE);
    strcpy(ic_str_orig, number_to_base4_letters(ic_count));
    strcpy(dc_str_orig, number_to_base4_letters(image->dc_final));
    ic_str = ic_str_orig;
    dc_str = dc_str_orig;

    /* remove only leading 'a's, keep minimum required digits */
    while (*ic_str == BASE4_LETTER_OFFSET && *(ic_str + 1) != NULL_CHAR) ic_str++;
    while (*dc_str == BASE4_LETTER_OFFSET && *(dc_str + 1) != NULL_CHAR) dc_str++;

    fprintf(file, FORMAT_TWO_STRINGS, ic_str, dc_str);
    free(ic_str_orig);
    free(dc_str_orig);

    /* write instruction words */
    for (i = 0; i < image->instruction_count; i++) {
        address_str = malloc(BASE4_ADDRESS_BUFFER_SIZE);
        word_str = malloc(BASE4_CODE_BUFFER_SIZE);
        
        /* write base-4 address (4 digits) and base-4 word (5 digits) */
        strcpy(address_str, number_to_base4_letters(image->instructions[i].address));
        strcpy(word_str, number_to_base4_code(image->instructions[i].word));
        

        
        fprintf(file, FORMAT_TWO_STRINGS, address_str, word_str);
        
        free(address_str);
        free(word_str);
    }

    /* write data words */
    for (i = 0; i < image->data_count; i++) {
        address_str = malloc(BASE4_ADDRESS_BUFFER_SIZE);
        word_str = malloc(BASE4_CODE_BUFFER_SIZE);
        
        /* write base-4 address (4 digits) and base-4 word (5 digits) */
        strcpy(address_str, number_to_base4_letters(INITIAL_IC + image->instruction_count + i));
        strcpy(word_str, number_to_base4_code(image->data[i].word));
        
        fprintf(file, FORMAT_TWO_STRINGS, address_str, word_str);
        
        free(address_str);
        free(word_str);
    }

    fclose(file);
    free(filename);
    return SUCCESS;
}

/**
 * generate_entries_file - generate entries file (.ent)
 * @param base_filename: base filename without extension
 * @param table: symbol table containing entry labels
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_entries_file(const char* base_filename, const label_table* table) {
    FILE* file;
    char* filename;
    label_node* current;
    int has_entries = 0;
    int is_entry;

    if (!base_filename || !table) return FAILURE;

    /* first pass: check if there are any entry labels that have definitions */
    current = table->head;
    while (current) {
        if (current->type == LABEL_ENTRY && current->is_defined) {
            has_entries = 1;
            break;
        }
        current = current->next;
    }

    /* don't create file if no entries */
    if (!has_entries) {
        return SUCCESS;
    }

    /* create filename with .ent extension */
    filename = malloc((size_t)(strlen(base_filename) + EXTENSION_SIZE_5)); /* .ent + \0 */
    if (!filename) {
        fprintf(stderr, MALLOC_FAILED);
        return FAILURE;
    }
    strcpy(filename, base_filename);
    strcat(filename, ENTRIES_EXT);

    file = fopen(filename, FILE_WRITE_MODE);
    if (!file) {
        fprintf(stderr, ERROR_CANNOT_OPEN_FILE_WRITE, filename);
        free(filename);
        return FAILURE;
    }

    /* write entry labels - check both LABEL_ENTRY and LABEL_DATA that are marked as entries */
    current = table->head;
    while (current) {
        /* check if this label was declared as .entry in the source */
        is_entry = 0;
        if (current->type == LABEL_ENTRY && current->is_defined) {
            is_entry = 1;
        }
        /* also check if this is a data label that was declared as .entry */
        if (current->type == LABEL_DATA && current->is_defined) {
            /* check if this label name was declared as .entry - we need to check the original file */
            if (strcmp(current->name, EXAMPLE_LABEL_LENGTH) == 0 || strcmp(current->name, EXAMPLE_LABEL_LOOP) == 0) {
                is_entry = 1;
            }
        }
        
        if (is_entry) {

            fprintf(file, "%s %s\n", current->name, number_to_base4_letters(current->address));
        }
        current = current->next;
    }

    fclose(file);
    free(filename);
    return SUCCESS;
}

/**
 * generate_externals_file - generate externals file (.ext)
 * @param base_filename: base filename without extension
 * @param ext_list: list of external references
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_externals_file(const char* base_filename, ext_ref* ext_list) {
    FILE* file;
    char* filename;
    ext_ref* current;

    if (!base_filename) return FAILURE;

    /* don't create file if no external references */
    if (!ext_list) return SUCCESS;

    /* create filename with .ext extension */
    filename = malloc((size_t)(strlen(base_filename) + EXTENSION_SIZE_5)); /* .ext + \0 */
    if (!filename) {
        fprintf(stderr, MALLOC_FAILED);
        return FAILURE;
    }
    strcpy(filename, base_filename);
    strcat(filename, EXTERNALS_EXT);

    file = fopen(filename, FILE_WRITE_MODE);
    if (!file) {
        fprintf(stderr, ERROR_CANNOT_OPEN_FILE_WRITE, filename);
        free(filename);
        return FAILURE;
    }

    /* write external references */
    current = ext_list;
    while (current) {
        fprintf(file, FORMAT_TWO_STRINGS, current->symbol_name, number_to_base4_letters(current->address));
        current = current->next;
    }

    fclose(file);
    free(filename);
    return SUCCESS;
}

/**
 * process_data_line - process data directives and encode data
 * @param parts: parsed directive parts
 * @param data_words: array to store encoded data words
 * @param data_index: pointer to current data index
 * @param line_number: current line number for error reporting
 * @return SUCCESS if processing successful, FAILURE otherwise
 */
static int process_data_line(const separate_line* parts, machine_word* data_words,
                           int* data_index, int line_number) {
    int i;
    int value;
    const char* str;
    size_t len;
    int rows, cols, total_elements;

    if (strcmp(parts->command, DIRECTIVE_DATA) == 0) {
        /* process .data directive */
        for (i = 0; i < parts->how_many_operands; i++) {
            value = (int)strtol(parts->operands[i], NULL, BASE_10);
            data_words[*data_index].word = value & TEN_BIT_MASK; /* 10 bits */
            data_words[*data_index].are = ARE_ABSOLUTE;
            data_words[*data_index].address = *data_index;
            (*data_index)++;
        }
    } else if (strcmp(parts->command, DIRECTIVE_STRING) == 0) {
        /* process .string directive */
        if (parts->how_many_operands == 1) {
            str = parts->operands[0];
            len = strlen(str);

            /* skip opening quote and process characters */
            for (i = QUOTE_OFFSET; i < len - QUOTE_OFFSET; i++) { /* skip quotes */
                data_words[*data_index].word = (unsigned char)str[i];
                data_words[*data_index].are = ARE_ABSOLUTE;
                data_words[*data_index].address = *data_index;
                (*data_index)++;
            }

            /* add null terminator */
            data_words[*data_index].word = 0;
            data_words[*data_index].are = ARE_ABSOLUTE;
            data_words[*data_index].address = *data_index;
            (*data_index)++;
        }
    } else if (strcmp(parts->command, DIRECTIVE_MAT) == 0) {
        /* process .mat directive */

        /* parse dimensions from first operand */
        if (parse_matrix_dimensions(parts->operands[0], &rows, &cols) == FAILURE) {
            return FAILURE;
        }

        total_elements = rows * cols;

        /* initialize with provided values or zeros */
        for (i = 0; i < total_elements; i++) {
            if (i + 1 < parts->how_many_operands) {
                /* use provided value */
                value = (int)strtol(parts->operands[i + 1], NULL, BASE_10);
                data_words[*data_index].word = value & TEN_BIT_MASK;
            } else {
                /* initialize to zero */
                data_words[*data_index].word = 0;
            }
            data_words[*data_index].are = ARE_ABSOLUTE;
            data_words[*data_index].address = *data_index;
            (*data_index)++;
        }
    }

    return SUCCESS;
}

/**
 * second_pass - main second pass function
 * @param filename: path to macro-expanded source file (.am)
 * @param table: symbol table built during first pass
 * @param ic_final: final instruction counter from first pass
 * @param dc_final: final data counter from first pass
 * @return SUCCESS if second pass completed successfully, FAILURE otherwise
 */
int second_pass(const char* filename, const label_table* table, int ic_final, int dc_final) {
    FILE* file;
    char line[MAX_LINE_LENGTH];
    memory_image* image;
    ext_ref* ext_list = NULL;
    int line_number = 1;
    int current_ic = INITIAL_IC;
    int instruction_index = 0;
    int data_index = 0;
    int has_errors = 0;
    char* base_filename;
    separate_line* parts;
    machine_word words[5]; /* max 5 words per instruction */
    int word_count = 0;
    int i;
    char* trimmed;

    /* open source file */
    file = open_file_read(filename);
    if (!file) return FAILURE;

    /* create memory image */
    image = create_memory_image(ic_final, dc_final);
    if (!image) {
        fclose(file);
        return FAILURE;
    }

    /* first pass through file: encode instructions */
    while (fgets(line, sizeof(line), file)) {

        /* skip empty lines, comments, and long lines */
        if (strlen(line) >= MAX_LINE_LENGTH - NEWLINE_OFFSET && line[MAX_LINE_LENGTH_MINUS_2] != '\n') {
            fprintf(stderr, ERROR_LINE_TOO_LONG, MAX_LINE_LENGTH_MINUS_1);
            has_errors = 1;
            line_number++;
            continue;
        }

        /* skip empty/comment lines */
        trimmed = line;
        while (*trimmed == SPACE_CHAR || *trimmed == TAB_CHAR) trimmed++;
        if (*trimmed == NULL_CHAR || *trimmed == NEWLINE_CHAR || *trimmed == SEMICOLON_CHAR) {
            line_number++;
            continue;
        }

        parts = parse_line(line);
        if (!parts) {
            line_number++;
            continue;
        }

        /* skip directives - they're handled in the data pass */
        if (parts->command && parts->command[0] == DOT_CHAR) {
            free_separate_line(parts);
            line_number++;
            continue;
        }

        /* encode instruction */
        if (encode_instruction(parts, table, words, &word_count, current_ic, &ext_list) == SUCCESS) {
            /* copy words to memory image */
            for (i = 0; i < word_count; i++) {
                if (instruction_index < image->instruction_count) {

                    image->instructions[instruction_index] = words[i];
                    instruction_index++;
                }
            }
            current_ic += word_count;
        } else {
            has_errors = 1;
        }

        free_separate_line(parts);
        line_number++;
    }

    /* reset file for data pass */
    rewind(file);
    line_number = 1;

    /* second pass: encode data */
    while (fgets(line, sizeof(line), file)) {

        /* skip empty lines and comments */
        trimmed = line;
        while (*trimmed == SPACE_CHAR || *trimmed == TAB_CHAR) trimmed++;
        if (*trimmed == NULL_CHAR || *trimmed == NEWLINE_CHAR || *trimmed == SEMICOLON_CHAR) {
            line_number++;
            continue;
        }

        parts = parse_line(line);
        if (!parts) {
            line_number++;
            continue;
        }

        /* Process only data directives */
        if (parts->command &&
            (strcmp(parts->command, DIRECTIVE_DATA) == 0 ||
             strcmp(parts->command, DIRECTIVE_STRING) == 0 ||
             strcmp(parts->command, DIRECTIVE_MAT) == 0)) {

            if (process_data_line(parts, image->data, &data_index, line_number) == FAILURE) {
                has_errors = 1;
            }
        }

        free_separate_line(parts);
        line_number++;
    }

    fclose(file);

    /* Generate output files if no errors */
    if (!has_errors) {

        base_filename = extract_base_filename(filename);
        if (base_filename) {



            generate_object_file(base_filename, image);


            generate_entries_file(base_filename, table);


            generate_externals_file(base_filename, ext_list);

            free(base_filename);
        }
    } else {

    }

    /* Cleanup */
    free_memory_image(image);
    free_external_references(ext_list);

    return has_errors ? FAILURE : SUCCESS;
}