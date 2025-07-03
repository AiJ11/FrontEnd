%{
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "ast.hh"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

// Root of our AST
Spec* astRoot = nullptr;

// Helper variables for building the AST
std::vector<std::unique_ptr<Decl>> globals;
std::vector<std::unique_ptr<Init>> inits;
std::vector<std::unique_ptr<FuncDecl>> functions;
std::vector<std::unique_ptr<API>> apis;

%}

%define parse.error verbose

%union {
    int intVal;
    std::string* stringVal;
    TypeExpr* typeExpr;
    Expr* expr;
    Var* var;
    FuncCall* funcCall;
    std::vector<std::unique_ptr<Expr>>* exprList;
    std::vector<std::unique_ptr<TypeExpr>>* typeList;
    Decl* decl;
    FuncDecl* funcDecl;
    Spec* spec;
    API* api;
    HTTPResponseCode httpCode;
}

/* Define all tokens here */
%token <stringVal> IDENTIFIER STRING_LITERAL
%token <intVal> NUMBER
%token COLON ARROW SEMICOLON
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET COMMA
%token EQUALS
%token PRECONDITION POSTCONDITION
%token TYPE_STRING TYPE_INT TYPE_BOOL TYPE_VOID TYPE_MAP
%token NIL OK

%type <typeExpr> type_expr
%type <expr> expr map_access map_update
%type <var> var
%type <funcCall> func_call
%type <exprList> expr_list
%type <typeList> type_list
%type <decl> global_decl
%type <funcDecl> func_decl
%type <api> api_block
%type <spec> program
%type <httpCode> http_code response_code

%start program

%%

program:
    declarations { 
        astRoot = new Spec(std::move(globals), std::move(inits), 
                           std::move(functions), std::move(apis));
        $$ = astRoot;
    }
    ;

declarations:
    declarations global_decl SEMICOLON {
        globals.push_back(std::unique_ptr<Decl>($2));
    }
    | declarations func_decl SEMICOLON {
        functions.push_back(std::unique_ptr<FuncDecl>($2));
    }
    | declarations api_block {
        apis.push_back(std::unique_ptr<API>($2));
    }
    | /* empty */ {}
    ;

global_decl:
    IDENTIFIER COLON type_expr {
        $$ = new Decl(*$1, std::unique_ptr<TypeExpr>($3));
        delete $1;
    }
    ;

func_decl:
    IDENTIFIER COLON type_list {
        // For now, default HTTP response code to OK_200
        std::vector<std::unique_ptr<TypeExpr>> returnTypes;
        
        // The last type in the list is the return type
        if (!$3->empty()) {
            returnTypes.push_back(std::move($3->back()));
            $3->pop_back();
        }
        
        $$ = new FuncDecl(*$1, std::move(*$3), 
                          std::make_pair(HTTPResponseCode::OK_200, std::move(returnTypes)));
        delete $1;
        delete $3;
    }
    ;

type_list:
    type_expr {
        $$ = new std::vector<std::unique_ptr<TypeExpr>>();
        $$->push_back(std::unique_ptr<TypeExpr>($1));
    }
    | type_list ARROW type_expr {
        $1->push_back(std::unique_ptr<TypeExpr>($3));
        $$ = $1;
    }
    ;

type_expr:
    TYPE_STRING { $$ = new TypeConst("string"); }
    | TYPE_INT  { $$ = new TypeConst("int"); }
    | TYPE_BOOL { $$ = new TypeConst("bool"); }
    | TYPE_VOID { $$ = new TypeConst("void"); }
    | TYPE_MAP  { $$ = new TypeConst("map"); }
    ;

/* Full API block implementation */
api_block:
    IDENTIFIER COLON PRECONDITION expr func_call ARROW LPAREN response_code RPAREN POSTCONDITION expr {
        // Handle full API specification:
        // SIGNUP_OK:
        // PRECONDITION: U[uid] = NIL
        // signup(uid, p) ==> (OK)
        // POSTCONDITION: U' = U[uid -> p]
        
        auto preExpr = $4;       // Precondition expression (U[uid] = NIL)
        auto funcCallExpr = $5;  // Function call (signup(uid, p))
        auto postExpr = $11;     // Postcondition expression (U' = U[uid -> p])
        
        auto apiCall = new APIcall(
            std::unique_ptr<FuncCall>(funcCallExpr),
            Response($8, std::unique_ptr<Expr>(new Var("result")))
        );
        
        $$ = new API(
            std::unique_ptr<Expr>(preExpr),
            std::unique_ptr<APIcall>(apiCall),
            Response($8, std::unique_ptr<Expr>(postExpr))
        );
        
        delete $1;
    }
    ;

expr:
    var { $$ = $1; }
    | STRING_LITERAL { $$ = new String(*$1); delete $1; }
    | NUMBER { $$ = new Num($1); }
    | func_call { $$ = $1; }
    | map_access { $$ = $1; }
    | map_update { $$ = $1; }
    | expr EQUALS expr {
        // For expressions like "U[uid] = NIL"
        auto left = $1;
        auto right = $3;
        
        // Create a function call for equality check
        auto funcName = new std::string("equals");
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::unique_ptr<Expr>(left));
        args.push_back(std::unique_ptr<Expr>(right));
        
        $$ = new FuncCall(*funcName, std::move(args));
        delete funcName;
    }
    | NIL {
        // Represent NIL as a special variable
        $$ = new Var("NIL");
    }
    ;

map_access:
    var LBRACKET expr RBRACKET {
        // For expressions like "U[uid]"
        auto mapVar = $1;
        auto indexExpr = $3;
        
        // Create a function call for map access
        auto funcName = new std::string("map_access");
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::unique_ptr<Expr>(mapVar));
        args.push_back(std::unique_ptr<Expr>(indexExpr));
        
        $$ = new FuncCall(*funcName, std::move(args));
        delete funcName;
    }
    ;

map_update:
    var LBRACKET expr ARROW expr RBRACKET {
        // For expressions like "U[uid -> p]"
        auto mapVar = $1;
        auto keyExpr = $3;
        auto valueExpr = $5;
        
        // Create a function call for map update
        auto funcName = new std::string("map_update");
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::unique_ptr<Expr>(mapVar));
        args.push_back(std::unique_ptr<Expr>(keyExpr));
        args.push_back(std::unique_ptr<Expr>(valueExpr));
        
        $$ = new FuncCall(*funcName, std::move(args));
        delete funcName;
    }
    ;

var:
    IDENTIFIER { $$ = new Var(*$1); delete $1; }
    ;

func_call:
    IDENTIFIER LPAREN expr_list RPAREN {
        $$ = new FuncCall(*$1, std::move(*$3));
        delete $1;
        delete $3;
    }
    | IDENTIFIER LPAREN RPAREN {
        $$ = new FuncCall(*$1, std::vector<std::unique_ptr<Expr>>());
        delete $1;
    }
    ;

expr_list:
    expr {
        $$ = new std::vector<std::unique_ptr<Expr>>();
        $$->push_back(std::unique_ptr<Expr>($1));
    }
    | expr_list COMMA expr {
        $1->push_back(std::unique_ptr<Expr>($3));
        $$ = $1;
    }
    ;

response_code:
    OK { $$ = HTTPResponseCode::OK_200; }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Error (line " << yylineno << "): " << s << std::endl;
}