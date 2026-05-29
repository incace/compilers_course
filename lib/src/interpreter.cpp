#include "interpreter.h"
#include <cmath>
#include <stdexcept>

using namespace std;

struct ReturnSignal {
  RuntimeValue value;
};

Interpreter::Interpreter(int maxLoopIterations)
    : maxLoopIterations(maxLoopIterations) {}

vector<string>
Interpreter::interpret(const vector<unique_ptr<Stmt>> &statements) {
  scopes.clear();
  output.clear();
  functions.clear();
  beginScope();
  try {
    for (const auto &stmt : statements)
      executeStatement(stmt.get());
  } catch (ReturnSignal &) {
    throw runtime_error(
        "[Interpreter Error] return is allowed only inside a function");
  }
  return output;
}

void Interpreter::executeStatement(Stmt *stmt) {
  if (auto *s = dynamic_cast<FunctionStmt *>(stmt)) {
    functions[s->name] = s;
  } else if (auto *s = dynamic_cast<ReturnStmt *>(stmt)) {
    throw ReturnSignal{s->value ? evaluateExpression(s->value.get()) : false};
  } else if (auto *s = dynamic_cast<VarStmt *>(stmt)) {
    declareVariable(s->name, s->initializer
                                 ? evaluateExpression(s->initializer.get())
                                 : 0.0);
  } else if (auto *s = dynamic_cast<PrintStmt *>(stmt)) {
    output.push_back(stringify(evaluateExpression(s->expression.get())));
  } else if (auto *s = dynamic_cast<IfStmt *>(stmt)) {
    if (isTrue(evaluateExpression(s->condition.get())))
      executeStatement(s->thenBranch.get());
    else if (s->elseBranch)
      executeStatement(s->elseBranch.get());
  } else if (auto *s = dynamic_cast<WhileStmt *>(stmt)) {
    int iters = 0;
    while (isTrue(evaluateExpression(s->condition.get()))) {
      if (++iters > maxLoopIterations)
        throw runtime_error("Infinite loop");
      executeStatement(s->body.get());
    }
  } else if (auto *s = dynamic_cast<BlockStmt *>(stmt)) {
    beginScope();
    for (auto &nested : s->statements)
      executeStatement(nested.get());
    endScope();
  } else if (auto *s = dynamic_cast<ExpressionStmt *>(stmt)) {
    evaluateExpression(s->expression.get());
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
  if (auto *e = dynamic_cast<CallExpr *>(expr)) {
    if (!functions.count(e->callee))
      throw runtime_error("Undefined function: " + e->callee);
    FunctionStmt *func = functions[e->callee];
    vector<RuntimeValue> args;
    for (auto &arg : e->arguments)
      args.push_back(evaluateExpression(arg.get()));
    beginScope();
    for (size_t i = 0; i < func->parameters.size(); ++i)
      declareVariable(func->parameters[i], args[i]);
    RuntimeValue ret = false;
    try {
      for (auto &s : func->body->statements)
        executeStatement(s.get());
    } catch (ReturnSignal &sig) {
      ret = sig.value;
    }
    endScope();
    return ret;
  }
  if (auto *e = dynamic_cast<AssignExpr *>(expr)) {
    RuntimeValue val = evaluateExpression(e->value.get());
    assignVariable(e->name, val);
    return val;
  }
  if (auto *e = dynamic_cast<BinaryExpr *>(expr)) {
    if (e->op == TokenType::AND)
      return isTrue(evaluateExpression(e->left.get())) &&
             isTrue(evaluateExpression(e->right.get()));
    if (e->op == TokenType::OR)
      return isTrue(evaluateExpression(e->left.get())) ||
             isTrue(evaluateExpression(e->right.get()));
    RuntimeValue l = evaluateExpression(e->left.get()),
                 r = evaluateExpression(e->right.get());
    if (e->op == TokenType::PLUS && holds_alternative<string>(l))
      return get<string>(l) + get<string>(r);
    if (e->op == TokenType::PLUS)
      return requireNumber(l, "+") + requireNumber(r, "+");
    if (e->op == TokenType::MINUS)
      return requireNumber(l, "-") - requireNumber(r, "-");
    if (e->op == TokenType::STAR)
      return requireNumber(l, "*") * requireNumber(r, "*");
    if (e->op == TokenType::SLASH)
      return requireNumber(l, "/") / requireNumber(r, "/");
    if (e->op == TokenType::LT)
      return requireNumber(l, "<") < requireNumber(r, "<");
    if (e->op == TokenType::LTEQ)
      return requireNumber(l, "<=") <= requireNumber(r, "<=");
    if (e->op == TokenType::GT)
      return requireNumber(l, ">") > requireNumber(r, ">");
    if (e->op == TokenType::GTEQ)
      return requireNumber(l, ">=") >= requireNumber(r, ">=");
    if (e->op == TokenType::EQEQ)
      return l == r;
    if (e->op == TokenType::NEQ)
      return l != r;
  }
  if (auto *e = dynamic_cast<UnaryExpr *>(expr)) {
    RuntimeValue r = evaluateExpression(e->right.get());
    if (e->op == TokenType::MINUS)
      return -requireNumber(r, "-");
    if (e->op == TokenType::EXCL)
      return !isTrue(r);
  }
  return false;
}

string Interpreter::stringify(const RuntimeValue &v) {
  if (holds_alternative<double>(v)) {
    double d = get<double>(v);
    if (floor(d) == d)
      return to_string((int)d);
    return to_string(d);
  }
  if (holds_alternative<string>(v))
    return get<string>(v);
  return get<bool>(v) ? "true" : "false";
}

bool Interpreter::isTrue(const RuntimeValue &v) {
  return holds_alternative<bool>(v) && get<bool>(v);
}
void Interpreter::beginScope() { scopes.push_back({}); }
void Interpreter::endScope() { scopes.pop_back(); }
void Interpreter::declareVariable(const string &n, const RuntimeValue &v) {
  scopes.back()[n] = v;
}
void Interpreter::assignVariable(const string &n, const RuntimeValue &v) {
  for (int i = scopes.size() - 1; i >= 0; --i)
    if (scopes[i].count(n)) {
      scopes[i][n] = v;
      return;
    }
}
RuntimeValue Interpreter::getVariable(const string &n) {
  for (int i = scopes.size() - 1; i >= 0; --i)
    if (scopes[i].count(n))
      return scopes[i][n];
  throw runtime_error("Undefined: " + n);
}
double Interpreter::requireNumber(const RuntimeValue &v, const string &c) {
  if (holds_alternative<double>(v))
    return get<double>(v);
  throw runtime_error("Expected number in " + c);
}