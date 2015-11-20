%option prefix="exp_"
%option header-file="exp-lexer.h"
%option outfile="exp-lexer.c"
%option nodefault noyywrap reentrant noyyalloc noyyrealloc noyyfree batch noinput
%option extra-type="exp_parse_env_t"
%{
#include <assert.h>

#include <hre/config.h>
#include <hre/user.h>
#include <hre-io/user.h>
#include <util-lib/chunk_support.h>
#include <ltsmin-lib/exp-parse-env.h>
#include <ltsmin-lib/exp-grammar.h>
#include <ltsmin-lib/exp-syntax.h>

void * yyalloc (size_t bytes, void * yyscanner){
    (void)yyscanner;
    return RTmalloc(bytes);
}
void * yyrealloc (void * ptr, size_t bytes, void * yyscanner){
    (void)yyscanner;
    return RTrealloc(ptr,bytes);
}

void   yyfree (void * ptr, void * yyscanner){
    (void)yyscanner;
    RTfree(ptr);
}


#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"

%}

white_space [ \t]+
number      [0-9]+
string      \"(([[:print:]]{-}[\\\"])|"\\\\"|"\\\"")*\"
action      [A-Za-z][A-Za-z0-9_]*
operator    [~!@<>=\-+/?&\\|*\[\]]+
endofline   (\n|\r|\n\r|\r\n)

%%

{white_space}  {yyextra->linepos+=strlen(yytext);} //white space, but not end of line.

{endofline} {
    yyextra->lineno++;
    yyextra->linepos=0;
}

"%".*   {} //comment

"par" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_PAR,0,yyextra);
}

"in" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_IN,0,yyextra);
}

"end" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_END,0,yyextra);
}

"*" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_STAR,0,yyextra);
}

"_" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_UNDERSCORE,0,yyextra);
}

"->" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_ARROW,0,yyextra);
}

"||" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_BARS,0,yyextra);
}

"," {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_COMMA,0,yyextra);
}

"(" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_LPAR,0,yyextra);
}

")" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_RPAR,0,yyextra);
}

"des" {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_DES,0,yyextra);
}

{string} {
    yyextra->linepos+=strlen(yytext);
    assert(strlen(yytext) >= 2);
    // trim the quote characters
    yytext[strlen(yytext) - 1] = '\0';
    char* str = yytext + sizeof(char);
    int value = SIput(yyextra->strings, str);
    //Print(infoLong, "lexer: STRING: %s %s (%d)", str, unescaped, value);
    expParse(yyextra->parser,TOKEN_STRING,value,yyextra);
}

{action} {
    yyextra->linepos+=strlen(yytext);
    int value = SIput(yyextra->strings, yytext);
    //Print(infoLong, "lexer: STRING: %s %s (%d)", str, unescaped, value);
    expParse(yyextra->parser,TOKEN_ACTION,value,yyextra);
}

{number} {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,TOKEN_NUMBER,atoi(yytext),yyextra);
}

<<EOF>> {
    yyextra->linepos+=strlen(yytext);
    expParse(yyextra->parser,0,0,yyextra);
    return 0;
}


. {
    yyextra->linepos++;
    HREmessage(error,"%s: Unexpected character %s (%x) at line %d, pos %d",
          yyextra->filename, yytext,yytext[0],yyextra->lineno+1,yyextra->linepos+1);
    expParse(yyextra->parser,TOKEN_ERROR,0,yyextra);
    /* Because the same lexer is used for multiple languages, we give the
       parser the opportunity to provide a sensible error message. */
    (void)yyunput;
}

%%


