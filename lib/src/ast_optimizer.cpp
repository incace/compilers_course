#include "ast_optimizer.h"

using namespace std;

vector<unique_ptr<Stmt>>
ASTOptimizer::optimize(vector<unique_ptr<Stmt>> statements) {
  scopes.clear();
  beginScope();
  vector<unique_ptr<Stmt>> optimized;
  for (auto &stmt : statements) {
    auto opt = optimizeStatement(move(stmt));
    if (opt)
      optimized.push_back(move(opt));
  }
  endScope();
  return optimized;
}

unique_ptr<Stmt> ASTOptimizer::optimizeStatement(unique_ptr<Stmt> stmt) {
  if (auto *s = dynamic_cast<VarStmt *>(stmt.get())) {
    if (s->initializer) {
      s->initializer = optimizeExpression(move(s->initializer));
      if (isConstant(s->initializer.get())) {
        setConstant(s->name, getConstantValue(s->initializer.get()));
      } else {
        removeConstant(s->name);
      }
    }
    return stmt;
  }
  if (auto *s = dynamic_cast<PrintStmt *>(stmt.get())) {
    s->expression = optimizeExpression(move(s->expression));
    return stmt;
  }
  if (auto *s = dynamic_cast<IfStmt *>(stmt.get())) {
    s->condition = optimizeExpression(move(s->condition));
    s->thenBranch = optimizeStatement(move(s->thenBranch));
    if (s->elseBranch)
      s->elseBranch = optimizeStatement(move(s->elseBranch));
    return stmt;
  }
  if (auto *s = dynamic_cast<WhileStmt *>(stmt.get())) {
    s->condition = optimizeExpression(move(s->condition));
    s->body = optimizeStatement(move(s->body));
    return stmt;
  }
  if (auto *s = dynamic_cast<BlockStmt *>(stmt.get())) {
    beginScope();
    for (auto &nested : s->statements)
      nested = optimizeStatement(move(nested));
    endScope();
    return stmt;
  }
  if (auto *s = dynamic_cast<ExpressionStmt *>(stmt.get())) {
    s->expression = optimizeExpression(move(s->expression));
    return stmt;
  }
  if (auto *s = dynamic_cast<ReturnStmt *>(stmt.get())) {
    if (s->value)
      s->value = optimizeExpression(move(s->value));
    return stmt;
  }
  return stmt;
}

unique_ptr<Expr> ASTOptimizer::optimizeExpression(unique_ptr<Expr> expr) {
  if (auto *e = dynamic_cast<BinaryExpr *>(expr.get())) {
    e->left = optimizeExpression(move(e->left));
    e->right = optimizeExpression(move(e->right));

    if (isConstant(e->left.get()) && isConstant(e->right.get())) {
      RuntimeValue l = getConstantValue(e->left.get());
      RuntimeValue r = getConstantValue(e->right.get());

      try {
        if (e->op == TokenType::PLUS) {
          if (holds_alternative<double>(l) && holds_alternative<double>(r))
            return valueToExpr(get<double>(l) + get<double>(r));
          if (holds_alternative<string>(l) && holds_alternative<string>(r))
            return valueToExpr(get<string>(l) + get<string>(r));
        }
        if (e->op == TokenType::MINUS)
          return valueToExpr(get<double>(l) - get<double>(r));
        if (e->op == TokenType::STAR)
          return valueToExpr(get<double>(l) * get<double>(r));
        if (e->op == TokenType::SLASH && get<double>(r) != 0)
          return valueToExpr(get<double>(l) / get<double>(r));

        if (e->op == TokenType::AND)
          return valueToExpr(get<bool>(l) && get<bool>(r));
        if (e->op == TokenType::OR)
          return valueToExpr(get<bool>(l) || get<bool>(r));
      } catch (...) {
      }
    }
  }

  if (auto *e = dynamic_cast<VariableExpr *>(expr.get())) {
    RuntimeValue val;
    if (getConstant(e->name, val))
      return valueToExpr(val);
  }

  if (auto *e = dynamic_cast<AssignExpr *>(expr.get())) {
    e->value = optimizeExpression(move(e->value));
    if (isConstant(e->value.get()))
      setConstant(e->name, getConstantValue(e->value.get()));
    else
      removeConstant(e->name);
  }

  return expr;
}

bool ASTOptimizer::isConstant(Expr *expr) {
  return dynamic_cast<NumberExpr *>(expr) || dynamic_cast<StringExpr *>(expr) ||
         dynamic_cast<BooleanExpr *>(expr);
}

RuntimeValue ASTOptimizer::getConstantValue(Expr *expr) {
  if (auto *e = dynamic_cast<NumberExpr *>(expr))
    return e->value;
  if (auto *e = dynamic_cast<StringExpr *>(expr))
    return e->value;
  if (auto *e = dynamic_cast<BooleanExpr *>(expr))
    return e->value;
  return 0.0;
}

unique_ptr<Expr> ASTOptimizer::valueToExpr(RuntimeValue value) {
  if (holds_alternative<double>(value))
    return make_unique<NumberExpr>(get<double>(value));
  if (holds_alternative<string>(value))
    return make_unique<StringExpr>(get<string>(value));
  if (holds_alternative<bool>(value))
    return make_unique<BooleanExpr>(get<bool>(value));
  return nullptr;
}

void ASTOptimizer::beginScope() { scopes.push_back({}); }
void ASTOptimizer::endScope() { scopes.pop_back(); }
void ASTOptimizer::setConstant(const string &name, RuntimeValue value) {
  scopes.back()[name] = value;
}
void ASTOptimizer::removeConstant(const string &name) {
  for (int i = scopes.size() - 1; i >= 0; --i)
    if (scopes[i].count(name)) {
      scopes[i].erase(name);
      return;
    }
}
bool ASTOptimizer::getConstant(const string &name, RuntimeValue &out) {
  for (int i = scopes.size() - 1; i >= 0; --i)
    if (scopes[i].count(name)) {
      out = scopes[i][name];
      return true;
    }
  return false;
}