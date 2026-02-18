#pragma once

enum class TokenType {
  NUMBER,
  ID,
  STRING,
  VAR,

  PRINT,
  IF,
  ELSE,
  WHILE, // while

  // Operators
  PLUS,
  MINUS,
  STAR,
  SLASH, // + - * /
  EQ,
  EQEQ,
  EXCL,
  NEQ, // = == ! !=
  LT,
  GT,
  LTEQ,
  GTEQ, // < > <= >=
  AND,
  OR, // && ||

  // Grouping & Punctuation
  LPAREN,
  RPAREN, // ( )
  LBRACE,
  RBRACE,    // { }
  SEMICOLON, // ;

  EOfF // Конец файла
};