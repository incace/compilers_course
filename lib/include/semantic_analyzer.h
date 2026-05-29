#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>

struct VariableInfo {
  bool isDefined;
  bool isInitialized;
  bool isUsed;
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
  void analyzeExpression(Expr *expression);

  void beginScope();
  void endScope();
  bool declareVariable(const std::string &name);
  VariableInfo *resolveVariable(const std::string &name);
  void setVariableInitialized(const std::string &name);
  void setVariableUsed(const std::string &name);
};