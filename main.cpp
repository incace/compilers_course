#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <iostream>
#include <vector>

void runTest(const std::string &name, const std::string &source) {
  std::cout << "TEST: " << name << "\nSource: " << source << "\n";
  try {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse();

    SemanticAnalyzer analyzer;
    auto [errors, warnings] = analyzer.analyze(ast);

    std::cout << "Errors:\n";
    if (errors.empty())
      std::cout << "  none\n";
    for (const auto &err : errors)
      std::cout << "  " << err << "\n";

    std::cout << "Warnings:\n";
    if (warnings.empty())
      std::cout << "  none\n";
    for (const auto &warn : warnings)
      std::cout << "  " << warn << "\n";

  } catch (const std::exception &e) {
    std::cout << "Runtime Error: " << e.what() << "\n";
  }
  std::cout << "-----------------------------------\n";
}

int main() {
  runTest("Duplicate declaration", "var x = 1; var x = 2;");
  runTest("Undeclared variable", "x = 5;");
  runTest("Uninitialized variable", "var x; print x;");
  runTest("Unused variable", "var x = 10;");
  runTest("Scoping test", "{ var y = 5; } print y;");
  return 0;
}