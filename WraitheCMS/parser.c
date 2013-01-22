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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------------
//
#define lxEOF    -1
#define lxTEXT    1
#define lxWORD    2
#define lxIF      3
#define lxELSE    4
#define lxENDIF   5
#define lxINCLUDE 6


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
    int         length;
    Lexeme     *prev;
    Lexeme     *next;
    char        data[1];
};


//----------------------------------------------------------------------
//
static Lexeme *ViewParser(WraitheCMS_SymTab *symtab, WraitheCMS_Stack *stack, const char **searchPath, Lexeme *lex, int doExecute, int ifLevel);
static Lexeme *ViewLexer(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Code(WraitheCMS_Source *source);
static Lexeme *ViewLexer_EOF(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Text(WraitheCMS_Source *source);
static Lexeme *ViewLexer_Word(WraitheCMS_Source *source);


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
// all ASTs start and end with no-ops
//
int ViewParse(WraitheCMS_SymTab *symtab, WraitheCMS_Stack *stack, const char **searchPath, WraitheCMS_Source *source) {
    // make sure we start at the top of the source
    //
    source->line = 1;
    source->curr = source->data->text;

    Lexeme *lex = ViewLexer(source);
    if (!lex) {
        perror("ViewLexer failed");
        return 0;
    }

    if (0) {
        Lexeme *view = lex;
        while (view) {
            printf(" view:\t%2d %2d >>> %s\n", view->line, view->kind, view->data);
            view = view->next;
        }
    }

    Lexeme *result = ViewParser(symtab, stack, searchPath, lex, 1, 0);
    if (result && result->kind == lxEOF) {
        return 1;
    }

    return 0;
}


//----------------------------------------------------------------------
//
//    view := word* END_OF_INPUT
//    word := TEXT | WORD | if
//    if   := IF word* ( ELSE word* )? ENDIF
//
//    view := word* END_OF_INPUT
//
Lexeme *ViewParser(WraitheCMS_SymTab *symtab, WraitheCMS_Stack *stack, const char **searchPath, Lexeme *lex, int doExecute, int ifLevel) {
    while (lex && lex->kind != lxEOF) {
        // normal action for text is to push it onto the stack
        if (lex->kind == lxTEXT) {
            printf("stack:\t%*c %-8s %s\n", ifLevel*2 + 1, '.', doExecute ? "push" : "ignore", lex->data);
            if (doExecute) {
                WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText(lex->data, lex->length));
            }
            lex = lex->next;
        } else if (lex->kind == lxWORD) {
            printf(".word:\t%*c %-8s %s\n", ifLevel*2 + 1, '.', lex->data, doExecute ? "exec" : "ignore");
            if (doExecute) {
                // lookup word
                WraitheCMS_Symbol *s = WraitheCMS_SymTab_Find(symtab, lex->data);
                if (!s) {
                    printf(" view:\twarn -- pushing null word '%s'\n", lex->data);
                    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText("", 0));
                } else {
                    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText(s->value, (int)strlen(s->value)));
                }
            }
            lex = lex->next;
        } else if (lex->kind == lxIF) {
            // check the state of the token
            int truth = ifLevel - 1;
            
            printf(".word:\t%*c %-8s %6s %s\n", ifLevel*2 + 1, '.', lex->data, doExecute ? "exec" : "ignore", truth ? "true" : "false");
            Lexeme *lIf = lex;
            
            // execute the TRUE branch
            //
            lex = ViewParser(symtab, stack, searchPath, lex->next, doExecute ? truth : 0, ifLevel+1);
            if (!lex) {
                printf("parser:\tif on line %d was not terminated\n", lIf->line);
                return 0;
            }
            
            // execute the FALSE branch
            //
            if (lex->kind == lxELSE) {
                printf(".word:\t%*c %-8s %s\n", ifLevel*2 + 1, '.', lex->data, !truth ? "exec" : "ignore");
                lex = ViewParser(symtab, stack, searchPath, lex->next, doExecute ? !truth : 0, ifLevel+1);
                if (!lex) {
                    printf("parser:\tif on line %d was not terminated with else ... endif\n", lIf->line);
                    return 0;
                }
            }
            
            // confirm that we have an ENDIF
            //
            if (lex->kind != lxENDIF) {
                printf("parser:\tif on line %d was not terminated with endif\n", lIf->line);
                return 0;
            }
            lex = lex->next;
        } else if (lex->kind == lxELSE) {
            return lex;
        } else if (lex->kind == lxENDIF) {
            return lex;
        } else if (lex->kind == lxINCLUDE) {
            if (doExecute) {
                WraitheCMS_Text *fileName = WraitheCMS_Stack_PopTop(stack);
                if (!fileName || fileName->isNull || fileName->text[0] == 0) {
                    printf(" view:\twarning -- ignoring null 'include'\n");
                } else {
                    printf(" view:\tinclude '%s'\n", fileName->text);

                    WraitheCMS_Source source;
                    source.source = fileName->text;
                    source.line   = 1;
                    source.curr   = 0;
                    source.data   = ReadFile(searchPath, source.source, 0, 0);
                    if (!source.data) {
                        perror(source.source);
                        printf(" view:\terror loading file '%s'\n", source.source);
                        return 0;
                    }
                    printf(">>>%s<<<\n\n", source.data->text);
                    
                    if (!ViewParse(symtab, stack, searchPath, &source)) {
                        printf(" view:\terror including file '%s'\n", source.source);
                        return 0;
                    }
                }
            }
            lex = lex->next;
        } else {
            printf("....?:\t%*c %-6s %s\n", ifLevel*2 + 1, '.', doExecute ? "exec" : "ignore", lex->data);
            lex = lex->next;
        }
    }
    return lex;
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
            text->length = (int)(stopText - startText);
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
        int length = (int) (stopWord - startWord);
        if (word->kind == lxTEXT) {
            // need to strip the < and > from the text
            //
            length--;
            if (length > 0 && startWord[length] == '>') {
                length--;
            }
            startWord++;
        }

        word->length = length;
        memcpy(word->data, startWord, length);

        if (word->kind == lxWORD && strcmp(word->data, "if") == 0) {
            word->kind = lxIF;
        } else if (word->kind == lxWORD && strcmp(word->data, "else") == 0) {
            word->kind = lxELSE;
        } else if (word->kind == lxWORD && strcmp(word->data, "endif") == 0) {
            word->kind = lxENDIF;
        } else if (word->kind == lxWORD && strcmp(word->data, "include") == 0) {
            word->kind = lxINCLUDE;
        }
    }
    
    return word;
}


