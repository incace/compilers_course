#pragma once
#include "ast.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum class DataType { UNKNOWN, NUMBER, STRING, BOOLEAN };

struct VariableInfo {
  bool isDefined = true;
  bool isInitialized = false;
  bool isUsed = false;
  DataType type = DataType::UNKNOWN;
};

class SemanticAnalyzer {
public:
  std::pair<std::vector<std::string>, std::vector<std::string>>
  analyze(const std::vector<std::unique_ptr<Stmt>> &statements);

private:
  std::vector<std::unordered_map<std::string, VariableInfo>> scopes;
  std::vector<std::string> errors;
  std::vector<std::string> warnings;

  void analyzeStatement(Stmt *statement);
  DataType analyzeExpression(Expr *expression);

  void beginScope();
  void endScope();
  VariableInfo *declareVariable(const std::string &name);
  VariableInfo *resolveVariable(const std::string &name);

  std::string typeToString(DataType t);
  void reportBinaryError(TokenType op, DataType l, DataType r);
};