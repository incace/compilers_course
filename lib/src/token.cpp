#include "token.h"
#include <iostream>

Token::Token(TokenType type, std::string value, int position)
    : type(type), value(value), position(position) {}

void Token::print() const {
  std::cout << "Token(Value: '" << value << "', Pos: " << position << ")"
            << std::endl;
}

TokenType Token::getType() const { return type; }
std::string Token::getValue() const { return value; }