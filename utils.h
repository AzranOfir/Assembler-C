#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/* basic size and length*/
#define MAX_LINE_LENGTH 81
#define MAX_WORD_LENGTH MAX_LINE_LENGTH
#define MAX_MACRO_NAME 31

/* directive name*/
#define DIRECTIVE_DATA ".data"
#define DIRECTIVE_STRING ".string"
#define DIRECTIVE_MAT ".mat"
#define DIRECTIVE_EXTERN ".extern"
#define DIRECTIVE_ENTRY ".entry"
#define MAX_LABEL_LENGTH 31
#define MAX_MACRO_BODY 1000
#define MAX_OPERANDS 1000

/* character constants for parsing */
#define SPACE_CHAR ' '
#define TAB_CHAR '\t'
#define NEWLINE_CHAR '\n'
#define NULL_CHAR '\0'
#define COLON ':'
#define DOT_CHAR '.'
#define OPEN_BRACKET '['
#define CLOSE_BRACKET ']'
#define COMMA_CHAR ','
#define QUOTE_CHAR '"'
#define SEMICOLON_CHAR ';'
#define CARRIAGE_RETURN_CHAR '\r'

/* macro processing keywords */
#define MCRO_KEYWORD "mcro"
#define MCROEND_KEYWORD "mcroend"
#define MCRO_LENGTH 4
#define MCRO_SPACE_OFFSET 5

/* returns value */
#define SUCCESS 1
#define FAILURE 0
#define YES 1
#define NO 0
#define EXIT_FAILURE_CODE 1

/* memory layout */
#define INITIAL_IC 100
#define INITIAL_DC 0
#define INITIAL_LINE_NUMBER 1

/* bit manipulation */
#define TEN_BIT_MASK 0x3FF          /* mask for 10-bit*/
#define TWO_BIT_MASK 0x3            /* mask for 2-bit */
#define FOUR_BIT_MASK 0xF           /* mask for 4-bit */
#define EIGHT_BIT_MASK 0xFF         /* mask for 8-bit */
#define SEVEN_BIT_MASK 0x7          /* mask for 7-bit */
#define THREE_FC_MASK 0x3FC         /* mask for 10-bit with 2-bit alignment */
#define NEWLINE_OFFSET 1            /* offset for newline character */
#define QUOTE_OFFSET 1              /* offset for quote characters */
#define EXTENSION_SIZE 4            /* size for file extension (.ob) */
#define EXTENSION_SIZE_5 5          /* size for file extension (.ent, .ext) */
#define BASE4_BUFFER_SIZE 5         /* buffer size for base-4 conversion */
#define BASE4_CODE_BUFFER_SIZE 6    /* buffer size for base-4 code */
#define BASE4_ADDRESS_BUFFER_SIZE 5 /* buffer size for base-4 address */
#define MAX_LINE_LENGTH_MINUS_1 (MAX_LINE_LENGTH - 1)  /* max line length minus 1 */
#define MAX_LINE_LENGTH_MINUS_2 (MAX_LINE_LENGTH - 2)  /* max line length minus 2 */

/* numeric constants */
#define BASE_10 10
#define REGISTER_NAME_LENGTH 2
#define MATRIX_DIMENSION_STRING_LENGTH 10
#define MIN_REGISTER_NUMBER 0
#define MAX_REGISTER_NUMBER 7

/* character constants */
#define REGISTER_PREFIX_CHAR 'r'
#define MIN_REGISTER_CHAR '0'
#define MAX_REGISTER_CHAR '7'
#define DIGIT_ZERO_ASCII '0'
#define BASE4_LETTER_OFFSET 'a'     /* base4 encoding starts with 'a' */
#define START_SHIFT 8               /* starting bit shift position */
#define BASE4_DIGITS_COUNT 5        /* number of base4 digits */
#define BASE4_ADDRESS_DIGITS 4      /* number of base4 digits - addresses */
#define BASE4_CODE_DIGITS 5         /* number of base4 digits - machine code */
#define BASE4_RADIX 4               /* base-4 number system */

/* instruction word bit positions and shifts */
#define OPCODE_SHIFT 6       /* bits 6-9: opcode field */
#define SRC_MODE_SHIFT 4     /* bits 4-5: source addressing mode */
#define DST_MODE_SHIFT 2     /* bits 2-3: destination addressing mode */
#define ARE_SHIFT 0          /* bits 0-1: A,R,E field */

/* instruction word bit field sizes */
#define OPCODE_BITS 4        /* opcode field size */
#define MODE_BITS 2          /* addressing mode field size */
#define ARE_BITS 2           /* A,R,E field size */

/* maximum values for instruction word fields */
#define MAX_OPCODE_VALUE 15  /* maximum opcode value (4 bits) */
#define MAX_MODE_VALUE 3     /* maximum addressing mode value (2 bits) */
#define MAX_ARE_VALUE 3      /* maximum A,R,E value (2 bits) */

/* bit masks for instruction word fields */
#define OPCODE_MASK ((1 << OPCODE_BITS) - 1)
#define MODE_MASK ((1 << MODE_BITS) - 1)
#define ARE_MASK ((1 << ARE_BITS) - 1)

/* string and number validation constants */
#define PLUS_SIGN '+'
#define MINUS_SIGN '-'
#define IMMEDIATE_PREFIX '#'
#define MIN_STRING_LENGTH 2  /* minimum string length (just quotes) */
#define NULL_TERMINATOR_SIZE 1  /* size for null \0 */

/* macro and label validation constants */
#define MIN_MACRO_NAME_LENGTH 1
#define REGISTER_BUFFER_SIZE 3
#define UNDERSCORE_CHAR '_'

/* register processing */
#define INVALID_REGISTER -1

/* character set definitions for parsing */
#define WHITESPACE_CHARS " \t\n\r"
#define CONTROL_CHARS "\x00-\x1F\x7F"

/* file operation modes */
#define FILE_READ_MODE "r"
#define FILE_WRITE_MODE "w"

/* file extension */
#define SOURCE_EXT ".as"
#define MACRO_EXT ".am"
#define OBJECT_EXT ".ob"
#define ENTRIES_EXT ".ent"
#define EXTERNALS_EXT ".ext"

/* matrix processing */
#define MAX_OPERAND_LENGTH (MAX_LINE_LENGTH - 1)
#define MAX_MATRIX_DIMENSION_LENGTH 10
#define MAX_MEMORY_SIZE 10000

/* error messages */
#define ERROR_LABEL_LENGTH_TOO_LONG "Error: label length exceeds maximum %d characters\n"
#define ERROR_UNLABEL "Error: missing label\n"
#define ERROR_INVALID_LABEL "Error: invalid label '%s'\n"
#define MALLOC_FAILED "Error: memory allocation failed\n"
#define ERROR_CANNOT_OPEN_FILE "Error: cannot open file '%s'\n"
#define ERROR_CANNOT_OPEN_FILE_WRITE "Error: cannot open file '%s' for writing\n"
#define ERROR_INVALID_OPERAND "Error: invalid operand '%s'\n"
#define ERROR_INVALID_REGISTER "Error: invalid register '%s' (must be r0-r7)\n"
#define ERROR_INVALID_IMMEDIATE "Error: invalid immediate value '%s'\n"

/* error messages with line number context */
#define ERROR_INVALID_IMMEDIATE_LINE "Error (line %d): invalid immediate value '%s'\n"
#define ERROR_INVALID_OPERAND_LINE "Error (line %d): invalid operand '%s'\n"
#define ERROR_INVALID_STRING_LINE "Error (line %d): invalid string format '%s'\n"
#define ERROR_PARSE_FAILED_LINE "Error (line %d): failed to parse line\n"
#define ERROR_LINE_TOO_LONG "Error: line exceeds maximum length of %d characters\n"
#define ERROR_LINE_TOO_LONG_DETAILED "Error: line too long (%lu characters, max %d)\n"
#define ERROR_LINE_CONTAINS_NON_PRINTABLE "Error: line contains non-printable characters\n"

/* valid error messages */
#define ERROR_IC_FINAL_TOO_SMALL "Error: final IC (%d) must be >= initial IC (%d)\n"
#define ERROR_DC_FINAL_NEGATIVE "Error: final DC (%d) must be >= 0\n"
#define ERROR_UNDEFINED_LABEL "Error: undefined label '%s'\n"
#define ERROR_INVALID_REGISTER_GENERAL "Error: invalid register '%s'\n"
#define ERROR_MATRIX_NOT_IMPLEMENTED "Error: matrix operand encoding not implemented\n"
#define ERROR_INVALID_ADDRESSING_MODE "Error: invalid addressing mode\n"

/* warning messages */
#define WARNING_OPCODE_OUT_OF_RANGE "Warning: opcode %d out of range (0-%d), using valid range\n"

/* format strings */
#define FORMAT_TWO_STRINGS "%s %s\n"

/* memory allocation context messages */
#define ALLOCATION_PURPOSE_LABEL "Context: label allocation"
#define ALLOCATION_PURPOSE_COMMAND "Context: command allocation"
#define ALLOCATION_PURPOSE_OPERAND "Context: operand allocation"
#define ALLOCATION_PURPOSE_FORMAT "Context: %s\n"

/* error messages for operand extraction */
#define ERROR_OPERAND_ALLOCATION_FAILED "Error: failed to allocate memory for operand %d\n"
#define ERROR_OPERAND_TOO_LONG "Error: operand exceeds maximum length of %d characters\n"



/* addressing modes enumeration */
typedef enum {
    MODE_IMMEDIATE = 0,    /* immediate - #number */
    MODE_DIRECT = 1,       /* direct - label */
    MODE_MATRIX,       /* matrix - label[reg][reg] */
    MODE_REGISTER     /* register - reg */
} addressing_mode;

/* A,R,E field values for machine code */
typedef enum {
    ARE_ABSOLUTE = 0,      /* A - 00 */
    ARE_EXTERNAL = 1,      /* E - 01 */
    ARE_RELOCATABLE = 2    /* R - 10 */
} are_type;

/* structure representing a parsed assembly line */
typedef struct {
    char *label;                          /* optional label */
    char *operands[MAX_OPERANDS];         /* array of operand strings */
    char *command;                        /* instruction or directive */
    int how_many_operands;                /* number of operands */
} separate_line;

/* function declarations */

/**
 * check if this is a valid directive
 * @param name: directive name to check
 * @return SUCCESS if valid directive, FAILURE otherwise
 */
int is_valid_directive(const char* name);

/**
 * check if this is a valid opcode
 * @param name: instruction name to check
 * @return SUCCESS if valid opcode, FAILURE otherwise
 */
int is_valid_opcode(const char* name);

/**
 * check if this is a valid macro name
 * @param name: macro name to check
 * @return SUCCESS if valid macro name, FAILURE otherwise
 */
int is_valid_macro_name(const char* name);

/**
 * Check if the name is a valid label name
 * @param label: label name to check
 * @param print_errors: whether to print error messages (1) or not (0) - depends on the calling function
 * @return SUCCESS if valid label, FAILURE otherwise
 */
int is_valid_label(const char* label, int print_errors);

/**
 * find operand mode for operand
 * @param operand: operand string
 * @return addressing mode bit mask, or FAILURE if invalid
 */
int get_operand_mode(const char* operand);

/**
 * free all allocated memory in a separate_line structure
 * @param line: pointer to separate_line structure we want to free
 */
void free_separate_line(separate_line* line);

/**
 * open_file_read - opens file for reading
 * @filename: path to file to open
 * returns file pointer or NULL if failed
 */
FILE* open_file_read(const char* filename);

/**
 * open_file_write - opens file for writing with error handling
 * @filename: file to open
 * returns file pointer or NULL if failed
 */
FILE* open_file_write(const char* filename);

/**
 * open_file_write_with_suffix - creates output file with suffix
 * @base_filename: base name without extension
 * @suffix: file extension
 * returns file pointer or NULL if failed
 */
FILE* open_file_write_with_suffix(const char* base_filename, const char* suffix);

/**
 * extract_base_filename - removes extension from filename
 * @filename: original filename with extension
 * returns new allocat string with base name, or NULL if failed
 */
char* extract_base_filename(const char* filename);

/**
 * is_valid_number - checks if string is a valid integer
 * @str: string to check
 * returns SUCCESS if valid number, FAILURE otherwise
 */
int is_valid_number(const char* str);

/**
 * number_to_base4_letters - converts number to base-4 letter encoding
 * @value: integer value to convert
 * returns static string containing base-4 representation
 */
char* number_to_base4_letters(int value);

/**
 * number_to_base4_code - converts number to base-4 code representation
 * @value: integer value to convert
 * returns static string containing base-4 code
 */
char* number_to_base4_code(int value);

#endif /* UTILS_H */