//
//  WraitheCMS.h
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

#ifndef WraitheCMS_WraitheCMS_h
#define WraitheCMS_WraitheCMS_h

//----------------------------------------------------------------------
//
typedef struct WraitheCMS_Text        WraitheCMS_Text;
typedef struct WraitheCMS_Source      WraitheCMS_Source;
typedef struct WraitheCMS_StackNode   WraitheCMS_StackNode;
typedef struct WraitheCMS_Stack       WraitheCMS_Stack;
typedef struct WraitheCMS_Symbol      WraitheCMS_Symbol;
typedef struct WraitheCMS_SymTab      WraitheCMS_SymTab;


//----------------------------------------------------------------------
//
struct WraitheCMS_Source {
    const char     *source;
    int             line;
    WraitheCMS_Text *data;
    char            *curr;
};


//----------------------------------------------------------------------
//
struct WraitheCMS_Stack {
    WraitheCMS_StackNode *top;
    WraitheCMS_StackNode *bottom;
    int                   size;
};


//----------------------------------------------------------------------
//
struct WraitheCMS_StackNode {
    WraitheCMS_StackNode *prev;
    WraitheCMS_StackNode *next;
    WraitheCMS_Text      *text;
};


//----------------------------------------------------------------------
//
struct WraitheCMS_Symbol {
    WraitheCMS_Symbol *prev;
    WraitheCMS_Symbol *next;
    const char        *name;
    const char        *value;
};


//----------------------------------------------------------------------
//
struct WraitheCMS_SymTab {
    WraitheCMS_Symbol *top;
    WraitheCMS_Symbol *bottom;
    int                size;
};


//----------------------------------------------------------------------
//
struct WraitheCMS_Text {
    int  isNull;
    int  length;
    char text[1];
};


//----------------------------------------------------------------------
//
WraitheCMS_StackNode *WraitheCMS_NewStackNode(void);
WraitheCMS_Stack     *WraitheCMS_NewStack(void);
WraitheCMS_Symbol    *WraitheCMS_NewSymbol(const char *name, const char *value);
WraitheCMS_SymTab    *WraitheCMS_NewSymTab(void);
WraitheCMS_Text      *WraitheCMS_NewText(const char *data_, int length);

WraitheCMS_Text      *WraitheCMS_Stack_PopTop(WraitheCMS_Stack *stack);
WraitheCMS_StackNode *WraitheCMS_Stack_PushTop(WraitheCMS_Stack *stack, WraitheCMS_Text *t);
void                  WraitheCMS_Stack_Dump(WraitheCMS_Stack *stack);

WraitheCMS_Symbol    *WraitheCMS_Symbol_Free(WraitheCMS_Symbol *symbol);

WraitheCMS_Symbol    *WraitheCMS_SymTab_Add(WraitheCMS_SymTab *symtab, const char *name, const char *value);
WraitheCMS_Symbol    *WraitheCMS_SymTab_Find(WraitheCMS_SymTab *symtab, const char *name);

int                   NameValue(WraitheCMS_SymTab *symtab, char *buffer);
WraitheCMS_Text      *ReadFile(const char **searchPath, const char *fileName, int forceNewLine, int trimTrailingNewline);
int                   ViewParse(WraitheCMS_SymTab *symtab, WraitheCMS_Stack *stack, const char **searchPath, WraitheCMS_Source *source);

#endif
