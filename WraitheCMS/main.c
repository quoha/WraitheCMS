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

    int idx;

    const char *searchPath[128];
    for (idx = 0; idx < 128; idx++) {
        searchPath[idx] = 0;
    }

    WraitheCMS_SymTab *symtab = WraitheCMS_NewSymTab();
    if (!symtab) {
        perror("newSymTab");
        return 2;
    }

    searchPath[0] = "/Users/mdhender/Software/WraitheCMS/data/";

    WraitheCMS_Text *siteFile = ReadFile(searchPath, "siteVariables", 0, 0);
    if (!siteFile) {
        perror("siteVariables");
        return 2;
    } else if (!NameValue(symtab, siteFile->text)) {
        printf("error:\tinvalid file 'siteVariables'\n");
        return 2;
    }

    WraitheCMS_Source source;
    source.source = "article.tpl";
    source.line   = 1;
    source.curr   = 0;
    source.data   = ReadFile(searchPath, source.source, 0, 0);
    if (!source.data) {
        perror(source.source);
        return 2;
    }
    printf(">>>%s<<<\n\n", source.data->text);

    WraitheCMS_Stack *stack = WraitheCMS_NewStack();
    WraitheCMS_Stack_PushTop(stack, 0);
    WraitheCMS_Stack_PushTop(stack, WraitheCMS_NewText("true", -1));

    if (!ViewParse(symtab, stack, searchPath, &source)) {
        printf("error:\tunable to successfully parse the view file '%s'\n", source.source);
        WraitheCMS_Stack_Dump(stack);
        return 2;
    }

    printf(" info:\tsuccessfully ran the view file %s\n", source.source);

    WraitheCMS_Stack_Dump(stack);

    return 0;
}

