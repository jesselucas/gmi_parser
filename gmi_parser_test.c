#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/queue.h>
#include <assert.h>
#include "gmi_parser.h"

void test_lines(char *);
void test_headers(char *);

/*
 * main used for testing
 */
int
main(void)
{
	char *gem_text;
	gem_text = strdup(
		"All the following examples are valid link lines:\n"
	       	"=> gemini://example.org/\n"
		"=> gemini://example.org/ An example link\n"
		"=> gemini://example.org/foo     Another example link at the same host\n"
		"=> foo/bar/baz.txt      A relative link\n"
		"=>      gopher://example.org:70/1 A gopher link\n"
		"```alt text\n"
		"some preformatted things\n"
		"=> preformatted link is not a link?\n"
		"# Or a header\n"
		"> Or paragraph\n"
		"```\n"
		"=> Links should be back\n"
		"# So should headers\n"
		"What about text?\n"
		"> Paragraphs working?\n"
		"> A verbose one huh?\n"
		"```\n"
		"Preformatting again\n"
		"# Isn't that enough\n"
		"``` yeah\n\0");
	if (gem_text == NULL)
		err(1, NULL);

	test_lines(gem_text);

	/* clean up */
	free(gem_text);
	return 0;
}

void
test_lines(char *gem_text) {

	/* Create new gmi and add all lines */
	struct gmi *g = gmi_new();

	int line_number = 0;
	char *gem_text_ref = gem_text;
	char *text_line;
	while((text_line = strsep(&gem_text_ref, "\n")) != NULL ) {
		gmi_parse_line(g, line_number++, text_line);
	}

	/* Print all lines in gmi struct */
	struct line *l = NULL;
	int line_total = 0;
	SLIST_FOREACH(l, g->lines, next) {
		printf("%d: type:%u text:%s\n", l->number, l->type, l->line);
		line_total++;
	}


	if((line_number - 1) != line_total) {
		errx(1, "line_number: %d != line_total: %d", line_number, line_total);
	}
	/*
	gem_test_ref = gem_test;
	while((text_line = strsep(&gem_test_ref, "\n")) != NULL ) {
	//	gmi_parse_line(g, line_number, text_line);
	//	line_number++;
	}
	*/

	gmi_free(g);
}

void
test_headers(char *gem_text)
{
	printf("text %s", gem_text);
}
