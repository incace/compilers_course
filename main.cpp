#include "lexer.h"
#include "token.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::string codeExample = "var x = 123; print x + 5;";

  try {
    Lexer lexer(codeExample);
    std::vector<Token> tokens = lexer.tokenize();
    std::cout << "Tokens:" << std::endl;
    for (const auto &token : tokens) {
      token.print();
    }
  } catch (const std::exception &e) {
    std::cerr << "Lexer Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}