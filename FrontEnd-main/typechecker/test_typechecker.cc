#include "ast.hh"
#include "Typechecker.hh"
#include "ASTVis.hpp"
#include <iostream>
#include <memory>

class TestVerifier : public ASTVisitor {
    Typechecker& checker;
    bool expectSuccess;
    
public:
    TestVerifier(Typechecker& tc, bool expectSuccess) 
        : checker(tc), expectSuccess(expectSuccess) {}

    void visit(const Spec& spec) override {
        bool result = checker.typeCheckSpec(spec);
        
        if (expectSuccess) {
            if (result) {
                std::cout << "✅ TEST PASSED: Valid specification typechecked successfully\n";
            } else {
                std::cerr << "❌ TEST FAILED (should have passed):\n";
                for (const auto& err : checker.errors()) {
                    std::cerr << "  - " << err << "\n";
                }
            }
        } else {
            if (!result) {
                std::cout << " ❌ TEST Failed: Invalid specification correctly rejected\n";
                std::cout << "Expected errors:\n";
                for (const auto& err : checker.errors()) {
                    std::cout << "  - " << err << "\n";
                }
            } else {
                std::cerr << "❌ TEST FAILED (should have failed)\n";
            }
        }
    }

    // Implement all required visitor methods
    void visit(const TypeConst&) override {}
    void visit(const FuncType&) override {}
    void visit(const MapType&) override {}
    void visit(const TupleType&) override {}
    void visit(const SetType&) override {}
    void visit(const Var&) override {}
    void visit(const FuncCall&) override {}
    void visit(const Num&) override {}
    void visit(const String&) override {}
    void visit(const Set&) override {}
    void visit(const Map&) override {}
    void visit(const Tuple&) override {}
    void visit(const Decl&) override {}
    void visit(const FuncDecl&) override {}
    void visit(const Init&) override {}
    void visit(const Response&) override {}
    void visit(const APIcall&) override {}
    void visit(const API&) override {}
    void visit(const Assign&) override {}
    void visit(const FuncCallStmt&) override {}
    void visit(const Program&) override {}
};

std::unique_ptr<Spec> createMockASTFromSampleAPI(bool makeInvalid = false) {
    // 1. Create and initialize all variables
    auto globals = std::vector<std::unique_ptr<Decl>>();
    auto inits = std::vector<std::unique_ptr<Init>>();
    
    // Main map U
    auto u_type = std::make_unique<MapType>(
        std::make_unique<TypeConst>("string"),
        std::make_unique<TypeConst>("string")
    );
    globals.push_back(std::make_unique<Decl>("U", std::move(u_type)));

    // Required variables
    inits.push_back(std::make_unique<Init>("uid", std::make_unique<String>("user123")));
    inits.push_back(std::make_unique<Init>("p", std::make_unique<String>("password")));
    inits.push_back(std::make_unique<Init>("NIL", std::make_unique<String>("")));
    inits.push_back(std::make_unique<Init>("U_prime", std::make_unique<Var>("U")));

    // 2. Function declaration
    auto signup_args = std::vector<std::unique_ptr<TypeExpr>>();
    signup_args.push_back(std::make_unique<TypeConst>("string"));
    signup_args.push_back(std::make_unique<TypeConst>("string"));
    
    auto signup_returns = std::vector<std::unique_ptr<TypeExpr>>();
    signup_returns.push_back(std::make_unique<TypeConst>("string"));
    
    auto funcs = std::vector<std::unique_ptr<FuncDecl>>();
    funcs.push_back(std::make_unique<FuncDecl>(
        "signup",
        std::move(signup_args),
        std::make_pair(HTTPResponseCode::OK_200, std::move(signup_returns))
    ));

    // 3. Precondition
    auto pre_args = std::vector<std::unique_ptr<Expr>>();
    {
        auto map_access_args = std::vector<std::unique_ptr<Expr>>();
        map_access_args.push_back(std::make_unique<Var>("U"));
        map_access_args.push_back(std::make_unique<Var>("uid"));
        pre_args.push_back(std::make_unique<FuncCall>("map_access", std::move(map_access_args)));
        pre_args.push_back(std::make_unique<Var>("NIL"));
    }
    auto pre = std::make_unique<FuncCall>("equals", std::move(pre_args));

    // 4. API Call
    auto call_args = std::vector<std::unique_ptr<Expr>>();
    call_args.push_back(std::make_unique<Var>("uid"));
    if (!makeInvalid) {
        call_args.push_back(std::make_unique<Var>("p"));
    }
    auto call = std::make_unique<FuncCall>("signup", std::move(call_args));
    
    auto apicall = std::make_unique<APIcall>(
        std::move(call),
        Response(HTTPResponseCode::OK_200, std::make_unique<Var>("OK"))
    );

    // 5. Postcondition
    auto post_args = std::vector<std::unique_ptr<Expr>>();
    {
        auto map_access_args = std::vector<std::unique_ptr<Expr>>();
        map_access_args.push_back(std::make_unique<Var>("U_prime"));
        map_access_args.push_back(std::make_unique<Var>("uid"));
        post_args.push_back(std::make_unique<FuncCall>("map_access", std::move(map_access_args)));
        post_args.push_back(std::make_unique<Var>("p"));
    }
    auto post = std::make_unique<FuncCall>("equals", std::move(post_args));

    // 6. Final spec
    auto blocks = std::vector<std::unique_ptr<API>>();
    blocks.push_back(std::make_unique<API>(
        std::move(pre),
        std::move(apicall),
        Response(HTTPResponseCode::OK_200, std::move(post))
    ));

    return std::make_unique<Spec>(
        std::move(globals),
        std::move(inits),
        std::move(funcs),
        std::move(blocks)
    );
}

void runTestSuite() {
    std::cout << "=== Running Positive Test ===\n";
    {
        Typechecker checker;
        auto spec = createMockASTFromSampleAPI(false);
        
        PrintVisitor printer;
        std::cout << "\n[Generated AST Structure]\n";
        spec->accept(printer);
        
        TestVerifier verifier(checker, true);
        spec->accept(verifier);
    }
    
    std::cout << "\n=== Running Negative Test ===\n";
    {
        Typechecker checker;
        auto spec = createMockASTFromSampleAPI(true);
        
        PrintVisitor printer;
        std::cout << "\n[Generated AST Structure]\n";
        spec->accept(printer);
        
        TestVerifier verifier(checker, false);
        spec->accept(verifier);
    }
}

int main() {
    runTestSuite();
    return 0;
}