#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/queue.h>
#include <assert.h>
#include <wchar.h>
#include <locale.h>

#include "gmi_parser.h"

char * gmi_sample(void);
struct gmi * gmi_from_str(char *);

void test_lines(void);
void test_headers(void);
void test_links(void);

/*
 * main used for testing
 */
int
main(void)
{
	setlocale(LC_CTYPE, "");
	printf("Running gmi_parser_test...\n");

	/* Run tests */
	test_lines();
	test_headers();
	test_links();

	/* Finished */
	wchar_t star = 0x2605;
	wprintf(L"%lcAll tests passed%lc\n", star, star);
	return 0;
}

/* Return allocated string of a sample gmi file
 * must be free'd
 */
char *
gmi_sample(void)
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

	return gem_text;
}

struct gmi *
gmi_from_str(char *str)
{
	/* Create new gmi and add all lines */
	struct gmi *g = gmi_new();
	int line_number = 0;
	char *str_ref = str;
	char *text_line;
	while((text_line = strsep(&str_ref, "\n")) != NULL ) {
		gmi_parse_line(g, line_number++, text_line);
	}
	g->linelen = line_number;

	return g;
}

void
test_lines(void)
{
	char *gem_text = gmi_sample();
	struct gmi *g = gmi_from_str(gem_text);

	/* Print all lines in gmi struct */
	struct line *l = NULL;
	int ltotal = 0;
	SLIST_FOREACH(l, g->lines, next) {
	//	printf("%d: type:%u text:%s\n", l->number, l->type, l->line);
		ltotal++;
	}

	if((g->linelen - 1) != ltotal) {
		errx(1, "line_number: %d != line_total: %d", g->linelen, ltotal);
	}
	gmi_free(g);
	free(gem_text);
}

void
test_headers(void)
{
	char *headers;
	headers = strdup(
		"# Header 1\n"
		"## Header 1\n"
		"### Header 1\n"
		"###                    Header space    \n"
		"#1## Header 1\n"
		"##1# Header 2\n\0"
	);
	enum linetype header_types[6] = {
		HEADING_1,
		HEADING_2,
		HEADING_3,
		HEADING_3,
		HEADING_1,
		HEADING_2,
	};

	struct line *l = NULL;
	struct gmi *g = gmi_from_str(headers);
	int ltotal = 0;
	SLIST_FOREACH(l, g->lines, next) {
		enum linetype type = header_types[ltotal];
		if(l->type != type){
			errx(1, "\"%s\" type: %d should equal %d", l->line, l->type, type);
		}
		ltotal++;
	}

	free(headers);
}

void
test_links(void)
{
	char *links;
	links = strdup(
		"=>link\n"
		"=not a link\n"
		"=> Link 2\n"
		"=>                    Link space    \n"
		"=> image.png\n"
		"=> image.jpeg\n\0"
	);
	enum linetype link_types[6] = {
		LINK,
		TEXT,
		LINK,
		LINK,
		LINK_IMG,
		LINK_IMG,
	};

	struct line *l = NULL;
	struct gmi *g = gmi_from_str(links);
	int ltotal = 0;
	SLIST_FOREACH(l, g->lines, next) {
		enum linetype type = link_types[ltotal];
		if(l->type != type){
			errx(1, "\"%s\" type: %d should equal %d", l->line, l->type, type);
		}
		ltotal++;
	}

	free(links);
}
