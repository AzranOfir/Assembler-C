#ifndef COMMANDS_H
#define COMMANDS_H

#include "utils.h"

/* Number of opcodes supported by the assembler */
#define NUM_OF_OPCODES 16

/* Operand count definitions */
#define SINGLE_OPERAND 1
#define DOUBLE_OPERAND 2
#define NO_OPERANDS 0

/* Array index constants */
#define FIRST_OPERAND_INDEX 0
#define SECOND_OPERAND_INDEX 1
#define LOOP_START_INDEX 0

/* Addressing mode bit masks for validation */
#define IMMEDIATE (1 << 0)     /* 0001 - #number */
#define DIRECT    (1 << 1)     /* 0010 - label */
#define MATRIX_ACCESS (1 << 2) /* 0100 - label[reg][reg] */
#define REGISTER  (1 << 3)     /* 1000 - reg */

/* Error message definitions for commands */
#define ERROR_COMMAND_NOT_FOUND "Error: command '%s' not found in instruction table\n"
#define ERROR_INVALID_SOURCE_MODE "Error: invalid source mode for command '%s'\n"
#define ERROR_INVALID_DESTINATION_MODE "Error: invalid destination mode for command '%s'\n"
#define ERROR_WRONG_OPERAND_COUNT "Error: command '%s' expects %d operands, got different count\n"
#define ERROR_INVALID_INSTRUCTION "Error: invalid instruction '%s' with operands\n"
#define ERROR_NULL_COMMAND "Error: null command name provided\n"

/* Opcode enumeration for all supported instructions */
typedef enum {
    INVALID = -1,
    MOV = 0,    /* move data between operands */
    CMP,        /* compare two operands */
    ADD,        /* add source to destination */
    SUB,        /* subtract source from destination */
    LEA,        /* load effective address */
    CLR,        /* clear operand to zero */
    NOT,        /* bitwise not operation */
    INC,        /* increment operand by one */
    DEC,        /* decrement operand by one */
    JMP,        /* unconditional jump */
    BNE,        /* branch if not equal */
    JSR,        /* jump to subroutine */
    RED,        /* read character from input */
    PRN,        /* print operand value */
    RTS,        /* return from subroutine */
    STOP        /* halt program execution */
} opcode_types;

/* Structure defining instruction properties and valid addressing modes */
typedef struct {
    const char *name;         /* instruction mnemonic */
    opcode_types opcode;      /* numeric opcode value */
    int num_of_operands;      /* number of operands required */
    int source_mode;          /* valid source addressing modes (bit mask) */
    int destination_mode;     /* valid destination addressing modes (bit mask) */
} command_instructions;

/* Global instruction table containing all supported commands */
extern const command_instructions instruction_table[NUM_OF_OPCODES];

/**
 * Retrieves instruction data by command name from the instruction table
 * @param command_name: string containing the instruction mnemonic
 * @return pointer to instruction structure, or NULL if not found
 */
const command_instructions *get_instruction(const char *command_name);

/**
 * Validates instruction with given addressing modes
 * @param command_name: instruction mnemonic to validate
 * @param source: source operand addressing mode (bit mask)
 * @param destination: destination operand addressing mode (bit mask)
 * @return SUCCESS if valid, FAILURE otherwise
 */
int check_instruction(const char *command_name, int source, int destination);

/**
 * Validates a complete parsed assembly line
 * @param line: pointer to parsed line structure containing label, command, and operands
 * @return SUCCESS if line is valid, FAILURE otherwise
 */
int check_line(separate_line* line);

#endif /* COMMANDS_H */