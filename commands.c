#include "commands.h"
#include <string.h>

/* instruction table containing all supported commands with their properties */
const command_instructions instruction_table[NUM_OF_OPCODES] = {
    {"mov", MOV, DOUBLE_OPERAND, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER, DIRECT | MATRIX_ACCESS | REGISTER},
    {"cmp", CMP, DOUBLE_OPERAND, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER},
    {"add", ADD, DOUBLE_OPERAND, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER, DIRECT | MATRIX_ACCESS | REGISTER},
    {"sub", SUB, DOUBLE_OPERAND, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER, DIRECT | MATRIX_ACCESS | REGISTER},
    {"not", NOT, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS | REGISTER},
    {"clr", CLR, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS | REGISTER},
    {"lea", LEA, DOUBLE_OPERAND, DIRECT | MATRIX_ACCESS, REGISTER},
    {"inc", INC, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS | REGISTER},
    {"dec", DEC, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS | REGISTER},
    {"jmp", JMP, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS},
    {"bne", BNE, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS},
    {"red", RED, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS | REGISTER},
    {"prn", PRN, SINGLE_OPERAND, NO_OPERANDS, IMMEDIATE | DIRECT | MATRIX_ACCESS | REGISTER},
    {"jsr", JSR, SINGLE_OPERAND, NO_OPERANDS, DIRECT | MATRIX_ACCESS},
    {"rts", RTS, NO_OPERANDS, NO_OPERANDS, NO_OPERANDS},
    {"stop", STOP, NO_OPERANDS, NO_OPERANDS, NO_OPERANDS}
};

/**
 * Retrieves instruction data by command name from the instruction table
 * @param command_name: string containing the instruction mnemonic
 * @return pointer to instruction structure, or NULL if not found
 */
const command_instructions *get_instruction(const char *command_name) {
    int i;

    /* validate input parameter */
    if (!command_name) {
        fprintf(stderr, ERROR_NULL_COMMAND);
        return NULL;
    }

    /* search for command in instruction table */
    for (i = LOOP_START_INDEX; i < NUM_OF_OPCODES; i++) {
        if (strcmp(command_name, instruction_table[i].name) == 0) {
            return &instruction_table[i];
        }
    }

    /* command not found in table */
    fprintf(stderr, ERROR_COMMAND_NOT_FOUND, command_name);
    return NULL;
}

/**
 * Validates instruction with given addressing modes
 * @param command_name: instruction mnemonic to validate
 * @param source: source operand addressing mode (bit mask)
 * @param destination: destination operand addressing mode (bit mask)
 * @return SUCCESS if valid, FAILURE otherwise
 */
int check_instruction(const char *command_name, const int source, const int destination) {
    const command_instructions *instruction;

    /* retrieve instruction from table */
    instruction = get_instruction(command_name);
    if (instruction == NULL) {
        return FAILURE;
    }

    /* validate single operand instructions */
    if (instruction->num_of_operands == SINGLE_OPERAND) {
        if ((destination & instruction->destination_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_DESTINATION_MODE, command_name);
            return FAILURE;
        }
    }
    /* validate double operand instructions */
    else if (instruction->num_of_operands == DOUBLE_OPERAND) {
        /* check destination mode validity */
        if ((destination & instruction->destination_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_DESTINATION_MODE, command_name);
            return FAILURE;
        }
        /* check source mode validity */
        if ((source & instruction->source_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_SOURCE_MODE, command_name);
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * Validates a complete parsed assembly line
 * @param line: pointer to parsed line structure containing label, command, and operands
 * @return SUCCESS if line is valid, FAILURE otherwise
 */
int check_line(separate_line *line) {
    const command_instructions *instruction;
    int dest_mode;
    int source_mode;

    /* validate input line exists */
    if (!line) {
        fprintf(stderr, ERROR_NULL_COMMAND);
        return FAILURE;
    }

    /* validate command exists */
    if (!line->command) {
        fprintf(stderr, ERROR_NULL_COMMAND);
        return FAILURE;
    }

    /* validate label if present */
    if (line->label && !is_valid_label(line->label, 1)) {
        return FAILURE; /* error message already printed by is_valid_label */
    }

    /* retrieve instruction from table */
    instruction = get_instruction(line->command);
    if (!instruction) {
        return FAILURE; /* error message already printed by get_instruction */
    }

    /* validate operand count matches instruction requirements */
    if (line->how_many_operands != instruction->num_of_operands) {
        fprintf(stderr, ERROR_WRONG_OPERAND_COUNT, line->command, instruction->num_of_operands);
        return FAILURE;
    }

    /* validate addressing modes for single operand instructions */
    if (instruction->num_of_operands == SINGLE_OPERAND) {
        dest_mode = get_operand_mode(line->operands[FIRST_OPERAND_INDEX]);
        if (dest_mode == FAILURE) {
            return FAILURE; /* error already printed by get_operand_mode */
        }
        if ((dest_mode & instruction->destination_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_DESTINATION_MODE, line->command);
            return FAILURE;
        }
    }
    /* validate addressing modes for double operand instructions */
    else if (instruction->num_of_operands == DOUBLE_OPERAND) {
        source_mode = get_operand_mode(line->operands[FIRST_OPERAND_INDEX]);
        dest_mode = get_operand_mode(line->operands[SECOND_OPERAND_INDEX]);

        if (source_mode == FAILURE || dest_mode == FAILURE) {
            return FAILURE; /* error already printed by get_operand_mode */
        }

        if ((source_mode & instruction->source_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_SOURCE_MODE, line->command);
            return FAILURE;
        }

        if ((dest_mode & instruction->destination_mode) == NO_OPERANDS) {
            fprintf(stderr, ERROR_INVALID_DESTINATION_MODE, line->command);
            return FAILURE;
        }
    }

    return SUCCESS;
}