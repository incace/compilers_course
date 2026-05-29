#include "semantic_analyzer.h"
#include <algorithm>

std::pair<std::vector<std::string>, std::vector<std::string>>
SemanticAnalyzer::analyze(
    const std::vector<std::unique_ptr<Stmt>> &statements) {
  scopes.clear();
  errors.clear();
  warnings.clear();

  beginScope();
  for (const auto &stmt : statements) {
    analyzeStatement(stmt.get());
  }
  endScope();

  return {errors, warnings};
}

void SemanticAnalyzer::analyzeStatement(Stmt *statement) {
  if (auto *s = dynamic_cast<VarStmt *>(statement)) {
    bool declared = declareVariable(s->name);
    if (s->initializer) {
      analyzeExpression(s->initializer.get());
      if (declared)
        setVariableInitialized(s->name);
    }
  } else if (auto *s = dynamic_cast<PrintStmt *>(statement)) {
    analyzeExpression(s->expression.get());
  } else if (auto *s = dynamic_cast<ExpressionStmt *>(statement)) {
    analyzeExpression(s->expression.get());
  } else if (auto *s = dynamic_cast<BlockStmt *>(statement)) {
    beginScope();
    for (const auto &nested : s->statements)
      analyzeStatement(nested.get());
    endScope();
  } else if (auto *s = dynamic_cast<IfStmt *>(statement)) {
    analyzeExpression(s->condition.get());
    analyzeStatement(s->thenBranch.get());
    if (s->elseBranch)
      analyzeStatement(s->elseBranch.get());
  } else if (auto *s = dynamic_cast<WhileStmt *>(statement)) {
    analyzeExpression(s->condition.get());
    analyzeStatement(s->body.get());
  }
}

void SemanticAnalyzer::analyzeExpression(Expr *expression) {
  if (!expression)
    return;

  if (auto *e = dynamic_cast<VariableExpr *>(expression)) {
    VariableInfo *info = resolveVariable(e->name);
    if (!info) {
      errors.push_back("Error: variable '" + e->name + "' is not declared");
    } else if (!info->isInitialized) {
      errors.push_back("Error: variable '" + e->name + "' is not initialized");
    }
    setVariableUsed(e->name);
  } else if (auto *e = dynamic_cast<BinaryExpr *>(expression)) {
    analyzeExpression(e->left.get());
    analyzeExpression(e->right.get());
  } else if (auto *e = dynamic_cast<UnaryExpr *>(expression)) {
    analyzeExpression(e->right.get());
  } else if (auto *e = dynamic_cast<AssignExpr *>(expression)) {
    analyzeExpression(e->value.get());
    if (!resolveVariable(e->name)) {
      errors.push_back("Error: variable '" + e->name + "' is not declared");
    } else {
      setVariableInitialized(e->name);
    }
  }
}

void SemanticAnalyzer::beginScope() { scopes.push_back({}); }

void SemanticAnalyzer::endScope() {
  auto current_scope = scopes.back();
  scopes.pop_back();
  for (auto const &[name, info] : current_scope) {
    if (!info.isUsed) {
      warnings.push_back("Warning: variable '" + name +
                         "' is declared but never used");
    }
  }
}

bool SemanticAnalyzer::declareVariable(const std::string &name) {
  if (scopes.back().count(name)) {
    errors.push_back("Error: variable '" + name + "' is already declared");
    return false;
  }
  scopes.back()[name] = {true, false, false};
  return true;
}

VariableInfo *SemanticAnalyzer::resolveVariable(const std::string &name) {
  for (int i = scopes.size() - 1; i >= 0; --i) {
    if (scopes[i].count(name))
      return &scopes[i][name];
  }
  return nullptr;
}

void SemanticAnalyzer::setVariableInitialized(const std::string &name) {
  VariableInfo *info = resolveVariable(name);
  if (info)
    info->isInitialized = true;
}

void SemanticAnalyzer::setVariableUsed(const std::string &name) {
  VariableInfo *info = resolveVariable(name);
  if (info)
    info->isUsed = true;
}