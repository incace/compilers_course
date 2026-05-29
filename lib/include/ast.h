#pragma once
#include "token_type.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Expr {
public:
  virtual ~Expr() = default;
  virtual void print(int lvl) = 0;
};

class NumberExpr : public Expr {
public:
  double value;
  explicit NumberExpr(double val) : value(val) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Number: " << value << "\n";
  }
};

class StringExpr : public Expr {
public:
  std::string value;
  explicit StringExpr(std::string val) : value(std::move(val)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "String: " << value << "\n";
  }
};

class BooleanExpr : public Expr {
public:
  bool value;
  explicit BooleanExpr(bool val) : value(val) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ')
              << "Bool: " << (value ? "true" : "false") << "\n";
  }
};

class VariableExpr : public Expr {
public:
  std::string name;
  explicit VariableExpr(std::string name) : name(std::move(name)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Variable: " << name << "\n";
  }
};

class BinaryExpr : public Expr {
public:
  std::unique_ptr<Expr> left;
  TokenType op;
  std::unique_ptr<Expr> right;
  BinaryExpr(std::unique_ptr<Expr> l, TokenType o, std::unique_ptr<Expr> r)
      : left(std::move(l)), op(o), right(std::move(r)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "BinaryExpr\n";
    left->print(lvl + 1);
    right->print(lvl + 1);
  }
};

class UnaryExpr : public Expr {
public:
  TokenType op;
  std::unique_ptr<Expr> right;
  UnaryExpr(TokenType o, std::unique_ptr<Expr> r)
      : op(o), right(std::move(r)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "UnaryExpr\n";
    right->print(lvl + 1);
  }
};

class AssignExpr : public Expr {
public:
  std::string name;
  std::unique_ptr<Expr> value;
  AssignExpr(std::string n, std::unique_ptr<Expr> v)
      : name(std::move(n)), value(std::move(v)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Assign: " << name << "\n";
    value->print(lvl + 1);
  }
};

class CallExpr : public Expr {
public:
  std::string callee;
  std::vector<std::unique_ptr<Expr>> arguments;
  CallExpr(std::string name, std::vector<std::unique_ptr<Expr>> args)
      : callee(std::move(name)), arguments(std::move(args)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Call: " << callee << "\n";
    for (auto &arg : arguments)
      arg->print(lvl + 1);
  }
};

class Stmt {
public:
  virtual ~Stmt() = default;
  virtual void print(int lvl) = 0;
};

class BlockStmt : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> statements;
  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> s)
      : statements(std::move(s)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Block\n";
    for (const auto &s : statements)
      s->print(lvl + 1);
  }
};

class ExpressionStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;
  explicit ExpressionStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
  void print(int lvl) override { expression->print(lvl); }
};

class VarStmt : public Stmt {
public:
  std::string name;
  std::unique_ptr<Expr> initializer;
  VarStmt(std::string n, std::unique_ptr<Expr> i)
      : name(std::move(n)), initializer(std::move(i)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Var: " << name << "\n";
    if (initializer)
      initializer->print(lvl + 1);
  }
};

class FunctionStmt : public Stmt {
public:
  std::string name;
  std::vector<std::string> parameters;
  std::shared_ptr<BlockStmt> body;
  FunctionStmt(std::string n, std::vector<std::string> p,
               std::shared_ptr<BlockStmt> b)
      : name(std::move(n)), parameters(std::move(p)), body(std::move(b)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Function: " << name << "\n";
    body->print(lvl + 1);
  }
};

class ReturnStmt : public Stmt {
public:
  std::unique_ptr<Expr> value;
  explicit ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Return\n";
    if (value)
      value->print(lvl + 1);
  }
};

class PrintStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;
  explicit PrintStmt(std::unique_ptr<Expr> e) : expression(std::move(e)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "Print\n";
    expression->print(lvl + 1);
  }
};

class IfStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> thenBranch;
  std::unique_ptr<Stmt> elseBranch;
  IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t,
         std::unique_ptr<Stmt> e)
      : condition(std::move(c)), thenBranch(std::move(t)),
        elseBranch(std::move(e)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "If\n";
    condition->print(lvl + 1);
    thenBranch->print(lvl + 1);
    if (elseBranch)
      elseBranch->print(lvl + 1);
  }
};

class WhileStmt : public Stmt {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;
  WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b)
      : condition(std::move(c)), body(std::move(b)) {}
  void print(int lvl) override {
    std::cout << std::string(lvl * 2, ' ') << "While\n";
    condition->print(lvl + 1);
    body->print(lvl + 1);
  }
};