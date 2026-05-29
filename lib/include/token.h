#pragma once
#include "token_type.h"
#include <string>

class Token {
public:
  Token(TokenType type, std::string value, int position, int line = 1,
        int column = 1);
  void print() const;
  TokenType getType() const;
  std::string getValue() const;
  int getLine() const;
  int getColumn() const;

private:
  TokenType type;
  std::string value;
  int position;
  int line;
  int column;
};