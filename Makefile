CC=clang
CC_FLAGS=-fpie -W -Wextra -g3 -O0 -Wall -Werror -pedantic

test:
	$(CC) $(CC_FLAGS) gmi_parser_test.c -o test_gmi_parser -include gmi_parser.c

all: test

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./test_*

tidy:
	clang-tidy *.c
