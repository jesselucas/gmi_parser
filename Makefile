CC=clang

all:
	$(CC) -fpie -W -Wextra -g3 -O0 -Wall -Werror -pedantic gmi_parser.c -o test_gmi_parser

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./test_gmi_parser 

tidy:
	clang-tidy gmi_parser.c
