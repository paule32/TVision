/*------------------------------------------------------------*/
/* filename -       edits.cpp                                 */
/*                                                            */
/* function(s)                                                */
/*                  TEditor member functions                  */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TEditor
#define Uses_TText
#include <tvision/tv.h>

#include <ctype.h>
#include <cctype>

char TEditor::bufChar( uint P ) { return buffer[bufPtr(P)]; }
uint TEditor::bufPtr ( uint P ) { return P < curPtr ? P : P + gapLen; }

bool tokenIsComment = false;

#include <string>

void
TEditor::formatLine(
    TScreenCell *DrawBuf,
    uint P,
    int Width,
    TAttrPair
    ) { 
    
    TSpan<TScreenCell> Cells(DrawBuf, Width);
    TStringView chars;
    TColorAttr  Color;
    std::string token;
    
    int  pos;
    char Ch;
    
    int X = 0;
    Color = EditorTextColor;
    token = "";
    
    while (1) {
        if (P >= bufLen || X >= Width) break;
        tokenIsComment = false;
        chars = bufChars(P);
        Color = EditorTextColor;
        Ch    = chars[0];
        
        // ----------------------------
        // parse white space's ...
        // ----------------------------
        if (Ch == ' ') {
            Color = EditorTextColor;
            while (X < Width) {
                ::setCell(Cells[X], Ch, Color);
                X++;
                P++;
                chars = bufChars(P);
                Ch = chars[0];
                if (Ch != ' ') break;
            }
        }
        
        // ----------------------------
        // parse token for Pascal DSL
        // ----------------------------
        if (((Ch >= 'A') && (Ch <= 'Z'))
        ||  ((Ch >= 'a') && (Ch <= 'z'))) {
            pos = X;
            ::setCell(Cells[X++], Ch, EditorTextColor);
            token.push_back(Ch);
            while (1) {
                if ((P >= bufLen) || (X >= Width)) break;
                P++;
                chars = bufChars(P);
                Ch = chars[0];
                if (((Ch >= 'A') && (Ch <= 'Z'))
                ||  ((Ch >= 'a') && (Ch <= 'z'))
                ||  ((Ch >= '0') && (Ch <= '9'))
                ||  (Ch == '_')) {
                    ::setCell(Cells[X++], Ch, EditorTextColor);
                    token.push_back(Ch);
                }   else break;
            }
            if (X >= Width) break;
            
            // ---------------------------------------------
            // convert token letter's to lower case letter's
            // ---------------------------------------------
            for (char &c : token)
            c = std::tolower(c);
            X = pos;
        
            auto it = EditorSyntaxToken.find(token);
            if (it != EditorSyntaxToken.end()) {
                for (char &c: token) {
                    if (X >= Width) break;
                    ::setCell(Cells[X++], c, it->second);
                }
            }   else {
                for (char &c: token) {
                    if (X >= Width) break;
                    ::setCell(Cells[X++], c, EditorTextColor);
                }
            }
            token = "";
        }
        
        // ----------------------------
        // parse comment's ...
        // ----------------------------
        if (Ch == '&') {
            Color = EditorTextColor;
            ::setCell(Cells[X], '&', Color);
            X++;
            P++;
            chars = bufChars(P);
            Ch = chars[0];
            if (P >= bufLen || X >= Width) break;
            
            // ----------------------------
            // dBase comment &&
            // ----------------------------
            if (Ch == '&') {
                tokenIsComment = true;
                ::setCell(Cells[X-1], Ch, EditorCommentColor);
                if (X < Width) {
                    do  {
                        ::setCell(Cells[X++], Ch, EditorCommentColor);
                        P++;
                        chars = bufChars(P);
                        Ch = chars[0];
                        if (Ch == '\r' || Ch == '\n') {
                            while (X < Width)
                            ::setCell(Cells[X++], ' ', Color);
                            P++;
                            break;
                        }
                    }   while (X < Width);
                    ++P;
                }   else break;
            }   else
            if (Ch == '\r' || Ch == '\n') {
                while (X < Width)
                ::setCell(Cells[X++], ' ', Color);
                P++;
                break;
            }   else {
                ::setCell(Cells[X], Ch, Color);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }
        }   else
        if (Ch == '/') {
            ::setCell(Cells[X], '/', Color);
            X++;
            P++;
            chars = bufChars(P);
            Ch = chars[0];
            if (P >= bufLen || X >= Width) break;
            
            // ----------------------------
            // c++ comment
            // ----------------------------
            if (Ch == '/') {
                tokenIsComment = true;
                ::setCell(Cells[X-1], Ch, EditorCommentColor);
                if (X < Width) {
                    do  {
                        ::setCell(Cells[X++], Ch, EditorCommentColor);
                        P++;
                        chars = bufChars(P);
                        Ch = chars[0];
                        if (Ch == '\r' || Ch == '\n') {
                            while (X < Width)
                            ::setCell(Cells[X++], ' ', Color);
                            P++;
                            break;
                        }
                    }   while (X < Width);
                    ++P;
                }   else break;
            }   else
                
            // ----------------------------
            // c comment
            // ----------------------------
            if (Ch == '*') {
                tokenIsComment = true;
                Color = EditorCommentColor;
                ::setCell(Cells[X-1], '/', Color);
                ::setCell(Cells[X  ], '*', Color);
                X++;
                P++;
                while (1) {
                    if (P >= bufLen || X >= Width) break;
                    chars = bufChars(P);
                    Ch = chars[0];
                    if (Ch == '*') {
                        ::setCell(Cells[X], '*', Color);
                        X++;
                        P++;
                        if (P >= bufLen || X >= Width) break;
                        chars = bufChars(P);
                        Ch = chars[0];
                        if (Ch == '/') {
                            ::setCell(Cells[X], Ch, Color);
                            tokenIsComment = false;
                            Color = EditorTextColor;
                            X++;
                            P++;
                            break;
                        }   else
                        if (Ch == '\r' || Ch == '\n') {
                            while (X < Width)
                            ::setCell(Cells[X++], ' ', Color);
                            P++;
                            break;
                        }   else {
                            ::setCell(Cells[X], Ch, Color);
                            X++;
                            P++;
                            if (P >= bufLen || X >= Width) break;
                        }
                    }   else
                    if (Ch == '\r' || Ch == '\n') {
                        while (X < Width)
                        ::setCell(Cells[X++], ' ', EditorCommentColor);
                        P++;
                        if (P >= bufLen || X >= Width) break;
                    }   else {
                        ::setCell(Cells[X], Ch, EditorCommentColor);
                        X++;
                        P++;
                        if (P >= bufLen || X >= Width) break;
                    }
                }
            }   else {
                ::setCell(Cells[X], Ch, EditorTextColor);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }
        }   else
        if (Ch == '(') {
            ::setCell(Cells[X], '(', EditorTextColor);
            X++;
            P++;
            chars = bufChars(P);
            Ch = chars[0];
            if (P >= bufLen || X >= Width) break;
            
            // ----------------------------
            // old style pascal comment
            // ----------------------------
            if (Ch == '*') {
                tokenIsComment = true;
                Color = EditorCommentColor;
                ::setCell(Cells[X-1], '(', Color);
                ::setCell(Cells[X  ], '*', Color);
                X++;
                P++;
                while (1) {
                    if (P >= bufLen || X >= Width) break;
                    chars = bufChars(P);
                    Ch = chars[0];
                    if (Ch == '*') {
                        ::setCell(Cells[X], '*', Color);
                        X++;
                        P++;
                        if (P >= bufLen || X >= Width) break;
                        chars = bufChars(P);
                        Ch = chars[0];
                        if (Ch == ')') {
                            ::setCell(Cells[X], Ch, Color);
                            tokenIsComment = false;
                            Color = EditorTextColor;
                            X++;
                            P++;
                            break;
                        }   else
                        if (Ch == '\r' || Ch == '\n') {
                            while (X < Width)
                            ::setCell(Cells[X++], ' ', Color);
                            P++;
                            break;
                        }   else {
                            ::setCell(Cells[X], Ch, Color);
                            X++;
                            P++;
                            if (P >= bufLen || X >= Width) break;
                        }
                    }   else
                    if (Ch == '\r' || Ch == '\n') {
                        while (X < Width)
                        ::setCell(Cells[X++], ' ', EditorCommentColor);
                        P++;
                        if (P >= bufLen || X >= Width) break;
                    }   else {
                        ::setCell(Cells[X], Ch, EditorCommentColor);
                        X++;
                        P++;
                        if (P >= bufLen || X >= Width) break;
                    }
                }
            }   else {
                ::setCell(Cells[X], Ch, EditorTextColor);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }
        }   else
        if (Ch == '*') {
            if (tokenIsComment)
            Color = EditorCommentColor; else
            Color = EditorTextColor;
            ::setCell(Cells[X], '*', Color);
            X++;
            P++;
            if (P >= bufLen || X >= Width) break;
            chars = bufChars(P);
            Ch = chars[0];
            
            // ----------------------------
            // dBase comment **
            // ----------------------------
            if (Ch == '*') {
                tokenIsComment = true;
                ::setCell(Cells[X-1], Ch, EditorCommentColor);
                if (X < Width) {
                    do  {
                        ::setCell(Cells[X++], Ch, EditorCommentColor);
                        P++;
                        chars = bufChars(P);
                        Ch = chars[0];
                        if (Ch == '\r' || Ch == '\n') {
                            while (X < Width)
                            ::setCell(Cells[X++], ' ', Color);
                            P++;
                            break;
                        }
                    }   while (X < Width);
                    ++P;
                }   else break;
            }   else
                
            // ----------------------------
            // end of c comment
            // ----------------------------
            if (Ch == '/') {
                tokenIsComment = false;
                Color = EditorTextColor;
                ::setCell(Cells[X], '/', EditorCommentColor);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }   else
                
            // ----------------------------
            // end of pascal comment
            // ----------------------------
            if (Ch == ')') {
                tokenIsComment = false;
                Color = EditorTextColor;
                ::setCell(Cells[X], ')', EditorCommentColor);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }   else {
                Color = EditorCommentColor;
                ::setCell(Cells[X], '*', EditorCommentColor);
                X++;
                P++;
                if (P >= bufLen || X >= Width) break;
            }
        }   else
        if (Ch == '\r' || Ch == '\n') {
            tokenIsComment = false;
            //Color = EditorTextColor;
            //::setCell(Cells[X-1], ' ', Color);
            while (X < Width)
            ::setCell(Cells[X++], ' ', Color);
            P++;
            break;
        }   else
        if (Ch == '\t') {
            if (tokenIsComment)
            Color = EditorCommentColor; else
            Color = EditorTextColor;
            if (X < Width) {
                do {
                    ::setCell(Cells[X++], ' ', Color);
                } while (X%8 != 0 && X < Width);
                ++P;
            }   else break;
        }   else {
            if (tokenIsComment)
            Color = EditorCommentColor; else
            Color = EditorTextColor;
            if (!formatCell(Cells, (uint&) X, chars, P, Color))
            break;
        }
    }
    while (X < Width)
    ::setCell(Cells[X++], ' ', Color);
}

uint TEditor::lineEnd( uint P )
{
    for (uint i = P; i < bufLen; ++i)
    {
        char c = bufChar(i);
        if (c == '\r' || c == '\n')
            return i;
    }
    return bufLen;
}

uint TEditor::lineStart( uint P )
{
    uint i = P;
    while (i--)
    {
        char c = bufChar(i);
        if (c == '\r')
        {
            if ( i + 1 != curPtr && i + 1 != bufLen &&
                 bufChar(i + 1) == '\n' )
                return i + 2;
            return i + 1;
        }
        else if (c == '\n')
            return i + 1;
    }
    return 0;
}

uint TEditor::nextChar( uint P )
{
    if (P + 1 < bufLen)
    {
        if (bufChar(P) == '\r' && bufChar(P + 1) == '\n')
            return P + 2;
        if (encSingleByte)
            return P + 1;
        else
            return P + TText::next(bufChars(P));
    }
    return bufLen;
}

uint TEditor::prevChar( uint P )
{
    if (P > 1)
    {
        if (bufChar(P - 2) == '\r' && bufChar(P - 1) == '\n')
            return P - 2;
        if (encSingleByte)
            return P - 1;
        else
        {
            TStringView t = bufPrevChars(P);
            return P - TText::prev(t, t.size());
        }
    }
    return 0;
}

extern "C" {

int countLines( const char *buf, uint count )
{
    int lines = 0;
    for (uint i = 0; i < count; ++i)
        if (buf[i] == '\r')
        {
            ++lines;
            if (i+1 < count && buf[i+1] == '\n')
                ++i;
        }
        else if (buf[i] == '\n')
            ++lines;
    return lines;
}

uint scan( const char *block, uint size, const char *str )
{
    uint i = 0, len = strlen(str);
    if (len > 0)
        while (i < size)
        {
            uint j = i, k = 0;
            while (j < size && block[j++] == str[k])
                if (++k == len)
                    return i;
            ++i;
        }
    return sfSearchFailed;
}

uint iScan( const char *block, uint size, const char *str )
{
    uint i = 0, len = strlen(str);
    if (len > 0)
        while (i < size)
        {
            uint j = i, k = 0;
            while (j < size && toupper(block[j++]) == toupper(str[k]))
                if (++k == len)
                    return i;
            ++i;
        }
    return sfSearchFailed;
}

}
