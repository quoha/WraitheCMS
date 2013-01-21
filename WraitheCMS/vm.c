//
//  vm.c
//  WraitheCMS
//
//  Created by Michael Henderson on 1/21/13.
//
// Copyright (c) 2013 Michael D Henderson
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#include "WraitheCMS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//----------------------------------------------------------------------
//
static int WraitheCMS_VM_Execute(WraitheCMS_VM *vm, WraitheCMS_AST *ast, WraitheCMS_Stack *stack);


//----------------------------------------------------------------------
//
WraitheCMS_VM *WraitheCMS_NewVM(void) {
    WraitheCMS_VM *vm = malloc(sizeof(*vm));
    if (!vm) {
        return 0;
    }
    memset(vm, 0, sizeof(*vm));
    vm->currInstruction = 0;
    vm->nextInstruction = 0;
    vm->traceLevel      = 5;
    vm->maxErrors       = 16;
    vm->errmgs    = malloc(sizeof(char *) * (vm->maxErrors + 1));
    if (!vm->errmgs) {
        free(vm);
        return 0;
    }
    int idx;
    for (idx = 0; idx <= vm->maxErrors; idx++) {
        vm->errmgs[idx] = 0;
    }

    vm->exec = WraitheCMS_VM_Execute;

    return vm;
}


//----------------------------------------------------------------------
//
int WraitheCMS_VM_Execute(WraitheCMS_VM *vm, WraitheCMS_AST *ast, WraitheCMS_Stack *stack) {
    int retVal = VM_OK;
    if (vm && ast && stack) {
        vm->currInstruction = vm->nextInstruction = ast;
        while (retVal == VM_OK && vm->nextInstruction) {
            ast = vm->currInstruction = vm->nextInstruction;
            if (!ast->code) {
                printf("   vm:\terror - instruction missing code\n");
                return VM_ERRINT;
            }
            retVal = ast->code(vm, stack);
            if (vm->currInstruction == vm->nextInstruction) {
                printf("   vm:\terror - instruction did not update program counter\n");
                return VM_ERRINT;
            }
        }
    }
    return retVal;
}

