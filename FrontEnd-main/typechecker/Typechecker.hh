#ifndef TYPECHECKER_HH
#define TYPECHECKER_HH

#include "ast.hh"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

enum class TyKind { Int, String, Bool, Map, Void, Unknown };

struct Type {
    TyKind kind;
    explicit Type(TyKind k = TyKind::Unknown) : kind(k) {}
    bool operator==(const Type &o) const { return kind == o.kind; }
    bool operator!=(const Type &o) const { return !(*this == o); }
    std::string str() const;
};

class Typechecker {
public:
    Typechecker();
    bool typeCheckSpec(const Spec& spec);
    const std::vector<std::string>& errors() const;

private:
    std::unordered_map<std::string, Type> globals;
    std::unordered_map<std::string, Type> variableEnv;
    std::unordered_map<std::string, std::pair<std::vector<Type>, Type>> functionEnv;
    std::vector<std::string> errs;

    void collectGlobals(const std::vector<std::unique_ptr<Decl>>& decls,
                      const std::vector<std::unique_ptr<Init>>& inits);
    void collectFunctions(const std::vector<std::unique_ptr<FuncDecl>>& funcs);
    
    Type typecheckExpr(const Expr* e);
    Type typecheckVar(const Var* v);
    Type typecheckFuncCall(const FuncCall* f);
    Type typecheckMapAccess(const FuncCall* f);
    Type typecheckBinExpr(const FuncCall* f);

    void checkAPIBlock(const API& api);
    Type checkCall(const APIcall& call);
    void checkPrecondition(const Expr* pre);
    void checkPostcondition(const Response& post, Type callTy);

    void report(const std::string& msg);
};

#endif