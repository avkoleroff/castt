#!/bin/bash

g++   -std=c++17   -Wall   -Wextra   parser/parser.cpp   main.cpp   -Iftl/projects/castt/lexer   -Iftl/projects/castt/parser   -o compiler
./compiler
