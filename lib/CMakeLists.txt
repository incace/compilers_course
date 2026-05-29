add_library(lexer_lib 
    src/lexer.cpp 
    src/token.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
    src/interpreter.cpp
)
target_include_directories(lexer_lib PUBLIC include)