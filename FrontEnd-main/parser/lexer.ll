%{
#include <string>
#include "ast.hh"
#include "parser.tab.hh" // Include the generated header with token definitions

#define YY_DECL int yylex()

extern "C" int yywrap() { return 1; }
%}

%option yylineno

%%

\n                     { yylineno++; /* count lines */ }
[ \t]+                 { /* ignore other whitespace */ }
"//".*                 { /* ignore comments */ }

":"                    { return COLON; }
"->"                   { return ARROW; }
";"                    { return SEMICOLON; }
"="                    { return EQUALS; }
"==>"                  { return ARROW; }
"POSTCONDITION:"       { return POSTCONDITION; }
"PRECONDITION:"        { return PRECONDITION; }

"string"               { return TYPE_STRING; }
"int"                  { return TYPE_INT; }
"bool"                 { return TYPE_BOOL; }
"void"                 { return TYPE_VOID; }
"map"                  { return TYPE_MAP; }

"NIL"                  { return NIL; }
"OK"                   { return OK; }

[a-zA-Z][a-zA-Z0-9_']*  { 
                         yylval.stringVal = new std::string(yytext);
                         return IDENTIFIER; 
                       }

[0-9]+                 {
                         yylval.intVal = std::stoi(yytext);
                         return NUMBER;
                       }

\"[^\"]*\"             {
                         // Remove the quotes
                         std::string str(yytext);
                         str = str.substr(1, str.size() - 2);
                         yylval.stringVal = new std::string(str);
                         return STRING_LITERAL;
                       }

"("                    { return LPAREN; }
")"                    { return RPAREN; }
"{"                    { return LBRACE; }
"}"                    { return RBRACE; }
"["                    { return LBRACKET; }
"]"                    { return RBRACKET; }
","                    { return COMMA; }

.                      { /* ignore any other character */ }

%%
