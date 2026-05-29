#include "interpreter.h"
#include <cmath>
#include <stdexcept>

using namespace std;

Interpreter::Interpreter(int maxLoopIterations)
    : maxLoopIterations(maxLoopIterations) {}

vector<string>
Interpreter::interpret(const vector<unique_ptr<Stmt>> &statements) {
  scopes.clear();
  output.clear();
  beginScope(); // Global scope
  for (const auto &stmt : statements) {
    executeStatement(stmt.get());
  }
  return output;
}

void Interpreter::executeStatement(Stmt *stmt) {
  if (auto *s = dynamic_cast<VarStmt *>(stmt)) {
    RuntimeValue val =
        s->initializer ? evaluateExpression(s->initializer.get()) : 0.0;
    declareVariable(s->name, val);
  } else if (auto *s = dynamic_cast<PrintStmt *>(stmt)) {
    output.push_back(stringify(evaluateExpression(s->expression.get())));
  } else if (auto *s = dynamic_cast<ExpressionStmt *>(stmt)) {
    evaluateExpression(s->expression.get());
  } else if (auto *s = dynamic_cast<BlockStmt *>(stmt)) {
    beginScope();
    for (const auto &nested : s->statements)
      executeStatement(nested.get());
    endScope();
  } else if (auto *s = dynamic_cast<IfStmt *>(stmt)) {
    if (isTrue(evaluateExpression(s->condition.get()))) {
      executeStatement(s->thenBranch.get());
    } else if (s->elseBranch) {
      executeStatement(s->elseBranch.get());
    }
  } else if (auto *s = dynamic_cast<WhileStmt *>(stmt)) {
    int iterations = 0;
    while (isTrue(evaluateExpression(s->condition.get()))) {
      if (++iterations > maxLoopIterations)
        throw runtime_error("Infinite loop detected");
      executeStatement(s->body.get());
    }
  }
}

RuntimeValue Interpreter::evaluateExpression(Expr *expr) {
  if (auto *e = dynamic_cast<NumberExpr *>(expr))
    return e->value;
  if (auto *e = dynamic_cast<StringExpr *>(expr))
    return e->value;
  if (auto *e = dynamic_cast<BooleanExpr *>(expr))
    return e->value;
  if (auto *e = dynamic_cast<VariableExpr *>(expr))
    return getVariable(e->name);

  if (auto *e = dynamic_cast<AssignExpr *>(expr)) {
    RuntimeValue val = evaluateExpression(e->value.get());
    assignVariable(e->name, val);
    return val;
  }

  if (auto *e = dynamic_cast<UnaryExpr *>(expr)) {
    RuntimeValue right = evaluateExpression(e->right.get());
    if (e->op == TokenType::MINUS)
      return -requireNumber(right, "unary minus");
    if (e->op == TokenType::EXCL)
      return !isTrue(right);
  }

  if (auto *e = dynamic_cast<BinaryExpr *>(expr)) {
    if (e->op == TokenType::AND) {
      return isTrue(evaluateExpression(e->left.get())) &&
             isTrue(evaluateExpression(e->right.get()));
    }
    if (e->op == TokenType::OR) {
      return isTrue(evaluateExpression(e->left.get())) ||
             isTrue(evaluateExpression(e->right.get()));
    }

    RuntimeValue l = evaluateExpression(e->left.get());
    RuntimeValue r = evaluateExpression(e->right.get());

    switch (e->op) {
    case TokenType::PLUS:
      if (holds_alternative<double>(l) && holds_alternative<double>(r))
        return get<double>(l) + get<double>(r);
      if (holds_alternative<string>(l) && holds_alternative<string>(r))
        return get<string>(l) + get<string>(r);
      throw runtime_error("Invalid operands for +");
    case TokenType::MINUS:
      return requireNumber(l, "-") - requireNumber(r, "-");
    case TokenType::STAR:
      return requireNumber(l, "*") * requireNumber(r, "*");
    case TokenType::SLASH: {
      double den = requireNumber(r, "/");
      if (den == 0)
        throw runtime_error("Division by zero");
      return requireNumber(l, "/") / den;
    }
    case TokenType::LT:
      return requireNumber(l, "<") < requireNumber(r, "<");
    case TokenType::LTEQ:
      return requireNumber(l, "<=") <= requireNumber(r, "<=");
    case TokenType::GT:
      return requireNumber(l, ">") > requireNumber(r, ">");
    case TokenType::GTEQ:
      return requireNumber(l, ">=") >= requireNumber(r, ">=");
    case TokenType::EQEQ:
      return l == r;
    case TokenType::NEQ:
      return l != r;
    default:
      break;
    }
  }
  return 0.0;
}

string Interpreter::stringify(const RuntimeValue &value) {
  if (holds_alternative<double>(value)) {
    double d = get<double>(value);
    if (floor(d) == d)
      return to_string((int)d);
    return to_string(d);
  }
  if (holds_alternative<string>(value))
    return get<string>(value);
  if (holds_alternative<bool>(value))
    return get<bool>(value) ? "true" : "false";
  return "";
}

bool Interpreter::isTrue(const RuntimeValue &value) {
  if (holds_alternative<bool>(value))
    return get<bool>(value);
  return false;
}

void Interpreter::beginScope() { scopes.push_back({}); }
void Interpreter::endScope() { scopes.pop_back(); }

void Interpreter::declareVariable(const string &name,
                                  const RuntimeValue &value) {
  scopes.back()[name] = value;
}

void Interpreter::assignVariable(const string &name,
                                 const RuntimeValue &value) {
  for (int i = scopes.size() - 1; i >= 0; --i) {
    if (scopes[i].count(name)) {
      scopes[i][name] = value;
      return;
    }
  }
  throw runtime_error("Undefined variable: " + name);
}

RuntimeValue Interpreter::getVariable(const string &name) {
  for (int i = scopes.size() - 1; i >= 0; --i) {
    if (scopes[i].count(name))
      return scopes[i][name];
  }
  throw runtime_error("Undefined variable: " + name);
}

double Interpreter::requireNumber(const RuntimeValue &value,
                                  const string &context) {
  if (holds_alternative<double>(value))
    return get<double>(value);
  throw runtime_error("Expected number in " + context);
}