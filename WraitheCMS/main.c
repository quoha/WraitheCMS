//
//  main.c
//  WraitheCMS
//
//  Created by Michael Henderson on 1/17/13.
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
#include "WraitheLexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//----------------------------------------------------------------------
//
int main(int argc, const char * argv[])
{
    printf("WraitheCMS greets YOU!\n\n");
    printf("WraitheCMS is licensed under the MIT license.\n");
    printf("Please see the file LICENSE for details.\n\n");

    const char *viewFile = "/Users/mdhender/Software/WraitheCMS/data/article.tpl";
    char *viewData = ReadFile(viewFile, 0, 0);
    if (!viewData) {
        perror(viewFile);
        return 2;
    }
    printf("%s\n", viewData);
    Lexer  *viewLexer = NewLexer(viewFile, viewData);
    if (!viewLexer) {
        perror("viewLexer");
        return 2;
    }
    Lexeme *view = ViewLexer(viewLexer);
    if (!view) {
        perror("ViewLexer");
        return 2;
    }
    while (view) {
        printf(" view:\t%2d %2d %s\n", view->line, view->kind, view->data);
        view = view->next;
    }

    WraitheCMS_Stack *stack = WraitheCMS_NewStack();
    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText("true", -1));
    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText(0, 0));
    
    WraitheCMS_AST *a1 = WraitheCMS_NewAST(F_NoOp);
    WraitheCMS_AST *a2 = WraitheCMS_NewAST(F_If);
    WraitheCMS_AST *a3 = WraitheCMS_NewAST(F_NoOp);
    WraitheCMS_AST *a4 = WraitheCMS_NewAST(F_NoOp);
    WraitheCMS_AST *a5 = WraitheCMS_NewAST(F_NoOp);
    if (!a1 || !a2 || !a3 || !a4 || !a5) {
        perror("new AST");
        return 2;
    }

    a1->bz  = a2;
    a2->bz  = a3;
    a2->bnz = a4;
    a3->bz  = a5;
    a4->bz  = a5;

    WraitheCMS_VM *vm = WraitheCMS_NewVM();
    if (!vm) {
        perror("new VM");
        return 0;
    }

    if (vm->exec(vm, a1, stack) != VM_OK) {
        printf("\nerror:\tvirtual machine failed\n\n");
        return 2;
    }
    
    return 0;
}

