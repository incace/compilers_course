#include "ast_optimizer.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>

void runOptimizationDemo() {
  std::string source = "var a = 1; "
                       "var b = 2; "
                       "var n = a + b; "            // Должно стать 3
                       "var x = (10 + 5) * 2 / 3; " // Должно стать 10
                       "print \"Result n:\"; "
                       "print n; "
                       "print \"Result x:\"; "
                       "print x;";

  std::cout << "--- Original Source ---\n" << source << "\n\n";

  Lexer lexer(source);
  Parser parser(lexer.tokenize());
  auto ast = parser.parse();

  std::cout << "--- AST Before Optimization ---\n";
  for (const auto &stmt : ast)
    stmt->print(0);

  ASTOptimizer optimizer;
  ast = optimizer.optimize(std::move(ast));

  std::cout << "\n--- AST After Optimization ---\n";
  for (const auto &stmt : ast)
    stmt->print(0);

  std::cout << "\n--- Execution Result ---\n";
  Interpreter interpreter;
  auto output = interpreter.interpret(ast);
  for (const auto &line : output)
    std::cout << line << "\n";
}

int main() {
  runOptimizationDemo();
  return 0;
}