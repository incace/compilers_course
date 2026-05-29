#pragma once

enum class TokenType {
  // Keywords
  ID,
  NUMBER,
  VAR,
  PRINT,
  IF,
  ELSE,
  WHILE,
  // Operators
  PLUS,
  MINUS,
  STAR,
  SLASH,
  EQ,
  EQEQ,
  EXCL,
  NEQ,
  LT,
  GT,
  LTEQ,
  GTEQ,
  AND,
  OR,
  // Delimiters
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  SEMICOLON,
  // Special
  EOfF
};