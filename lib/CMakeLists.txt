add_library(lexer_lib 
    src/lexer.cpp 
    src/token.cpp
    src/parser.cpp
)
target_include_directories(lexer_lib PUBLIC include)