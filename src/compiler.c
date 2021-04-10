/*
 *  dP                         oo          .8888b                   dP
 *  88                                     88   "                   88
 *  88d888b. 88d888b. .d8888b. dP 88d888b. 88aaa  dP    dP .d8888b. 88  .dP
 *  88'  `88 88'  `88 88'  `88 88 88'  `88 88     88    88 88'  `"" 88888"
 *  88.  .88 88       88.  .88 88 88    88 88     88.  .88 88.  ... 88  `8b.
 *  88Y8888' dP       `88888P8 dP dP    dP dP     `88888P' `88888P' dP   `YP
 *
 * Authored in 2013.  See README for a list of contributors.
 * Released into the public domain.
 *
 * Any being (not just humans) is free to copy, modify, publish, use, compile,
 * sell or distribute this software, either in source code form or as a
 * compiled binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of
 * this software dedicate any and all copyright interest in the software to the
 * public domain.  We make this dedication for the benefit of the public at
 * large and to the detriment of our heirs and successors.  We intend this
 * dedication to be an overt act of relinquishment in perpetuity of all present
 * and future rights to this software under copyright law.
 *
 * The software is provided AS IS, without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and non-infringement.  In no event shall
 * the authors be liable for any claim, damages or other liability, whether in
 * an action of contract, tort or otherwise, arising from, out of or in
 * connection with the software or the use or other dealings in the software.
 *
 * This software is completely unlicensed. */

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define B_VERSION_STRING "0.4"
#define B_BUILD_FEATURES "core:llvm-ir:bin"

#define B_TRUE 1
#define B_FALSE 0

#define B_GENERIC_ADDRESS_SPACE 0

static char* B_INVOCATION = NULL;

static size_t B_CONTAINER_LENGTH = 30000;

static int B_SHOULD_READ_FROM_STDIN = B_FALSE;
static char const* B_INPUT_FILENAME = NULL;

static int B_SHOULD_EMIT_C_CODE = B_FALSE;
static char const* B_C_CODE_FILENAME = "brainfuck.c";

static int B_SHOULD_EMIT_LLVM_IR = B_FALSE;
static char const* B_LLVM_IR_FILENAME = "brainfuck.l";

static int B_SHOULD_OPTIMIZE_CODE = B_TRUE;
static int B_SHOULD_PRINT_BYTECODE_DISASSEMBLY = B_FALSE;
static int B_SHOULD_EXPLAIN_CODE = B_FALSE;
static int B_SHOULD_INTERPRET_CODE = B_TRUE;
static int B_SHOULD_COMPILE_AND_EXECUTE = B_FALSE;

enum instruction {
  B_INVALID = 0x00,
  B_MOVE_POINTER_LEFT = 0x3C,    /* < */
  B_MOVE_POINTER_RIGHT = 0x3E,   /* > */
  B_INCREMENT_CELL_VALUE = 0x2B, /* + */
  B_DECREMENT_CELL_VALUE = 0x2D, /* - */
  B_OUTPUT_CELL_VALUE = 0x2E,    /* . */
  B_INPUT_CELL_VALUE = 0x2C,     /* , */
  B_BRANCH_FORWARD = 0x5B,       /* [ */
  B_BRANCH_BACKWARD = 0x5D,      /* ] */
  B_TERMINATE = 0xFF
};

struct opcode {
  enum instruction instruction;
  size_t auxiliary;
};

struct program {
  struct opcode* opcodes;
  size_t number_of_opcodes;
};

static inline long get_file_length(FILE* file) {
  long position = 0L;
  long length = 0L;

  if (file == NULL) {
    abort();
  }

  position = ftell(file);

  if (position == -1L || fseek(file, 0, SEEK_END) != 0) {
    abort();
  }

  length = ftell(file);

  if (length == -1L || fseek(file, position, SEEK_SET) != 0) {
    abort();
  }

  return length;
}

static inline char* read_file(char const* filename) {
  char* contents = NULL;

  FILE* file = fopen(filename, "rt");
  long file_length = 0;

  if (file == NULL) {
    abort();
  }

  file_length = get_file_length(file);

  if (file_length == 0) {
    printf("%s: nothing to do\n", B_INVOCATION);
    fclose(file);
    abort();
  }

  contents = malloc(sizeof(char) * file_length);

  if (contents == NULL) {
    fclose(file);
    abort();
  }

  if (fread(contents, file_length, 1, file) != 1UL) {
    fclose(file);
    abort();
  }

  fclose(file);
  return contents;
}

static inline char* read_stdin(void) {
  char buffer[1024];

  size_t content_size = 1;
  char* contents = malloc(sizeof(char) * 1024);

  if (contents == NULL) {
    abort();
  }

  *contents = '\0';

  while (fgets(buffer, 1024, stdin)) {
    char* old_contents = contents;

    content_size += strlen(buffer);
    contents = realloc(contents, sizeof(char) * content_size);

    if (contents == NULL) {
      free(old_contents);
      abort();
    }

    strcat(contents, buffer);
  }

  return contents;
}

static inline int is_brainfuck_command(int command) {
  char const commands[] = {'<', '>', '+', '-', '.', ',', '[', ']'};
  return (memchr(commands, command, sizeof(commands)) != NULL);
}

static inline char* sanitize(char** source_code) {
  int i = 0;
  int j = 0;

  char* output = NULL;

  if (source_code == NULL || *source_code == NULL) {
    abort();
  }

  output = malloc(sizeof(char) * strlen(*source_code));

  if (output == NULL) {
    abort();
  }

  for (i = 0; (*source_code)[i]; ++i) {
    if (is_brainfuck_command((*source_code)[i])) {
      output[j++] = (*source_code)[i];
    }
  }

  free(*source_code);
  *source_code = output;

  return output;
}

static struct program* run_length_encode(char const* source_code) {
  size_t i = 0;
  char const* command = NULL;

  struct program* program = NULL;
  struct opcode opcode;

  if (source_code == NULL) {
    abort();
  }

  program = malloc(sizeof(struct program));

  if (program == NULL) {
    abort();
  }

  program->opcodes = malloc(sizeof(struct opcode) * (strlen(source_code) + 1));

  if (program->opcodes == NULL) {
    free(program);
    abort();
  }

  opcode.instruction = B_INVALID;
  opcode.auxiliary = 0;

  for (command = source_code; *command; ++command) {
    switch (*command) {
      case B_OUTPUT_CELL_VALUE:
      case B_INPUT_CELL_VALUE:
      case B_BRANCH_FORWARD:
      case B_BRANCH_BACKWARD:
        if (opcode.instruction != B_INVALID) {
          memcpy(program->opcodes + i, &opcode, sizeof(opcode));
          ++i;
        }

        opcode.instruction = *command;
        opcode.auxiliary = 1;

        break;

      default:
        if (opcode.instruction != *command) {
          if (opcode.instruction != B_INVALID) {
            memcpy(program->opcodes + i, &opcode, sizeof(opcode));
            ++i;
          }

          opcode.instruction = *command;
          opcode.auxiliary = 1;

          break;
        }

        ++(opcode.auxiliary);
    }
  }

  memcpy(program->opcodes + i, &opcode, sizeof(opcode));
  ++i;

  opcode.instruction = B_TERMINATE;
  opcode.auxiliary = 0;

  memcpy(program->opcodes + i, &opcode, sizeof(opcode));
  ++i;

  program->number_of_opcodes = i;
  return program;
}

static struct program* link_branches(struct program* program) {
  int i = 0;
  int j = 0;

  long* stack = NULL;

  if (program == NULL || program->opcodes == NULL) {
    abort();
  }

  stack = malloc(sizeof(long) * program->number_of_opcodes);

  if (stack == NULL) {
    abort();
  }

  for (i = 0; i != (int)program->number_of_opcodes; ++i) {
    switch (program->opcodes[i].instruction) {
      case B_BRANCH_FORWARD:
        stack[j++] = i;
        break;

      case B_BRANCH_BACKWARD:
        --j;

        program->opcodes[i].auxiliary = stack[j];
        program->opcodes[stack[j]].auxiliary = i;

      default:
        break;
    }
  }

  return program;
}

static void interpret(struct program const* program) {
  size_t i = 0;

  char* container = NULL;
  char* pointer = NULL;

  if (program == NULL || program->opcodes == NULL) {
    abort();
  }

  container = calloc(B_CONTAINER_LENGTH, sizeof(char));

  if (container == NULL) {
    abort();
  }

  pointer = container;

  for (; i != program->number_of_opcodes; ++i) {
    switch (program->opcodes[i].instruction) {
      case B_MOVE_POINTER_LEFT:
        pointer -= program->opcodes[i].auxiliary;
        break;

      case B_MOVE_POINTER_RIGHT:
        pointer += program->opcodes[i].auxiliary;
        break;

      case B_INCREMENT_CELL_VALUE:
        *pointer += program->opcodes[i].auxiliary;
        break;

      case B_DECREMENT_CELL_VALUE:
        *pointer -= program->opcodes[i].auxiliary;
        break;

      case B_OUTPUT_CELL_VALUE:
        putchar(*pointer);
        break;

      case B_INPUT_CELL_VALUE:
        *pointer = getchar();
        break;

      case B_BRANCH_FORWARD:
        if (*pointer == 0) {
          i = program->opcodes[i].auxiliary;
        }

        break;

      case B_BRANCH_BACKWARD:
        if (*pointer != 0) {
          i = program->opcodes[i].auxiliary;
        }

        break;

      case B_TERMINATE:
        if (i != program->number_of_opcodes - 1) {
          printf("%s: premature termination @ %zd\n", B_INVOCATION, i);
        }

      default:
        break;
    }
  }

  free(container);
}