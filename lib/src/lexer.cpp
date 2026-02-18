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

    if (isalpha(current)) {
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
  while (isdigit(peek())) {
    next();
  }

  string value = input.substr(start, position - start);
  result.emplace_back(TokenType::NUMBER, value, start);
}


void Lexer::tokenizeWord(vector<Token> &result) {
  size_t start = position;
  while (isalnum(peek())) {
    next();
  }

  string word = input.substr(start, position - start);

  auto it = keywords.find(word);
  if (it != keywords.end()) {
    result.emplace_back(it->second, word, start);
  } else {
    result.emplace_back(TokenType::ID, word, start);
  }
}

void Lexer::tokenizeOperator(vector<Token> &result) {
  char current = peek();
  size_t start = position;

  switch (current) {
  case '+':
    next();
    result.emplace_back(TokenType::PLUS, "+", start);
    break;
  case '-':
    next();
    result.emplace_back(TokenType::MINUS, "-", start);
    break;
  case '*':
    next();
    result.emplace_back(TokenType::STAR, "*", start);
    break;
  case '/':
    next();
    result.emplace_back(TokenType::SLASH, "/", start);
    break;
  case '=':
    next();
    result.emplace_back(TokenType::EQ, "=", start);
    break;
  case ';':
    next();
    result.emplace_back(TokenType::SEMICOLON, ";", start);
    break;
  default:
    throw runtime_error("Unexpected character: " + string(1, current));
  }
}

char Lexer::peek() const {
  if (position >= length)
    return '\0';
  return input[position];
}

char Lexer::next() {
  if (position >= length)
    return '\0';
  return input[position++];
}