//
//  stack.c
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
WraitheCMS_StackNode *WraitheCMS_NewStackNode(void) {
    WraitheCMS_StackNode *n = malloc(sizeof(*n));
    if (n) {
        n->prev = 0;
        n->next = 0;
        n->text = 0;
    }
    return n;
}


//----------------------------------------------------------------------
//
WraitheCMS_Stack *WraitheCMS_NewStack(void) {
    WraitheCMS_Stack *s = malloc(sizeof(*s));
    if (s) {
        memset(s, 0, sizeof(*s));
    }
    return s;
}


//----------------------------------------------------------------------
//
void WraitheCMS_Stack_Dump(WraitheCMS_Stack *stack) {
    int idx = 0;
    WraitheCMS_StackNode *node = stack->top;
    while (node) {
        printf("stack:\t%4d %s\n", idx++, node->text ? node->text->text : "(null)");
        node = node->prev;
    }
}

//----------------------------------------------------------------------
//
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


//----------------------------------------------------------------------
//
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

