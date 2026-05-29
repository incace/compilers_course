#include "lexer.h"
#include <cctype>
#include <stdexcept>

using namespace std;

const unordered_map<string, TokenType> Lexer::keywords = {
    {"var", TokenType::VAR},       {"print", TokenType::PRINT},
    {"if", TokenType::IF},         {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},   {"fun", TokenType::FUN},
    {"return", TokenType::RETURN}, {"true", TokenType::TRUE},
    {"false", TokenType::FALSE}};

Lexer::Lexer(string input)
    : input(move(input)), length(this->input.length()), position(0) {}

char Lexer::peek() const {
  return (position >= length) ? '\0' : input[position];
}
char Lexer::peekNext() const {
  return (position + 1 >= length) ? '\0' : input[position + 1];
}

char Lexer::next() {
  if (position >= length)
    return '\0';
  return input[position++];
}

pair<int, int> Lexer::getCoords(size_t pos) {
  int line = 1, col = 1;
  for (size_t i = 0; i < pos; ++i) {
    if (input[i] == '\n') {
      line++;
      col = 1;
    } else
      col++;
  }
  return {line, col};
}

vector<Token> Lexer::tokenize() {
  vector<Token> result;
  while (position < length) {
    char current = peek();
    if (isspace(current)) {
      next();
      continue;
    }
    if (isdigit(current)) {
      tokenizeNumber(result);
      continue;
    }
    if (current == '"') {
      tokenizeString(result);
      continue;
    }
    if (isalpha(current) || current == '_') {
      tokenizeWord(result);
      continue;
    }
    tokenizeOperator(result);
  }
  auto [l, c] = getCoords(position);
  result.emplace_back(TokenType::EOfF, "", position, l, c);
  return result;
}

void Lexer::tokenizeNumber(vector<Token> &result) {
  size_t start = position;
  auto [l, c] = getCoords(start);
  while (isdigit(peek()))
    next();
  if (peek() == '.' && isdigit(peekNext())) {
    throw runtime_error("[Lexer Error] Line " + to_string(l) + ", Col " +
                        to_string(c) + ": Float literals are not supported.");
  }
  result.emplace_back(TokenType::NUMBER, input.substr(start, position - start),
                      start, l, c);
}

void Lexer::tokenizeString(vector<Token> &result) {
  size_t start = position;
  auto [l, c] = getCoords(start);
  next();
  string val = "";
  while (peek() != '"' && peek() != '\0') {
    if (peek() == '\n')
      throw runtime_error("[Lexer Error] Line " + to_string(l) +
                          ": Unterminated string literal.");
    val += next();
  }
  if (peek() != '"')
    throw runtime_error("[Lexer Error] Line " + to_string(l) +
                        ": Unterminated string literal.");
  next();
  result.emplace_back(TokenType::STRING, val, start, l, c);
}

void Lexer::tokenizeWord(vector<Token> &result) {
  size_t start = position;
  auto [l, c] = getCoords(start);
  while (isalnum(peek()) || peek() == '_')
    next();
  string word = input.substr(start, position - start);
  auto it = keywords.find(word);
  if (it != keywords.end())
    result.emplace_back(it->second, word, start, l, c);
  else
    result.emplace_back(TokenType::ID, word, start, l, c);
}

void Lexer::tokenizeOperator(vector<Token> &result) {
  size_t start = position;
  auto [l, c] = getCoords(start);
  char cur = peek();
  char n = peekNext();

  auto add = [&](TokenType t, string v, int len) {
    result.emplace_back(t, v, start, l, c);
    for (int i = 0; i < len; ++i)
      next();
  };

  if (cur == '=' && n == '=')
    add(TokenType::EQEQ, "==", 2);
  else if (cur == '=')
    add(TokenType::EQ, "=", 1);
  else if (cur == '!' && n == '=')
    add(TokenType::NEQ, "!=", 2);
  else if (cur == '!')
    add(TokenType::EXCL, "!", 1);
  else if (cur == '<' && n == '=')
    add(TokenType::LTEQ, "<=", 2);
  else if (cur == '<')
    add(TokenType::LT, "<", 1);
  else if (cur == '>' && n == '=')
    add(TokenType::GTEQ, ">=", 2);
  else if (cur == '>')
    add(TokenType::GT, ">", 1);
  else if (cur == '&' && n == '&')
    add(TokenType::AND, "&&", 2);
  else if (cur == '|' && n == '|')
    add(TokenType::OR, "||", 2);
  else if (cur == '+')
    add(TokenType::PLUS, "+", 1);
  else if (cur == '-')
    add(TokenType::MINUS, "-", 1);
  else if (cur == '*')
    add(TokenType::STAR, "*", 1);
  else if (cur == '/')
    add(TokenType::SLASH, "/", 1);
  else if (cur == '(')
    add(TokenType::LPAREN, "(", 1);
  else if (cur == ')')
    add(TokenType::RPAREN, ")", 1);
  else if (cur == '{')
    add(TokenType::LBRACE, "{", 1);
  else if (cur == '}')
    add(TokenType::RBRACE, "}", 1);
  else if (cur == ';')
    add(TokenType::SEMICOLON, ";", 1);
  else if (cur == ',')
    add(TokenType::COMMA, ",", 1);
  else
    throw runtime_error("[Lexer Error] Line " + to_string(l) + ", Col " +
                        to_string(c) +
                        ": Unexpected character: " + string(1, cur));
}