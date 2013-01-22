//
//  symtab.c
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
WraitheCMS_Symbol *WraitheCMS_FreeSymbol(WraitheCMS_Symbol *symbol) {
    if (symbol) {
        if (symbol->value) {
            free(symbol->value);
        }
        if (symbol->name) {
            free(symbol->name);
        }
        free(symbol);
        symbol = 0;
    }
    return symbol;
}

//----------------------------------------------------------------------
//
WraitheCMS_Symbol *WraitheCMS_NewSymbol(const char *name, const char *value) {
    WraitheCMS_Symbol *s = malloc(sizeof(*s));
    if (s) {
        s->prev  = 0;
        s->next  = 0;
        s->name  = strdup(name  ? name  : "");
        s->value = strdup(value ? value : "");
        
        if (!s->name || !s->value) {
            return WraitheCMS_FreeSymbol(s);
        }
    }

    return s;
}


//----------------------------------------------------------------------
//
WraitheCMS_SymTab    *WraitheCMS_NewSymTab(void) {
    WraitheCMS_SymTab *symtab = malloc(sizeof(*symtab));
    if (symtab) {
        symtab->top = WraitheCMS_NewSymbol("true", "true");
        if (!symtab->top) {
            return 0;
        }
        symtab->bottom = symtab->top;
        symtab->size   = 1;
    }
    return symtab;
}


//----------------------------------------------------------------------
//
WraitheCMS_Symbol *WraitheCMS_SymTab_Add(WraitheCMS_SymTab *symtab, const char *name, const char *value) {
    if (symtab) {
        symtab->top->next = WraitheCMS_NewSymbol(name, value);
        if (symtab->top->next) {
            symtab->top->next->prev = symtab->top;
            symtab->top = symtab->top->next;
            return symtab->top;
        }
    }
    return 0;
}


//----------------------------------------------------------------------
//
WraitheCMS_Symbol *WraitheCMS_SymTab_Find(WraitheCMS_SymTab *symtab, const char *name) {
    if (symtab) {
        WraitheCMS_Symbol *s = symtab->top;
        while (s) {
            if (strcmp(name, s->name) == 0) {
                return s;
            }
            s = s->prev;
        }
    }
    return 0;
}
