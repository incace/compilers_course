#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <iostream>

void runTest(const std::string &name, const std::string &source) {
  std::cout << "=== Case: " << name << " ===\n";
  try {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse();
    SemanticAnalyzer semantic;
    auto [errors, warnings] = semantic.analyze(ast);
    if (!errors.empty()) {
      for (auto &e : errors)
        std::cout << "  " << e << "\n";
      return;
    }
    Interpreter interpreter;
    auto output = interpreter.interpret(ast);
    for (auto &line : output)
      std::cout << "  " << line << "\n";
  } catch (const std::exception &e) {
    std::cout << "  Runtime Error: " << e.what() << "\n";
  }
}

int main() {
  runTest("Recursion: Factorial", "fun fact(n) { if (n <= 1) return 1; return "
                                  "n * fact(n - 1); } print fact(5);");

  runTest(
      "Function local scope",
      "var x = 100; fun add(x, d) { return x + d; } print add(5, 7); print x;");

  runTest("Return outside function", "return 1;");

  runTest("Unknown function", "print missing(1);");

  return 0;
}