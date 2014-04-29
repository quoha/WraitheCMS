//
//  lexer.c
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

#include "WraitheLexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if 0
//----------------------------------------------------------------------
//
static Lexeme *ViewLexer_Code(Lexer *l);
static Lexeme *ViewLexer_EOF(Lexer *l);
static Lexeme *ViewLexer_Text(Lexer *l);
static Lexeme *ViewLexer_Word(Lexer *l);


//----------------------------------------------------------------------
//
Lexer *NewLexer(const char *source, char *data) {
    Lexer *l = malloc(sizeof(*l));
    if (l) {
        l->source    = source;
        l->line      = 1;
        l->curr      = data;
    }
    return l;
}


//----------------------------------------------------------------------
// (text | ("<cms" word* "/>") )*
//
Lexeme *ViewLexer(Lexer *l) {
    Lexeme *root = 0;
    Lexeme *tail = 0;
    while (*(l->curr)) {
        Lexeme *text = ViewLexer_Text(l);
        if (text) {
            printf("  lex:\tfound text\n");
            if (!tail) {
                root = tail = text;
            } else {
                tail->next       = text;
                tail->next->prev = tail;
                tail             = tail->next;
            }
            while (tail->next) {
                tail = tail->next;
            }
        }

        Lexeme *code = ViewLexer_Code(l);
        if (code) {
            printf("  lex:\tfound code\n");
            if (!tail) {
                root = tail = code;
            } else {
                tail->next       = code;
                tail->next->prev = tail;
                tail             = tail->next;
            }
            while (tail->next) {
                tail = tail->next;
            }
        }

        l->curr++;
    }

    if (!tail) {
        root = tail = ViewLexer_EOF(l);
    } else {
        tail->next       = ViewLexer_EOF(l);
        tail->next->prev = tail;
        tail             = tail->next;
    }

    return root;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Code(Lexer *l) {
    Lexeme *code = 0;

    if (l && strncmp(l->curr, "<cms", 4) == 0) {
        // skip the <cms
        //
        l->curr += 4;

        code = ViewLexer_Word(l);
        if (code) {
            Lexeme *tail = code;
            tail->next = ViewLexer_Word(l);
            while (tail->next) {
                tail->next->prev = tail;
                tail             = tail->next;
                tail->next       = ViewLexer_Word(l);
            }
        }

        // skip the />
        //
        if (*(l->curr)) {
            l->curr += 2;
        }
    }
    
    return code;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_EOF(Lexer *l) {
    Lexeme *t = 0;
    if (l) {
        t = malloc(sizeof(*t));
        memset(t, 0, sizeof(*t));
        t->source = l->source;
        t->line   = l->line;
        t->kind   = lxEOF;
    }
    
    return t;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Text(Lexer *l) {
    Lexeme *text = 0;
    if (l) {
        // gather all characters up to the start of code
        //
        int         startLine = l->line;
        const char *startText = l->curr;
        
        while (*(l->curr) && strncmp(l->curr, "<cms", 4) != 0) {
            if (*(l->curr) == '\n') {
                l->line++;
            }
            l->curr++;
        }

        const char *stopText = l->curr;

        if (startText < stopText) {
            text = malloc(sizeof(*text) + stopText - startText);
            memset(text, 0, sizeof(*text) + stopText - startText);
            text->source = l->source;
            text->line   = startLine;
            text->kind   = lxTEXT;
            memcpy(text->data, startText, stopText - startText);
        }
    }

    return text;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Word(Lexer *l) {
    // words ignore whitespace
    //
    while (*(l->curr) && isspace(*(l->curr))) {
        if (*(l->curr) == '\n') {
            l->line++;
        }
        l->curr++;
    }
    
    int         startLine = l->line;
    const char *startWord = l->curr;

    // handle quoted and non-quoted words
    //
    if (*(l->curr) == '<') {
        while (*(l->curr) && *(l->curr) != '>' && strncmp(l->curr, "/>", 2) != 0) {
            l->curr++;
        }
        if (*(l->curr) == '>') {
            l->curr++;
        }
    } else {
        while (*(l->curr) && !isspace(*(l->curr)) && strncmp(l->curr, "/>", 2) != 0) {
            l->curr++;
        }
    }

    const char *stopWord = l->curr;

    // words ignore trailing whitespace, too
    //
    while (*(l->curr) && isspace(*(l->curr))) {
        if (*(l->curr) == '\n') {
            l->line++;
        }
        l->curr++;
    }
    
    Lexeme *word = 0;

    if (startWord < stopWord) {
        word = malloc(sizeof(*word) + stopWord - startWord);
        memset(word, 0, sizeof(*word) + stopWord - startWord);
        word->source = l->source;
        word->line   = startLine;
        word->kind   = (*startWord == '<') ? lxTEXT : lxWORD;
        memcpy(word->data, startWord, stopWord - startWord);
    }

    return word;
}


#endif
