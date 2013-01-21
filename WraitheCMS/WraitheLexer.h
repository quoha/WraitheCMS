//
//  WraitheLexer.h
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

#ifndef WraitheCMS_WraitheLexer_h
#define WraitheCMS_WraitheLexer_h

//----------------------------------------------------------------------
//
#define lxEOF  -1
#define lxTEXT  1
#define lxWORD  2


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
Lexer  *NewLexer(const char *source, char *data);
Lexeme *ArticleLexer(Lexer *l);
Lexeme *ViewLexer(Lexer *l);

#endif
