#ifndef PARSER_H
#define PARSER_H

#include "utils.h"

/**
 * parse_line - parse a single assembly line
 * @line: input line to parse
 * @return pointer to allocated separate_line on success, NULL on failure
 */
separate_line* parse_line(const char* line);

/**
 * extract_label - extract label from a line
 * @line: source line
 * @return heap-allocated label string or NULL on failure
 */
char* extract_label(const char* line);

/**
 * extract_command - extract command token from a line
 * @line: source line
 * @return heap-allocated command string or NULL on failure
 */
char* extract_command(const char* line);

/**
 * extract_operands - extract operand strings from a line
 * @line: source line
 * @count: output parameter for number of operands found
 * @return array of heap-allocated operand strings or NULL on failure
 */
char** extract_operands(const char* line, int* count);

#endif /* PARSER_H */