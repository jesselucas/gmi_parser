#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/queue.h>

#include "gmi_parser.h"

/*
 * main currently used for testing
 */
int
main(void)
{
	printf("nothing to see... yet\n");
	
	char *gem_test;
	gem_test = strdup(
		"All the following examples are valid link lines:\n"
	       	"=> gemini://example.org/\n"
		"=> gemini://example.org/ An example link\n"
		"=> gemini://example.org/foo     Another example link at the same host"
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
	if (gem_test == NULL)
		err(1, NULL);

	/* Create new gmi and add all lines */
	struct gmi *g = gmi_new();
	int line_number = 0;

	char *gem_test_ref = gem_test;
	char *text_line = strsep(&gem_test_ref, "\n");
	printf("text line? %s\n", text_line);
	while(text_line != NULL ) {
		gmi_parse_line(g, line_number, text_line);
		text_line = strsep(&gem_test_ref, "\n");
		line_number++;
	}

	/* Print all lines in gmi struct */
	struct line *l = NULL;
	SLIST_FOREACH(l, g->lines, next)
		printf("%d: type:%u text:%s\n", l->number, l->type, l->line);

	gmi_free(g);
	free(gem_test);

	return 0;
}
