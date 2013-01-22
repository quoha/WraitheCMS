//
//  ast.c
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
WraitheCMS_AST *WraitheCMS_NewAST(int (*code)(WraitheCMS_VM *vm, WraitheCMS_Stack *stack)) {
    WraitheCMS_AST *ast = (WraitheCMS_AST *)malloc(sizeof(*ast));
    if (!ast) {
        return 0;
    }
    memset(ast, 0, sizeof(*ast));
    ast->code = code;
    ast->next = 0;
    ast->data = 0;
    return ast;
}


//----------------------------------------------------------------------
//
int F_If(WraitheCMS_VM *vm, WraitheCMS_Stack *stack) {
    if (vm->traceLevel > 3) {
        printf("   vm:\texecuting if\n");
    }
    if (!stack) {
        // error - need a stack
        printf("error:\tIF function requires a stack\n");
        return VM_ERR;
    } else if (stack->size < 1) {
        // error - need a condition on the stack
        printf("error:\tIF function requires a condition on the stack\n");
        return VM_ERR;
    }
    WraitheCMS_Text *t = WraitheCMS_Stack_PopTop(stack);
    if (t->isNull) {
        printf("   vm:\texecuting if (null)\n");
        vm->nextInstruction = (WraitheCMS_AST *)(vm->currInstruction->data);
    } else {
        printf("   vm:\texecuting if (%s)\n", t->text);
        vm->nextInstruction = vm->currInstruction->next;
    }
    return VM_OK;
}


//----------------------------------------------------------------------
//
int F_NoOp(WraitheCMS_VM *vm, WraitheCMS_Stack *stack) {
    vm->nextInstruction = vm->currInstruction->next;
    if (vm->traceLevel > 3) {
        printf("   vm:\texecuting no-op\n");
    }
    return VM_OK;
}

