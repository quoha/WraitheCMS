//
//  readFile.c
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
#include <sys/stat.h>


//----------------------------------------------------------------------
//
// Text(fileName, forceNewLine, trimTrailingNewLine)
//   reads file into data
// fileName            name of file to read data from
// forceNewLine        if true, ensure that the buffer ends on a new-line
// trimTrailingNewLine if true, remove last new-line from file
//
// will leave text set to NULL and isNull set to TRUE if there are errors
//
WraitheCMS_Text *ReadFile(const char *fileName, int forceNewLine, int trimTrailingNewline) {
    WraitheCMS_Text *text = 0;

	struct stat statBuf;
	if (stat(fileName, &statBuf) == 0) {
        // allocate enough space for the file and the new line plus nil terminator
        //
        text = malloc(sizeof(*text) * (statBuf.st_size + 2));

        if (text) {
            // set those two extra bytes to zero so that we don't forget to
            // do it later. they ensure that we end up nil-terminated.
            //
            text->length                 = (int)statBuf.st_size;
            text->text[text->length    ] = 0;
            text->text[text->length + 1] = 0;

            // only do the file read if it has data in it
            //
            if (statBuf.st_size > 0) {
                FILE *fp = fopen(fileName, "r");
                if (!fp || fread(text->text, statBuf.st_size, 1, fp) != 1) {
                    // delete text on any error
                    //
                    free(text);
                    text = 0;
                }
                if (fp) {
                    fclose(fp);
                }
            }
        }
        
        // do the logic to force or trim trailing newline
        //
        if (text) {
            if (forceNewLine) {
                if (text->length == 0) {
                    text->text[0] = '\n';
                    text->text[1] = 0;
                } else if (text->text[text->length - 1] != '\n') {
                    text->text[text->length] = '\n';
                }
            } else if (trimTrailingNewline) {
                if (text->length > 0 && text->text[text->length - 1] != '\n') {
                    text->text[text->length - 1] = 0;
                }
            }

            text->isNull = (text->text[0] == 0) ? 1 : 0;

        }
    }

    return text;
}

