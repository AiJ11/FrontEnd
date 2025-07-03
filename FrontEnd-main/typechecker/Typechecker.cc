#include "ast.hh"
#include "Typechecker.hh"
#include <sstream>
#include <iostream>
#include <cassert>

static std::string kindName(TyKind k) {
    switch (k) {
        case TyKind::Int: return "int";
        case TyKind::String: return "string";
        case TyKind::Bool: return "bool";
        case TyKind::Map: return "map";
        case TyKind::Void: return "void";
        default: return "unknown";
    }
}

std::string Type::str() const { return kindName(kind); }

Typechecker::Typechecker() {}

bool Typechecker::typeCheckSpec(const Spec& spec) {
    globals.clear();
    variableEnv.clear();
    functionEnv.clear();
    errs.clear();

    collectGlobals(spec.globals, spec.init);
    collectFunctions(spec.functions);

    for (const auto& api : spec.blocks) {
        checkAPIBlock(*api);
    }

    return errs.empty();
}

void Typechecker::collectGlobals(const std::vector<std::unique_ptr<Decl>>& decls,
                               const std::vector<std::unique_ptr<Init>>& inits) {
    // Standard variables
    variableEnv["uid"] = Type(TyKind::String);
    variableEnv["p"] = Type(TyKind::String);
    variableEnv["NIL"] = Type(TyKind::String);
    variableEnv["U_prime"] = Type(TyKind::Map);
    variableEnv["OK"] = Type(TyKind::String);

    // Declared globals
    for (const auto& decl : decls) {
        if (decl->type->typeExpression == TypeExpression::MAP_TYPE) {
            globals[decl->name] = Type(TyKind::Map);
        } else {
            globals[decl->name] = Type(TyKind::String);
        }
    }

    // Initializations
    for (const auto& init : inits) {
        variableEnv[init->varName] = typecheckExpr(init->expr.get());
    }
}

void Typechecker::collectFunctions(const std::vector<std::unique_ptr<FuncDecl>>& funcs) {
    for (const auto& func : funcs) {
        std::vector<Type> paramTypes;
        for (const auto& param : func->params) {
            paramTypes.push_back(Type(TyKind::String)); // Simplified typing
        }
        
        Type returnType = Type(TyKind::String);
        functionEnv[func->name] = {paramTypes, returnType};
    }
}

void Typechecker::checkAPIBlock(const API& api) {
    checkPrecondition(api.pre.get());
    Type callTy = checkCall(*api.call);
    checkPostcondition(api.response, callTy);
}

Type Typechecker::typecheckExpr(const Expr* e) {
    switch (e->expressionType) {
        case ExpressionType::VAR: return typecheckVar(static_cast<const Var*>(e));
        case ExpressionType::FUNCTIONCALL_EXPR: {
            auto call = static_cast<const FuncCall*>(e);
            if (call->name == "map_access") return typecheckMapAccess(call);
            if (call->name == "equals") return typecheckBinExpr(call);
            return typecheckFuncCall(call);
        }
        case ExpressionType::STRING: return Type(TyKind::String);
        case ExpressionType::NUM: return Type(TyKind::Int);
        default:
            report("Unknown expression type");
            return Type();
    }
}

Type Typechecker::typecheckVar(const Var* v) {
    if (variableEnv.count(v->name)) return variableEnv[v->name];
    if (globals.count(v->name)) return globals[v->name];
    report("Undefined variable: " + v->name);
    return Type();
}

Type Typechecker::typecheckFuncCall(const FuncCall* f) {
    if (!functionEnv.count(f->name)) {
        report("Undefined function: " + f->name);
        return Type();
    }

    const auto& [paramTypes, returnType] = functionEnv[f->name];
    if (f->args.size() != paramTypes.size()) {
        report("Arity mismatch for function: " + f->name);
        return Type();
    }

    for (size_t i = 0; i < f->args.size(); i++) {
        Type argType = typecheckExpr(f->args[i].get());
        if (argType != paramTypes[i]) {
            report("Type mismatch in argument " + std::to_string(i+1) + 
                  " for function " + f->name);
        }
    }

    return returnType;
}

Type Typechecker::typecheckMapAccess(const FuncCall* f) {
    if (f->args.size() != 2) {
        report("map_access requires exactly 2 arguments");
        return Type();
    }
    
    Type base = typecheckExpr(f->args[0].get());
    Type key = typecheckExpr(f->args[1].get());
    
    if (base.kind != TyKind::Map) {
        report("First argument to map_access must be a map");
    }
    if (key.kind != TyKind::String) {
        report("Second argument to map_access must be a string");
    }
    
    return Type(TyKind::String);
}

Type Typechecker::typecheckBinExpr(const FuncCall* f) {
    if (f->args.size() != 2) {
        report("equals requires exactly 2 arguments");
        return Type();
    }
    
    Type lhs = typecheckExpr(f->args[0].get());
    Type rhs = typecheckExpr(f->args[1].get());
    
    if (lhs != rhs) {
        report("Type mismatch in equality comparison");
    }
    
    return Type(TyKind::Bool);
}

Type Typechecker::checkCall(const APIcall& call) {
    return typecheckFuncCall(call.call.get());
}

void Typechecker::checkPrecondition(const Expr* pre) {
    if (pre) typecheckExpr(pre);
}

void Typechecker::checkPostcondition(const Response& post, Type callTy) {
    if (callTy.kind == TyKind::Unknown) {
        report("Call result type unknown in postcondition");
    }
    if (post.expr) typecheckExpr(post.expr.get());
}

const std::vector<std::string>& Typechecker::errors() const {
    return errs;
}

void Typechecker::report(const std::string& msg) {
    errs.push_back(msg);
}