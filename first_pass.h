#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "labelTable.h"

/**
 * run the assembler first pass over a source file
 * @param filename: path to macro-expanded source file (.am)
 * @return SUCCESS if first pass completed successfully, FAILURE otherwise
 */
int first_pass(const char* filename);

/**
 * Advanced version: run first pass using given label table, return final IC/DC
 * @param filename: path to macro-expanded source file (.am)
 * @param table: symbol table to populate during first pass
 * @param outIC: output parameter for final instruction counter
 * @param outDC: output parameter for final data counter
 * @return SUCCESS if first pass completed successfully, FAILURE otherwise
 */
int first_pass_on_table(const char* filename, label_table* table, int* outIC, int* outDC);

#endif /* FIRST_PASS_H */ 