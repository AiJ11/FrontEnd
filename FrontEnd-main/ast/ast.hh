#ifndef AST_HH
#define AST_HH

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "ASTVis.hpp"

using namespace std;

enum class HTTPResponseCode {
    OK_200,
    CREATED_201,
    BAD_REQUEST_400,
};

enum ExpressionType {
    MAP,
    VAR,
    STRING,
    NUM,
    TUPLE,
    SET,
    FUNCTIONCALL_EXPR,
    POLYMORPHIC_FUNCTIONCALL_EXPR
};

enum TypeExpression {
    TYPE_CONST,
    TYPE_VARIABLE,
    FUNC_TYPE,
    MAP_TYPE,
    SET_TYPE,
    TUPLE_TYPE
};

enum StatementType {
    ASSIGN,
    FUNCTIONCALL_STMT,
    DECL,
};

// Forward declarations
class TypeExpr;
class Expr;
class Var;
class FuncCall;
class Num;
class String;
class Set;
class Map;
class Tuple;
class Decl;
class FuncDecl;
class Init;
class Response;
class APIcall;
class API;
class Spec;
class Stmt;
class Assign;
class FuncCallStmt;
class Program;

class TypeExpr {
public:
    virtual ~TypeExpr() = default;
    virtual void accept(ASTVisitor &visitor) const = 0;
    virtual std::unique_ptr<TypeExpr> clone() const = 0;
    TypeExpression typeExpression;

protected:
    explicit TypeExpr(TypeExpression typeExpr) : typeExpression(typeExpr) {}
};

class TypeConst : public TypeExpr {
public:
    explicit TypeConst(string name) : TypeExpr(TYPE_CONST), name(move(name)) {}
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypeExpr> clone() const override { return make_unique<TypeConst>(name); }
    
    string name;
};

class FuncType : public TypeExpr {
public:
    FuncType(vector<unique_ptr<TypeExpr>> params, unique_ptr<TypeExpr> returnType)
        : TypeExpr(FUNC_TYPE), params(move(params)), returnType(move(returnType)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypeExpr> clone() const override {
        vector<unique_ptr<TypeExpr>> clonedParams;
        for (const auto &param : params) {
            clonedParams.push_back(param->clone());
        }
        return make_unique<FuncType>(move(clonedParams), returnType ? returnType->clone() : nullptr);
    }
    
    vector<unique_ptr<TypeExpr>> params;
    unique_ptr<TypeExpr> returnType;
};

class MapType : public TypeExpr {
public:
    MapType(unique_ptr<TypeExpr> domain, unique_ptr<TypeExpr> range)
        : TypeExpr(MAP_TYPE), domain(move(domain)), range(move(range)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypeExpr> clone() const override {
        return make_unique<MapType>(domain ? domain->clone() : nullptr, 
                                  range ? range->clone() : nullptr);
    }
    
    unique_ptr<TypeExpr> domain;
    unique_ptr<TypeExpr> range;
};

    /*class TupleType : public TypeExpr {
public:
    explicit TupleType(vector<unique_ptr<TypeExpr>> elements)
        : TypeExpr(TUPLE_TYPE), elements(move(elements)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypeExpr> clone() const override {
        vector<unique_ptr<TypeExpr>> clonedElements;
        for (const auto &elem : elements) {
            clonedElements.push_back(elem->clone());
        }
        return make_unique<TupleType>(move(clonedElements));
    }
    
    vector<unique_ptr<TypeExpr>> elements;
};*/

class TupleType : public TypeExpr {
    public:
        explicit TupleType(std::vector<std::unique_ptr<TypeExpr>> elements)
            : TypeExpr(TypeExpression::TUPLE_TYPE), elements(std::move(elements)) {}
        
        void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
        
        std::vector<std::unique_ptr<TypeExpr>> elements;
    };

class SetType : public TypeExpr {
public:
    explicit SetType(unique_ptr<TypeExpr> elementType)
        : TypeExpr(SET_TYPE), elementType(move(elementType)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypeExpr> clone() const override {
        return make_unique<SetType>(elementType ? elementType->clone() : nullptr);
    }
    
    unique_ptr<TypeExpr> elementType;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ASTVisitor &visitor) const = 0;
    ExpressionType expressionType;

protected:
    explicit Expr(ExpressionType type) : expressionType(type) {}
};

class Var : public Expr {
public:
    explicit Var(string name) : Expr(VAR), name(move(name)) {}
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    string name;
};

class FuncCall : public Expr {
public:
    FuncCall(string name, vector<unique_ptr<Expr>> args)
        : Expr(FUNCTIONCALL_EXPR), name(move(name)), args(move(args)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    string name;
    vector<unique_ptr<Expr>> args;
};

class Num : public Expr {
public:
    explicit Num(int value) : Expr(NUM), value(value) {}
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    int value;
};

class String : public Expr {
public:
    explicit String(string value) : Expr(STRING), value(move(value)) {}
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    string value;
};

class Set : public Expr {
public:
    explicit Set(vector<unique_ptr<Expr>> elements)
        : Expr(SET), elements(move(elements)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    vector<unique_ptr<Expr>> elements;
};

class Map : public Expr {
public:
    explicit Map(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> value)
        : Expr(MAP), value(move(value)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> value;
};

class Tuple : public Expr {
    public:
        explicit Tuple(std::vector<std::unique_ptr<Expr>> elements)
            : Expr(ExpressionType::TUPLE), elements(std::move(elements)) {}
        
        void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
        
        std::vector<std::unique_ptr<Expr>> elements;
    };

class Decl {
public:
    Decl(string name, unique_ptr<TypeExpr> type)
        : name(move(name)), type(move(type)) {}
    
    virtual ~Decl() = default;
    virtual void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    string name;
    unique_ptr<TypeExpr> type;
};

class FuncDecl {
public:
    FuncDecl(string name,
             vector<unique_ptr<TypeExpr>> params,
             pair<HTTPResponseCode, vector<unique_ptr<TypeExpr>>> returnType)
        : name(move(name)), params(move(params)), returnType(move(returnType)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    string name;
    vector<unique_ptr<TypeExpr>> params;
    pair<HTTPResponseCode, vector<unique_ptr<TypeExpr>>> returnType;
};

class Init {
public:
    Init(string varName, unique_ptr<Expr> expr)
        : varName(move(varName)), expr(move(expr)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    string varName;
    unique_ptr<Expr> expr;
};

class Response {
public:
    Response(HTTPResponseCode code, unique_ptr<Expr> expr)
        : code(code), expr(move(expr)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    HTTPResponseCode code;
    unique_ptr<Expr> expr;
};

class APIcall {
public:
    APIcall(unique_ptr<FuncCall> call, Response response)
        : call(move(call)), response(move(response)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    unique_ptr<FuncCall> call;
    Response response;
};

class API {
public:
    API(unique_ptr<Expr> pre, unique_ptr<APIcall> call, Response response)
        : pre(move(pre)), call(move(call)), response(move(response)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    unique_ptr<Expr> pre;
    unique_ptr<APIcall> call;
    Response response;
};

class Spec {
public:
    Spec(vector<unique_ptr<Decl>> globals,
         vector<unique_ptr<Init>> init,
         vector<unique_ptr<FuncDecl>> functions,
         vector<unique_ptr<API>> blocks)
        : globals(move(globals)), init(move(init)), 
          functions(move(functions)), blocks(move(blocks)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    vector<unique_ptr<Decl>> globals;
    vector<unique_ptr<Init>> init;
    vector<unique_ptr<FuncDecl>> functions;
    vector<unique_ptr<API>> blocks;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(ASTVisitor &visitor) const = 0;
    StatementType statementType;

protected:
    explicit Stmt(StatementType type) : statementType(type) {}
};

class Assign : public Stmt {
public:
    Assign(unique_ptr<Var> left, unique_ptr<Expr> right)
        : Stmt(ASSIGN), left(move(left)), right(move(right)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    unique_ptr<Var> left;
    unique_ptr<Expr> right;
};

class FuncCallStmt : public Stmt {
public:
    explicit FuncCallStmt(unique_ptr<FuncCall> call)
        : Stmt(FUNCTIONCALL_STMT), call(move(call)) {}
    
    void accept(ASTVisitor &visitor) const override { visitor.visit(*this); }
    
    unique_ptr<FuncCall> call;
};

class Program {
public:
    explicit Program(vector<unique_ptr<Stmt>> statements)
        : statements(move(statements)) {}
    
    void accept(ASTVisitor &visitor) const { visitor.visit(*this); }
    
    vector<unique_ptr<Stmt>> statements;
};

#endif // AST_HH
