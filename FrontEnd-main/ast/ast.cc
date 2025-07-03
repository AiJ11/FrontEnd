#include "ast.hh"
#include "ASTVis.hpp"
#include <iostream>
#include <sstream>
#include <string>

// Implementation of PrintVisitor methods

void PrintVisitor::visit(const TypeConst& node) {
    std::cout << "TypeConst: " << node.name << std::endl;
}

void PrintVisitor::visit(const FuncType& node) {
    std::cout << "FuncType:\n  Parameters: " << node.params.size() << std::endl;
    for (const auto& param : node.params) {
        std::cout << "    ";
        param->accept(*this);
    }
    std::cout << "  Return Type: ";
    if (node.returnType) node.returnType->accept(*this);
    else std::cout << "void" << std::endl;
}

void PrintVisitor::visit(const MapType& node) {
    std::cout << "MapType:\n  Domain: ";
    if (node.domain) node.domain->accept(*this);
    std::cout << "  Range: ";
    if (node.range) node.range->accept(*this);
}

void PrintVisitor::visit(const Tuple& node) {
    std::cout << "Tuple with " << node.elements.size() << " elements:" << std::endl;
    for (const auto& elem : node.elements) {
        std::cout << "  ";
        elem->accept(*this);
    }
}

void PrintVisitor::visit(const SetType& node) {
    std::cout << "SetType:\n  Element Type: ";
    if (node.elementType) node.elementType->accept(*this);
}

void PrintVisitor::visit(const Decl& node) {
    std::cout << "Declaration: " << node.name << "\n  Type: ";
    if (node.type) node.type->accept(*this);
}

void PrintVisitor::visit(const Var& node) {
    std::cout << "Variable: " << node.name << std::endl;
}

void PrintVisitor::visit(const FuncCall& node) {
    std::cout << "Function Call: " << node.name << "\n  Arguments: " << node.args.size() << std::endl;
    for (const auto& arg : node.args) {
        std::cout << "    ";
        arg->accept(*this);
    }
}

void PrintVisitor::visit(const Num& node) {
    std::cout << "Number: " << node.value << std::endl;
}

void PrintVisitor::visit(const String& node) {
    std::cout << "String: " << node.value << std::endl;
}

void PrintVisitor::visit(const Set& node) {
    std::cout << "Set with " << node.elements.size() << " elements:" << std::endl;
    for (const auto& elem : node.elements) {
        std::cout << "  ";
        elem->accept(*this);
    }
}

void PrintVisitor::visit(const Map& node) {
    std::cout << "Map with " << node.value.size() << " entries:" << std::endl;
    for (const auto& entry : node.value) {
        std::cout << "  ";
        entry.first->accept(*this);
        std::cout << " -> ";
        entry.second->accept(*this);
        std::cout << std::endl;
    }
}

void PrintVisitor::visit(const TupleType& node) {
    std::cout << "TupleType with " << node.elements.size() << " elements:" << std::endl;
    for (const auto& elem : node.elements) {
        std::cout << "  ";
        elem->accept(*this);
    }
}
    /*void PrintVisitor::visit(const Tuple& node) {
    std::cout << "Tuple with " << node.expr.size() << " elements:" << std::endl;
    for (const auto& elem : node.expr) {
        std::cout << "  ";
        elem->accept(*this);
    }
}*/

void PrintVisitor::visit(const FuncDecl& node) {
    std::cout << "Function Declaration: " << node.name << std::endl;
    std::cout << "  Parameters: " << node.params.size() << std::endl;

    std::cout << "  Return HTTP Code: ";
    switch(node.returnType.first) {
        case HTTPResponseCode::OK_200: std::cout << "OK_200"; break;
        case HTTPResponseCode::CREATED_201: std::cout << "CREATED_201"; break;
        case HTTPResponseCode::BAD_REQUEST_400: std::cout << "BAD_REQUEST_400"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << "\n  Return Types: " << node.returnType.second.size() << std::endl;
    for (const auto& rt : node.returnType.second) {
        std::cout << "    ";
        rt->accept(*this);
    }
}

void PrintVisitor::visit(const Init& node) {
    std::cout << "Initialization: " << node.varName << "\n  Value: ";
    if (node.expr) node.expr->accept(*this);
}

void PrintVisitor::visit(const Response& node) {
    std::cout << "Response:\n  HTTP Code: ";
    switch(node.code) {
        case HTTPResponseCode::OK_200: std::cout << "OK_200"; break;
        case HTTPResponseCode::CREATED_201: std::cout << "CREATED_201"; break;
        case HTTPResponseCode::BAD_REQUEST_400: std::cout << "BAD_REQUEST_400"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << "\n  Expression: ";
    if (node.expr) node.expr->accept(*this);
}

void PrintVisitor::visit(const APIcall& node) {
    std::cout << "API Call:\n  Function: ";
    if (node.call) node.call->accept(*this);
    std::cout << "  Response:\n";
    visit(node.response);
}

void PrintVisitor::visit(const API& node) {
    std::cout << "API Specification:\n  Precondition: ";
    if (node.pre) node.pre->accept(*this);
    std::cout << "\n  API Call: ";
    if (node.call) node.call->accept(*this);
    std::cout << "\n  Response:\n";
    visit(node.response);
}

void PrintVisitor::visit(const Spec& node) {
    std::cout << "Specification:\n  Globals (" << node.globals.size() << "):\n";
    for (const auto& global : node.globals) {
        std::cout << "    ";
        global->accept(*this);
    }
    std::cout << "  Initializations (" << node.init.size() << "):\n";
    for (const auto& init : node.init) {
        std::cout << "    ";
        init->accept(*this);
    }
    std::cout << "  Functions (" << node.functions.size() << "):\n";
    for (const auto& func : node.functions) {
        std::cout << "    ";
        func->accept(*this);
    }
    std::cout << "  API Blocks (" << node.blocks.size() << "):\n";
    for (const auto& block : node.blocks) {
        std::cout << "    ";
        block->accept(*this);
    }
}

void PrintVisitor::visit(const Assign& node) {
    std::cout << "Assignment:\n  Left: ";
    node.left->accept(*this);
    std::cout << "  Right: ";
    node.right->accept(*this);
}

void PrintVisitor::visit(const FuncCallStmt& node) {
    std::cout << "Function Call Statement:\n  ";
    if (node.call) node.call->accept(*this);
}

void PrintVisitor::visit(const Program& node) {
    std::cout << "Program with " << node.statements.size() << " statements:\n";
    for (const auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}
