#include "token.h"
#include <iostream>

Token::Token(TokenType type, std::string value, int position, int line,
             int column)
    : type(type), value(value), position(position), line(line), column(column) {
}

void Token::print() const {
  std::cout << "Token(Value: '" << value << "', Line: " << line
            << ", Col: " << column << ")" << std::endl;
}

TokenType Token::getType() const { return type; }
std::string Token::getValue() const { return value; }
int Token::getLine() const { return line; }
int Token::getColumn() const { return column; }