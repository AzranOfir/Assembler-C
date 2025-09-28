#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "labelTable.h"
#include "utils.h"

/* hardcoded label name definitions */
#define EXAMPLE_LABEL_LENGTH "LENGTH"
#define EXAMPLE_LABEL_LOOP "LOOP"

/* machine word structure for storing encoded instructions */
typedef struct {
    unsigned int word;      /* 10-bit machine word */
    are_type are;          /* A,R,E field value */
    int address;           /* memory address of this word */
} machine_word;

/* memory image structures */
typedef struct {
    machine_word *instructions;    /* instruction memory */
    machine_word *data;           /* data memory */
    int instruction_count;        /* number of instruction words */
    int data_count;              /* number of data words */
    int ic_final;                /* final IC value */
    int dc_final;                /* final DC value */
} memory_image;

/* external reference structure for .ext file */
typedef struct ext_ref {
    char symbol_name[MAX_LABEL_LENGTH + 1];
    int address;
    struct ext_ref *next;
} ext_ref;

/* entry symbol structure for .ent file */
typedef struct entry_symbol {
    char symbol_name[MAX_LABEL_LENGTH + 1];
    int address;
    struct entry_symbol *next;
} entry_symbol;

/* function declarations */

/**
 * second_pass - main second pass function
 * @param filename: path to macro-expanded source file (.am)
 * @param table: symbol table built during first pass
 * @param ic_final: final instruction counter from first pass
 * @param dc_final: final data counter from first pass
 * @return SUCCESS if second pass completed successfully, FAILURE otherwise
 */
int second_pass(const char* filename, const label_table* table, int ic_final, int dc_final);

/* memory image management */
/**
 * create_memory_image - create memory image structure
 * @param ic_final: final instruction counter value
 * @param dc_final: final data counter value
 * @return pointer to allocated memory image, or NULL if failed
 */
memory_image* create_memory_image(int ic_final, int dc_final);

/**
 * free_memory_image - free memory image structure
 * @param image: memory image to free
 */
void free_memory_image(memory_image* image);

/* instruction encoding */
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
                      machine_word* words, int* word_count, int current_ic, ext_ref** ext_list);

/* operand encoding */
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
                  machine_word* word, int current_address, ext_ref** ext_list);

/* output file generation */
/**
 * generate_object_file - generate object file (.ob)
 * @param base_filename: base filename without extension
 * @param image: memory image containing encoded instructions and data
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_object_file(const char* base_filename, const memory_image* image);

/**
 * generate_entries_file - generate entries file (.ent)
 * @param base_filename: base filename without extension
 * @param table: symbol table containing entry labels
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_entries_file(const char* base_filename, const label_table* table);

/**
 * generate_externals_file - generate externals file (.ext)
 * @param base_filename: base filename without extension
 * @param ext_list: list of external references
 * @return SUCCESS if file generated successfully, FAILURE otherwise
 */
int generate_externals_file(const char* base_filename, ext_ref* ext_list);

/* utility functions */
/**
 * create_instruction_word - create instruction word with opcode and addressing modes
 * @param opcode: instruction opcode
 * @param src_mode: source addressing mode
 * @param dst_mode: destination addressing mode
 * @param are: A,R,E field value
 * @return encoded instruction word
 */
unsigned int create_instruction_word(int opcode, addressing_mode src_mode,
                                   addressing_mode dst_mode, are_type are);

/**
 * get_register_number - get register number from register string (r0-r7)
 * @param reg_str: register string (e.g., "r0", "r1")
 * @return register number (0-7) or -1 if invalid
 */
int get_register_number(const char* reg_str);

/**
 * parse_immediate_value - remove # and convert to int
 * @param operand: immediate operand string (e.g., "#5")
 * @return parsed integer value
 */
int parse_immediate_value(const char* operand);

/* external reference management */
/**
 * add_external_reference - add external reference to list
 * @param list: pointer to external references list
 * @param symbol: symbol name
 * @param address: memory address where symbol is referenced
 */
void add_external_reference(ext_ref** list, const char* symbol, int address);

/**
 * free_external_references - free external reference list
 * @param list: external references list to free
 */
void free_external_references(ext_ref* list);

/* entry symbol management */
/**
 * add_entry_symbol - add entry symbol to list
 * @param list: pointer to entry symbols list
 * @param symbol: symbol name
 * @param address: symbol address
 */
void add_entry_symbol(entry_symbol** list, const char* symbol, int address);

/**
 * free_entry_symbols - free entry symbol list
 * @param list: entry symbols list to free
 */
void free_entry_symbols(entry_symbol* list);

#endif /* SECOND_PASS_H */