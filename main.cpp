#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <iostream>
#include <string>
#include <vector>

// Вспомогательная функция для запуска тестов, аналогичная той, что в
// Python-коммите
void runTest(const std::string &name, const std::string &source) {
  std::cout << "==============================================================="
               "=================\n";
  std::cout << "Case: " << name << "\n";
  std::cout << "Source:\n" << source << (source.back() == '\n' ? "" : "\n");

  try {
    // 1. Лексический анализ
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // 2. Синтаксический анализ
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> ast = parser.parse();

    // 3. Семантический анализ
    SemanticAnalyzer analyzer;
    auto [errors, warnings] = analyzer.analyze(ast);

    std::cout << "Tokens: " << tokens.size() << "\n";
    std::cout << "AST: " << ast.size() << "\n";

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
    // Ловим ошибки лексера (например, на Float) или парсера
    std::cout << "Runtime error:\n  " << e.what() << "\n";
  }
}

int main() {
  // Тесты из коммита lab4demo
  runTest("number inference", "var x = 1;\nprint x;\n");

  runTest("string concatenation", "var s = \"a\" + \"b\";\nprint s;\n");

  runTest("boolean condition", "var flag = true;\nif (flag) print 1;\n");

  runTest("assignment type mismatch", "var x = 1;\nx = \"test\";\n");

  runTest("string plus number is forbidden", "print \"a\" + 1;\n");

  runTest("numeric condition is forbidden", "if (1) print 1;\n");

  runTest("boolean operators", "var flag = true && false;\nprint flag;\n");

  runTest("float literals are rejected", "var x = 3.1;");

  return 0;
}