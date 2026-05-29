#include "semantic_analyzer.h"

using namespace std;

string SemanticAnalyzer::typeToString(DataType t) {
  switch (t) {
  case DataType::NUMBER:
    return "number";
  case DataType::STRING:
    return "string";
  case DataType::BOOLEAN:
    return "boolean";
  default:
    return "unknown";
  }
}

pair<vector<string>, vector<string>>
SemanticAnalyzer::analyze(const vector<unique_ptr<Stmt>> &statements) {
  scopes.clear();
  errors.clear();
  warnings.clear();
  beginScope();
  for (const auto &stmt : statements)
    analyzeStatement(stmt.get());
  endScope();
  return {errors, warnings};
}

void SemanticAnalyzer::analyzeStatement(Stmt *statement) {
  if (auto *s = dynamic_cast<VarStmt *>(statement)) {
    auto *info = declareVariable(s->name);
    if (s->initializer) {
      DataType t = analyzeExpression(s->initializer.get());
      if (info && t != DataType::UNKNOWN) {
        info->type = t;
        info->isInitialized = true;
      }
    }
  } else if (auto *s = dynamic_cast<PrintStmt *>(statement)) {
    analyzeExpression(s->expression.get());
  } else if (auto *s = dynamic_cast<IfStmt *>(statement)) {
    if (analyzeExpression(s->condition.get()) != DataType::BOOLEAN)
      errors.push_back("semantic error: if condition must be boolean");
    analyzeStatement(s->thenBranch.get());
    if (s->elseBranch)
      analyzeStatement(s->elseBranch.get());
  } else if (auto *s = dynamic_cast<WhileStmt *>(statement)) {
    if (analyzeExpression(s->condition.get()) != DataType::BOOLEAN)
      errors.push_back("semantic error: while condition must be boolean");
    analyzeStatement(s->body.get());
  } else if (auto *s = dynamic_cast<BlockStmt *>(statement)) {
    beginScope();
    for (const auto &nested : s->statements)
      analyzeStatement(nested.get());
    endScope();
  } else if (auto *s = dynamic_cast<ExpressionStmt *>(statement)) {
    analyzeExpression(s->expression.get());
  }
}

DataType SemanticAnalyzer::analyzeExpression(Expr *expression) {
  if (!expression)
    return DataType::UNKNOWN;

  if (dynamic_cast<NumberExpr *>(expression))
    return DataType::NUMBER;
  if (dynamic_cast<StringExpr *>(expression))
    return DataType::STRING;
  if (dynamic_cast<BooleanExpr *>(expression))
    return DataType::BOOLEAN;

  if (auto *e = dynamic_cast<VariableExpr *>(expression)) {
    auto *info = resolveVariable(e->name);
    if (!info) {
      errors.push_back("semantic error: " + e->name + " is not declared");
      return DataType::UNKNOWN;
    }
    info->isUsed = true;
    if (!info->isInitialized) {
      errors.push_back("semantic error: " + e->name + " is not initialized");
      return DataType::UNKNOWN;
    }
    return info->type;
  }

  if (auto *e = dynamic_cast<AssignExpr *>(expression)) {
    DataType valType = analyzeExpression(e->value.get());
    auto *info = resolveVariable(e->name);
    if (!info) {
      errors.push_back("semantic error: " + e->name + " is not declared");
      return DataType::UNKNOWN;
    }

    if (info->type == DataType::UNKNOWN) {
      info->type = valType;
      info->isInitialized = true;
    } else if (valType != DataType::UNKNOWN && info->type != valType) {
      errors.push_back("semantic error: cannot assign " +
                       typeToString(valType) + " to " + e->name + " of type " +
                       typeToString(info->type));
    }
    return info->type;
  }

  if (auto *e = dynamic_cast<BinaryExpr *>(expression)) {
    DataType l = analyzeExpression(e->left.get());
    DataType r = analyzeExpression(e->right.get());
    if (l == DataType::UNKNOWN || r == DataType::UNKNOWN)
      return DataType::UNKNOWN;

    if (e->op == TokenType::PLUS) {
      if (l == DataType::NUMBER && r == DataType::NUMBER)
        return DataType::NUMBER;
      if (l == DataType::STRING && r == DataType::STRING)
        return DataType::STRING;
    } else if (e->op == TokenType::MINUS || e->op == TokenType::STAR ||
               e->op == TokenType::SLASH) {
      if (l == DataType::NUMBER && r == DataType::NUMBER)
        return DataType::NUMBER;
    } else if (e->op == TokenType::LT || e->op == TokenType::LTEQ ||
               e->op == TokenType::GT || e->op == TokenType::GTEQ) {
      if (l == DataType::NUMBER && r == DataType::NUMBER)
        return DataType::BOOLEAN;
    } else if (e->op == TokenType::EQEQ || e->op == TokenType::NEQ) {
      if (l == r)
        return DataType::BOOLEAN;
    } else if (e->op == TokenType::AND || e->op == TokenType::OR) {
      if (l == DataType::BOOLEAN && r == DataType::BOOLEAN)
        return DataType::BOOLEAN;
    }
    errors.push_back("semantic error: operator not supported for these types");
    return DataType::UNKNOWN;
  }
  return DataType::UNKNOWN;
}

void SemanticAnalyzer::beginScope() { scopes.push_back({}); }
void SemanticAnalyzer::endScope() {
  for (auto const &[name, info] : scopes.back())
    if (!info.isUsed)
      warnings.push_back("warning: " + name + " is declared but never used");
  scopes.pop_back();
}

VariableInfo *SemanticAnalyzer::declareVariable(const string &name) {
  if (scopes.back().count(name)) {
    errors.push_back("semantic error: " + name + " already declared");
    return nullptr;
  }
  return &(scopes.back()[name] = {true, false, false, DataType::UNKNOWN});
}

VariableInfo *SemanticAnalyzer::resolveVariable(const string &name) {
  for (int i = scopes.size() - 1; i >= 0; --i)
    if (scopes[i].count(name))
      return &scopes[i][name];
  return nullptr;
}