#include "lexer.h"
#include <cctype>
#include <stdexcept>

using namespace std;

const unordered_map<string, TokenType> Lexer::keywords = {
    {"var", TokenType::VAR},
    {"print", TokenType::PRINT},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE}};

Lexer::Lexer(string input)
    : input(move(input)), length(this->input.length()), position(0) {}

char Lexer::peek() const {
  if (position >= length)
    return '\0';
  return input[position];
}

char Lexer::peekNext() const {
  if (position + 1 >= length)
    return '\0';
  return input[position + 1];
}

char Lexer::next() {
  if (position >= length)
    return '\0';
  return input[position++];
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
    if (isalpha(current) || current == '_') {
      tokenizeWord(result);
      continue;
    }
    tokenizeOperator(result);
  }
  result.emplace_back(TokenType::EOfF, "", position);
  return result;
}

void Lexer::tokenizeNumber(vector<Token> &result) {
  size_t start = position;
  while (isdigit(peek()))
    next();
  result.emplace_back(TokenType::NUMBER, input.substr(start, position - start),
                      start);
}

void Lexer::tokenizeWord(vector<Token> &result) {
  size_t start = position;
  while (isalnum(peek()) || peek() == '_')
    next();
  string word = input.substr(start, position - start);
  auto it = keywords.find(word);
  if (it != keywords.end())
    result.emplace_back(it->second, word, start);
  else
    result.emplace_back(TokenType::ID, word, start);
}

void Lexer::tokenizeOperator(vector<Token> &result) {
  char current = peek();
  char n = peekNext();
  size_t start = position;

  if (current == '=' && n == '=') {
    next();
    next();
    result.emplace_back(TokenType::EQEQ, "==", start);
  } else if (current == '=') {
    next();
    result.emplace_back(TokenType::EQ, "=", start);
  } else if (current == '!' && n == '=') {
    next();
    next();
    result.emplace_back(TokenType::NEQ, "!=", start);
  } else if (current == '!') {
    next();
    result.emplace_back(TokenType::EXCL, "!", start);
  } else if (current == '<' && n == '=') {
    next();
    next();
    result.emplace_back(TokenType::LTEQ, "<=", start);
  } else if (current == '<') {
    next();
    result.emplace_back(TokenType::LT, "<", start);
  } else if (current == '>' && n == '=') {
    next();
    next();
    result.emplace_back(TokenType::GTEQ, ">=", start);
  } else if (current == '>') {
    next();
    result.emplace_back(TokenType::GT, ">", start);
  } else if (current == '&' && n == '&') {
    next();
    next();
    result.emplace_back(TokenType::AND, "&&", start);
  } else if (current == '|' && n == '|') {
    next();
    next();
    result.emplace_back(TokenType::OR, "||", start);
  } else if (current == '+') {
    next();
    result.emplace_back(TokenType::PLUS, "+", start);
  } else if (current == '-') {
    next();
    result.emplace_back(TokenType::MINUS, "-", start);
  } else if (current == '*') {
    next();
    result.emplace_back(TokenType::STAR, "*", start);
  } else if (current == '/') {
    next();
    result.emplace_back(TokenType::SLASH, "/", start);
  } else if (current == '(') {
    next();
    result.emplace_back(TokenType::LPAREN, "(", start);
  } else if (current == ')') {
    next();
    result.emplace_back(TokenType::RPAREN, ")", start);
  } else if (current == '{') {
    next();
    result.emplace_back(TokenType::LBRACE, "{", start);
  } else if (current == '}') {
    next();
    result.emplace_back(TokenType::RBRACE, "}", start);
  } else if (current == ';') {
    next();
    result.emplace_back(TokenType::SEMICOLON, ";", start);
  } else
    throw runtime_error("Unexpected character: " + string(1, current));
}