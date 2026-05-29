#pragma once
#include "token.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Lexer {
public:
  explicit Lexer(std::string input);
  std::vector<Token> tokenize();

private:
  std::string input;
  size_t length;
  size_t position;
  char peek() const;
  char peekNext() const;
  char next();
  void tokenizeNumber(std::vector<Token> &result);
  void tokenizeString(std::vector<Token> &result);
  void tokenizeWord(std::vector<Token> &result);
  void tokenizeOperator(std::vector<Token> &result);
  std::pair<int, int> getCoords(size_t pos);
  static const std::unordered_map<std::string, TokenType> keywords;
};