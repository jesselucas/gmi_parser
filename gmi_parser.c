#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include <sys/queue.h>

/*
 * Simple text/gemini parser
 * https://gitlab.com/gemini-specification/gemini-text/-/blob/master/specification.gmi
 * Based on V0.15.0, March 5th, 2021
 *
 * Four core line types (in any order)
 * Text lines
 * Link lines =>
 * Preformatting toggle lines ```
 * Preformatted text lines 
 *
 * Three advanced line types
 * Heading lines #, ##, ###
 * Unordered list items *
 * Quote lines >
 *
 * TODO
 * Convert to library
 * Write proper tests
 * Write example 
 */
enum linetype {
	TEXT,
	LINK,
	LINK_IMG,
	PRE_TOGGLE,
	PRE_TEXT,
	HEADING_1,
	HEADING_2,
	HEADING_3,
	LIST,
	QUOTE
};

struct line 		*line_new();
struct line_list 	*push_line(struct line_list *, struct line *);
void			 line_free(struct line *);
struct gmi		*gmi_new();
void		 	 gmi_parse_line(struct gmi *, int, char *);
void			 gmi_free(struct gmi *);

struct line		*parse_line(enum linetype, int, char *);

struct line {
	SLIST_ENTRY(line) 	 next;
	int 			 number;
	enum linetype 		 type;
	char 			*line;
};
SLIST_HEAD(line_list, line);

/*
 * Create a new line struct to represent
 * each of the line types. Typically stored on
 * a gmi struct
 */
struct line *
line_new()
{
	struct line *l;
	if ((l = calloc(1, sizeof(struct line))) == NULL)
		err(1, NULL);

	return l;
}

struct line_list *
push_line(struct line_list *ll, struct line *l)
{
	if (ll == NULL) {
		if ((ll = calloc(1, sizeof(*ll))) == NULL) {
			err(1, NULL);
		}
		SLIST_INIT(ll);
	}

	if (l == NULL)
		err(1, "l is NULL");

	/* Check to see if the line list has a head */
	if(ll->slh_first == NULL) {
		SLIST_INSERT_HEAD(ll, l, next);
	} else {
		/* Find the last line and insert after */
		struct line *l1 = NULL;
		struct line *tail = NULL;
		SLIST_FOREACH(l1, ll, next)	/* Forward traversal. */
			tail = l1;
		
		SLIST_INSERT_AFTER(tail, l, next);
	}
	return ll;
}

void
line_free(struct line *l)
{
	l->line = NULL;
	l->number = 0;
	free(l);
}

struct line *
parse_line(enum linetype type, int number, char *line) {
	struct line *l = line_new();
	l->type = type;
	l->number = number;
	l->line = line;

	return l;
}

struct gmi {
	struct line_list 	*lines;
	_Bool 			 preformat_mode;
};

/*
 * Create a new gmi struct to represent the entirity
 * of a single .gmi document
 */
struct gmi *
gmi_new()
{
	struct gmi *g;
	if ((g = calloc(1, sizeof(struct gmi))) == NULL)
		err(1, NULL);

	return g;
}

/* Parse line to gmi struct */
void
gmi_parse_line(struct gmi *g, int line_number, char *line)
{
	if(g == NULL)
		err(1, NULL);

	if (line == NULL)
		err(1, NULL);

	size_t len = strlen(line);
	if (len <= 0)
		return;

	struct line *l = NULL;
	if (g->preformat_mode == true) {
		if (len >= 3) {
			/* If preformat mode is on toggle it off when ``` */
			if(line[0] == '`' && line[1] == '`' && line[2] == '`') {
				/* TODO strip any text following ``` */
				g->preformat_mode = false;
				l = parse_line(PRE_TOGGLE, line_number, line);
				goto done;
			}
		}

		l = parse_line(PRE_TEXT, line_number, line);
		goto done;
	}

	switch (line[0]) {
		case '=':
			/* TODO check for image extension
			 * and use LINK_IMG */
			l = parse_line(LINK, line_number, line);
			break;
		case '`':
			if (len >= 3 && line[1] == '`' && line[2] == '`') {
				g->preformat_mode = true;
				return;
			}
			break;
		case '#':
			l = parse_line(HEADING_1, line_number, line);
			break;
		case '*':
			l = parse_line(LIST, line_number, line);
			break;
		case '>':
			l = parse_line(QUOTE, line_number, line);
			break;
		default:
			l = parse_line(TEXT, line_number, line);
			break;
	}

done:
	g->lines = push_line(g->lines, l);
}

/* Free any allocated memory associated with a gmi struct */
void
gmi_free(struct gmi *g)
{
	struct line *l = NULL;
	while (!SLIST_EMPTY(g->lines)) {
		l = SLIST_FIRST(g->lines);
		SLIST_REMOVE_HEAD(g->lines, next);
		line_free(l);
	}
	/*
	SLIST_FOREACH(l, g->lines, next)
		SLIST_REMOVE_HEAD(g->lines, next);
		line_free(l);
		*/

	free(g->lines);
	g->lines = NULL;
	free(g);
}

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
