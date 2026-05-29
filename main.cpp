#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <iostream>

void runInterpreterTest(const std::string &name, const std::string &source) {
  std::cout << "=== Test: " << name << " ===\n";
  try {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse();

    SemanticAnalyzer semantic;
    auto [errors, warnings] = semantic.analyze(ast);
    if (!errors.empty()) {
      for (auto &e : errors)
        std::cerr << e << "\n";
      return;
    }

    Interpreter interpreter;
    auto output = interpreter.interpret(ast);

    std::cout << "Output:\n";
    for (const auto &line : output)
      std::cout << "  " << line << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Runtime Error: " << e.what() << "\n";
  }
  std::cout << "\n";
}

int main() {
  runInterpreterTest(
      "Math and Variables",
      "var a = 10; var b = 4; var res = (a + b) * 2 - a / 2; print res;");

  runInterpreterTest("Shadowing",
                     "var x = 10; { var x = 3; print x + 1; } print x;");

  runInterpreterTest("While loop",
                     "var total = 0; var i = 1; while (i <= 5) { total = total "
                     "+ i; i = i + 1; } print total;");

  runInterpreterTest(
      "Conditions & Strings",
      "var t = 15; if (t == 15) print \"ok\"; else print \"bad\";");

  return 0;
}