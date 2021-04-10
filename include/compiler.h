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
