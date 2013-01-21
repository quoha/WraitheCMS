//
//  main.c
//  WraitheCMS
//
//  Created by Michael Henderson on 1/17/13.
//  Copyright (c) 2013 Michael D Henderson. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WraitheCMS_Text WraitheCMS_Text;
struct WraitheCMS_Text {
    int  isNull;
    int  length;
    char text[1];
};

WraitheCMS_Text *WraitheCMS_NewText(const char *data_, int length) {
    if (length < 0) {
        length = (int)strlen(data_ ? data_ : "");
    }

    WraitheCMS_Text *t = malloc(sizeof(*t) + length);
    if (t) {
        t->length = length;
        t->isNull = data_ ? 0 : 1;
        if (t->isNull || t->length == 0) {
            t->text[0] = 0;
        } else {
            memcpy(t->text, data_, t->length);
            t->text[length] = 0;
        }
    }

    return t;
}





typedef struct WraitheCMS_StackNode WraitheCMS_StackNode;
struct WraitheCMS_StackNode {
    struct WraitheCMS_StackNode *prev;
    struct WraitheCMS_StackNode *next;
    struct WraitheCMS_Text      *text;
};

typedef struct WraitheCMS_Stack WraitheCMS_Stack;
struct WraitheCMS_Stack {
    struct WraitheCMS_StackNode *top;
    struct WraitheCMS_StackNode *bottom;
    int                          size;
};

typedef struct WraitheCMS_VM WraitheCMS_VM;
struct WraitheCMS_VM {
    struct WraitheCMS_AST *currInstruction;
    struct WraitheCMS_AST *nextInstruction;
    int                    maxErrors;
    int                    traceLevel;
    char                 **errmgs;
};

typedef struct WraitheCMS_AST WraitheCMS_AST;
struct WraitheCMS_AST {
    char            *sourceName;
    int              sourceLine;
    int            (*code)(struct WraitheCMS_VM *vm, struct WraitheCMS_Stack *stack);
    WraitheCMS_AST  *bz;
    WraitheCMS_AST  *bnz;
    void            *data;
};


#define AST WraitheCMS_AST
#define VM  WraitheCMS_VM
#define VM_OK     0
#define VM_ERRINT 1
#define VM_ERR    2
#define SN_NULL   0
#define SN_TEXT   1

WraitheCMS_AST *WraitheCMS_AST_New(int (*code)(WraitheCMS_VM *vm, WraitheCMS_Stack *stack)) {
    WraitheCMS_AST *ast = (WraitheCMS_AST *)malloc(sizeof(*ast));
    if (!ast) {
        return 0;
    }
    memset(ast, 0, sizeof(*ast));
    ast->code = code;
    ast->bz   = 0;
    ast->bnz  = 0;
    return ast;
}

WraitheCMS_VM *WraitheCMS_VM_New(void) {
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
    return vm;
}

int WraitheCMS_VM_Execute(WraitheCMS_VM *vm, WraitheCMS_Stack *stack) {
    int retVal = VM_OK;
    if (vm) {
        while (retVal == VM_OK && vm->nextInstruction) {
            WraitheCMS_AST *ast = vm->currInstruction = vm->nextInstruction;
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

WraitheCMS_StackNode *WraitheCMS_NewStackNode(void) {
    WraitheCMS_StackNode *n = malloc(sizeof(*n));
    if (n) {
        n->prev = 0;
        n->next = 0;
        n->text = 0;
    }
    return n;
}

WraitheCMS_Stack *WraitheCMS_Stack_New(void) {
    WraitheCMS_Stack *s = malloc(sizeof(*s));
    if (s) {
        memset(s, 0, sizeof(*s));
    }
    return s;
}

WraitheCMS_Text *WraitheCMS_Stack_PopTop(WraitheCMS_Stack *stack) {
    WraitheCMS_Text *t = 0;
    if (stack && stack->top) {
        stack->size--;

        WraitheCMS_StackNode *n = stack->top;
        if (!stack->top->prev) {
            stack->top = stack->bottom = 0;
        } else {
            stack->top = stack->top->prev;
            stack->top->next = 0;
        }
        t = n->text;
        free(n);
    }
    return t;
}

WraitheCMS_StackNode *WraitheCMS_Stack_PushTop(WraitheCMS_Stack *stack, WraitheCMS_Text *t) {
    WraitheCMS_StackNode *n = 0;
    if (stack && t) {
        n = WraitheCMS_NewStackNode();
        if (n) {
            stack->size++;
            n->text = t;
            if (!stack->bottom) {
                stack->bottom = stack->top = n;
            } else {
                n->prev = stack->top;
                stack->top->next = n;
                stack->top = n;
            }
        }
    }
    return n;
}

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
        vm->nextInstruction = vm->currInstruction->bnz;
    } else {
        printf("   vm:\texecuting if (%s)\n", t->text);
        vm->nextInstruction = vm->currInstruction->bz;
    }
    return VM_OK;
}

int F_NoOp(WraitheCMS_VM *vm, WraitheCMS_Stack *stack) {
    vm->nextInstruction = vm->currInstruction->bz;
    if (vm->traceLevel > 3) {
        printf("   vm:\texecuting no-op\n");
    }
    return VM_OK;
}

int main(int argc, const char * argv[])
{
    // insert code here...
    printf("Hello, World!\n");

    WraitheCMS_Stack *stack = WraitheCMS_Stack_New();
    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText("true", -1));
    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText(0, 0));
    
    WraitheCMS_AST *a1 = WraitheCMS_AST_New(F_NoOp);
    WraitheCMS_AST *a2 = WraitheCMS_AST_New(F_If);
    WraitheCMS_AST *a3 = WraitheCMS_AST_New(F_NoOp);
    WraitheCMS_AST *a4 = WraitheCMS_AST_New(F_NoOp);
    WraitheCMS_AST *a5 = WraitheCMS_AST_New(F_NoOp);
    if (!a1 || !a2 || !a3 || !a4 || !a5) {
        perror("new AST");
        return 2;
    }

    a1->bz  = a2;
    a2->bz  = a3;
    a2->bnz = a4;
    a3->bz  = a5;
    a4->bz  = a5;

    WraitheCMS_VM *vm = WraitheCMS_VM_New();
    if (!vm) {
        perror("new VM");
        return 0;
    }

    vm->nextInstruction = a1;
    if (WraitheCMS_VM_Execute(vm, stack) != VM_OK) {
        printf("\nerror:\tvirtual machine failed\n\n");
        return 2;
    }
    
    return 0;
}

