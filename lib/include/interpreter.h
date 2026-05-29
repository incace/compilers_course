#pragma once
#include "ast.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// Тип данных времени выполнения
using RuntimeValue = std::variant<double, std::string, bool>;

class Interpreter {
public:
  explicit Interpreter(int maxLoopIterations = 10000);
  std::vector<std::string>
  interpret(const std::vector<std::unique_ptr<Stmt>> &statements);
  std::string stringify(const RuntimeValue &value);

private:
  int maxLoopIterations;
  std::vector<std::unordered_map<std::string, RuntimeValue>> scopes;
  std::vector<std::string> output;

  void executeStatement(Stmt *stmt);
  RuntimeValue evaluateExpression(Expr *expr);

  bool isTrue(const RuntimeValue &value);
  void beginScope();
  void endScope();
  void declareVariable(const std::string &name, const RuntimeValue &value);
  void assignVariable(const std::string &name, const RuntimeValue &value);
  RuntimeValue getVariable(const std::string &name);

  double requireNumber(const RuntimeValue &value, const std::string &context);
  bool requireBoolean(const RuntimeValue &value, const std::string &context);
};