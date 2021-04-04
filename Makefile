CC=clang
CFLAGS += -fpie -W -Wextra -g3 -O0 -Wall -Werror -pedantic -Wparentheses -Wimplicit-fallthrough

test:
	$(CC) $(CFLAGS) gmi_parser_test.c -o test_gmi_parser -include gmi_parser.c

all: test

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./test_*

tidy:
	clang-tidy *.c
