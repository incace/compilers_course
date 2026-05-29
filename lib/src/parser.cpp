#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    statements.push_back(declaration());
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
  if (match({TokenType::VAR}))
    return varDeclaration();
  return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
  Token name = consume(TokenType::ID, "Expect variable name.");
  std::unique_ptr<Expr> initializer = nullptr;
  if (match({TokenType::EQ}))
    initializer = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
  return std::make_unique<VarStmt>(name.getValue(), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match({TokenType::IF}))
    return ifStatement();
  if (match({TokenType::WHILE}))
    return whileStatement();
  if (match({TokenType::PRINT}))
    return printStatement();
  if (match({TokenType::LBRACE}))
    return std::make_unique<BlockStmt>(block());
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
  consume(TokenType::LPAREN, "Expect '(' after 'if'.");
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::RPAREN, "Expect ')' after if condition.");
  std::unique_ptr<Stmt> thenBranch = statement();
  std::unique_ptr<Stmt> elseBranch = nullptr;
  if (match({TokenType::ELSE}))
    elseBranch = statement();
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
  consume(TokenType::LPAREN, "Expect '(' after 'while'.");
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::RPAREN, "Expect ')' after while condition.");
  std::unique_ptr<Stmt> body = statement();
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::printStatement() {
  std::unique_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  std::unique_ptr<Expr> expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check(TokenType::RBRACE) && !isAtEnd()) {
    statements.push_back(declaration());
  }
  consume(TokenType::RBRACE, "Expect '}' after block.");
  return statements;
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
  std::unique_ptr<Expr> expr = logicalOr();
  if (match({TokenType::EQ})) {
    std::unique_ptr<Expr> value = assignment();
    if (auto *v = dynamic_cast<VariableExpr *>(expr.get())) {
      return std::make_unique<AssignExpr>(v->name, std::move(value));
    }
    throw std::runtime_error("Invalid assignment target.");
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
  std::unique_ptr<Expr> expr = logicalAnd();
  while (match({TokenType::OR})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = logicalAnd();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
  std::unique_ptr<Expr> expr = equality();
  while (match({TokenType::AND})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = equality();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();
  while (match({TokenType::EQEQ, TokenType::NEQ})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();
  while (
      match({TokenType::LT, TokenType::LTEQ, TokenType::GT, TokenType::GTEQ})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();
  while (match({TokenType::PLUS, TokenType::MINUS})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();
  while (match({TokenType::STAR, TokenType::SLASH})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::EXCL, TokenType::MINUS})) {
    TokenType op = previous().getType();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({TokenType::NUMBER}))
    return std::make_unique<NumberExpr>(std::stod(previous().getValue()));
  if (match({TokenType::STRING}))
    return std::make_unique<StringExpr>(previous().getValue());
  if (match({TokenType::TRUE}))
    return std::make_unique<BooleanExpr>(true);
  if (match({TokenType::FALSE}))
    return std::make_unique<BooleanExpr>(false);
  if (match({TokenType::ID}))
    return std::make_unique<VariableExpr>(previous().getValue());
  if (match({TokenType::LPAREN})) {
    auto expr = expression();
    consume(TokenType::RPAREN, "Expect ')' after expression.");
    return expr;
  }
  Token t = peek();
  throw std::runtime_error(
      "[Parser Error] Line " + std::to_string(t.getLine()) + ", Col " +
      std::to_string(t.getColumn()) + ": Expect expression.");
}

bool Parser::match(std::initializer_list<TokenType> types) {
  for (TokenType type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) {
  if (isAtEnd())
    return false;
  return peek().getType() == type;
}

Token Parser::advance() {
  if (!isAtEnd())
    position++;
  return previous();
}

bool Parser::isAtEnd() { return peek().getType() == TokenType::EOfF; }
Token Parser::peek() { return tokens[position]; }
Token Parser::previous() { return tokens[position - 1]; }

Token Parser::consume(TokenType type, const std::string &message) {
  if (check(type))
    return advance();
  throw std::runtime_error(message);
}