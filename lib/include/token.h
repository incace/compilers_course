#pragma once
#include "./token_type.h"
#include <string>

class Token {
public:
  Token(TokenType type, std::string value, int position);
  void print() const;

private:
  TokenType type;
  std::string value;
  int position;
};