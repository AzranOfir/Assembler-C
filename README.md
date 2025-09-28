# Assembler-C

A two-pass assembler implementation for a custom assembly language with support for macros, multiple addressing modes, and various data types.

## Features

- **Two-pass assembly process** with comprehensive error checking
- **Macro support** with definition and expansion capabilities
- **Multiple addressing modes**: immediate, direct, matrix access, and register
- **Data directives**: `.data`, `.string`, `.mat` (matrix)
- **Symbol management**: `.entry` and `.extern` declarations
- **Base-4 output format** for machine code generation
- **Multiple output files**: object code, entries, and externals

## Architecture

The assembler is organized into several key modules:

### Core Components

- **`assembler.c`** - Main program entry point and file processing orchestration
- **`first_pass.c/h`** - Symbol table construction and initial analysis
- **`second_pass.c/h`** - Code generation and address resolution
- **`parser.c/h`** - Line parsing and syntax analysis
- **`utils.c/h`** - Common utilities and validation functions

### Specialized Modules

- **`macro.c/h`** - Macro definition and expansion system
- **`commands.c/h`** - Instruction set definition and validation
- **`labelTable.c/h`** - Symbol table management with linked list implementation

## Supported Instructions

The assembler supports 16 instruction types with various operand configurations:

### Data Movement
- `mov` - Move data between operands
- `lea` - Load effective address

### Arithmetic Operations
- `add` - Addition
- `sub` - Subtraction
- `inc` - Increment
- `dec` - Decrement

### Logic Operations
- `cmp` - Compare operands
- `not` - Bitwise NOT
- `clr` - Clear to zero

### Control Flow
- `jmp` - Unconditional jump
- `bne` - Branch if not equal
- `jsr` - Jump to subroutine
- `rts` - Return from subroutine

### Input/Output
- `red` - Read input
- `prn` - Print output

### Program Control
- `stop` - Halt execution

## Addressing Modes

1. **Immediate** (`#value`) - Direct value
2. **Direct** (`label`) - Memory address reference
3. **Matrix Access** (`label[reg][reg]`) - Two-dimensional array indexing
4. **Register** (`r0-r7`) - CPU register reference

## Data Directives

- **`.data`** - Define integer constants
- **`.string`** - Define null-terminated strings
- **`.mat`** - Define matrices with dimensions and optional initial values
- **`.extern`** - Declare external symbols
- **`.entry`** - Mark symbols for export

## Usage

```bash
./assembler file1.as file2.as file3.as ...
```

### Input Files
- Source files must have `.as` extension
- May contain macro definitions and assembly instructions

### Output Files
For each successfully assembled source file `filename.as`:

- **`filename.am`** - Macro-expanded source code
- **`filename.ob`** - Object code in base-4 format
- **`filename.ent`** - Entry symbols (if any)
- **`filename.ext`** - External references (if any)

## Assembly Process

### Phase 1: Macro Expansion
- Processes `mcro` and `mcroend` directives
- Expands macro calls inline
- Generates `.am` file with expanded source

### Phase 2: First Pass
- Builds symbol table with all labels
- Calculates memory layout for instructions and data
- Validates syntax and addressing modes
- Resolves label addresses

### Phase 3: Second Pass
- Generates machine code for instructions
- Encodes data segments
- Resolves external references
- Produces output files

## Memory Layout

- **Instructions**: Start at address 100 (base 10)
- **Data**: Placed immediately after instructions
- **10-bit word size** with A,R,E (Absolute/Relocatable/External) encoding

## Error Handling

The assembler provides comprehensive error reporting including:

- Syntax errors with line numbers
- Invalid label names and duplicates
- Undefined symbol references
- Invalid addressing mode combinations
- File I/O errors

## Example Assembly Code

```assembly
; Define external symbols
.extern EXTERNAL_VAR

; Define a macro
mcro PRINT_REG
    prn r1
mcroend

; Main program
MAIN: mov #5, r1        ; Load immediate value
      PRINT_REG         ; Call macro
      lea ARRAY[r2][r3], r4  ; Matrix addressing
      jmp END

; Data section
ARRAY: .mat [2][3] 1 2 3 4 5 6
MSG:   .string "Hello World"
NUMS:  .data 10, 20, 30

; Entry points
.entry MAIN
.entry ARRAY

END: stop
```

## Building

The project uses standard C compilation:

```bash
gcc -o assembler *.c
```

## Technical Details

### Symbol Table
- Implemented as linked list for dynamic sizing
- Supports label types: CODE, DATA, EXTERNAL, ENTRY
- Address resolution in two phases

### Instruction Encoding
- 10-bit machine words
- Opcode in bits 6-9
- Addressing modes in bits 2-5
- A,R,E field in bits 0-1

### Base-4 Output
- Addresses encoded as 4-digit base-4 (a,b,c,d format)
- Machine code as 5-digit base-4
- Mapping: 0→'a', 1→'b', 2→'c', 3→'d'

## Limitations

- Maximum line length: 80 characters
- Maximum label length: 30 characters
- Register range: r0-r7
- Memory size constraints based on 10-bit addressing

## Error Codes

The assembler returns:
- `0` - Success (all files assembled)
- `1` - Failure (one or more files failed)

Error messages are written to stderr with descriptive context including line numbers where applicable.
