//
//  parmfile.c
//  WraitheCMS
//
//  Created by Michael Henderson on 1/22/13.
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
#include <string.h>
#include <ctype.h>

int NameValue(WraitheCMS_SymTab *symtab, char *buffer) {
    char *s = buffer;
    while (*s) {
        char *p   = s;
        char *end = s;
        while (*end && *end != '\n') {
            end++;
        }
        s = end;
        if (*s) {
            *(s++) = 0;
        }
        
        if (isspace(*p)) {
            while (*p && isspace(*p)) {
                *(p++) = 0;
            }
            continue;
        }
        
        // name ( '=' ('"' value '"')? )? ';'
        //
        char *value = 0;
        char *name  = p;
        while (*p && !isspace(*p)) {
            p++;
        }
        while (*p && isspace(*p)) {
            *(p++) = 0;
        }
        
        if (*p == '=') {
            *(p++) = 0;
            while (*p && isspace(*p)) {
                *(p++) = 0;
            }
            if (*p == ';') {
                // end of line
                value = p;
            } else if (*p == '"' || *p == '\'') {
                char q = *p;
                *(p++) = 0;
                value = p;
                while (*p) {
                    if (*p == '\\' && *(p+1) == q) {
                        p += 2;
                    } else if (*p == q) {
                        break;
                    } else {
                        p++;
                    }
                }
                if (*p != q) {
                    printf("error:\tunterminated %c in file\n", q);
                    return 0;
                }
                
                while (*p && *p != ';') {
                    *(p++) = 0;
                }
                
                char *x = value;
                while (*x) {
                    if (*x == '\\' && *(x + 1) == q) {
                        char *src = x + 1;
                        char *tgt = x;
                        while (*src) {
                            *(tgt++) = *(src++);
                        }
                        *tgt = 0;
                    }
                    x++;
                }
            } else {
                value = p;
                while (*p && *p != ';') {
                    p++;
                }
            }
        }
        
        if (*p == ';') {
            *(p++) = 0;
        } else if (strcmp(name, "<@<@>@>") == 0) {
            name  = "articleText";
            value = s;
            while (*s) {
                s++;
            }
        } else {
            printf("\nerror:\tinvalid line\n");
            return 0;
        }

        WraitheCMS_SymTab_Add(symtab, name, value);
    }

    return 1;
}

