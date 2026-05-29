#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <string>

int main() {
  // Сложный пример кода, покрывающий новые возможности
  std::string code = "var x = 10; "
                     "if (x > 5) { "
                     "  print x + 1; "
                     "} else { "
                     "  x = 0; "
                     "} "
                     "while (x < 15) { "
                     "  x = x + 1; "
                     "  print x; "
                     "}";

  try {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "AST Structure:\n";
    for (const auto &stmt : ast) {
      stmt->print(0);
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}