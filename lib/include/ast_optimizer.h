#pragma once
#include "ast.h"
#include "interpreter.h" // Используем RuntimeValue для хранения констант
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ASTOptimizer {
public:
  std::vector<std::unique_ptr<Stmt>>
  optimize(std::vector<std::unique_ptr<Stmt>> statements);

private:
  std::vector<std::unordered_map<std::string, RuntimeValue>> scopes;

  std::unique_ptr<Stmt> optimizeStatement(std::unique_ptr<Stmt> stmt);
  std::unique_ptr<Expr> optimizeExpression(std::unique_ptr<Expr> expr);

  void beginScope();
  void endScope();
  void setConstant(const std::string &name, RuntimeValue value);
  bool getConstant(const std::string &name, RuntimeValue &outValue);
  void removeConstant(const std::string &name);

  bool isConstant(Expr *expr);
  RuntimeValue getConstantValue(Expr *expr);
  std::unique_ptr<Expr> valueToExpr(RuntimeValue value);
};