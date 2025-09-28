#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "macro.h"
#include "first_pass.h"
#include "second_pass.h"
#include "labelTable.h"

/* file extension*/
#define AS_EXTENSION ".as"
#define AM_EXTENSION ".am"
#define AS_EXT_LENGTH 3
#define AM_EXT_LENGTH 4
#define MIN_FILENAME_LENGTH 4
#define MIN_ARGC 2

/* error messages */
#define ERROR_NO_INPUT_FILES "Error: No input files specified\n\n"
#define ERROR_INVALID_FILENAME "Error: Invalid filename '%s' (must end with .as)\n"
#define ERROR_BASE_FILENAME_FAILED "Error: Failed to extract base filename from '%s'\n"
#define ERROR_FIRST_PASS_FAILED "Error: First pass failed for file '%s'\n"
#define ERROR_SECOND_PASS_FAILED "Error: Second pass failed for file '%s'\n"

/* success messages */
#define MSG_PROCESSING_FILE "Processing file: %s\n"
#define MSG_PHASE_1 "  Phase 1: Expanding macros...\n"
#define MSG_PHASE_2 "  Phase 2: First pass analysis...\n"  
#define MSG_PHASE_3 "  Phase 3: Second pass and code generation...\n"
#define MSG_SUCCESS "  Successfully processed '%s'\n"
#define MSG_FAILED "  Failed to process '%s'\n"

/* usage and status messages */
#define MSG_USAGE_FORMAT "Usage: %s file1.as file2.as file3.as ...\n"
#define MSG_DESCRIPTION "\nDescription:\n"
#define MSG_ASSEMBLER_DESC "  Assembler for custom assembly language\n"
#define MSG_PROCESSES_DESC "  Processes .as source files and generates:\n"
#define MSG_AM_FILES_DESC "    - .am files (macro expanded source)\n"
#define MSG_OB_FILES_DESC "    - .ob files (object code)\n"
#define MSG_ENT_FILES_DESC "    - .ent files (entry symbols, if any)\n"
#define MSG_EXT_FILES_DESC "    - .ext files (external references, if any)\n"
#define MSG_EXAMPLES "\nExamples:\n"
#define MSG_EXAMPLE1 "  %s prog1.as\n"
#define MSG_EXAMPLE2 "  %s file1.as file2.as file3.as\n"
#define MSG_ASSEMBLER_STARTED "Assembler started\n"
#define MSG_SEPARATOR "###############\n"
#define MSG_NEWLINE "\n"
#define MSG_ASSEMBLY_SUMMARY "Assembly Summary\n"
#define MSG_SEPARATOR2 "################\n"
#define MSG_TOTAL_FILES "Total files processed: %d\n"
#define MSG_SUCCESSFUL_FILES "Successful: %d\n"
#define MSG_FAILED_FILES "Failed: %d\n"
#define MSG_SOME_FAILED "\nSome files failed to assemble. Check error messages above.\n"
#define MSG_ALL_SUCCESS "\nAll files assembled successfully!\n"



/**
 * process a single source file through all assembler phases
 * @param filename: path to the source file (.as extension)
 * @return SUCCESS if file processed successfully, FAILURE otherwise
 */
static int process_file(const char* filename) {
    char* base_filename;
    char* macro_filename;
    label_table table;
    int ic_final, dc_final;
    int result = SUCCESS;

    printf(MSG_PROCESSING_FILE, filename);

    /* extract base filename - no exstention */
    base_filename = extract_base_filename(filename);
    if (!base_filename) {
        fprintf(stderr, ERROR_BASE_FILENAME_FAILED, filename);
        return FAILURE;
    }

    /* create macro filename (.am) */
    macro_filename = malloc(strlen(base_filename) + 4); /* .am + \0 */
    if (!macro_filename) {
        fprintf(stderr, MALLOC_FAILED);
        free(base_filename);
        return FAILURE;
    }
    strcpy(macro_filename, base_filename);
    strcat(macro_filename, MACRO_EXT);

    /* 1: Macro expansion */
    printf(MSG_PHASE_1);
    expand_macros(filename, macro_filename);

    /* initialize label table */
    init_label_table(&table);

    /* 2: first pass */
    printf(MSG_PHASE_2);
    if (first_pass_on_table(macro_filename, &table, &ic_final, &dc_final) == FAILURE) {
        fprintf(stderr, ERROR_FIRST_PASS_FAILED, filename);
        result = FAILURE;
    } else {


        /* 3: second pass */
        printf(MSG_PHASE_3);

        if (second_pass(macro_filename, &table, ic_final, dc_final) == FAILURE) {
            fprintf(stderr, ERROR_SECOND_PASS_FAILED, filename);
            result = FAILURE;
        } else {

        }
    }

    /* free ll memory */
    free_label_table(&table);
    free(base_filename);
    free(macro_filename);

    if (result == SUCCESS) {
        printf(MSG_SUCCESS, filename);
    } else {
        printf(MSG_FAILED, filename);
    }

    return result;
}

/**
 * check that the input filename has .as extension
 * @param filename: filename string to validate
 * @return SUCCESS if filename is valid, FAILURE otherwise
 */
static int validate_filename(const char* filename) {
    size_t len;

    if (!filename) {
        return FAILURE;
    }

    len = strlen(filename);
    if (len < 4) {
        return FAILURE;
    }

    /* Check if filename ends with .as */
    if (strcmp(filename + len - 3, SOURCE_EXT) != 0) {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 * print usage information and help text
 * @param program_name: name of the executable program
 */
static void print_usage(const char* program_name) {
    printf(MSG_USAGE_FORMAT, program_name);
    printf(MSG_DESCRIPTION);
    printf(MSG_ASSEMBLER_DESC);
    printf(MSG_PROCESSES_DESC);
    printf(MSG_AM_FILES_DESC);
    printf(MSG_OB_FILES_DESC);
    printf(MSG_ENT_FILES_DESC);
    printf(MSG_EXT_FILES_DESC);
    printf(MSG_EXAMPLES);
    printf(MSG_EXAMPLE1, program_name);
    printf(MSG_EXAMPLE2, program_name);
}

/**
 * main
 * @param argc: number of command line arguments
 * @param argv: array of command line argument strings
 * @return 0 on success, EXIT_FAILURE_CODE on failure
 */
int main(int argc, char* argv[]) {
    int i;
    int total_files = 0;
    int successful_files = 0;
    int failed_files = 0;

    /* check command line arguments */
    if (argc < 2) {
        fprintf(stderr, ERROR_NO_INPUT_FILES);
        print_usage(argv[0]);
        return EXIT_FAILURE_CODE;
    }

    printf(MSG_ASSEMBLER_STARTED);
    printf(MSG_SEPARATOR);
    printf(MSG_NEWLINE);

    /* process each input file */
    for (i = 1; i < argc; i++) {
        total_files++;

        /* check filename */
        if (validate_filename(argv[i]) == FAILURE) {
            fprintf(stderr, ERROR_INVALID_FILENAME, argv[i]);
            failed_files++;
            continue;
        }

        /* Process the file */
        if (process_file(argv[i]) == SUCCESS) {
            successful_files++;
        } else {
            failed_files++;
        }

        printf(MSG_NEWLINE);
    }

    /* summary */
    printf(MSG_ASSEMBLY_SUMMARY);
    printf(MSG_SEPARATOR2);
    printf(MSG_TOTAL_FILES, total_files);
    printf(MSG_SUCCESSFUL_FILES, successful_files);
    printf(MSG_FAILED_FILES, failed_files);

    if (failed_files > 0) {
        printf(MSG_SOME_FAILED);
        return EXIT_FAILURE_CODE;
    }

    printf(MSG_ALL_SUCCESS);
    return 0;
}