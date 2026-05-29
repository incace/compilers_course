#include "semantic_analyzer.h"

using namespace std;

pair<vector<string>, vector<string>>
SemanticAnalyzer::analyze(const vector<unique_ptr<Stmt>> &statements) {
  scopes.clear();
  errors.clear();
  warnings.clear();
  functionDepth = 0;
  beginScope();
  for (const auto &stmt : statements)
    analyzeStatement(stmt.get());
  endScope();
  return {errors, warnings};
}

void SemanticAnalyzer::analyzeStatement(Stmt *statement) {
  if (auto *s = dynamic_cast<FunctionStmt *>(statement)) {
    auto *info = declareVariable(s->name);
    if (info) {
      info->type = DataType::FUNCTION;
      info->arity = s->parameters.size();
      info->isInitialized = true;
    }
    beginScope();
    functionDepth++;
    for (const auto &param : s->parameters) {
      auto *pInfo = declareVariable(param);
      if (pInfo)
        pInfo->isInitialized = true;
    }
    for (const auto &stmt : s->body->statements)
      analyzeStatement(stmt.get());
    functionDepth--;
    endScope();
  } else if (auto *s = dynamic_cast<ReturnStmt *>(statement)) {
    if (functionDepth == 0)
      errors.push_back(
          "semantic error: return is allowed only inside a function");
    if (s->value)
      analyzeExpression(s->value.get());
  } else if (auto *s = dynamic_cast<VarStmt *>(statement)) {
    auto *info = declareVariable(s->name);
    if (s->initializer) {
      DataType t = analyzeExpression(s->initializer.get());
      if (info) {
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

  if (auto *e = dynamic_cast<CallExpr *>(expression)) {
    auto *info = resolveVariable(e->callee);
    if (!info || info->type != DataType::FUNCTION) {
      errors.push_back("semantic error: function " + e->callee +
                       " is not declared");
    } else if (info->arity != (int)e->arguments.size()) {
      errors.push_back("semantic error: function " + e->callee + " expects " +
                       to_string(info->arity) + " arguments");
    }
    for (auto &arg : e->arguments)
      analyzeExpression(arg.get());
    return DataType::UNKNOWN;
  }

  if (auto *e = dynamic_cast<VariableExpr *>(expression)) {
    auto *info = resolveVariable(e->name);
    if (!info) {
      errors.push_back("semantic error: " + e->name + " is not declared");
      return DataType::UNKNOWN;
    }
    if (info->type == DataType::FUNCTION) {
      errors.push_back("semantic error: function " + e->name +
                       " cannot be used as a value");
      return DataType::UNKNOWN;
    }
    info->isUsed = true;
    if (!info->isInitialized)
      errors.push_back("semantic error: " + e->name + " is not initialized");
    return info->type;
  }

  if (auto *e = dynamic_cast<BinaryExpr *>(expression)) {
    DataType l = analyzeExpression(e->left.get());
    DataType r = analyzeExpression(e->right.get());

    // Операторы сравнения и логики всегда возвращают BOOLEAN
    if (e->op == TokenType::EQEQ || e->op == TokenType::NEQ ||
        e->op == TokenType::LT || e->op == TokenType::LTEQ ||
        e->op == TokenType::GT || e->op == TokenType::GTEQ ||
        e->op == TokenType::AND || e->op == TokenType::OR) {
      return DataType::BOOLEAN;
    }

    // Арифметические операторы
    if (e->op == TokenType::PLUS) {
      if (l == DataType::STRING || r == DataType::STRING)
        return DataType::STRING;
      return DataType::NUMBER;
    }
    return DataType::NUMBER;
  }

  if (auto *e = dynamic_cast<UnaryExpr *>(expression)) {
    DataType r = analyzeExpression(e->right.get());
    if (e->op == TokenType::EXCL)
      return DataType::BOOLEAN;
    return DataType::NUMBER;
  }

  if (auto *e = dynamic_cast<AssignExpr *>(expression)) {
    return analyzeExpression(e->value.get());
  }

  return DataType::UNKNOWN;
}

void SemanticAnalyzer::beginScope() { scopes.push_back({}); }
void SemanticAnalyzer::endScope() {
  for (auto const &[name, info] : scopes.back())
    if (!info.isUsed && info.type != DataType::FUNCTION)
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