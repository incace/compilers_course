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
  if (match({TokenType::FUN})) {
    std::string name =
        consume(TokenType::ID, "Expect function name.").getValue();
    consume(TokenType::LPAREN, "Expect '(' after function name.");
    std::vector<std::string> params;
    if (!check(TokenType::RPAREN)) {
      do {
        params.push_back(
            consume(TokenType::ID, "Expect parameter name.").getValue());
      } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    consume(TokenType::LBRACE, "Expect '{' before function body.");
    return std::make_unique<FunctionStmt>(name, params,
                                          std::make_shared<BlockStmt>(block()));
  }
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
  if (match({TokenType::RETURN})) {
    std::unique_ptr<Expr> val = nullptr;
    if (!check(TokenType::SEMICOLON))
      val = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after return.");
    return std::make_unique<ReturnStmt>(std::move(val));
  }
  if (match({TokenType::LBRACE}))
    return std::make_unique<BlockStmt>(block());
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
  consume(TokenType::LPAREN, "Expect '(' after 'if'.");
  auto condition = expression();
  consume(TokenType::RPAREN, "Expect ')' after if condition.");
  auto thenBranch = statement();
  std::unique_ptr<Stmt> elseBranch = nullptr;
  if (match({TokenType::ELSE}))
    elseBranch = statement();
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
  consume(TokenType::LPAREN, "Expect '(' after 'while'.");
  auto condition = expression();
  consume(TokenType::RPAREN, "Expect ')' after while condition.");
  auto body = statement();
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::printStatement() {
  auto value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  auto expr = expression();
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
  auto expr = logicalOr();
  if (match({TokenType::EQ})) {
    auto value = assignment();
    if (auto *v = dynamic_cast<VariableExpr *>(expr.get())) {
      return std::make_unique<AssignExpr>(v->name, std::move(value));
    }
    throw std::runtime_error("Invalid assignment target.");
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
  auto expr = logicalAnd();
  while (match({TokenType::OR})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, logicalAnd());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
  auto expr = equality();
  while (match({TokenType::AND})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, equality());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  auto expr = comparison();
  while (match({TokenType::EQEQ, TokenType::NEQ})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, comparison());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  auto expr = term();
  while (
      match({TokenType::LT, TokenType::LTEQ, TokenType::GT, TokenType::GTEQ})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, term());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = factor();
  while (match({TokenType::PLUS, TokenType::MINUS})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, factor());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = unary();
  while (match({TokenType::STAR, TokenType::SLASH})) {
    TokenType op = previous().getType();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, unary());
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::EXCL, TokenType::MINUS})) {
    TokenType op = previous().getType();
    return std::make_unique<UnaryExpr>(op, unary());
  }
  return parseCall();
}

std::unique_ptr<Expr> Parser::parseCall() {
  auto expr = primary();
  while (match({TokenType::LPAREN})) {
    std::vector<std::unique_ptr<Expr>> args;
    if (!check(TokenType::RPAREN)) {
      do {
        args.push_back(expression());
      } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expect ')' after arguments.");
    if (auto *v = dynamic_cast<VariableExpr *>(expr.get())) {
      expr = std::make_unique<CallExpr>(v->name, std::move(args));
    } else
      throw std::runtime_error("Can only call functions.");
  }
  return expr;
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
  return !isAtEnd() && peek().getType() == type;
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
  Token t = peek();
  throw std::runtime_error("[Parser Error] Line " +
                           std::to_string(t.getLine()) + ", Col " +
                           std::to_string(t.getColumn()) + ": " + message);
}