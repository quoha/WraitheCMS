//
//  parser.c
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
#include "WraitheLexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------------
//
#define lxEOF  -1
#define lxTEXT  1
#define lxWORD  2
#define lxIF    3
#define lxELSE  4
#define lxENDIF 5


//----------------------------------------------------------------------
//
typedef struct Lexer Lexer;
typedef struct Lexeme Lexeme;


//----------------------------------------------------------------------
//
struct Lexer {
    int         line;
    const char *source;
    const char *curr;
};


//----------------------------------------------------------------------
//
struct Lexeme {
    const char *source;
    int         line;
    int         kind;
    Lexeme     *prev;
    Lexeme     *next;
    char        data[1];
};


//----------------------------------------------------------------------
//
typedef struct PState PState;
struct PState {
    Lexeme *lex;
    void   *root;
    void   *tail;
};


//----------------------------------------------------------------------
//
static Lexeme *ViewLexer(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Code(WraitheCMS_Source *source);
static Lexeme *ViewLexer_EOF(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Text(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Word(WraitheCMS_Source *source);
static int     Accept(PState *ps, int kind);
static int     AcceptElse(PState *ps);
static int     AcceptEndIf(PState *ps);
static int     AcceptIf(PState *ps);
static int     AcceptText(PState *ps);
static int     AcceptWord(PState *ps);
static int     Expect(PState *ps, int kind);
static int     Match(PState *ps, int kind);
static int     ParseIf(PState *ps);
static int     ParseWord(PState *ps);



//----------------------------------------------------------------------
//
//    view := word* END_OF_INPUT
//    word := TEXT | WORD | if
//    if   := IF word* ( ELSE word* )? ENDIF
//
//    view := word* END_OF_INPUT
//
//============================================================================
//
//    view := word* END_OF_INPUT
//
//
//            word*  -> while lexeme in First(word)
//                      do
//                          Accept(lexeme)
//                      end;
//
//
//            END_OF_INPUT -> Accept(END_OF_INPUT);
//

void *ViewParse(WraitheCMS_Source *source) {
    printf("parse:\tentered\n");

    PState ps;
    ps.root = 0;
    ps.tail = 0;

    source->line = 1;
    source->curr = source->data->text;

    ps.lex  = ViewLexer(source);
    if (!ps.lex) {
        perror("ViewLexer");
        return 0;
    }
    Lexeme *view = ps.lex;
    while (view) {
        printf(" view:\t%2d %2d >>> %s\n", view->line, view->kind, view->data);
        view = view->next;
    }

    // parse the input
    //
    while (ps.lex->kind != lxEOF && ParseWord(&ps)) {
        //
    }
    
    if (!Expect(&ps, lxEOF)) {
        // error - did not parse to end of input
        //
        printf("parse:\tfailed - did not find EOF as expected\n");
        return 0;
    }
    
    printf("parse:\texiting\n");
    
    return ps.root;
}


//----------------------------------------------------------------------
//
int Accept(PState *ps, int kind) {
    if (Match(ps, kind)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int AcceptElse(PState *ps) {
    if (Match(ps, lxELSE)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int AcceptEndIf(PState *ps) {
    if (Match(ps, lxENDIF)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int AcceptIf(PState *ps) {
    if (Match(ps, lxIF)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int AcceptText(PState *ps) {
    if (Match(ps, lxTEXT)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int AcceptWord(PState *ps) {
    if (Match(ps, lxWORD)) {
        ps->lex = ps->lex->next;
        return 1;
    }
    return 0;
}


//----------------------------------------------------------------------
//
int Expect(PState *ps, int kind) {
    if (Accept(ps, kind)) {
        return 1;
    }
    printf("parse:\terror:\texpected %d found %d\n", kind, ps->lex ? ps->lex->kind : -1);
    return 0;
}


//----------------------------------------------------------------------
//
int Match(PState *ps, int kind) {
    return (ps->lex && ps->lex->kind == kind) ? 1 : 0;
}


//----------------------------------------------------------------------
//
//    word := TEXT | WORD | if
//
int ParseWord(PState *ps) {
    printf("parseWord line %d %s\n", ps->lex->line, ps->lex->data);
    Lexeme *lex = ps->lex;
    
    if (AcceptText(ps)) {
        return 1;
    }

    if (AcceptWord(ps)) {
        return 1;
    }

    if (ParseIf(ps)) {
        return 1;
    }

    printf("parse:\texpected text, word or if on line %d\n", lex->line);
    printf("\tfound %d\n", lex->kind);
    return 0;
}


//----------------------------------------------------------------------
//
//    if   := IF word* ( ELSE word* )? ENDIF
//
int ParseIf(PState *ps) {
    printf("parseIf   line %d %s\n", ps->lex->line, ps->lex->data);

    Lexeme *pIf = ps->lex;
    if (!AcceptIf(ps)) {
        printf("parse:\texpected if on line %5d\n", pIf->line);
        return 0;
    }

    Lexeme *pThen = 0;
    while (!Match(ps, lxELSE) && !Match(ps, lxENDIF) && !Match(ps, lxEOF)) {
        if (!pThen) {
            pThen = ps->lex;
            printf("parseThen line %d %s\n", pThen->line, pThen->data);
        }
        if (!ParseWord(ps)) {
            printf("parse:\nexpected word on line %5d\n", ps->lex->line);
            return 0;
        }
    }

    Lexeme *pElse = ps->lex;
    printf("parseElse line %d %s\n", pElse->line, pElse->data);
    if (!AcceptElse(ps)) {
        pElse = 0;
    }

    while (!Match(ps, lxENDIF) && !Match(ps, lxEOF)) {
        if (!ParseWord(ps)) {
            printf("parse:\nexpected word on line %5d\n", ps->lex->line);
            return 0;
        }
    }

    Lexeme *pEndIf = ps->lex;
    printf("parseEnIf line %d %s\n", pEndIf->line, pEndIf->data);
    if (!AcceptEndIf(ps)) {
        printf("parse:\nexpected endif on line %d to %d %s\n", pIf->line, pEndIf->line, ps->lex->data);
        return 0;
    }

    return 1;
}


//----------------------------------------------------------------------
// (text | ("<cms" word* "/>") )*
//
Lexeme *ViewLexer(WraitheCMS_Source *source) {
    Lexeme *root = 0;
    Lexeme *tail = 0;
    while (*(source->curr)) {
        Lexeme *text = ViewLexer_Text(source);
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
        
        Lexeme *code = ViewLexer_Code(source);
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
    }
    
    if (!tail) {
        root = tail = ViewLexer_EOF(source);
    } else {
        tail->next       = ViewLexer_EOF(source);
        tail->next->prev = tail;
        tail             = tail->next;
    }
    
    return root;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Code(WraitheCMS_Source *source) {
    Lexeme *code = 0;
    
    if (source && strncmp(source->curr, "<cms", 4) == 0) {
        // skip the <cms
        //
        source->curr += 4;
        
        code = ViewLexer_Word(source);
        if (code) {
            Lexeme *tail = code;
            tail->next = ViewLexer_Word(source);
            while (tail->next) {
                tail->next->prev = tail;
                tail             = tail->next;
                tail->next       = ViewLexer_Word(source);
            }
        }
        
        // skip the />
        //
        if (*(source->curr)) {
            source->curr += 2;
        }
    }
    
    return code;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_EOF(WraitheCMS_Source *source) {
    Lexeme *t = 0;
    if (source) {
        t = malloc(sizeof(*t));
        memset(t, 0, sizeof(*t));
        t->source = source->source;
        t->line   = source->line;
        t->kind   = lxEOF;
    }
    
    return t;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Text(WraitheCMS_Source *source) {
    Lexeme *text = 0;
    if (source) {
        // gather all characters up to the start of code
        //
        int         startLine = source->line;
        const char *startText = source->curr;
        
        while (*(source->curr) && strncmp(source->curr, "<cms", 4) != 0) {
            if (*(source->curr) == '\n') {
                source->line++;
            }
            source->curr++;
        }
        
        const char *stopText = source->curr;
        
        if (startText < stopText) {
            text = malloc(sizeof(*text) + stopText - startText);
            memset(text, 0, sizeof(*text) + stopText - startText);
            text->source = source->source;
            text->line   = startLine;
            text->kind   = lxTEXT;
            memcpy(text->data, startText, stopText - startText);

            char *p = text->data;
            while (*p) {
                if (*p == '\n') {
                    *p = '~';
                }
                p++;
            }
        }
    }
    
    return text;
}


//----------------------------------------------------------------------
//
Lexeme *ViewLexer_Word(WraitheCMS_Source *source) {
    // words ignore whitespace
    //
    while (*(source->curr) && isspace(*(source->curr))) {
        if (*(source->curr) == '\n') {
            source->line++;
        }
        source->curr++;
    }
    
    int         startLine = source->line;
    const char *startWord = source->curr;
    
    // handle quoted and non-quoted words
    //
    if (*(source->curr) == '<') {
        while (*(source->curr) && *(source->curr) != '>' && strncmp(source->curr, "/>", 2) != 0) {
            source->curr++;
        }
        if (*(source->curr) == '>') {
            source->curr++;
        }
    } else {
        while (*(source->curr) && !isspace(*(source->curr)) && strncmp(source->curr, "/>", 2) != 0) {
            source->curr++;
        }
    }
    
    const char *stopWord = source->curr;
    
    // words ignore trailing whitespace, too
    //
    while (*(source->curr) && isspace(*(source->curr))) {
        if (*(source->curr) == '\n') {
            source->line++;
        }
        source->curr++;
    }
    
    Lexeme *word = 0;
    
    if (startWord < stopWord) {
        word = malloc(sizeof(*word) + stopWord - startWord);
        memset(word, 0, sizeof(*word) + stopWord - startWord);
        word->source = source->source;
        word->line   = startLine;
        word->kind   = (*startWord == '<') ? lxTEXT : lxWORD;
        memcpy(word->data, startWord, stopWord - startWord);
        if (word->kind == lxWORD && strcmp(word->data, "if") == 0) {
            word->kind = lxIF;
        } else if (word->kind == lxWORD && strcmp(word->data, "else") == 0) {
            word->kind = lxELSE;
        } else if (word->kind == lxWORD && strcmp(word->data, "endif") == 0) {
            word->kind = lxENDIF;
        }
    }
    
    return word;
}


